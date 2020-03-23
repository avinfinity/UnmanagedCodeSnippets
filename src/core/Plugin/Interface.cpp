//****************************************************************************
// (c) 2008 - 2010 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#include "../include/openOR/Plugin/Interface.hpp"

//--------------------------------------------------------------------------------
// Improve debug and error messages. (on GCC)
//--------------------------------------------------------------------------------
#include <boost/config.hpp>
#if (defined(__GNUC__) || defined(__GNUG__))
   
   #include <cxxabi.h>
   #include <vector>


   namespace openOR { namespace Plugin { namespace Detail {
      std::string demangle(const char* name) {
         size_t size=2024;
         std::vector<char> buffer(size, 0);
         int status;
         std::string res = abi::__cxa_demangle (name, &buffer[0], &size, &status);
         return res;
      }
   } } }

#else

   //--------------------------------------------------------------------------------
   // No demangling simple pass through
   //--------------------------------------------------------------------------------
   namespace openOR { namespace Plugin { namespace Detail {
      std::string demangle(const char* name) { return std::string(name); }
   } } }
#endif

