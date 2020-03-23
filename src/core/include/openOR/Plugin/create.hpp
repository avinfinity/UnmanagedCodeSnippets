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

#ifndef openOR_Plugin_create_hpp
#define openOR_Plugin_create_hpp

#include <boost/tr1/memory.hpp>
#include <boost/call_traits.hpp>

#include <openOR/Utility/is_base_of.hpp>
#include <openOR/Plugin/Registration.hpp>
#include <openOR/Plugin/detail/cast.hpp>
#include <openOR/Plugin/detail/Wrapper.hpp>
#include <openOR/Plugin/detail/reg.hpp>
#include <openOR/Plugin/DisableAutoDelete.hpp>
#include <openOR/Plugin/AnyPtr.hpp>


// This is for use with Qt objects, so that we can handle Qt's memory 
// management transparently. Neither run-/link- nor compile-time 
// dependencies are introduced!
class QObject;

namespace openOR {

   //----------------------------------------------------------------------------
   //! \brief Creation of a new object using the default constructor.
   //! 
   //! Creates a new object of type 'ImplType' and returns a smart pointer of it.
   //! The returned pointer can be casted to any supported interface class using 
   //! function 'interface_cast'.
   //! 
   //! \tparam ImplType type of the object which has to be created
   //!
   //! \warning ImplType should be a polymorphic in order to enable casts with 
   //!          function 'interface_cast', otherwise use function 'createInterfaceOf'
   //! \ingroup openOR_core
   //----------------------------------------------------------------------------
   template<class ImplType>
   std::tr1::shared_ptr<ImplType> createInstanceOf() {
      typedef typename Plugin::Detail::WrapperAssigner<
                                          ImplType, 
                                          Utility::is_base_of<QObject, ImplType>::value
                                       >::WrapperType WrapperType;
      
      WrapperType* pWrapper = new WrapperType();
      std::tr1::shared_ptr<WrapperType> p = std::tr1::shared_ptr<WrapperType>(pWrapper, pWrapper->deleter());
      pWrapper->setWrapperBasePtr(p);
      return p;
   }

   //----------------------------------------------------------------------------
   //! \brief Creation of a new object with one constructor parameter.
   //! 
   //! Creates a new object of type 'ImplType' and returns a smart pointer of it.
   //! The returned pointer can be casted to any supported interface class using 
   //! function 'interface_cast'.
   //! 
   //! \tparam ImplType type of the object which has to be created
   //! \tparam Param0 type of the first constructor parameter
   //! \param param0 first constructor parameter
   //!
   //! \warning ImplType should be a polymorphic in order to enable casts with 
   //!          function 'interface_cast', otherwise use function 'createInterfaceOf'
   //! \ingroup openOR_core
   //----------------------------------------------------------------------------
   template<class ImplType, class Param0>
   std::tr1::shared_ptr<ImplType> createInstanceOf(typename boost::call_traits<Param0>::param_type param0) {
      typedef typename Plugin::Detail::WrapperAssigner<
                                          ImplType, 
                                          Utility::is_base_of<QObject, ImplType>::value
                                       >::WrapperType WrapperType;
      
      WrapperType* pWrapper = new WrapperType(param0);
      std::tr1::shared_ptr<WrapperType> p = std::tr1::shared_ptr<WrapperType>(pWrapper, pWrapper->deleter());
      pWrapper->setWrapperBasePtr(p);
      return p;
   }

   //----------------------------------------------------------------------------
   //! \brief Creation of a new object with two constructor parameter.
   //! 
   //! Creates a new object of type 'ImplType' and returns a smart pointer of it.
   //! The returned pointer can be casted to any supported interface class using 
   //! function 'interface_cast'.
   //! 
   //! \tparam ImplType type of the object which has to be created
   //! \tparam Param0 type of the first constructor parameter
   //! \tparam Param1 type of the second constructor parameter
   //! \param param0 first constructor parameter
   //! \param param1 second constructor parameter
   //!
   //! \warning ImplType should be a polymorphic in order to enable casts with 
   //!          function 'interface_cast', otherwise use function 'createInterfaceOf'
   //! \ingroup openOR_core
   //----------------------------------------------------------------------------
   template<class ImplType, class Param0, class Param1>
   std::tr1::shared_ptr<ImplType> createInstanceOf(typename boost::call_traits<Param0>::param_type param0, 
                                                   typename boost::call_traits<Param1>::param_type param1)
   {
      typedef typename Plugin::Detail::WrapperAssigner<
                                          ImplType, 
                                          Utility::is_base_of<QObject, ImplType>::value
                                       >::WrapperType WrapperType;
      
      WrapperType* pWrapper = new WrapperType(param0, param1);
      std::tr1::shared_ptr<WrapperType> p = std::tr1::shared_ptr<WrapperType>(pWrapper, pWrapper->deleter());
      pWrapper->setWrapperBasePtr(p);
      return p;
   }


