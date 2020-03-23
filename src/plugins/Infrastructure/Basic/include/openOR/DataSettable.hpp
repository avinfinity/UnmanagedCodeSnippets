//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//! OPENOR_INTERFACE_FILE(Basic)
//****************************************************************************
/**
 * @file
 * @author Christian Winne
 * @ingroup Basic
 */
#ifndef openOR_DataSettable_hpp
#define openOR_DataSettable_hpp

#include <boost/tr1/memory.hpp>

#include <openOR/Plugin/CreateInterface.hpp>
#include <openOR/Plugin/AnyPtr.hpp>
#include <openOR/Callable.hpp>

#include <openOR/Plugin/Interface.hpp> // to be removed

namespace openOR {

   /**
    * \brief Interface for objects which work with data that are provided as AnyPtr
    *
    * The internal identification of given data is normally done in two ways:
    * - If the implementation expects only one object of a certain class type, it should try to cast the data to the needed class type using "interface_cast<Type>(data)"
    * - If more than one object of one class type is needed, a selection using the identification string is possible
    * @ingroup Basic
    */
   struct DataSettable
   {
      /**
       * @param data data object
       * @param name name of the data within a data container (DataPool)
       */
      virtual void setData(const AnyPtr& data, const std::string& name) = 0;
   };


   /**
    *
    */
   struct DataGettable
   {
      virtual AnyPtr getData() const = 0;
   };


   /**
    *
    */
   struct DataAddable
   {
      virtual void add(const AnyPtr& data) = 0;
   };



   /**
    *
    */
   template<class Type>
   struct Settable {
      virtual void set(const std::tr1::shared_ptr<Type>& p) = 0;
   };


   /**
    * @tparam Type Type of object which can be get as a smart pointer 
    */
   template<class Type>
   struct Gettable {
      virtual std::tr1::shared_ptr<Type> get() const = 0;
   };


   /**
    *
    */
   template<class Type>
   struct Addable {
      virtual void add(std::tr1::shared_ptr<Type> p) = 0;
   };



   template<class Type>
   struct DataToSettableLinker : Callable {
      virtual void set(std::tr1::shared_ptr<Type> pData) = 0;
      virtual void setSettable(std::tr1::shared_ptr<Settable<Type> > pSettable) = 0;   
   };


   template<class Type>
   struct Holder {
      typedef Type ContentType;
      typedef std::tr1::shared_ptr<ContentType> ContentPointer;

      virtual std::tr1::shared_ptr<ContentType> get() const = 0;
      virtual void set(std::tr1::shared_ptr<ContentType> p) = 0;
   };



   template<class Type>
   struct Creator {
      virtual std::tr1::shared_ptr<Type> operator()() const = 0;
   };
}



OPENOR_CREATE_INTERFACE(openOR::DataSettable)
   void setData(const AnyPtr& data, const std::string& name) { adaptee()->setData(data, name); }
OPENOR_CREATE_INTERFACE_END


OPENOR_CREATE_INTERFACE(openOR::DataGettable)
   AnyPtr getData() const { return adaptee()->getData(); }
OPENOR_CREATE_INTERFACE_END


OPENOR_CREATE_INTERFACE(openOR::DataAddable)
   void addData(const AnyPtr& data) { adaptee()->addData(data); }
OPENOR_CREATE_INTERFACE_END


OPENOR_CREATE_TEMPLATED_INTERFACE_1(Type, openOR::Settable<Type>)
   void set(const std::tr1::shared_ptr<Type>& p) { adaptee()->set(p); }
OPENOR_CREATE_INTERFACE_END


OPENOR_CREATE_TEMPLATED_INTERFACE_1(Type, openOR::Gettable<Type>)
   std::tr1::shared_ptr<Type> get() const { return adaptee()->get(/*p TODO isn't needed any more*/); }
OPENOR_CREATE_INTERFACE_END


OPENOR_CREATE_TEMPLATED_INTERFACE_1(Type, openOR::Addable<Type>)
   void add(std::tr1::shared_ptr<Type> p) { adaptee()->add(p); }
OPENOR_CREATE_INTERFACE_END



OPENOR_CREATE_TEMPLATED_INTERFACE_1(Type, openOR::Holder<Type>)
  std::tr1::shared_ptr<Type> get() const { return adaptee()->get(); }
  void set(std::tr1::shared_ptr<Type> p) { adaptee()->set(p); }
OPENOR_CREATE_INTERFACE_END


OPENOR_CREATE_TEMPLATED_INTERFACE_1(Type, openOR::DataToSettableLinker<Type>)
  void operator()() const { adaptee()->operator()(); }
  void set(std::tr1::shared_ptr<Type> pData) { adaptee()->set(pData); }
  void setSettable(std::tr1::shared_ptr<Settable<Type> > pSettable) { adaptee()->setSettable(pSettable); }   
OPENOR_CREATE_INTERFACE_END


OPENOR_CREATE_TEMPLATED_INTERFACE_1(Type, openOR::Creator<Type>)
   std::tr1::shared_ptr<Type> operator()() { return adaptee()->operator()(); }
OPENOR_CREATE_INTERFACE_END



#include <openOR/Plugin/interface_cast.hpp>

namespace openOR {

   // Convenience Helper
   // TODO: can we do anything sensible with AnyPlugin as Data?
   // TODO: the same for add?

   template<class Type>
   void set(const AnyPtr& object, const std::tr1::shared_ptr<Type>& pData) {
      try_interface_cast< Settable<Type> >(object)->set(pData);
   }


   template<class ObjType, class DataType>
   void set( const std::tr1::shared_ptr< ObjType >& pSettable, const std::tr1::shared_ptr<DataType>& pData ) {
      try_interface_cast<std::tr1::shared_ptr< Settable<DataType> > >( pSettable )->set( pData );
   }
}


#endif