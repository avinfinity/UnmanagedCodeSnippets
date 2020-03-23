//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR commercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//! OPENOR_INTERFACE_FILE(Image_ImageIO)
//! \file
//! \ingroup Image_ImageIO
//****************************************************************************

#ifndef openOR_ImageIO_FileImporter_hpp
#define openOR_ImageIO_FileImporter_hpp

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <openOR/Defs/Image_ImageIO.hpp>
#include <openOR/Plugin/AnyPtr.hpp>
#include <openOR/Callable.hpp>
#include <openOR/Progressable.hpp>
#include <openOR/Cancelable.hpp>
#include <openOR/DataSettable.hpp>
#include <openOR/Image/Filename.hpp>
#include <openOR/Image/Image2DRawData.hpp>
#include <openOR/Image/Image2DSize.hpp>
#include <openOR/Image/Image3DRawData.hpp>
#include <openOR/Image/Image3DSize.hpp>
#include <openOR/Image/InfoPatient.hpp>
#include <openOR/Image/InfoProjection.hpp>
#include <openOR/Image/InfoDevice.hpp>
#include <openOR/Image/InfoData.hpp>
#include <openOR/Image/DataCollector.hpp>

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>

#include <openOR/Log/Logger.hpp>
#   define OPENOR_MODULE_NAME "Image.ImageIO"
#   include <openOR/Log/ModuleFilter.hpp>

namespace openOR {
	namespace Image {

		template<typename T>
		class FileImporter : public Callable, public Progressable, public Cancelable, public DataSettable {
		public:

			FileImporter() :
			  m_filenames(),
				  m_progressValue(0.0),
				  m_isCanceled(false)
			  {
			  }

			  ~FileImporter() {
				  if (m_file.is_open()) { m_file.close(); }
			  }

			  void setFilenames(const std::vector<std::string>& filenames) {
				  m_filenames.clear();
				  for (std::vector<std::string>::const_iterator it = filenames.begin(); it != filenames.end(); it++) {
					  m_filenames.push_back(Filename(*it));
				  }
			  }

			  void setFilename(const std::string& filename) {
				  // clear old set of filenames
				  m_filenames.clear();

				  // check, if file exists
				  //std::fstream file;
				  //file.open(filename.c_str(), std::fstream::in);
				  //if (!file.good()) {
				  //LOG(Log::Level::Error, OPENOR_MODULE, Log::msg("Given filename points to an empty or nonexisting file."));
				  //return;
				  //}
				  m_filenames.push_back(Filename(filename));
				  //file.close();
			  }

			  void setData(const AnyPtr& pData, const std::string&) {

				  if (!m_pImage2D) { m_pImage2D = interface_cast<Image2DRawData<T> >(pData); }
				  if (!m_pImage3D) { m_pImage3D = interface_cast<Image3DRawData<T> >(pData); }
				  if (!m_pSize2D) { m_pSize2D = interface_cast<Image2DSize>(pData); }
				  if (!m_pSize3D) { m_pSize3D = interface_cast<Image3DSize>(pData); }
				  if (!m_pPatient) { m_pPatient = interface_cast<InfoPatient>(pData); }
				  if (!m_pDevice) { m_pDevice = interface_cast<InfoDevice>(pData); }
				  if (!m_pData) { m_pData = interface_cast<InfoData<T> >(pData); }
				  if (!m_pProjection) { m_pProjection = interface_cast<InfoProjection>(pData); }
				  if (!m_pDataCollector) { m_pDataCollector = interface_cast<DataCollector<T> >(pData); }
			  }

			  void setData(const std::vector<AnyPtr>& vecData) {
				  std::tr1::shared_ptr<Image2DRawData<T> > pImage2D;
				  std::tr1::shared_ptr<Image2DSize> pSize2D;
				  std::tr1::shared_ptr<InfoProjection> pProjection;

				  for (int n = 0; n < (int)vecData.size(); n++) {
					  pImage2D = interface_cast<Image2DRawData<T> >(vecData[n]);
					  if (pImage2D) { m_pImages2D.push_back(pImage2D); }

					  pSize2D = interface_cast<Image2DSize>(vecData[n]);
					  if (pSize2D) { m_pSizes2D.push_back(pSize2D); }

					  pProjection = interface_cast<InfoProjection>(vecData[n]);
					  if (pProjection) { m_pProjections.push_back(pProjection); }

				  }
			  }

			  // Progressable interface
			  double progress() const {
				  return std::max<double>(std::min<double>(m_progressValue, 1.0), 0.0);
			  }
			  std::string description() const {
				  return m_progressDescription;
			  }

			  // Cancelable interface

			  // should be overwritten by all children classes
			  void cancel() {
				  m_isCanceled = true;
				  setProgress(1.0, "Canceled");
			  }

			  bool isCanceled() const {
				  return m_isCanceled;
			  }

		protected:

			void open(Filename filename) const {
				if (m_file.is_open()) {
					m_file.clear();
					m_file.close();
				}
				m_file.clear();
				m_file.open(filename.complete.c_str(), std::ios::in | std::ios::binary);

				if(!m_file.good()) {
					m_file.close();
					LOG(Log::Level::Error, OPENOR_MODULE,
						Log::msg("[File Importer]: Cannot open file %1%.") % filename.complete.c_str()
						);
					return;
				}
			}

			void read(void* address, int bytes) const {
				if (!m_file.is_open()) { return; }
				m_file.read(reinterpret_cast<char*>(address), bytes);
				if (m_file.fail() | m_file.eof()) {
					LOG(Log::Level::Error, OPENOR_MODULE,
						Log::msg("[File Importer]: Cannot read %1% bytes - reached end of file. %2% bytes read only.") % bytes % m_file.gcount()
						);
				}
			}

			void close() const {
				if (m_file.is_open()) {
					m_file.close();
					if (!m_file.good() && !m_file.eof()) {
						LOG(Log::Level::Error, OPENOR_MODULE,
							Log::msg("[File Exporter]: Closing file failed.")
							);
					}
				}
			}

			void setProgress(const double& value, const std::string& description) const {
				m_progressValue = value;
				m_progressDescription = description;
			}

			mutable std::ifstream m_file;
			mutable std::vector<openOR::Image::Filename> m_filenames;

			mutable std::tr1::shared_ptr<Image2DRawData<T> > m_pImage2D;
			mutable std::tr1::shared_ptr<Image3DRawData<T> > m_pImage3D;
			mutable std::tr1::shared_ptr<Image2DSize> m_pSize2D;
			mutable std::tr1::shared_ptr<Image3DSize> m_pSize3D;
			mutable std::tr1::shared_ptr<InfoData<T> > m_pData;
			mutable std::tr1::shared_ptr<InfoDevice> m_pDevice;
			mutable std::tr1::shared_ptr<InfoPatient> m_pPatient;
			mutable std::tr1::shared_ptr<InfoProjection> m_pProjection;
			mutable std::tr1::shared_ptr<DataCollector<T> > m_pDataCollector;

			mutable std::vector<std::tr1::shared_ptr<Image2DRawData<T> > > m_pImages2D;
			mutable std::vector<std::tr1::shared_ptr<Image2DSize> > m_pSizes2D;
			mutable std::vector<std::tr1::shared_ptr<InfoProjection> > m_pProjections;

		private:
			mutable double m_progressValue;
			mutable std::string m_progressDescription;
			mutable bool m_isCanceled;
		};
	}
}


#endif
