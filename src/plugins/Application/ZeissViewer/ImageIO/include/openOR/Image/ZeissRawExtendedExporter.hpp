//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//! OPENOR_INTERFACE_FILE(Image_ImageIO)
//! \file
//! \ingroup Image_ImageIO
//****************************************************************************

#ifndef openOR_ImageIO_ZeissRawExtendedExporter_hpp
#define openOR_ImageIO_ZeissRawExtendedExporter_hpp

#include <openOR/Defs/Zeiss_ImageIO.hpp>

#include <openOR/Image/RegionOfInterest.hpp> //Image_Regions
#include <openOR/Image/DataCollector.hpp> //Image_Utility
#include <openOR/Image/FileExporter.hpp> //Image_ImageIO
#include <openOR/Image/Filename.hpp> //Image_ImageIO
#include <openOR/Image/Image1DData.hpp> //Image_ImageData
#include <openOR/Image/Image2DData.hpp> //Image_ImageData
#include <openOR/Image/Image3DData.hpp> //Image_ImageData
#include <openOR/Image/DataWrapper.hpp> //Image_Utility
#include <openOR/Math/vector.hpp> //openOr_core

#include <boost/format.hpp>
#include <boost/type_traits/is_floating_point.hpp>
#include <boost/filesystem.hpp>

#include <exception>
#include <stdexcept>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>

#include <openOR/Log/Logger.hpp> //openOR_core
#   define OPENOR_MODULE_NAME "Image.ZeissRawExtendedExporter"
#   include <openOR/Log/ModuleFilter.hpp>

#define LOG_THROW_RUNTIME(level, fmt) { boost::format __fmt = fmt; LOG(level, OPENOR_MODULE, __fmt); throw std::runtime_error(__fmt.str()); }
#define LOG_THROW_LOGIC(level, fmt) { boost::format __fmt = fmt; LOG(level, OPENOR_MODULE, __fmt); throw std::logic_error(__fmt.str()); }

#define ZRE_MAX_OPEN_FILES 64

namespace openOR {
	namespace Image {

		//! Extended exporter of Zeiss Volume. It exports the regions of interest as specified from the input volume (through DataCollector interface)
		//! Preconditions
		//! - Regions of interest all fit into the mask volume (front lower left >= (0,0,0), back upper right <= (mask width, mask height, mask depth)
		//! - Mask size is at least 16x16x16
		//! - The volume must be uploaded slice-by-slice (or its multiplies, but there should never be a not full slice loaded)
		template<typename T, T maskVal = T(), typename MaskT = uint32>
		class ZeissRawExtendedExporter : public FileExporter<T>, public DataCollector<T> {
		public:

			ZeissRawExtendedExporter() :
			  m_pTodo(new std::vector<AnyPtr>()),
				  m_pCurrentPos(new Math::Vector3ui()),
				  m_pMaskScaling(new Math::Vector3ui()),
				  m_vpDataFiles(new std::vector<std::tr1::shared_ptr<std::ofstream> >()),
				  m_vpFilenames(new std::vector<std::string>()),
				  m_vpOpen(new std::vector<bool>())
			  {
				  std::fill_n(m_pCurrentPos->begin(), 3, 0);
				  std::fill_n(m_pMaskScaling->begin(), 3, 0);
				  maskedArr[0] = maskVal;
				  this->setProgress(0.0, "Initializing");
			  }

			  virtual ~ZeissRawExtendedExporter() {
			  }

			  void reset(const bool& resetOut = false) {
				  closeDataFiles();
				  m_vpFilenames->resize(0);
				  m_vpDataFiles->resize(0);
				  m_regionsOfInterest.resize(0);
				  m_pTodo->resize(0);
				  m_vpOpen->resize(0);
				  m_pInSize = std::tr1::shared_ptr<Image3DSize>();
				  m_pMask = std::tr1::shared_ptr<Image3DData<MaskT> >();
				  std::fill_n(m_pCurrentPos->begin(), 3, 0);
				  std::fill_n(m_pMaskScaling->begin(), 3, 0);
			  }

