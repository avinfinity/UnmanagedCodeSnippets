//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR commercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#ifndef openOR_Plugin_LibHandle_windows_hpp
#define openOR_Plugin_LibHandle_windows_hpp

#include "../include/openOR/Plugin/Library.hpp"

#include <windows.h>

#ifndef OPENOR_PRIVATE_DLL_MODE_APPENDIX
   #define OPENOR_PRIVATE_DLL_MODE_APPENDIX ""
#endif

#include "../include/openOR/Log/Logger.hpp"
#   define OPENOR_MODULE_NAME "Core.Plugin.Library"
#   include "../include/openOR/Log/ModuleFilter.hpp"

namespace openOR {
   namespace Plugin {
      namespace Detail {

         std::string platform_library_extension_windows() {
            return std::string(OPENOR_PRIVATE_DLL_MODE_APPENDIX) + ".dll";
         }

         std::string platform_library_prefix_windows() {
            return "";
         }

         //----------------------------------------------------------------------------
         //! \brief Dynamic Shared Object Loading (Platform Dependent: Windows)
         //!
         //! windows api based dynamic shared object (DLL) loader
         //!
         //----------------------------------------------------------------------------
         struct LibHandle_windows : LibHandle {

            LibHandle_windows(const std::string& fileName, bool unload = false) :
                  m_handle(),
                  m_unload(unload) {
               std::string fileNameSo = fileName + platform_library_extension_windows();
               LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("Opening DLL Library '%1%'") % fileNameSo)
               m_handle = LoadLibraryA(fileNameSo.c_str());
               if (m_handle == NULL) {
                  throw library_open_failure(fileNameSo);
               }
            }

            virtual ~LibHandle_windows() {

               if (m_unload) {
                  FreeLibrary(m_handle);
               }
            }

         private:
            HMODULE  m_handle;
            bool     m_unload;

         };

         bool scan_file_for_library_windows(const std::string& fullfileName) {

            LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("Scanning DLL Library '%1%'") % fullfileName)
            HMODULE handle = LoadLibraryA(fullfileName.c_str());
            if (handle == NULL) {
               return false;
            }
            //FreeLibrary( handle );
            return true;
         }

      };
   };
};

#endif //openOR_Plugin_LibHandle_windows_hpp
