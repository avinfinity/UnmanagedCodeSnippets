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

#ifndef openOR_Plugin_CreateInterface_hpp
#define openOR_Plugin_CreateInterface_hpp

#include <stdexcept>
#include <openOR/Plugin/detail/Base.hpp>
#include <openOR/Plugin/detail/AdapterBase.hpp>

namespace openOR {
   namespace Plugin {
      namespace Detail {
      
         //! \internal
         template <class PluginType, class InterfaceType>
         struct Adapter : private std::runtime_error {
            // No default implementation. This may not be used without a specialization.
         };
         
      }
   }
}

//--------------------------------------------------------------------------------
//! \brief Default adapter class definition for the interface class 'INTERFACE'
//--------------------------------------------------------------------------------
#define OPENOR_CREATE_INTERFACE(INTERFACE)                                             \
   namespace openOR { namespace Plugin { namespace Detail {                            \
   template <class PluginType> OPENOR_CREATE_INTERFACE_INTERNAL(INTERFACE, PluginType) 

//--------------------------------------------------------------------------------
//! \brief Default adapter class definition for the interface class 'INTERFACE' 
//! with one template parameter 'TPARAM1'
//--------------------------------------------------------------------------------
#define OPENOR_CREATE_TEMPLATED_INTERFACE_1(TPARAM1, INTERFACE)                        \
   namespace openOR { namespace Plugin { namespace Detail {                            \
   template <class TPARAM1, class PluginType>                                          \
   OPENOR_CREATE_INTERFACE_INTERNAL(INTERFACE, PluginType)

//--------------------------------------------------------------------------------
//! \brief Default adapter class definition for the interface class 'INTERFACE' 
//! with two template parameter 'TPARAM1' and 'TPARAM2'
//--------------------------------------------------------------------------------
#define OPENOR_CREATE_TEMPLATED_INTERFACE_2(TPARAM1, TPARAM2, INTERFACE)               \
   template <class TPARAM1, class TPARAM2, class _PluginType>                          \
   OPENOR_CREATE_INTERFACE_INTERNAL(INTERFACE, _PluginType)

//--------------------------------------------------------------------------------
//! \brief Specialized adapter class definition for the interface class 'INTERFACE' 
//! for the plugin type 'PLUGIN'
//--------------------------------------------------------------------------------
#define OPENOR_ADAPT_INTERFACE(INTERFACE, PLUGIN)                                      \
   namespace openOR { namespace Plugin { namespace Detail {                            \
   template <> OPENOR_CREATE_INTERFACE_INTERNAL(INTERFACE, PLUGIN)

//--------------------------------------------------------------------------------
//! \brief General (default and specialized) adapter class definition.
//! \internal
//--------------------------------------------------------------------------------
#define OPENOR_CREATE_INTERFACE_INTERNAL(INTERFACE, PLUGIN)                            \
   struct Adapter< PLUGIN, INTERFACE > :                                               \
      public INTERFACE,                                                                \
      public openOR::Plugin::Detail::AdapterBase< PLUGIN, INTERFACE >                  \
   {                                                                                   \
      Adapter(const std::tr1::shared_ptr<WrapperBase< PLUGIN > >& pWrapperBase) :      \
         openOR::Plugin::Detail::AdapterBase< PLUGIN, INTERFACE >(pWrapperBase)        \
      {}                                                                               \
                                                                                       \
      virtual ~Adapter() {}                                                            \
      std::tr1::shared_ptr< PLUGIN > adaptee() const {                                 \
         return openOR::Plugin::Detail::AdapterBase< PLUGIN, INTERFACE >::m_pAdaptee;  \
      }

//----------------------------------------------------------------------------
//! \def OPENOR_CREATE_INTERFACE_END
//! \brief End of a default interface adapter
//----------------------------------------------------------------------------
#define OPENOR_CREATE_INTERFACE_END }; } } }

//----------------------------------------------------------------------------
//! \def OPENOR_ADAPT_INTERFACE_END
//! \brief End of a specialized interface adapter
//----------------------------------------------------------------------------
#define OPENOR_ADAPT_INTERFACE_END }; } } }

#endif //openOR_Plugin_CreateInterface_hpp