			  //! tag      | type                   | meaning
			  //! -----------------------------------------------------------
			  //! inSize   | Image3DSize            | the size of the input bytestream *
			  //! mask     | Image3DData<uint32>    | the mask *
			  //!    **    | Image**Data<T>         | input raw data
			  //!
			  //! * - needs to be set before calling the ()()-operator
			  void setData(const AnyPtr& data, const std::string& name) {
				  //std::cout << "setData()" << std::endl;
				  if (std::tr1::shared_ptr<Image3DSize> pData = interface_cast<Image3DSize>(data)) {
					  if (name == "inSize") {
						  m_pInSize = pData;
						  calcMaskScaling();
						  return;
					  }
				  }
				  if (std::tr1::shared_ptr<Image3DData<MaskT> > pData = interface_cast<Image3DData<MaskT> >(data)) {
					  if (name == "mask") {
						  m_pMask = pData;
						  calcMaskScaling();
						  calcPartitioning();
						  return;
					  }
				  }
				  m_pTodo->push_back(data);
			  }

			  //! The setter for the regions of interest. If the mask does not equal the size of the input volume,
			  //! the roi coordinates are recalculated into the domain of the volume just as the mask is
			  //!
			  void setData(const std::vector<RegionOfInterest>& data, const std::string& name) {
				  m_regionsOfInterest = data;
				  calcPartitioning();
			  }

			  void setData(const T& fill, const std::string& name) {
				  maskedArr[0] = fill;
			  }

