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

#ifndef openOR_ImageIO_ZeissRawExporter_hpp
#define openOR_ImageIO_ZeissRawExporter_hpp

#include <openOR/Defs/Zeiss_ImageIO.hpp> //Image_ImageIO
#include <openOR/Image/FileExporter.hpp> //Image_ImageIO
#include <openOR/Image/Filename.hpp>  //Image_ImageIO
#include <openOR/Image/Image3DSize.hpp>  //Image_ImageData
#include <openOR/Image/Image3DRawData.hpp> //Image_ImageData
#include <openOR/Math/vector.hpp> //openOR_core

#include <openOR/Log/Logger.hpp> //openOr_core

#include <boost/format.hpp>
#include <boost/type_traits/is_floating_point.hpp>
#include <boost/filesystem.hpp>

#include <exception>
#include <stdexcept>
#include <string>
#include <iostream>

#define LOG_THROW_RUNTIME(level, fmt) { boost::format __fmt = fmt; LOG(level, OPENOR_MODULE, __fmt); throw std::runtime_error(__fmt.str()); }
#define LOG_THROW_LOGIC(level, fmt) { boost::format __fmt = fmt; LOG(level, OPENOR_MODULE, __fmt); throw std::logic_error(__fmt.str()); }

namespace openOR {
   namespace Image {

      template<typename T>
      class ZeissRawExporter : public FileExporter<T> {
      public:

         ZeissRawExporter() {
         }

         ~ZeissRawExporter() {
         }

         void operator()() const {
            //std::cout << "exporter" << std::endl;
            bool is2D = false;
            Math::Vector3ui vSize;
            Math::Vector3d res;
            if (this->m_pSize3D) {
               if (this->m_pSize2D) {
                  LOG_THROW_LOGIC(Log::Level::Error, Log::msg("[ZeissRawExporter] Both 2D and 3D size specified."));
               }
               vSize = this->m_pSize3D->size();
               res = this->m_pSize3D->sizeMM();
               if (this->m_pImage2D) {
                  LOG_THROW_LOGIC(Log::Level::Error, Log::msg("[ZeissRawExporter] Supplied an 2D image for 3D size specification."));
               }
               if (!this->m_pImage3D) {
                  LOG_THROW_LOGIC(Log::Level::Error, Log::msg("[ZeissRawExporter] No 3D image data supplied."));
               }
            } else if (this->m_pSize2D) {
               is2D = true;
               vSize(0) = this->m_pSize2D->size()(0);
               vSize(1) = this->m_pSize2D->size()(1);
               vSize(2) = 1;
               res(0) = this->m_pSize2D->sizeMM()(0);
               res(1) = this->m_pSize2D->sizeMM()(1);
               res(2) = 0;
               if (this->m_pImage3D) {
                  LOG_THROW_LOGIC(Log::Level::Error, Log::msg("[ZeissRawExporter] Supplied an 3D image for 2D size specification."));
               }
               if (!this->m_pImage2D) {
                  LOG_THROW_LOGIC(Log::Level::Error, Log::msg("[ZeissRawExporter] No 2D image data supplied."));
               }
            } else {
               LOG_THROW_LOGIC(Log::Level::Error, Log::msg("[ZeissRawExporter] Neither 2D nor 3D size specified."));
            }

            if (this->m_filenames[0].empty()) {
               LOG_THROW_RUNTIME(Log::Level::Warning, Log::msg("[ZeissRawExporter] No output file specified."));
            }
            
            if (boost::is_floating_point<T>::value) {
               LOG_THROW_LOGIC(Log::Level::Error, Log::msg("[ZeissRawExporter] The passed export type is floating point - unknown formatting"));
            }

            if (!this->open(this->m_filenames[0].complete)) {
               LOG_THROW_RUNTIME(Log::Level::Error, Log::msg("[ZeissRawExporter] Cannot open output file for writing."));
            }

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
            std::stringstream dataFileending;
            dataFileending << "uint" << (sizeof(T) * 8);
            Filename dataFilename = this->m_filenames[0];
            dataFilename.ending = dataFileending.str();
            dataFilename.refactor();
            std::string dataFile = dataFilename.complete;
            this->m_file << "Name = " << dataFilename.name << "." << dataFilename.ending << std::endl;
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
            this->m_file << "text = Description" << std::endl;

            if (!this->m_file.good()) {
               this->close();
               LOG_THROW_RUNTIME(Log::Level::Error, Log::msg("[ZeissRawExporter] Problem writing the header file."));
            }

            this->close();

            if (!this->open(dataFile)) {
               LOG_THROW_RUNTIME(Log::Level::Error, Log::msg("[ZeissRawExporter] Cannot open output data file for writing."));
            }

            long long imgSize = static_cast<long long>(vSize(0)) * vSize(1) * vSize(2);
            long long toWrite = static_cast<long long>(imgSize) * sizeof(T);
            const T* data = is2D ? this->m_pImage2D->data() : this->m_pImage3D->data();

            std::cout << "Write volume" << std::endl;
            if (!this->write(reinterpret_cast<const void*>(data), toWrite)) {
               this->close();
               LOG_THROW_RUNTIME(Log::Level::Error, Log::msg("[ZeissRawExporter] Problem writing data file"));
            }
            this->close();
         }
      };

      typedef ZeissRawExporter<openOR::uint8> ZeissRawExporterUI8;
      typedef ZeissRawExporter<openOR::uint16> ZeissRawExporterUI16;
   }
}

#undef LOG_THROW_RUNTIME
#undef LOG_THROW_LOGIC

#endif
