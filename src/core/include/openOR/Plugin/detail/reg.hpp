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

#ifndef openOR_Plugin_detail_reg_hpp
#define openOR_Plugin_detail_reg_hpp

#include <boost/tr1/memory.hpp>

#include <boost/type_traits/is_base_of.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/size.hpp>

#include <openOR/Plugin/detail/Base.hpp>
#include <openOR/Plugin/detail/interfacePtr.hpp>

      
namespace openOR {
   namespace Plugin {
      namespace Detail {

         //--------------------------------------------------------------------------------
         //! \brief Casting of a wrapper object toward a registered interface.
         //! \internal
         //! \tparam _WrapperBaseType type of the wrapper base of the concrete implementation object
         //! \tparam _InterfaceSequence boost::mpl::vector type sequence of supported interface classes.
         //--------------------------------------------------------------------------------
         template<class _WrapperBaseType, class _InterfaceSequence>
         struct Impl2InterfaceCastBase
         {
            typedef _WrapperBaseType WrapperBaseType;
            typedef typename _WrapperBaseType::ImplType ImplType;
            typedef _InterfaceSequence InterfaceSequence;

            //--------------------------------------------------------------------------------
            //! \brief Casting of a wrapperbase-object to a registered interface class.
            //! \internal
            //! \param interfaceTypeInfo type info of the requested interface class
            //! \param pWrapperBase smart pointer to a wrapper object 
            //! \return smart pointer to an adapter object fulfilling the requested interface encapsulated in a boost::any object
            //--------------------------------------------------------------------------------
            static boost::any cast(const std::type_info& interfaceTypeInfo, const std::tr1::shared_ptr<WrapperBaseType>& pWrapperBase)
            {
               if (!pWrapperBase)
                  return boost::any();
               return CastHelper<InterfaceSequence, boost::mpl::size<InterfaceSequence>::value>::cast(interfaceTypeInfo, pWrapperBase);
            }

         protected:
            
            //--------------------------------------------------------------------------------
            //! \brief Recursive iteration of registered interface sequence (boost::mpl::vector)
            //! \pre boost::mpl::size<InterfaceSeq>::value > 0
            //! \internal
            //--------------------------------------------------------------------------------
            template<class InterfaceSeq, int>
            struct CastHelper
            {
               typedef typename boost::mpl::pop_front<InterfaceSeq>::type RestSeq;
               typedef typename boost::mpl::front<InterfaceSeq>::type InterfaceType;

               static boost::any cast(const std::type_info& interfaceTypeInfo, const std::tr1::shared_ptr<WrapperBaseType>& pWrapperBase)
               {
                  if (typeid(InterfaceType) == interfaceTypeInfo)
                     return Detail::Pointer2InterfaceCreator<WrapperBaseType, InterfaceType, boost::is_base_of<InterfaceType, WrapperBaseType>::value>::create(pWrapperBase);
                  else
                     return CastHelper<RestSeq, boost::mpl::size<RestSeq>::value>::cast(interfaceTypeInfo, pWrapperBase);
               }

            };

            //--------------------------------------------------------------------------------
            //! \brief Recursive iteration of registered interface sequence (boost::mpl::vector)
            //! \pre boost::mpl::size<InterfaceSeq>::value == 0
            //! \internal
            //--------------------------------------------------------------------------------
            template<class InterfaceSeq>
            struct CastHelper<InterfaceSeq, 0>
            {
               static boost::any cast(const std::type_info&, const std::tr1::shared_ptr<WrapperBaseType>&)
               {
                  return boost::any();
               }
            };

         };

      } //namespace Detail
   } 
}

#endif
