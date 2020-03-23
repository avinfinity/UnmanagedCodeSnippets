//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//! OPENOR_INTERFACE_FILE(openOR_core)
//****************************************************************************
//! @file
//! @ingroup openOR_core

#ifndef openOR_Plugin_Library_hpp
#define openOR_Plugin_Library_hpp

#include <string>
#include <stdexcept>

#include <boost/utility.hpp>
#include <boost/tr1/memory.hpp>

#include "../coreDefs.hpp"
#include "../Log/Logger.hpp"

namespace openOR {
   namespace Plugin {

      // Forward Decl
      namespace Detail { struct LibHandle; }

      //----------------------------------------------------------------------------
      //! \brief Signals a failure in Loading a shared library.
      //!
      //! The most probable reasons for this is that the file isn't a
      //! DLL or that it doesn't exist at all.
      //!
      //----------------------------------------------------------------------------
      struct OPENOR_CORE_API library_open_failure : public std::runtime_error {
         library_open_failure(const std::string& name = "") :
               std::runtime_error("Failed to open library: " + name) {
            LOG(Log::Level::Debug, Log::noAttribs, Log::msg("Exception thrown: Failed to open library: " + name));
         }
      };

      //----------------------------------------------------------------------------
      //! \brief Open a dynamic shared object from a file.
      //!
      //! Each instanciation of this class will hold a refcounted handle to
      //! one DLL object. If the parameter unload is set (\sa Library(string, bool))
      //! the DLL will automatically be unloaded when the last refering object gets
      //! deleted (on platforms that support this).
      //! Currently unloading is not implemented anyway.
      //!
      //! \note If you use any function from an unloaded DLL the result is undefined,
      //!   i.e. it will most likely crash.
      //!
      //----------------------------------------------------------------------------
      struct OPENOR_CORE_API Library {

         Library();

         //----------------------------------------------------------------------------
         //! \brief Open a dynamic shared object with the given filename.
         //!
         //! \param[in] fileName The base filename of the DLL to load.
         //!   \warning <b>DO NOT</b> specify the extention of the library,
         //!      it will be portably appended for you according to the OS's preferences
         //!   \note Unless you specify an absolute path search path for loading
         //!      DLLs is different depending on the underlyin OS's behaviour.
         //! \param[in] unload Toggles automatical unload of the DLL.
         //!   (<b>Default = false</b>)
         //!   \note Use this feature only if you know what you are doing.
         //!      You need to make sure that at least one copy of the Library
         //!      object for a specific library exists while anybody uses any feature
         //!      of the library.
         //! \exception library_open_failure Will be thrown if the library could not
         //!   be loaded.
         //----------------------------------------------------------------------------
         Library(const std::string& fileName, bool unload = false);

      private:
         std::tr1::shared_ptr<Detail::LibHandle> m_handle;
      };

      namespace Detail {

         //----------------------------------------------------------------------------
         //! \brief Dynamic DLL Handle (Platform independent abstraction)
         //----------------------------------------------------------------------------
         struct LibHandle : boost::noncopyable {
            virtual ~LibHandle() = 0;

            static std::tr1::shared_ptr<LibHandle> create(const std::string& fileName, bool unload = false);
         protected:
            LibHandle(); // cannot be default constructed!
         };

         //----------------------------------------------------------------------------
         //! \brief Retrurns a library name.
         //!
         //! \param fileName
         //! if \c is_potential_library(fileName) == true \c , this function will
         //! return the libraries name without any platform specific embellishments.
         //! Library needs this kind of library name as a parameter.
         //----------------------------------------------------------------------------
         std::string library_name(const std::string& fileName);

         //----------------------------------------------------------------------------
         //! \brief true if the fileName can be a library.
         //----------------------------------------------------------------------------
         bool is_potential_library(const std::string& fileName);

         //----------------------------------------------------------------------------
         //! \brief the platform (+config) dependent library extention.
         //----------------------------------------------------------------------------
         std::string platform_library_extension();

         //----------------------------------------------------------------------------
         //! \brief the platform (+config) dependent library prefix. (might be empty)
         //----------------------------------------------------------------------------
         std::string platform_library_prefix();

         //----------------------------------------------------------------------------
         //! \brief Scans a DLL if it contains openOR objects
         //! \warning this loads the library and registers its objects.
         //! DO NOT USE UNLESS YOU UNDERSTAND ALL IMPLICATIONS THIS BRINGS.
         //----------------------------------------------------------------------------
         bool scan_file_for_library(const std::string& fullfileName);
      }
   }
}

#endif // openOR_Plugin_Library_hpp