			  void operator()() const {
				  if (!m_pInSize || !m_pMask || m_regionsOfInterest.size() == 0) {
					  closeDataFiles();
					  LOG_THROW_LOGIC(Log::Level::Error, Log::msg("[ZeissRawExtendedExporter] Trying to commit data with no size or mask or regions of interest set"));
				  }

				  while (!m_pTodo->empty()) {
					  AnyPtr data = m_pTodo->back(); m_pTodo->pop_back();

					  const T* pFromData = NULL;
					  size_t end;

					  //depending on the kind of data set the corresponding end index
					  if (std::tr1::shared_ptr<Image1DData<T> > pData = interface_cast<Image1DData<T> >(data)) {
						  end = getWidth(pData);
						  pFromData = pData->data();
					  }

					  if (std::tr1::shared_ptr<Image2DData<T> > pData = interface_cast<Image2DData<T> >(data)) {
						  end = getWidth(pData) * getHeight(pData);
						  pFromData = pData->data();
					  }

					  if (std::tr1::shared_ptr<Image3DData<T> > pData = interface_cast<Image3DData<T> >(data)) {
						  end = static_cast<size_t>(width(pData)) * height(pData) * depth(pData);
						  pFromData = pData->data();
					  }

					  if (std::tr1::shared_ptr<DataWrapper<const T> > pData = interface_cast<DataWrapper<const T> >(data)) {
						  end = pData->size();
						  pFromData = pData->data();
					  }

					  if (!pFromData) {
						  continue;
					  }

					  uint32 volW = m_pInSize->size()(0), volH = m_pInSize->size()(1),  volD = m_pInSize->size()(2);
					  uint32 currX = (*m_pCurrentPos)(0), currY = (*m_pCurrentPos)(1), currZ = (*m_pCurrentPos)(2);

					  if (currX + currY + currZ == 0) {
						  m_vpDataFiles->resize(m_regionsOfInterest.size());
						  m_vpFilenames->resize(m_regionsOfInterest.size());
						  m_vpOpen->resize(m_regionsOfInterest.size());
						  for (int i = 0; i < m_regionsOfInterest.size(); i++) {
							  writeHeader(i);
						  }
						  this->setProgress(.01, "Initialized...");
					  }

					  uint32 *roiLX, *roiUX, *roiLY, *roiUY, *roiLZ, *roiUZ;
					  uint32 *roiInd;

					  roiLX = new uint32[m_regionsOfInterest.size()];
					  roiUX = new uint32[m_regionsOfInterest.size()];
					  roiLY = new uint32[m_regionsOfInterest.size()];
					  roiUY = new uint32[m_regionsOfInterest.size()];
					  roiLZ = new uint32[m_regionsOfInterest.size()];
					  roiUZ = new uint32[m_regionsOfInterest.size()];
					  roiInd = new uint32[m_regionsOfInterest.size()];

					  size_t maxx = 0;

					  for (int j = 0; j < m_regionsOfInterest.size(); j++) {
						  RegionOfInterest roi = m_regionsOfInterest[j];
						  Math::Vector3ui fll = roi.frontLowerLeft();
						  Math::Vector3ui bur = roi.backUpperRight();
						  roiLX[j] = fll(0);
						  roiLY[j] = fll(1);
						  roiLZ[j] = fll(2);
						  roiUX[j] = bur(0);
						  roiUY[j] = bur(1);
						  roiUZ[j] = bur(2);
						  roiInd[j] = roi.index();

						  if (bur(0) - fll(0) > maxx) maxx = (bur(0) - fll(0));
					  }

					  uint32 maskX = (*m_pMaskScaling)(0), maskY = (*m_pMaskScaling)(1), maskZ = (*m_pMaskScaling)(2);
					  uint32 maskW = m_pMask->size()(0), maskH = m_pMask->size()(1), maskD = m_pMask->size()(2);

					  T* buf = new T[maxx * maskX];

					  Math::Vector3ui mSize = m_pMask->size();
					  uint32 stepX = mSize(0) / 16, stepY = mSize(1) / 16, stepZ = mSize(2) / 16;

					  const MaskT* maskData = m_pMask->data();
					  size_t size = end;

					  for (size_t i = 0; i < size; i++) {
						  if (this->isCanceled()) {
							  closeDataFiles();
							  break;
						  }
						  // make sure we are inside the mask
						  uint32 currInMaskX = currX / maskX;
						  uint32 currInMaskY = currY / maskY;
						  uint32 currInMaskZ = currZ / maskZ;
						  size_t maskInd = static_cast<size_t>(currInMaskZ) * maskH * maskW + currInMaskY * maskW + currInMaskX;
						  if (currInMaskX < maskW && currInMaskY < maskH && currInMaskZ < maskD) {
							  uint32 partX = currInMaskX / stepX; if (partX >= 16) partX = 15;
							  uint32 partY = currInMaskY / stepY; if (partY >= 16) partY = 15;
							  uint32 partZ = currInMaskZ / stepZ; if (partZ >= 16) partZ = 15;
							  size_t partitionInd = partZ * 256 + partY * 16 + partX;
							  int rois = m_partitions[partitionInd].regionInds.size();
							  for (int j = 0; j < rois; j++) {
								  int regionInd = m_partitions[partitionInd].regionInds[j];
								  // make sure we are inside the roi (the rois are calculated with respect the reduced volume size)
								  if (currInMaskX == roiLX[regionInd] && (currX % maskX) == 0 && currInMaskY >= roiLY[regionInd] && currInMaskY < roiUY[regionInd] && currInMaskZ >= roiLZ[regionInd] && currInMaskZ < roiUZ[regionInd]) {
									  // make sure the mask says we write - if the mask is 0, write out the background
									  size_t dest = 0;
									  for (size_t k = 0; k < (roiUX[regionInd] - roiLX[regionInd]); k++) {
										  if (maskData[maskInd + k] == roiInd[regionInd] || maskData[maskInd + k] == 0) for (int l = 0; l < maskX; l++) buf[dest] = pFromData[i + dest++];
										  else for (int l = 0; l < maskX; l++) buf[dest++] = maskedArr[0];
									  }

									  getFileHandle(regionInd)->write(reinterpret_cast<const char*>(buf), sizeof(T) * maskX * (roiUX[regionInd] - roiLX[regionInd]));
								  }
							  }
						  }
						  // advance the position, make sure to wrap around
						  if (++currX >= volW) {
							  currX = 0;
							  if (++currY >= volH) {
								  currY = 0;
								  if (++currZ >= volD) {
									  currZ = 0;
									  closeDataFiles();
									  this->setProgress(1.0, "Finishing...");
									  if (++i != end) {
										  LOG(Log::Level::Warning, OPENOR_MODULE,
											  Log::msg("[ZeissRawExtendedExporter] Got too much data - igoring the rest")
											  );
									  }
								  }
							  }
						  }
					  }

					  delete[] buf;
					  delete[] roiLX;
					  delete[] roiUX;
					  delete[] roiLY;
					  delete[] roiUY;
					  delete[] roiLZ;
					  delete[] roiUZ;
					  delete[] roiInd;

					  size_t currPos = static_cast<size_t>(volW) * volH * currZ + volW * currY + currX;

					  if (currPos != 0) {
						  this->setProgress(.01 + static_cast<double>(currPos) * .98 / (volW * volH * volD), "Exporting...");
					  }

					  (*m_pCurrentPos)(0) = currX;
					  (*m_pCurrentPos)(1) = currY;
					  (*m_pCurrentPos)(2) = currZ;
				  }
			  }

