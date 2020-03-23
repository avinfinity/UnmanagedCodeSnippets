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

#ifndef openOR_ImageIO_ZeissRawImporter_hpp
#define openOR_ImageIO_ZeissRawImporter_hpp

#include <openOR/Defs/Zeiss_ImageIO.hpp>
#include <openOR/Image/FileImporter.hpp> //Image_ImageIO

#include <openOR/Image/Image2DSize.hpp> //Image_ImageData
#include <openOR/Image/Image2DRawData.hpp> //Image_ImageData
#include <openOR/Image/Image3DSize.hpp> //Image_ImageData
#include <openOR/Image/Image3DRawData.hpp> //Image_ImageData
#include <openOR/Image/Image3DSizeDescriptor.hpp> //Image_ImageData

#include <openOR/Math/vector.hpp> //openOR_core
#include <openOR/Math/create.hpp> //openOR_core

#include <openOR/Plugin/create.hpp> //openOR_core

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>
#include <fstream>
#include <typeinfo>

#include <cstdlib>

#include <algorithm>

#include <openOR/Log/Logger.hpp> //openOr_core
#   define OPENOR_MODULE_NAME "ImageIO.ZeissRawImporter"
#   include <openOR/Log/ModuleFilter.hpp>

namespace openOR {
	namespace Image {      

		template<typename T>
		class ZeissRawImporter : public FileImporter<T> {
		public:

			ZeissRawImporter() {
				m_scaling = 1;
				m_readSlices = 1;
				m_skipHeader = 0;
				m_isRawData = false;
			}

			~ZeissRawImporter() {

			}

			void setScalingFactor(unsigned short reduce) {
				m_scaling = std::max<short>(reduce, 1);
			}

			void setIsRawData(bool isRawData, size_t skipHeader = 0) {
				m_skipHeader = skipHeader;
				m_isRawData = isRawData;
			}

