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

#ifndef openOR_Plugin_detail_interfacePtr_hpp
#define openOR_Plugin_detail_interfacePtr_hpp

#include <boost/tr1/memory.hpp>
#include <openOR/Plugin/CreateInterface.hpp>

namespace openOR {
   namespace Plugin {
      namespace Detail {

         //--------------------------------------------------------------------------------
         //! \brief Creator of smart pointer to a new adapter object which fulfills the 
         //! requested interface class.
         //!
         //! \internal
         //! 
         //! \tparam WrapperBaseType type of the wrapper base
         //! \tparam InterfaceType type of the requested interface
         //! \tparam IsWrapperBaseTypeDerivedFromInterface Reports if the WrapperBaseType is derived from InterfaceType
         //--------------------------------------------------------------------------------
         template<class WrapperBaseType, class InterfaceType, bool IsWrapperBaseTypeDerivedFromInterface>
         struct Pointer2InterfaceCreator
         {
            static std::tr1::shared_ptr<InterfaceType> create(const std::tr1::shared_ptr<WrapperBaseType>& pWrapperBase)
            {
               typedef Adapter<typename WrapperBaseType::ImplType, InterfaceType> AdapterType;
               return std::tr1::shared_ptr<InterfaceType>(new AdapterType(pWrapperBase));
            }
         };

         //--------------------------------------------------------------------------------
         //! \brief Creator of smart pointer to the wrapper object which fulfills the 
         //! requested interface class.
         //!
         //! \internal 
         //! 
         //! \tparam WrapperBaseType type of the wrapper base
         //! \tparam InterfaceType type of the requested interface
         //! \tparam IsWrapperBaseTypeDerivedFromInterface Reports if the WrapperBaseType is derived from InterfaceType
         //--------------------------------------------------------------------------------
         template<class WrapperBaseType, class InterfaceType>
         struct Pointer2InterfaceCreator<WrapperBaseType, InterfaceType, true>
         {
            static std::tr1::shared_ptr<InterfaceType> create(const std::tr1::shared_ptr<WrapperBaseType>& pWrapperBase)
            {
               return std::tr1::shared_ptr<InterfaceType>(pWrapperBase);
            }
         };


      } //namespace Detail
   }
}

#endif