		private:

			struct SubVolumeInfo {
				std::vector<int> regionInds;
			};

			void calcMaskScaling() {
				if (!m_pInSize || !m_pMask) {
					// not enough data yet
					return;
				}

				uint32 inW = m_pInSize->size()(0), inH = m_pInSize->size()(1), inD = m_pInSize->size()(2);
				uint32 maskW = m_pMask->size()(0), maskH = m_pMask->size()(1), maskD = m_pMask->size()(2);

				//if (inW % maskW != 0 || inH % maskH != 0 || inD % maskD != 0) {
				//   LOG_THROW_LOGIC(Log::Level::Error, Log::msg("Input size is not a multiple of the mask size!"));
				//}

				(*m_pMaskScaling)(0) = inW / maskW;
				(*m_pMaskScaling)(1) = inH / maskH;
				(*m_pMaskScaling)(2) = inD / maskD;
			}

			void calcPartitioning() {
				if (m_regionsOfInterest.empty() || !m_pMask) {
					return;
				}

				for (int i = 0; i < 4096; i++) {
					m_partitions[i].regionInds.resize(0);
				}

				Math::Vector3ui mSize = m_pMask->size();
				uint32 stepX = mSize(0) / 16, stepY = mSize(1) / 16, stepZ = mSize(2) / 16;

				for (int z = 0; z < 16; z++) {
					for (int y = 0; y < 16; y++) {
						for (int x = 0; x < 16; x++) {
							uint32 startX = stepX * x, startY = stepY * y, startZ = stepZ * z, endX = stepX * (x + 1), endY = stepY * (y + 1), endZ = stepZ * (z + 1);

							if (z == 15) endZ += 16;
							if (y == 15) endY += 16;
							if (x == 15) endX += 16;

							for (int i = 0; i < m_regionsOfInterest.size(); i++) {
								Math::Vector3ui fll = m_regionsOfInterest[i].frontLowerLeft();
								Math::Vector3ui bur = m_regionsOfInterest[i].backUpperRight();

								size_t roiInd = z * 256 + y * 16 + x;
								// if one of the corners is inside, the region overlapps
								if (fll(0) >= endX || fll(1) >= endY || fll(2) >= endZ || bur(0) <= startX || bur(1) <= startY || bur(2) <= startZ) continue;
								else m_partitions[roiInd].regionInds.push_back(i);
							}
						}
					}
				}
			}

