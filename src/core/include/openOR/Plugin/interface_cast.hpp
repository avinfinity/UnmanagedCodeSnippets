//****************************************************************************
// (c) 2008 - 2010 by the openOR Team
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

#ifndef openOR_Plugin_interface_cast_hpp
#define openOR_Plugin_interface_cast_hpp

#include <string>
#include <typeinfo>

#include <boost/tr1/memory.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/type_traits/is_polymorphic.hpp>
#include <boost/type_traits/is_base_of.hpp>

#include <openOR/Plugin/detail/cast.hpp>
#include <openOR/Plugin/exception.hpp>
#include <openOR/Log/Logger.hpp>


namespace openOR {
   namespace Plugin {
    
      //----------------------------------------------------------------------------
      //! \brief Casting of an object to a supported interface.
      //!
      //! The requested interface can be supported via direct inheritance and via registered adapter classes.
      //! 
      //! \tparam InterfaceTo type of the interface class the pointer should be casted to.
      //! \tparam InterfaceFrom type of the input interface 
      //! \param pFrom smart pointer which has to be casted to the 'InterfaceTo' interface class.
      //! \warning The object has to be created using the function 'createInstanceOf'.
      //! \ingroup openOR_core
      //----------------------------------------------------------------------------
      template<class InterfaceTo, class InterfaceFrom>
      std::tr1::shared_ptr<InterfaceTo> interface_cast(const std::tr1::shared_ptr<InterfaceFrom>& pFrom) {
         
         return Detail::Caster<InterfaceTo, InterfaceFrom, boost::is_base_of<InterfaceTo, InterfaceFrom>::value>::cast(pFrom, false);
      }
   
      //----------------------------------------------------------------------------
      //! \brief Casting of an object to a supported interface.
      //!
      //! The requested interface can be supported via direct inheritance and via registered adapter classes.
      //! 
      //! \tparam InterfaceTo type of the interface class the pointer should be casted to.
      //! \tparam InterfaceFrom type of the input interface 
      //! \param pFrom smart pointer which has to be casted to the 'InterfaceTo' interface class.
      //! \warning The object has to be created using the function 'createInstanceOf'.
      //! \throw std::bad_cast in case of failure
      //! \ingroup openOR_core
      //----------------------------------------------------------------------------
      template<class InterfaceTo, class InterfaceFrom>
      std::tr1::shared_ptr<InterfaceTo> try_interface_cast(const std::tr1::shared_ptr<InterfaceFrom>& pFrom) {
         
         return Detail::Caster<InterfaceTo, InterfaceFrom, boost::is_base_of<InterfaceTo, InterfaceFrom>::value>::cast(pFrom, true);
      }

   } // end NS

   using Plugin::interface_cast;
   using Plugin::try_interface_cast;
}

#endif //openOR_Plugin_interface_cast_hpp
