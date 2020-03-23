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
#ifndef openOR_PropertyAccess_hpp
#define openOR_PropertyAccess_hpp

#include <openOR/Plugin/CreateInterface.hpp>
#include <boost/property_tree/ptree.hpp>

namespace openOR {
   
   /**
    * \brief interface for loading and saving of the property data
    * @ingroup Basic
    */
   struct Configurable
   {
      virtual void load(const boost::property_tree::ptree& properties) = 0;

      virtual void save(boost::property_tree::ptree& properties) const = 0;
   };


   /**
    * \brief interface for access of the property data of a plugin
    * @ingroup Basic
    */
   struct PropertyAccess : public Configurable
   {

      template<class Type>
      Type get(const std::string& name, const Type& defaultValue) const
      {
         try {
            return get<Type>(name);
         } catch(...) {
            return defaultValue;
         }
      }


      template<class Type>
      Type get(const std::string& name) const
      {
         Type val;
         boost::any any = boost::any(val);
         getProperty(name, any);
         return boost::any_cast<Type>(any);
      }


      template<class Type>
      bool set(const std::string& name, const Type& propertydata)
      {
         try {
            setProperty(name, propertydata);
         } catch(...) {
            return false;
         }
         return true;
      }


   private:

      virtual void setProperty(const std::string& name, const boost::any& propertydata) = 0;
      virtual void getProperty(const std::string& name, boost::any& propertydata) const = 0;
   };

}

OPENOR_CREATE_INTERFACE(openOR::Configurable)
void load(const boost::property_tree::ptree& properties) { adaptee()->load(properties); }
void save(boost::property_tree::ptree& properties) const  { adaptee()->save(properties); }
OPENOR_CREATE_INTERFACE_END


OPENOR_CREATE_INTERFACE(openOR::PropertyAccess)
   void setProperty(const std::string& name, const boost::any& propertydata) { adaptee()->setProperty(name, propertydata); }
   void getProperty(const std::string& name, boost::any& propertydata) const { adaptee()->getProperty(name, propertydata); }
   void load(const boost::property_tree::ptree& properties) { adaptee()->load(properties); }
   void save(boost::property_tree::ptree& properties) const  { adaptee()->save(properties); }
OPENOR_CREATE_INTERFACE_END






#define OPENOR_PROPERTYACCESS_SET_BEGIN                                                         \
virtual void setProperty(const std::string& name, const boost::any& propertydata)               \
{                                                                                               \


#define OPENOR_PROPERTYACCESS_SET_ENTRY_EX(namestring, Type, command)                           \
   if (name == namestring)                                                                      \
   {                                                                                            \
      const Type* p = boost::any_cast<Type>(&propertydata);                                     \
      const Type& r = *p;                                                                       \
      command;                                                                                  \
      return;                                                                                   \
   }                                                                                            \

#define OPENOR_PROPERTYACCESS_SET_ENTRY(namestring, Type, var) OPENOR_PROPERTYACCESS_SET_ENTRY_EX(namestring, Type, var=r)


#define OPENOR_PROPERTYACCESS_SET_END                                                           \
   throw std::runtime_error(std::string("Could not find property with name") + name);           \
}




#define OPENOR_PROPERTYACCESS_GET_BEGIN                                                         \
virtual void getProperty(const std::string& name, boost::any& propertydata) const               \
{                                                                                               \

#define OPENOR_PROPERTYACCESS_GET_ENTRY(namestring, Type, var)                                  \
   if (name == namestring)                                                                      \
   {                                                                                            \
      Type* p = boost::any_cast<Type>(&propertydata);                                           \
      *p = var;                                                                                 \
      return;                                                                                   \
   }                                                                                            \

#define OPENOR_PROPERTYACCESS_GET_END                                                           \
   throw std::runtime_error(std::string("Could not find property with name") + name);           \
}


#endif