			std::tr1::shared_ptr<std::ofstream> getFileHandle(const int& ind) const {
				if (m_vpOpen->at(ind)) return m_vpDataFiles->at(ind);

				int openFiles = 0;
				for (int i = 0; i < m_vpDataFiles->size(); i++) {
					if (m_vpOpen->at(ind)) openFiles++;
				}

				// close files advancing from begind -- possibly closing roi handles for already processed rois
				if (openFiles == ZRE_MAX_OPEN_FILES) {
					bool found = false;
					for (int z = 0; z < 16 && !found; z++) {
						for (int y = 0; y < 16 && !found; y++) {
							for (int x = 0; x < 16 && !found; x++) {
								size_t roiInd = z * 256 + y * 16 + x;
								for (int i = 0; i < m_partitions[roiInd].regionInds.size() && !found; i++) {
									int currentRoi = m_partitions[roiInd].regionInds[i];
									if (m_vpOpen->at(currentRoi)) {
										m_vpDataFiles->at(currentRoi)->close();
										m_vpOpen->at(currentRoi) = false;
										found = true;
									}
								}
							}
						}
					}
				}

				m_vpDataFiles->at(ind)->open(m_vpFilenames->at(ind).c_str(), std::ios::binary | std::ios::ate | std::ios::app | std::ios::out);
				m_vpOpen->at(ind) = true;
				return m_vpDataFiles->at(ind);
			}

			void closeDataFiles() const {
				for (int i = 0; i < m_vpDataFiles->size(); i++) {
					if (m_vpDataFiles->at(i)->is_open()) {
						m_vpDataFiles->at(i)->close();
					}
				}
			}

