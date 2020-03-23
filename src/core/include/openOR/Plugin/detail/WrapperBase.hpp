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

#ifndef openOR_Plugin_detail_WrapperBase_hpp
#define openOR_Plugin_detail_WrapperBase_hpp

#include <boost/tr1/memory.hpp>

#include <openOR/Plugin/detail/Base.hpp>

namespace openOR {
   namespace Plugin {
      namespace Detail {

         //----------------------------------------------------------------------------
         //! \brief Default registration class for a plugin without any registered interface class. 
         //! \internal
         //----------------------------------------------------------------------------
         template<class WrapperBaseType>
         struct Impl2InterfaceCaster {
            static boost::any cast( const std::type_info&, 
                                    const std::tr1::shared_ptr<WrapperBaseType>& )
            {
               return boost::any();
            }
         };

         //---------------------------------------------------------------------------- 
         //! \brief Base class for all wrapper objects.
         //! \internal
         //----------------------------------------------------------------------------
         template<class ImplType_>
         struct WrapperBase : public ImplType_, public Base {

            typedef ImplType_ ImplType;
            typedef WrapperBase<ImplType> WrapperBaseType; 

            WrapperBase() :  
               ImplType() 
            {}

            template<class Param0>
            WrapperBase(Param0 param0) :  
               ImplType(param0) 
            {}

            template<class Param0, class Param1>
            WrapperBase(Param0 param0, Param1 param1) :  
               ImplType(param0, param1) 
            {}

            template<class Param0, class Param1, class Param2>
            WrapperBase(Param0 param0, Param1 param1, Param2 param2) :
               ImplType(param0, param1, param2) 
            {}

            virtual ~WrapperBase() {}

            void setWrapperBasePtr(const std::tr1::shared_ptr<WrapperBaseType>& pWrapperBase) {
               m_pWrapperBase = pWrapperBase;
            }

            virtual std::tr1::shared_ptr<Base> wrapperBasePtr() const {
               return m_pWrapperBase.lock();
            }

            virtual boost::any castToRegisteredInterface(const std::type_info& interfaceTypeInfo) const {
               return openOR::Plugin::Detail::
                           Impl2InterfaceCaster<WrapperBaseType>::
                           cast(interfaceTypeInfo, m_pWrapperBase.lock());
            }

            virtual const bool isWrapper() const { return true; }
            virtual const std::type_info& typeId() const { return typeid(ImplType); }

         private:

            std::tr1::weak_ptr<WrapperBaseType> m_pWrapperBase;
         };
         
      } // end NS
   }
}



#endif