			void operator()() const {
				// importer expects only one file
				if (this->m_filenames.size() != 1) {
					LOG(Log::Level::Warning, OPENOR_MODULE, 
						Log::msg("No file specified from which the image might be loaded! Set filename correctly.")
						);
					return;
				}

				if (this->m_pImage2D != NULL && this->m_pSize2D != NULL) {
					MetaInformation meta;

					// set progress
					this->setProgress(0.0, "Initialization");

					// *** read header of file ***
					getMetaInfo(meta);

					// *** validity check of header information ***
					if (meta.datatype.empty()) {
						LOG(Log::Level::Debug, OPENOR_MODULE, 
							Log::msg("Datatype of raw file not supported. Expecting uint16 or uint8.")
							);
						return;
					}
					if (meta.datatype != typeid(T).name()) {
						LOG(Log::Level::Error, OPENOR_MODULE, 
							Log::msg("Given type of importer (%1%) doesn't match the provided type of pixel/voxel data (%2%).")
							% typeid(T).name() % meta.datatype
							);
						return;
					}
					// *** end of validity check ***


					// decide what ImageData must be expected
					// an image
					// 
					this->m_pSize2D->setSize(Math::create<Math::Vector2ui>(meta.size(0), meta.size(1)));
					this->m_pSize2D->setSizeMM(Math::create<Math::Vector2d>(meta.sizeMM(0) * meta.size(0), meta.sizeMM(1) * meta.size(1)));

					if (meta.datafile.path.empty()) {
						meta.datafile.path = this->m_filenames.at(0).path;
						meta.datafile.refactor();
					}

					// *** read raw data
					this->open(meta.datafile);
					this->read(reinterpret_cast<char*>(this->m_pImage2D->mutableData()), static_cast<size_t>(meta.size(0)) * meta.size(1) * meta.bytes);
					this->close();
				} else if (this->m_pImage3D != NULL && this->m_pSize3D != NULL) {
					MetaInformation meta;
					this->m_filenames[0].refactor();
					if (!this->m_isRawData) {
						// set progress
						this->setProgress(0.0, "Initialization");

						// *** read header of file ***
						getMetaInfo(meta);

						// *** validity check of header information ***
						if (meta.datatype.empty()) {
							LOG(Log::Level::Debug, OPENOR_MODULE, 
								Log::msg("Datatype of raw file not supported. Expecting uint16 or uint8.")
								);
							return;
						}
						if (meta.datatype != typeid(T).name()) {
							LOG(Log::Level::Error, OPENOR_MODULE, 
								Log::msg("Given type of importer (%1%) doesn't match the provided type of pixel/voxel data (%2%).")
								% typeid(T).name() % meta.datatype
								);
							return;
						}
						// *** end of validity check ***
					} else {
						meta.size = this->m_pSize3D->size();
						meta.sizeMM = Math::create<Math::Vector3d>(widthMM(this->m_pSize3D) / width(this->m_pSize3D), heightMM(this->m_pSize3D) / height(this->m_pSize3D), depthMM(this->m_pSize3D) / depth(this->m_pSize3D));
						meta.bytes = 2;
						meta.headersize = m_skipHeader;
						meta.datafile = this->m_filenames[0];

						interface_cast<Image::Image3DSize>(this->m_pImage3D)->setSize(this->m_pSize3D->size());
						interface_cast<Image::Image3DSize>(this->m_pImage3D)->setSizeMM(this->m_pSize3D->sizeMM());
					}

					// set progress
					this->setProgress(0.0, "Initialization");

					// *** read header of file ***
					getMetaInfo(meta);

					// *** validity check of header information ***
					if (meta.datatype.empty()) {
						LOG(Log::Level::Debug, OPENOR_MODULE, 
							Log::msg("Datatype of raw file not supported. Expecting uint16 or uint8.")
							);
						return;
					}
					if (meta.datatype != typeid(T).name()) {
						LOG(Log::Level::Error, OPENOR_MODULE, 
							Log::msg("Given type of importer (%1%) doesn't match the provided type of pixel/voxel data (%2%).")
							% typeid(T).name() % meta.datatype
							);
						return;
					}
					// *** end of validity check ***


					// decide what ImageData must be expected
					// an image
					LOG(Log::Level::Info, OPENOR_MODULE,
						Log::msg("Reading image into continious dataset")
						);
					// set size of volume
					bool allocSuccess = false;
					short reduceFactor = 1, n = 0;
					while (!allocSuccess & n < 4) {
						reduceFactor = m_scaling + n;
						float reduce = 1.0f / float(reduceFactor);
						try {
							this->m_pSize3D->setSize(Math::create<Math::Vector3ui>(meta.size(0) * reduce, meta.size(1) * reduce, meta.size(2) * reduce));
							allocSuccess = true;
						} catch (...) {
							LOG(Log::Level::Warning, OPENOR_MODULE, 
								Log::msg("[Zeiss Raw Importer]: Provided volume is too large (%1% x %2% x %3%). Try to reduce the size (x1/%4%).")
								% meta.size(0) % meta.size(1) % meta.size(2) % (reduceFactor + 1)
								);

							n++;
						}
					}
					this->m_pSize3D->setSizeMM(Math::create<Math::Vector3d>(meta.sizeMM(0) * meta.size(0), meta.sizeMM(1) * meta.size(1), meta.sizeMM(2) * meta.size(2)));
					if (!allocSuccess) {
						LOG(Log::Level::Warning, OPENOR_MODULE, 
							Log::msg("Setting the size failed.")
							);
					}

					// refactor filename of data file
					if (meta.datafile.path.empty()) {
						meta.datafile.path = this->m_filenames.at(0).path;
						meta.datafile.refactor();
					}

					// *** read raw data
					this->open(meta.datafile);
					if (!this->m_file.good()) { return; }

					// skip header
					char* header = new char[meta.headersize];
					this->read(header, meta.headersize);
					delete [] header;

					// read volume data
					size_t nWidth = meta.size(0);
					size_t nHeight = meta.size(1);
					size_t nDepth = reduceFactor;
					size_t nNormFactor = reduceFactor * reduceFactor * reduceFactor;

					char* pTempData = new char[static_cast<long long>(nWidth) * nHeight * nDepth * meta.bytes];
					float fTmp = 0;
					size_t nZIndex = 0;
					size_t nYIndex = 0;
					long long maxIdx = static_cast<long long>(this->m_pSize3D->size()(0)) * this->m_pSize3D->size()(1) * this->m_pSize3D->size()(2);
					size_t curIdx = 0;
					double progress = 0.98f / float(maxIdx);

					bool isUI8 = false, isUI16 = false, isUI32 = false;

					if (meta.bytes == 1 && meta.datatype == typeid(uint8).name()) {
						isUI8 = true;
					} else if (meta.bytes == 2 && meta.datatype == typeid(uint16).name()) {
						isUI16 = true;
					} else if (meta.bytes == 4 && meta.datatype == typeid(uint32).name()) {
						isUI32 = true;
					} else {
						LOG(Log::Level::Warning, OPENOR_MODULE, 
							Log::msg("Don't know how to import %1% with %2% bytes width.") % meta.datatype % meta.bytes
							);
					}

					while (curIdx < maxIdx && !this->m_file.eof() && !this->isCanceled()) {
						// read small slice (slice thickness is reduceFactor)
						this->read(pTempData, meta.bytes * nWidth * nHeight * nDepth);

						// for each cluster (cubic subvolume) of slice ... 
						for (size_t nY = 0; nY < (nHeight - reduceFactor + 1); nY += reduceFactor) {
							for (size_t nX = 0; nX < (nWidth - reduceFactor + 1); nX += reduceFactor) {

								// ... get all voxels and sum up their values
								fTmp = 0;
								for (size_t z = 0; z < reduceFactor; ++z) {
									nZIndex = z * nWidth * nHeight;
									for (size_t y = 0; y < reduceFactor; ++y) {
										nYIndex = (y + nY) * nWidth + nZIndex;
										for (size_t x = 0; x < reduceFactor; ++x) {
											if (isUI8) {
												fTmp += reinterpret_cast<uint8*>(pTempData)[x + nX + nYIndex];
											} else if (isUI16) {
												fTmp += reinterpret_cast<uint16*>(pTempData)[x + nX + nYIndex];
											} else if (isUI32) {
												fTmp += reinterpret_cast<uint32*>(pTempData)[x + nX + nYIndex];
											}
										}
									}
								}

								// calc mean of cluster
								T val = static_cast<T>(floor((fTmp / static_cast<float>(nNormFactor)) + 0.5));
								this->m_pImage3D->mutableData()[curIdx] = val;
								++curIdx;
							}
						}

						this->setProgress(progress * curIdx, "Reading volume data...");
					}


					this->close();

					delete [] pTempData;

					this->setProgress(1.0, "Finishing...");
				} else if (this->m_pDataCollector) {
					MetaInformation meta;
					this->m_filenames[0].refactor();
					if (!this->m_isRawData) {
						// set progress
						this->setProgress(0.0, "Initialization");

						// *** read header of file ***
						getMetaInfo(meta);

						// *** validity check of header information ***
						if (meta.datatype.empty()) {
							LOG(Log::Level::Debug, OPENOR_MODULE, 
								Log::msg("Datatype of raw file not supported. Expecting uint16 or uint8.")
								);
							return;
						}
						if (meta.datatype != typeid(T).name()) {
							LOG(Log::Level::Error, OPENOR_MODULE, 
								Log::msg("Given type of importer (%1%) doesn't match the provided type of pixel/voxel data (%2%).")
								% typeid(T).name() % meta.datatype
								);
							return;
						}
						// *** end of validity check ***
					} else {
						if (!this->m_pSize3D) {
							LOG(Log::Level::Error, OPENOR_MODULE,
								Log::msg("No size information provided for reading a raw file")
								);
						}
						meta.size = this->m_pSize3D->size();
						meta.sizeMM = Math::create<Math::Vector3d>(widthMM(this->m_pSize3D) / width(this->m_pSize3D), heightMM(this->m_pSize3D) / height(this->m_pSize3D), depthMM(this->m_pSize3D) / depth(this->m_pSize3D));
						meta.bytes = 2;
						meta.headersize = m_skipHeader;
						meta.datafile = this->m_filenames[0];
					}

					LOG(Log::Level::Info, OPENOR_MODULE,
						Log::msg("Reading image into data collector")
						);

					std::tr1::shared_ptr<Image3DSizeDescriptor> size = createInstanceOf<Image3DSizeDescriptor>();
					size->setSize(meta.size);
					size->setSizeMM(Math::create<Math::Vector3d>(meta.sizeMM(0) * meta.size(0), meta.sizeMM(1) * meta.size(1), meta.sizeMM(2) * meta.size(2)));
					this->m_pDataCollector->setData(size, "inSize");

					if (meta.datafile.path.empty()) {
						meta.datafile.path = this->m_filenames.at(0).path;
						meta.datafile.refactor();
					}
					this->open(meta.datafile);

					if (!this->m_file.good()) { return; /* TODO: I/O-error */ }

					char* header = new char[meta.headersize];
					this->read(header, meta.headersize);
					delete [] header;

					uint nWidth = meta.size(0);
					uint nHeight = meta.size(1);

					T* pTempData = new T[nWidth * nHeight * meta.bytes * m_readSlices];

					for (int i = 0; i < meta.size(2); i += m_readSlices) {
						int rest = meta.size(2) - i;
						if (rest > m_readSlices) rest = m_readSlices;
						this->read(reinterpret_cast<char*>(pTempData), rest * meta.bytes * nWidth * nHeight);
						if (!this->m_file.good()) {
							delete [] pTempData;

							this->close();
							return;
						}
						this->m_pDataCollector->setData(pTempData, rest * nWidth * nHeight, "in");
						(*(this->m_pDataCollector))();
						this->setProgress(static_cast<float>(i - 10) / meta.size(2), "Loading volume...");

						if (this->isCanceled()) {
							break;
						}
					}

					if (this->isCanceled()) {
						std::tr1::shared_ptr<Cancelable> toCancel = interface_cast<Cancelable>(this->m_pDataCollector);
						if (toCancel) {
							toCancel->cancel();
						}
					}

					this->setProgress(1.0f, "Finishing...");

					delete [] pTempData;

					this->close();
				}

				LOG(Log::Level::Info, OPENOR_MODULE,
					Log::msg("Done reading")
					);


				return;
			}
			//TODO pivate?
			//private: 
			class MetaInformation {
			public:

				MetaInformation() {
					size = Math::create<Math::Vector3ui>(0, 0, 0);
					sizeMM = Math::create<Math::Vector3d>(0.0, 0.0, 0.0);
				}
				~MetaInformation() {

				}

				void dbgLogOut() {
					LOG(Log::Level::Debug, OPENOR_MODULE,
						Log::msg("Meta File Information\n"
						"  Bytes per Voxel:    %1%\n"
						"  Headersize:         %2%\n"
						"  Datatype:           '%3%'\n"
						"  Size (in Pixel):    (%4%, %5%, %6%)\n"
						"  Size (in mm):       (%7%, %8%, %9%)\n") 
						% int(bytes) % headersize % datatype
						% size(0) % size(1) % size(2)
						% sizeMM(0) % sizeMM(1) % sizeMM(2)
						);
				}

				uint8 bytes;
				std::string datatype;
				Math::Vector3ui size;
				Math::Vector3d sizeMM;
				unsigned int headersize;
				Filename datafile;
			};

			void getMetaInfo(MetaInformation& info) const {
				if (!m_isRawData) {
					std::string buffer;
					int bits = 0;
					std::string strDatatype;
					Math::Vector3d tmpSizeMM;
					int scaleMM = 1;

					this->open(this->m_filenames.at(0));

					while (this->m_file.good()) {
						// read one line of the file
						getline(this->m_file, buffer);

						// remove trailing cr- or lf-sequence
						size_t pos = buffer.find_last_of(0x0D);
						if (pos != std::string::npos) { buffer = buffer.substr(0, pos); }

						// divide read line into key and value (if applicable)
						pos = buffer.rfind(" = ");
						if (pos == std::string::npos) { continue; }

						std::string key = buffer.substr(0, pos);
						std::string value = buffer.substr(pos + 3);

						// check key and variables accordingly
						if (key == "bitsperelement") {
							try {
								bits = boost::lexical_cast<int>(value);
							} catch (...) {
							}
						} else if (key == "datatype") {
							strDatatype = value;
						} else if (key == "resolution") {
							castFromString<double, 3>(tmpSizeMM, value);
						} else if (key == "size") {
							castFromString<unsigned int, 3>(info.size, value);
						} else if (key == "unit") {
							if (value == "mm") {
								scaleMM = 1;
							} else if (value == "cm") {
								scaleMM = 10;
							}
						} else if (key == "SkipHeader") {
							try {
								info.headersize = boost::lexical_cast<unsigned int>(value);
							} catch (...) {
							}
						} else if (key == "Name") {
							info.datafile = Filename(value);
						}
					}

					tmpSizeMM(0) *= scaleMM;
					tmpSizeMM(1) *= scaleMM;
					tmpSizeMM(2) *= scaleMM;
					info.sizeMM = tmpSizeMM;

					if (strDatatype == "unsigned integer") {
						if (bits == 16) {
							info.datatype = std::string(typeid(uint16).name());
							info.bytes = 2;
						} else if (bits == 8) {
							info.datatype = std::string(typeid(uint8).name());
							info.bytes = 1;
						}
					}

					info.dbgLogOut();

					this->close();
				} else {
					info.size = this->m_pSize3D->size();
					info.sizeMM = Math::create<Math::Vector3d>(widthMM(this->m_pSize3D) / width(this->m_pSize3D), heightMM(this->m_pSize3D) / height(this->m_pSize3D), depthMM(this->m_pSize3D) / depth(this->m_pSize3D));
					info.bytes = 2;
					info.headersize = m_skipHeader;
					info.datafile = this->m_filenames[0];
				}
			}

			template<typename TYPE, int SIZE>
			void castFromString(boost::numeric::ublas::bounded_vector<TYPE, SIZE>& vector, const std::string& value) const {
				typedef boost::tokenizer<boost::char_separator<char> > Tok;
				size_t n = 0;
				boost::char_separator<char> sep(" ");
				Tok tokens(value, sep);
				for(Tok::iterator token = tokens.begin(); ((token != tokens.end()) && (n < SIZE)); ++token, ++n) {
					try {
						vector(n) = boost::lexical_cast<TYPE>(*token);
					} catch(...) {
						LOG(Log::Level::Debug, OPENOR_MODULE, 
							Log::msg("lexical cast of [ %1% ] failed ( %2% )") % value % *token
							);
						return;
					}
				}
			}

			unsigned short m_readSlices;
			unsigned short m_scaling;
			size_t m_skipHeader;
			bool m_isRawData;
		};

		typedef ZeissRawImporter<openOR::uint8> ZeissRawImporterUI8;
		typedef ZeissRawImporter<openOR::uint16> ZeissRawImporterUI16;

	}
}

#endif
