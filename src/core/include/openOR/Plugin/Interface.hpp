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

#ifndef openOR_Plugin_Interface_hpp
#define openOR_Plugin_Interface_hpp

#include <openOR/coreDefs.hpp>
#include <openOR/Plugin/AnyPtr.hpp>

namespace openOR {
   
   namespace Plugin {

      namespace Detail {
         
         template < class From_type, class To_type, bool, bool> struct InterfaceCaster;
         std::string OPENOR_CORE_API demangle(const char* name); 
      }

      //----------------------------------------------------------------------------
      //! \brief A Handle for a plugin.
      //!
      //! Might be changed into a real type soon
      //----------------------------------------------------------------------------
      typedef AnyPtr AnyPlugin;

   }
}

#endif //openOR_Plugin_Interface_hpp
