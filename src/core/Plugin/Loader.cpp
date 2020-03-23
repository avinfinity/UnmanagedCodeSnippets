//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR commercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#include "../include/openOR/Plugin/Loader.hpp"

#include <openOR/Plugin/Config.hpp>
#include "autoOpenLibrary.hpp"

#include "../include/openOR/Log/Logger.hpp"
#   define OPENOR_MODULE_NAME "Core.Plugin.Loader"
#   include "../include/openOR/Log/ModuleFilter.hpp"

#include <iostream>

namespace openOR {
   namespace Plugin {

      //--------------------------------------------------------------------------------

      AnyPlugin Loader::create_instance_of(const Key_type& className) {

         return class_factory(className)();
      }

      //--------------------------------------------------------------------------------

      Loader::Factory_type Loader::class_factory(const Key_type& className) {

         FactoryMap_type::const_iterator fac = factory_map().find(className);
         if (use_autoload() && (fac == factory_map().end())) {
            // if autoloading is used, try to open the library which contains className.
            auto_open_library(config(), className);
            fac = factory_map().find(className);   // it should be there now.
         }

         if (fac == factory_map().end()) {
            throw unregistered_plugin_error(className);
         }

         return fac->second;
      }

      //--------------------------------------------------------------------------------

      bool Loader::register_factory(const Key_type& className, Factory_type cfp) {
         assert(cfp != NULL && "Cannot set empty factory pointer!");
         assert((factory_map().find(className) == factory_map().end())
                && "Multiple registration of a factory!"
               );

         if (!cfp) {
            // this should never happen, if it does it is a bug in the calling code!
            // (hence the assert above.) This additional check is to prevent failures
            // in release builds, where brocken plugins could crash the application.
            LOG(Log::Level::Warning, OPENOR_MODULE,
                Log::msg("Tried to register Plugin '%1%' without factory.") % className
               );
            return false;
         }

         if ((factory_map().find(className) != factory_map().end())) {
            // this should never happen, if it does it is a bug in the calling code!
            // (hence the assert above.) This additional check is to prevent
            // code injections in already running applications.
            LOG(Log::Level::Warning, OPENOR_MODULE,
                Log::msg("Tried to re-register existing Plugin '%1%'.") % className
               );
            return false;
         }

         factory_map()[className] = cfp;
         LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("Registered Plugin Class '%1%'.") % className);

         return true;
      }

      void Loader::autoload(const Config& config) {
         Loader::config() = config;
         use_autoload() = true;

      }

      void Loader::autoload_off() {
         use_autoload() = false;
      }

      std::vector<std::string> Loader::registered_classes() {

         std::vector<std::string> classes;

         for (FactoryMap_type::const_iterator   fac = factory_map().begin(),
               end = factory_map().end();
               fac != end; ++fac) {
            classes.push_back(fac->first);
         }

         return classes;
      }

      //--------------------------------------------------------------------------------

      Config& Loader::config() {
         static Config staticConfig;
         return staticConfig;
      }

      bool& Loader::use_autoload() {
         static bool staticUseAutoload = false;
         return staticUseAutoload;
      }

      //--------------------------------------------------------------------------------

      Loader::FactoryMap_type& Loader::factory_map() {
         static FactoryMap_type staticMap;
         return staticMap;
      }

      //--------------------------------------------------------------------------------

   }
}

//--------------------------------------------------------------------------------
