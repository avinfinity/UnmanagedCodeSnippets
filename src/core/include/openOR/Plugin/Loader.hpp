//****************************************************************************
// (c) 2008 - 2010 by the openOR Team
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

#ifndef openOR_Plugin_Loader_hpp
#define openOR_Plugin_Loader_hpp

#include <openOR/coreDefs.hpp>
#include <openOR/Log/Logger.hpp>

#include <openOR/Plugin/interface_cast.hpp>
#include <openOR/Plugin/AnyPtr.hpp>
#include <openOR/Plugin/Interface.hpp>

#include <vector>
#include <string>
#include <map>

#include <boost/function.hpp>
#include <boost/tr1/memory.hpp>

// For better debuggability
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

namespace openOR {

   //----------------------------------------------------------------------------
   //! \ingroup Core
   //! \brief Plugin creation, (auto)loading and usage.
   //!
   //----------------------------------------------------------------------------
   namespace Plugin {

      // fwd declarations.
      struct Config;

      //----------------------------------------------------------------------------
      //! \brief Signals a failure in creating an instance from a shared library.
      //!
      //! This exception will be thrown when the requested class has not been
      //! registered with the class loader. This can have two reasons, either
      //! the DLL was not opened with \c Library before, or the DLL doesn't
      //! include the register mechanism for the class. (\sa OPENOR_REGISTER_PLUGIN)
      //----------------------------------------------------------------------------
      struct OPENOR_CORE_API unregistered_plugin_error : public std::runtime_error {
         unregistered_plugin_error(const std::string& name = "") :
               std::runtime_error("Trying to instantiate unregistered plugin: " + name) {
            LOG(Log::Level::Debug, Log::noAttribs, Log::msg("Exception thrown: Trying to instantiate unregistered plugin: " + name));
         }
      };

      //----------------------------------------------------------------------------
      //! \brief Instantiation of Plugin classes.
      //----------------------------------------------------------------------------
      struct OPENOR_CORE_API Loader {

         typedef std::string                          Key_type;
         typedef boost::function < AnyPlugin() >      Factory_type;
         typedef std::map< Key_type, Factory_type >   FactoryMap_type;

         //----------------------------------------------------------------------------
         //! \brief Create an instance of the the class className.
         //!
         //! \param [in] className A string which contains the name of the class
         //!   which is to be instantiated.
         //! \pre className has been registered with \c register_factory()
         //! \exception unregistered_plugin_error Thrown if precondition is not met.
         //! \return A \c boost::any which contains a \c tr1::shared_ptr<className>
         //!   pointing to the newly instantiated object.
         //----------------------------------------------------------------------------
         static AnyPlugin create_instance_of(const Key_type& className);

         //----------------------------------------------------------------------------
         //! \brief Returns the factory function of the the class className.
         //!
         //! You usually want to call create instance directly, this function
         //! is a small optimisation if you want to crate a lot of instances in one go.
         //! \param [in] className A string which contains the name of the class
         //!   for which the factory function should be returned.
         //! \pre className has been registered with \c register_factory()
         //! \exception unregistered_plugin_error Thrown if precondition is not met.
         //! \return A factory function which creates a \c boost::any which contains
         //!   an \c tr1::shared_ptr<className> pointing to the newly instantiated object.
         //----------------------------------------------------------------------------
         static Factory_type class_factory(const Key_type& className);

         //----------------------------------------------------------------------------
         //! \brief Register a class for dynamic instantiation.
         //!
         //! In most cases you want to use the macro OPENOR_REGISTER_PLUGIN( class )
         //! to register a class.
         //!
         //! \param [in] className A string containing the name of the registered class.
         //! \param [in] cfp A function pointer to a factory function which creates
         //!   instances of the class, and returns a \c tr1::shared_ptr to the class
         //!   wrapped in a \c boost::any.
         //!   \note cfp may not be \c NULL or invalid.
         //! \pre className has not been registered before
         //! \return \b true if the class has been registered,
         //!   \b false if the preconditions are violated.
         //----------------------------------------------------------------------------
         static bool register_factory(const Key_type& className, Factory_type cfp);

         //----------------------------------------------------------------------------
         //! \brief Turn on plugin autoloading
         //!
         //! \param [in] config A \c Plugin::Config defining plugin to library name
         //!   resolution and application specific search paths.
         //!
         //! The methods class_factory and create_instance_of can usually only return
         //! factorys/classes of libraries which have been loaded previously. If you
         //! turn on autoloading, the \c Plugin::Config object you provide will guide
         //! the autoloading mechanism to open the plugin library file which contains
         //! the requested class.
         //! \note If autoloading is turned on class_factory and create_instance_of
         //!   might also throw library_open_failure.
         //----------------------------------------------------------------------------
         static void autoload(const Config& config);

         //----------------------------------------------------------------------------
         //! \brief Swich off the plugin autoloading mechanism.
         //!
         //! switches the autoload mechanism off again, as is the default.
         //----------------------------------------------------------------------------
         static void autoload_off();

         //----------------------------------------------------------------------------
         //! \brief Get a list of all currently registered classes.
         //----------------------------------------------------------------------------
         static std::vector< std::string > registered_classes();

      private:

         // TODO: turn this class into a Singleton, so that i can turn
         // these into the member variables they ought to be.
         OPENOR_CORE_LOCAL static FactoryMap_type& factory_map();
         OPENOR_CORE_LOCAL static Config&          config();
         OPENOR_CORE_LOCAL static bool&            use_autoload();
      };

      // NOTE: we could easily add runtime checks like plugin version, autoinstansiation, etc here.

      //----------------------------------------------------------------------------
      //! \def OPENOR_SAFE_CREATE_INSTANCE_OF
      //! \brief Safe instantiation of plugin classes.
      //!
      //! to get a instance of a known type use:
      //! \code
      //! shared_ptr<MyPlugin> myPlugin = OPENOR_SAFE_CREATE_INSTANCE_OF( MyPlugin );
      //! \endcode
      //! this is guaranteed to either return a valid instance of \c MyPlugin or
      //! throw an exception
      //! \note this is equivalent to the slightly more verbose
      //!   createInstanceOf, so if you want to avoid macros ...
      //----------------------------------------------------------------------------
      // Do we want this?
      #define OPENOR_SAFE_CREATE_INSTANCE_OF( NAME )  \
         openOR::Plugin::createInstanceOf<NAME>( #NAME )


      //----------------------------------------------------------------------------
      //! \brief Safe instantiation of plugin classes.
      //!
      //! to get a instance of a known type use:
      //! \code
      //! shared_ptr<MyPlugin> myPlugin = createInstanceOf<MyPlugin>( "MyPlugin" );
      //! \endcode
      //! this is guaranteed to either return a valid instance of \c MyPlugin or
      //! throw an exception
      //----------------------------------------------------------------------------
      template <class PluginInterface_type>
      inline std::tr1::shared_ptr<PluginInterface_type> createInstanceOf(const std::string& className) {
         using namespace openOR::Plugin;
         return try_interface_cast<PluginInterface_type>(Loader::create_instance_of(className));
      }

   }
}

#endif //openOR_Plugin_Loader_hpp
