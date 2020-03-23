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

#ifndef openOR_ImageIO_FileExporter_hpp
#define openOR_ImageIO_FileExporter_hpp

#include <openOR/Defs/Image_ImageIO.hpp>
#include <openOR/Plugin/AnyPtr.hpp> //openOR_core
#include <openOR/Callable.hpp> //basic
#include <openOR/Progressable.hpp> //basic
#include <openOR/DataSettable.hpp> //basic
#include <openOR/Cancelable.hpp> //basic
#include <openOR/Image/Filename.hpp> //Image_ImageIO
#include <openOR/Image/Image2DRawData.hpp> //Image_Utility
#include <openOR/Image/Image3DRawData.hpp> //Image_Utility
#include <openOR/Image/Image2DSize.hpp> //Image_Utility
#include <openOR/Image/Image3DSize.hpp> //IMage_Utility
#include <openOR/Image/InfoData.hpp> //Image_Utility
#include <openOR/Image/InfoDevice.hpp> //Image_Utility
#include <openOR/Image/InfoPatient.hpp> //Image_Utility
#include <openOR/Image/InfoProjection.hpp> //Image_Utlity

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

#include <openOR/Log/Logger.hpp> //openOR_core
//#   define OPENOR_MODULE_NAME "Image.ImageIO"
#   include <openOR/Log/ModuleFilter.hpp> //openOR_core

namespace openOR {
	namespace Image {

		template<typename T>
		class FileExporter : public Callable, public DataSettable, public Progressable, public Cancelable {
		public:

			FileExporter() :
			  m_filenames(),
				  m_isCanceled(false),
				  m_progressValue(0.0)
			  {
			  }

			  ~FileExporter() {
				  if (m_file.is_open()) { m_file.close(); }
			  }

			  void setFilename(const std::string& filename) {
				  m_filenames.clear();
				  m_filenames.push_back(Filename(filename));
			  }

			  void setFilenames(const std::vector<std::string>& filenames) {
				  m_filenames.clear();
				  for (std::vector<std::string>::const_iterator it = filenames.begin(); it != filenames.end(); it++) {
					  m_filenames.push_back(Filename(*it));
				  }
			  }

			  void setData(const AnyPtr& pData, const std::string& name = "") {
				  if (name == "filename") {
					  std::tr1::shared_ptr<Filename> pFilename = pData.as<Filename>();
					  m_filenames.clear();
					  m_filenames.push_back(*(pFilename.get()));
				  }
				  if (!m_pImage2D) { m_pImage2D = interface_cast<Image2DRawData<T> >(pData); }
				  if (!m_pImage3D) { m_pImage3D = interface_cast<Image3DRawData<T> >(pData); }
				  if (!m_pSize2D) { m_pSize2D = interface_cast<Image2DSize>(pData); }
				  if (!m_pSize3D) { m_pSize3D = interface_cast<Image3DSize>(pData); }
				  if (!m_pPatient) { m_pPatient = interface_cast<InfoPatient>(pData); }
				  if (!m_pDevice) { m_pDevice = interface_cast<InfoDevice>(pData); }
				  if (!m_pData) { m_pData = interface_cast<InfoData<T> >(pData); }
				  if (!m_pProjection) { m_pProjection = interface_cast<InfoProjection>(pData); }
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
			  void cancel() {
				  m_isCanceled = true;
				  setProgress(1.0, "Canceled");
			  }

			  bool isCanceled() const {
				  return m_isCanceled;
			  }

		protected:
			bool open(Filename filename) const {
				std::string myFilename = filename.empty() ? (m_filenames.size() > 0) ? m_filenames[0].complete : std::string() : filename.complete;
				if (myFilename.empty()) { return false; }

				m_file.close();
				m_file.clear();
				m_file.open(myFilename.c_str(), std::ios::binary);

				if(!m_file.good()) {
					m_file.close();
					LOG(Log::Level::Error, OPENOR_MODULE, 
						Log::msg("[File Exporter]: Cannot open file %1%.") % myFilename
						);
					return false;
				}
				return true;
			}

			bool write(const void* address, long long bytes) const {
				m_file.write(reinterpret_cast<const char*>(address), bytes);
				if (m_file.bad()) {
					LOG(Log::Level::Error, OPENOR_MODULE, 
						Log::msg("[File Exporter]: Writing of %1% bytes failed.") % bytes
						);
					return false;
				}
				return true;
			}

			bool close() const {
				if (m_file.is_open()) {
					m_file.close();
					if (!m_file.good()) {
						LOG(Log::Level::Error, OPENOR_MODULE, 
							Log::msg("[File Exporter]: Closing file failed.")
							);
						return false;
					}
				}
				return true;
			}

			void setProgress(const double& value, const std::string& description) const {
				m_progressValue = value;
				m_progressDescription = description;
			}

			mutable std::ofstream m_file;
			mutable std::vector<Filename> m_filenames;

			mutable std::tr1::shared_ptr<Image2DRawData<T> > m_pImage2D;
			mutable std::tr1::shared_ptr<Image3DRawData<T> > m_pImage3D;
			mutable std::tr1::shared_ptr<Image2DSize> m_pSize2D;
			mutable std::tr1::shared_ptr<Image3DSize> m_pSize3D;
			mutable std::tr1::shared_ptr<Image::InfoData<T> > m_pData;
			mutable std::tr1::shared_ptr<Image::InfoDevice> m_pDevice;
			mutable std::tr1::shared_ptr<Image::InfoPatient> m_pPatient;
			mutable std::tr1::shared_ptr<Image::InfoProjection> m_pProjection;

			mutable std::vector<std::tr1::shared_ptr<Image2DRawData<T> > > m_pImages2D;
			mutable std::vector<std::tr1::shared_ptr<Image2DSize> > m_pSizes2D;
			mutable std::vector<std::tr1::shared_ptr<InfoProjection> > m_pProjections;
		private:
			mutable std::string m_progressDescription;
			mutable double m_progressValue;
			mutable bool m_isCanceled;
		};
	}
}

#endif