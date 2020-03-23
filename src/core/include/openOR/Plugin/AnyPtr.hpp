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
//! @file
//! @ingroup openOR_core
//****************************************************************************

#ifndef openOR_Plugin_AnyPtr_hpp
#define openOR_Plugin_AnyPtr_hpp

#include <openOR/Plugin/detail/Base.hpp>
#include <openOR/Plugin/detail/AnyPtrT.hpp>
#include <openOR/Plugin/detail/cast.hpp>


namespace openOR {
   namespace Plugin {

      //----------------------------------------------------------------------------
      //! \brief Encapsulation of an smart pointer of an object.
      //!
      //! This class is used to commit object pointer to a function 
      //! independent from the concrete object type and the supported interfaces.
      //!
      //! The encapsulated object have to be created using 'createInstanceOf' or 'createInterfaceOf'.
      //----------------------------------------------------------------------------
      typedef Detail::AnyPtrT<Detail::Base>  AnyPtr;
      typedef Detail::OrderingAnyPtrT<Detail::Base>  OrderingAnyPtr;
      
      //----------------------------------------------------------------------------
      //! \brief Casting of an AnyPtr object to a supported interface.
      //!
      //! The requested interface can be supported via direct inheritance and via registered adapter classes.
      //! 
      //! \tparam InterfaceTo type of the interface class the pointer should be casted to.
      //! \param anyPtr anyPtr-object which encapsulates a smart pointer which has to be casted to the 'InterfaceTo' interface class.
      //! \warning The object encapsulated in 'anyPtr' has to be created using the function 'createInstanceOf' or 'createInterfaceOf'.
      //----------------------------------------------------------------------------
      template<class InterfaceTo>
      std::tr1::shared_ptr<InterfaceTo> interface_cast(const AnyPtr& anyPtr) {
         
         std::tr1::shared_ptr<Detail::Base> pBase = anyPtr.asBase<InterfaceTo>();
         return Detail::cast<InterfaceTo>(pBase, false);
      }


      //----------------------------------------------------------------------------
      //! \brief Casting of an AnyPtr object to a supported interface.
      //!
      //! The requested interface can be supported via direct inheritance and via registered adapter classes.
      //! 
      //! \tparam InterfaceTo type of the interface class the pointer should be casted to.
      //! \param anyPtr anyPtr-object which encapsulates a smart pointer which has to be casted to the 'InterfaceTo' interface class.
      //! \warning The object encapsulated in 'anyPtr' has to be created using the function 'createInstanceOf' or 'createInterfaceOf'.
      //! \throw std::bad_cast in case of failure
      //! \ingroup openOR_core
      //----------------------------------------------------------------------------
      template<class InterfaceTo>
      std::tr1::shared_ptr<InterfaceTo> try_interface_cast(const AnyPtr& anyPtr) {
         
         std::tr1::shared_ptr<Detail::Base> pBase = anyPtr.asBase<InterfaceTo>();
         if (!pBase) {
            LOG(Log::Level::Debug, Log::noAttribs, 
                  Log::msg("openOR::interface_cast error: No valid object pointer in AnyPtr for casting to interface '%1%") 
                  % typeid(InterfaceTo).name()
               );
            throw std::bad_cast();
         }
         return Detail::cast<InterfaceTo>(pBase, true);
      }

   } // end NS

   using Plugin::AnyPtr;
   using Plugin::OrderingAnyPtr;
   using Plugin::interface_cast;
   using Plugin::try_interface_cast;
}

#endif