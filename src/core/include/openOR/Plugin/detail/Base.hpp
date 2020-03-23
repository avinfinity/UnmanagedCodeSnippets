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

#ifndef openOR_Plugin_detail_Base_hpp
#define openOR_Plugin_detail_Base_hpp

#include <typeinfo>

#include <boost/any.hpp>
#include <boost/tr1/memory.hpp>

namespace openOR {
   namespace Plugin {
      namespace Detail {

         //----------------------------------------------------------------------------
         //! \brief Common base class for wrapper and adapter classes.
         //! \internal
         //----------------------------------------------------------------------------
         struct Base {
            
            //----------------------------------------------------------------------------
            //! \brief Used to get the corresponding wrapper object of an adapter object
            //! \return smart pointer to the base interface of the wrapper object
            //----------------------------------------------------------------------------
            virtual std::tr1::shared_ptr<Base> wrapperBasePtr() const = 0;

            //---------------------------------------------------------------------------- 
            //! \brief Used to cast a wrapper object to a registered interface (use of adapter classes)
            //! \param interfaceTypeInfo type info of the destination interface type
            //! \return interface smart pointer wrapped in a boost::any object if successful
            //----------------------------------------------------------------------------
            virtual boost::any castToRegisteredInterface(const std::type_info& interfaceTypeInfo) const = 0;

            //----------------------------------------------------------------------------
            //! \brief Enables the distinction between wrapper and adapter objects which all support this Base interface
            //----------------------------------------------------------------------------
            virtual const bool isWrapper() const = 0;

            //----------------------------------------------------------------------------
            //! \brief Returns the typeinfo of the Plugin type of the wrapper or adapter object
            //----------------------------------------------------------------------------
            virtual const std::type_info& typeId() const = 0;


         };

      } // end NS
   }
}

#endif