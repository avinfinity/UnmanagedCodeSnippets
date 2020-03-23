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

#ifndef openOR_Plugin_detail_AdapterBase_hpp
#define openOR_Plugin_detail_AdapterBase_hpp

#include <openOR/Plugin/detail/Base.hpp>
#include <openOR/Plugin/detail/WrapperBase.hpp>

namespace openOR {
   namespace Plugin {
      namespace Detail {

         template<class ImplType, class InterfaceType>
         struct AdapterBase : public Base {

            typedef WrapperBase<ImplType> WrapperBaseType;
                                                                                                                  
            AdapterBase(const std::tr1::shared_ptr<WrapperBaseType>& pWrapperBase) :
               m_pAdaptee(pWrapperBase)  
            {}

            virtual ~AdapterBase() {}

            virtual std::tr1::shared_ptr<Detail::Base> wrapperBasePtr() const { return m_pAdaptee; }
         
            virtual boost::any castToRegisteredInterface(const std::type_info& interfaceTypeInfo) const {
               return m_pAdaptee->castToRegisteredInterface(interfaceTypeInfo);
            }

            virtual const bool            isWrapper() const { return false; }
            virtual const std::type_info& typeId()    const { return typeid(ImplType); }

         protected:
            const std::tr1::shared_ptr<WrapperBaseType> m_pAdaptee;

         private:
            // To avoid MSVC warning C4512 explicitly define (private) assignment operator for this class.
            AdapterBase& operator=(const AdapterBase& tmp);
         };
         
      } // end NS
   }
}


#endif