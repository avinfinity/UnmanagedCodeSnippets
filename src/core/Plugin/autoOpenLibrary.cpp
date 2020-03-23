//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR commercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#include "autoOpenLibrary.hpp"

#include "../include/openOR/Plugin/Config.hpp"

namespace openOR {
   namespace Plugin {

      Library auto_open_library(const Config& config,
                                const std::string& pluginName,
                                bool unloadableLib /* = false */
                               ) {
         Library result;
         bool found = false;

         if (!config.full_library_path(pluginName).empty()) {
            // The config tells us the full name of the library (w/o extension!)
            // so we search it here. if we are unsuccessful the exception will
            // be passed on to the user.
            result = Library(config.full_library_path(pluginName).string());
            found = true;
         }

         // Search the user defined plugin Library search path.
         for (Config::SearchPath::const_iterator   sp = config.search_path().begin(),
               spEnd = config.search_path().end();
               ((sp != spEnd) && !found); ++sp) {
            // This might seem a bit tricky, since we rely on exceptions for control flow.
            // But since Library throws on failure (which is quite sensible in general) we
            // need to deal with it here.
            try {
               result = Library((*sp / config.library_base_name(pluginName)).string());
               // this will be set only if the Loading of the library worked,
               // since if it doesn't an exception is thrown.
               found = true;
            } catch (const library_open_failure&) {
               // Nothing to do here, we just try on
            }
         }

         if (!found) {
            // The user defined search path didn't contain the library ...
            if (config.search_system_path()) {
               // ... so we let the OS search at the usual places.
               result = Library(config.library_base_name(pluginName));
               // NOTE: if this throws we can't autoload, so we let the exception pass, to inform the user.
            } else {
               // ... and the user doesn't want us to search the system pathes, so no joy!
               throw library_open_failure("Could not find library for plugin " + pluginName + ".");
            }
         }

         return result;
      }

   }
}

