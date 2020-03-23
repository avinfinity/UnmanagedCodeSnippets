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

#ifndef openOR_Plugin_exception_hpp
#define openOR_Plugin_exception_hpp

#include <openOR/coreDefs.hpp>
#include <openOR/Log/Logger.hpp>

namespace openOR {
   namespace Plugin {
      
      //----------------------------------------------------------------------------
      //! \brief Signals that an object was not correctly created using function 
      //!        'createInstanceOf' or 'createInterfaceOf'.
      //! \ingroup openOR_core
      //----------------------------------------------------------------------------
      struct OPENOR_CORE_API IncorrectObjectCreationError : public std::bad_cast 
      {
         IncorrectObjectCreationError()
         :  std::bad_cast() 
         {
            LOG(Log::Level::Debug, Log::noAttribs, 
                Log::msg("openOR::interface_cast error: Object was not correctly created using function 'createInstanceOf' or 'createInterfaceOf'")
               );
         }
         
         virtual const char *what() const throw() {
            return "openOR::interface_cast error: Object was not correctly created using function 'createInstanceOf' or 'createInterfaceOf'";
         }
         
      };

      //----------------------------------------------------------------------------
      //! \brief Signals that a interface pointer cannot be casted to an concrete class.
      //----------------------------------------------------------------------------
      struct OPENOR_CORE_API UnsupportedInterfaceError : public std::bad_cast {
         UnsupportedInterfaceError(const std::string& class_name = "*unknown_class*", 
                                   const std::string& interface_name = "*unknown_interface*") 
         try :
         std::bad_cast(),
         m_className(class_name),
         m_interfaceName(interface_name),
         m_validMembers(true)
         {
            LOG(Log::Level::Debug, Log::noAttribs, 
                Log::msg("Cast from interface '" + interface_name + "' to '" + class_name + "' failed."));
         } catch (...) {
            // failed to initialize our members, but no need to panic, yet.
            m_validMembers = false;
         }
         
         virtual ~UnsupportedInterfaceError() throw() {}
         
         virtual const char *what() const throw() {     // WORKAROUND: Visual Studio has problems with function level try blocks.
            try {
               // try to give the best possible message.
               return m_validMembers 
               ? std::string("Cast from interface '" + m_interfaceName + "' to '" + m_className + "' failed.").c_str()
               : "UnsupportedInterfaceError";
            } catch (...) {
               // this failed to, but we can still give a sensible answer.
               LOG(Log::Level::Error, Log::noAttribs, Log::msg("UnsupportedInterfaceError::what() failed."));
               return "UnsupportedInterfaceError";
            }
         }
         
      private:
         std::string m_className;
         std::string m_interfaceName;
         // This needs to stay the last declared member!
         bool m_validMembers;
      };
      
   } // end NS
}

#endif