   //----------------------------------------------------------------------------
   //! \brief Creation of a new object with two constructor parameter.
   //! 
   //! Creates a new object of type 'ImplType' and returns a smart pointer of it.
   //! The returned pointer can be casted to any supported interface class using 
   //! function 'interface_cast'.
   //! 
   //! \tparam ImplType type of the object which has to be created
   //! \tparam Param0 type of the first constructor parameter
   //! \tparam Param1 type of the second constructor parameter
   //! \tparam Param2 type of the third constructor parameter
   //! \param param0 first constructor parameter
   //! \param param1 second constructor parameter
   //! \param param2 third constructor parameter
   //!
   //! \warning ImplType should be a polymorphic in order to enable casts with 
   //!          function 'interface_cast', otherwise use function 'createInterfaceOf'
   //! \ingroup openOR_core
   //----------------------------------------------------------------------------
   template<class ImplType, class Param0, class Param1, class Param2>
   std::tr1::shared_ptr<ImplType> createInstanceOf(typename boost::call_traits<Param0>::param_type param0, 
                                                   typename boost::call_traits<Param1>::param_type param1,
                                                   typename boost::call_traits<Param2>::param_type param2)
   {
      typedef typename Plugin::Detail::WrapperAssigner<
                                          ImplType, 
                                          Utility::is_base_of<QObject, ImplType>::value
                                       >::WrapperType WrapperType;
      
      WrapperType* pWrapper = new WrapperType(param0, param1, param2);
      std::tr1::shared_ptr<WrapperType> p = std::tr1::shared_ptr<WrapperType>(pWrapper, pWrapper->deleter());
      pWrapper->setWrapperBasePtr(p);
      return p;
   }


//----------------------------------------------------------------------------

   //----------------------------------------------------------------------------
   //! \brief Creation of a new object using the default constructor.
   //! 
   //! Creates a new object of type 'ImplType' and returns a smart pointer of the supported interface 'InterfaceType'.
   //! The returned pointer can be casted to any supported interface class using function 'interface_cast'.
   //! 
   //! \tparam ImplType type of the object which has to be created
   //! \ingroup openOR_core
   //----------------------------------------------------------------------------
   template<class ImplType, class InterfaceType>
   std::tr1::shared_ptr<InterfaceType> createInterfaceOf()
   {
      typedef typename Plugin::Detail::WrapperAssigner<
                                          ImplType, 
                                          Utility::is_base_of<QObject, ImplType>::value
                                       >::WrapperType WrapperType;
      typedef typename WrapperType::WrapperBaseType WrapperBaseType;
      
      WrapperType* pWrapper = new WrapperType();
      std::tr1::shared_ptr<WrapperType> p = std::tr1::shared_ptr<WrapperType>(pWrapper, pWrapper->deleter());
      pWrapper->setWrapperBasePtr(p);
      return Plugin::Detail::Pointer2InterfaceCreator<
                                 WrapperBaseType, 
                                 InterfaceType, 
                                 boost::is_base_of<InterfaceType, WrapperBaseType>::value
                              >::create(p);
   }
   
   

   
   //----------------------------------------------------------------------------
   //! \brief Creation of a new object using the default constructor.
   //! 
   //! Creates a new object of type 'ImplType' and returns a AnyPtr.
   //! The returned AnyPtr can be casted to any supported interface class using 
   //! function 'interface_cast'.
   //! 
   //! \tparam ImplType type of the object which has to be created
   //! \ingroup openOR_core
   //----------------------------------------------------------------------------
   
   
   template<class ImplType>
   Plugin::AnyPtr createPluginInstanceOf() {
      typedef typename Plugin::Detail::WrapperAssigner<
                                          ImplType, 
                                          Utility::is_base_of<QObject, ImplType>::value
                                       >::WrapperType WrapperType;
      typedef typename WrapperType::WrapperBaseType WrapperBaseType;
      
      WrapperType* pWrapper = new WrapperType();
      std::tr1::shared_ptr<WrapperType> p = std::tr1::shared_ptr<WrapperType>(pWrapper, pWrapper->deleter());
      pWrapper->setWrapperBasePtr(p);
      
      return p;
   } 
   
   
} // end NS

#endif