			void writeHeader(const size_t& ind) const {
				RegionOfInterest roi = m_regionsOfInterest[ind];
				int index = roi.index();

				Filename roiHName = this->m_filenames[0];
				roiHName.refactor();
				std::stringstream ending;
				ending << "_part_" << ind;
				roiHName.name += ending.str();
				roiHName.refactor();

				if (!this->open(roiHName)) {
					LOG_THROW_RUNTIME(Log::Level::Error, Log::msg("[ZeissRawExtendedExporter] Cannot open output file for writing (&1&)") % roiHName.complete);
				}

				uint32 maskX = (*m_pMaskScaling)(0), maskY = (*m_pMaskScaling)(1), maskZ = (*m_pMaskScaling)(2);

				Math::Vector3ui vSize =  Math::create<Math::Vector3ui>(roi.backUpperRight()(0) - roi.frontLowerLeft()(0), roi.backUpperRight()(1) - roi.frontLowerLeft()(1), roi.backUpperRight()(2) - roi.frontLowerLeft()(2));
				vSize(0) *= maskX; vSize(1) *= maskY; vSize(2) *= maskZ;
				Math::Vector3d reduction = Math::create<Math::Vector3d>(static_cast<double>(m_pInSize->size()(0)) / vSize(0), static_cast<double>(m_pInSize->size()(1)) / vSize(1), static_cast<double>(m_pInSize->size()(2)) / vSize(2));
				Math::Vector3d res = Math::create<Math::Vector3d>(m_pInSize->sizeMM()(0) / reduction(0), m_pInSize->sizeMM()(1) / reduction(1), m_pInSize->sizeMM()(2) / reduction(2));

				this->m_file << "{volume1}" << std::endl;
				this->m_file << "[representation]" << std::endl;
				this->m_file << "size = " << vSize(0) << " " << vSize(1) << " " << vSize(2) << std::endl;
				this->m_file << "mirror = 0 0 0 0 " << std::endl;
				this->m_file << "datatype = unsigned integer" << std::endl;
				this->m_file << "datarange = 0 -1" << std::endl;
				this->m_file << "bitsperelement = " << (sizeof(T) * 8) << std::endl;
				this->m_file << "[file1]" << std::endl;
				this->m_file << "SkipHeader = 0" << std::endl;
				this->m_file << "RegionOfInterestStart = 0 0 0" << std::endl;
				this->m_file << "RegionOfInterestEnd = " << (vSize(0) - 1) << " " << (vSize(1) - 1) << " " << (vSize(2) - 1) << std::endl;
				this->m_file << "FileFormat = raw" << std::endl;
				this->m_file << "Size = " << vSize(0) << " " << vSize(1) << " " << vSize(2) << std::endl;
				std::stringstream dataFileStream;
				dataFileStream << roiHName.name << ".uint" << (sizeof(T) * 8);
				std::string dataFile = dataFileStream.str();
				this->m_file << "Name = " << dataFile << std::endl;
				this->m_file << "Datatype = unsigned integer" << std::endl;
				this->m_file << "datarange = 0 -1 " << std::endl;
				this->m_file << "BitsPerElement = " << (sizeof(T) * 8) << std::endl;
				this->m_file << "{volumeprimitive12}" << std::endl;
				this->m_file << "[geometry]" << std::endl;
				this->m_file << "resolution = " << std::setprecision(20) << res(0)/vSize(0) << " " << res(1)/vSize(1) << " " << res(2)/vSize(2) << std::endl;
				this->m_file << "unit = mm" << std::endl;
				this->m_file << "[volume]" << std::endl;
				this->m_file << "volume = volume1" << std::endl;
				this->m_file << "[description]" << std::endl;
				this->m_file << "text = part_" << index << std::endl;
				this->m_file << "PartOffset = " << roi.frontLowerLeft()(0) * (*m_pMaskScaling)(0) << " " << roi.frontLowerLeft()(1) * (*m_pMaskScaling)(1) << " " << roi.frontLowerLeft()(2) * (*m_pMaskScaling)(2) << std::endl;

				std::string dataFileStr = (boost::filesystem::path(roiHName.path) / boost::filesystem::path(dataFile)).string();

				if (!this->m_file.good()) {
					this->close();
					LOG_THROW_RUNTIME(Log::Level::Error, Log::msg("[ZeissRawExtendedExporter] Problem writing the header file (&1&).") % roiHName.complete);
				}

				this->close();

				m_vpDataFiles->at(ind) = std::tr1::shared_ptr<std::ofstream>(new std::ofstream());
				m_vpDataFiles->at(ind)->open(dataFileStr.c_str(), std::ios::binary | std::ios::trunc | std::ios::out);
				m_vpFilenames->at(ind) = dataFileStr;

				if (!m_vpDataFiles->at(ind)->good()) {
					closeDataFiles();
					LOG_THROW_RUNTIME(Log::Level::Error, Log::msg("[ZeissRawExtendedExporter] Cannot open output data file for writing (&1&).") % dataFile);
				}

				m_vpDataFiles->at(ind)->close();
				m_vpOpen->at(ind) = false;
			}

			std::tr1::shared_ptr<std::vector<std::string> > m_vpFilenames;
			SubVolumeInfo m_partitions[4096]; // 16*16*16
			std::tr1::shared_ptr<Math::Vector3ui> m_pCurrentPos, m_pMaskScaling;
			std::vector<RegionOfInterest> m_regionsOfInterest;
			std::tr1::shared_ptr<Image3DSize> m_pInSize;
			std::tr1::shared_ptr<std::vector<AnyPtr> > m_pTodo;
			std::tr1::shared_ptr<Image3DData<MaskT> > m_pMask;
			std::tr1::shared_ptr<std::vector<bool> > m_vpOpen;
			std::tr1::shared_ptr<std::vector<std::tr1::shared_ptr<std::ofstream> > > m_vpDataFiles;
			T maskedArr[1];
		};

		typedef ZeissRawExtendedExporter<openOR::uint8> ZeissRawExtendedExporterUI8;
		typedef ZeissRawExtendedExporter<openOR::uint16> ZeissRawExtendedExporterUI16;
		typedef ZeissRawExtendedExporter<openOR::uint16, 0, openOR::uint16> ZeissRawExtendedExporterUI16MaskUI16;
	}
}

#undef LOG_THROW_RUNTIME
#undef LOG_THROW_LOGIC

#endif
