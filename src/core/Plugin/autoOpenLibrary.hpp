//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR commercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#ifndef openOR_Plugin_AutoOpenLibrary_hpp
#define openOR_Plugin_AutoOpenLibrary_hpp

#include "../include/openOR/Plugin/Library.hpp"
#include "../include/openOR/coreDefs.hpp"

#include <boost/tr1/memory.hpp>

namespace openOR {
   namespace Plugin {

      // fwd declarations
      struct Config;

      //----------------------------------------------------------------------------
      //! \brief Tries to open the Library according to the Plugin::Config
      //!
      //! \param [in] config The Plugin::Config used to find the library.
      //! \param [in] pluginName The name of the PluginClass.
      //! \param [in] unloadableLib If set to true Library unloding will be
      //!   enabled. \sa Library.
      //! \returns A Library handle.
      //! \exception library_open_failure will be thrown if no matching library
      //!   could be found for the plugin.
      //----------------------------------------------------------------------------
      Library OPENOR_CORE_API auto_open_library(const Config& config,
                                                const std::string& pluginName,
                                                bool unloadableLib = false
                                               );

   }
}

#endif //openOR_Plugin_AutoOpenLibrary_hpp

