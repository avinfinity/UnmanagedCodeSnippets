//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR commercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#include "../include/openOR/Plugin/Library.hpp"
#include "../include/openOR/Log/Logger.hpp"

#include <boost/config.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tr1/memory.hpp>

#ifndef NDEBUG
   #define OPENOR_PRIVATE_DLL_MODE_APPENDIX "d"
#endif

namespace openOR {
   namespace Plugin {

      Library::Library() : m_handle(std::tr1::shared_ptr<Detail::LibHandle>()) {}
      Library::Library(const std::string& fileName, bool unload /* = false */) :
            m_handle(Detail::LibHandle::create(fileName, unload)) {}

   }
}

#ifdef BOOST_WINDOWS
   #include "LibHandle_windows.hpp"
#else
   // for Unix (Linux and Mac OS X)
   #include "LibHandle_dlopen.hpp"
#endif

namespace openOR {
   namespace Plugin {
      namespace Detail {

         std::tr1::shared_ptr<LibHandle> LibHandle::create(const std::string& fileName, bool unload /* = false */) {

            #ifdef BOOST_WINDOWS
               return std::tr1::shared_ptr<Detail::LibHandle>(new LibHandle_windows(fileName, unload));
            #else
               return std::tr1::shared_ptr<Detail::LibHandle>(new LibHandle_dlopen(fileName, unload));
            #endif
         }

         LibHandle::LibHandle() {}
         LibHandle::~LibHandle() {}

         std::string library_name(const std::string& fileName) {

            typedef boost::iterator_range<std::string::const_iterator> strT;

            strT end = (boost::starts_with(fileName, platform_library_prefix()))
                       ? boost::find_tail(fileName, fileName.length() - platform_library_prefix().length())
                       : fileName;

            strT lname = (boost::ends_with(end, platform_library_extension()))
                         ? boost::find_head(end, end.size() - platform_library_extension().length())
                         : end;

            return std::string(lname.begin(), lname.end());
         }

         bool is_potential_library(const std::string& fileName) {
            return boost::starts_with(fileName, platform_library_prefix()) && boost::ends_with(fileName, platform_library_extension());
         }


         bool scan_file_for_library(const std::string& fullfileName) {
            #ifdef BOOST_WINDOWS
               return scan_file_for_library_windows(fullfileName);
            #else
               return scan_file_for_library_dlopen(fullfileName);
            #endif
         }

         std::string platform_library_extension() {
            #ifdef BOOST_WINDOWS
               return platform_library_extension_windows();
            #else
               return platform_library_extension_dlopen();
            #endif
         }

         std::string platform_library_prefix() {
            #ifdef BOOST_WINDOWS
               return platform_library_prefix_windows();
            #else
               return platform_library_prefix_dlopen();
            #endif
         }
      }
   }
}

#undef OPENOR_PRIVATE_DEBUG_DLL_APPENDIX

