//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR commercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//! OPENOR_INTERFACE_FILE(openOR_core)
//****************************************************************************
//! @file
//! @ingroup openOR_core

#ifndef openOR_Plugin_Registration_hpp
#define openOR_Plugin_Registration_hpp

#include <boost/mpl/vector.hpp>
#include <boost/parameter/aux_/parenthesized_type.hpp>
#include <boost/preprocessor/seq/enum.hpp>

#include <openOR/Plugin/Interface.hpp>
#include <openOR/Plugin/Loader.hpp>
#include <openOR/Plugin/CreateInterface.hpp>
#include <openOR/Plugin/create.hpp>
#include <openOR/Plugin/detail/WrapperBase.hpp>
#include <openOR/Plugin/detail/reg.hpp>
#include <openOR/Log/Logger.hpp>
#   define OPENOR_MODULE_NAME "Core.Tracing.Plugin"
#   include <openOR/Log/ModuleFilter.hpp>

//----------------------------------------------------------------------------
//! \def OPENOR_REGISTER_PLUGIN
//! \brief Register a class for dynamic instantiation.
//!
//! Use this macro to make a class usable as a plugin.
//! \ingroup openOR_core
//----------------------------------------------------------------------------
#define OPENOR_REGISTER_PLUGIN( NAME )                                                             \
   namespace {                                                                                     \
      template <> struct openOR_Plugin_Detail<NAME> {                                              \
         static bool is_registered;                                                                \
      };                                                                                           \
      bool openOR_Plugin_Detail<NAME>::is_registered =                                             \
            openOR::Plugin::Loader::register_factory(#NAME, &openOR::createPluginInstanceOf<NAME>);\
   }

//----------------------------------------------------------------------------
//! \def OPENOR_REGISTER_PLUGIN_ALIAS
//!
//! \brief Register an alias for an existing Plugin.
//! \ingroup openOR_core
//----------------------------------------------------------------------------
#define OPENOR_REGISTER_PLUGIN_ALIAS( NAME, ALIAS )                                                \
   namespace {                                                                                     \
      static bool isAlias_##ALIAS =  (!openOR_Plugin_Detail<NAME>::is_registered) ? false :        \
                                     openOR::Plugin::Loader::register_factory( #ALIAS,             \
                                        openOR::Plugin::Loader::class_factory(#NAME) );            \
   }


//----------------------------------------------------------------------------
//! \brief Registering one or more interface types for a plugin type
//! Usage: OPENOR_REGISTER_INTERFACES ( (MyClass), (Interface1) (Interface2) ... ) )
//! \note: There are NO commas between the Interfaces! 
//! \ingroup openOR_core
//----------------------------------------------------------------------------
#define OPENOR_REGISTER_INTERFACES(IMPL_TYPE, INTERFACES )                                                              \
   namespace openOR { namespace Plugin { namespace Detail {                                                             \
   template<>                                                                                                           \
   struct Impl2InterfaceCaster<openOR::Plugin::Detail::WrapperBase< BOOST_PARAMETER_PARENTHESIZED_TYPE(IMPL_TYPE) > >   \
      :   Impl2InterfaceCastBase<openOR::Plugin::Detail::WrapperBase< BOOST_PARAMETER_PARENTHESIZED_TYPE(IMPL_TYPE) >,  \
             boost::mpl::vector< BOOST_PP_SEQ_ENUM(INTERFACES) > >                                                      \
   {};                                                                                                                  \
   } } }

//----------------------------------------------------------------------------
//! \brief Registering exactly one interface types for a plugin type
//! Usage: OPENOR_REGISTER_PLUGIN_INTERFACE ( MyClass, Interface1 )
//! \deprecated
//! \ingroup openOR_core
//----------------------------------------------------------------------------
#define OPENOR_REGISTER_PLUGIN_INTERFACE( PLUGIN, INTERFACE ) OPENOR_REGISTER_INTERFACES( (PLUGIN), (INTERFACE) )

#define OPENOR_COMMENT_PLUGIN_BEGIN(PLUGIN_NAME) namespace openOR {}
#define OPENOR_COMMENT_PLUGIN_SIGNAL(SIGNAL_NAME, SIGNATURE, DESCRIPTION) namespace openOR {}
#define OPENOR_COMMENT_PLUGIN_PROPERTY(PROPERTY_NAME, TYPE, SET, GET, LOAD, SAVE, INITIAL, DESCRIPTION) namespace openOR {}
#define OPENOR_COMMENT_PLUGIN_END namespace openOR {}


namespace {

   //----------------------------------------------------------------------------
   //! \brief INTERNAL Automatic plugin Registration backend
   //! \internal
   //!
   //! used in the OPENOR_REGISTER_* macros
   //----------------------------------------------------------------------------
   // The auto-registration of plugins exploits the static initialisation of
   // (unused) variables which is the only portable facility that guarantees
   // code execution on plugin load.
   // Owing to the ODR each plugin needs a variable with a unique name.
   // This template class is used as a Tag to guarantee the uniqueness of the name.
   // GOTCHA: static initialisation rules (see section 3.6.2 of the ISO C++ standard)
   // are complex, and contain a gotcha which is quite relevant here.
   // If we were to use implicit template instantiation the compiler would be free
   // to postpone the initialisation of the variables until they are used (i.e. never),
   // which in this case inhibits plugin registration.
   template < class Plugin_type, class Interface_type = Plugin_type > struct openOR_Plugin_Detail {};
}



#undef OPENOR_MODULE_NAME
 
#endif //openOR_Plugin_Registration_hpp
