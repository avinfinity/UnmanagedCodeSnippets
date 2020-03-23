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
/**
 * @file
 * @ingroup openOR_core
 */
#ifndef openOR_coreDefs_hpp
#define openOR_coreDefs_hpp

#include <boost/config.hpp>

#ifdef BOOST_MSVC
#   if !defined(openOR_core_STATIC) && !defined(OPENOR_BUILD_AS_STATIC)
#      ifdef openOR_core_EXPORTS
#         define OPENOR_CORE_API __declspec(dllexport)
#      else
#         define OPENOR_CORE_API __declspec(dllimport)
#      endif
       // VS7 doesn't like the usage of STL or boost templates in exported classes.
#      pragma warning( disable: 4251 )
#      pragma warning( disable: 4275 )
       // MSVC deprecates functions, which have a better MSVC equivalent, but are not portable.
#      pragma warning( disable: 4996 )
#   endif
#else
   // #ifdef ((defined(__GNUC__) || defined(__GNUG__)) && defined(USE_GCC_VISIBILITY))
   //    #define OPENOR_CORE_API __attribute__ ((visibility("default")))
   //    #define OPENOR_CORE_LOCAL __attribute__ ((visibility("hidden")))
   // #endif // GCC
#endif // MSVC

// default to nothing
#ifndef OPENOR_CORE_API
#   define OPENOR_CORE_API
#endif

//----------------------------------------------------------------------------
//! \def OPENOR_CORE_API
//! \brief Portable definition for cross DLL linkage
//!
//! Will handle the import/export problem on Windows.
//! Also makes optimized DSO visibility available for gcc builds.
//!   This is disabled by default to enable you need to set
//!   USE_GCC_VISIBILITY in your buildsystem.
//! (see: http://gcc.gnu.org/wiki/Visibility for details)
//! \ingroup openOR_core
//----------------------------------------------------------------------------

#ifndef OPENOR_CORE_LOCAL
#   define OPENOR_CORE_LOCAL
#endif

//----------------------------------------------------------------------------
//! \def OPENOR_CORE_LOCAL
//! \brief Mark a function or class local to the DLL
//!
//! If you mark functions or classes with this macro their symbols will not
//! appear in the symboltable of the DSO. You need to be sure that the
//! function or class is realy only used locally, and never called by or
//! given to anything outside the class.
//! \note this is a pure optimisation, and currently effects only gcc
//----------------------------------------------------------------------------


#ifdef BOOST_MSVC
#   define OPENOR_CURRENT_FUNCTION __FUNCTION__
#else
//#   ifdef (defined(__GNUC__) || defined(__GNUG__))
#      define OPENOR_CURRENT_FUNCTION __PRETTY_FUNCTION__
//#   endif // GCC
#endif // MSVC

// default to __FUNCTION__
#ifndef OPENOR_CURRENT_FUNCTION
#   define OPENOR_CURRENT_FUNCTION __FUNCTION__
#endif




//----------------------------------------------------------------------------
//! \def OPENOR_INORE_UNUSED(x)
//! \brief Mark a function or class local to the DLL
//----------------------------------------------------------------------------
// TODO: There was a nice article on how to do this properly somewhere
//       need to find it again and do it the way they propose.
#define OPENOR_IGNORE_UNUSED(x) (void)(x)


namespace openOR {

   //----------------------------------------------------------------------------
   //! \brief Contais the global version numbers for openOR
   //! \ingroup openOR_core
   //----------------------------------------------------------------------------
   struct OPENOR_CORE_API Version {

      //----------------------------------------------------------------------------
      //! \brief Release version number in freeform string format.
      //!
      //! Should be set by the Buildsystem through the use of the Preprocessor
      //! directive OPENOR_VERSION. Prefered format is
      //! "<Major>.<Minor>.<Patchlevel>". Defaults to "unknown".
      //----------------------------------------------------------------------------
      static const char* number();

      //----------------------------------------------------------------------------
      //! \brief Build or Revision number in freeform string format.
      //!
      //! Should be set by the Buildsystem through the use of the Preprocessor
      //! directive OPENOR_BUILDID. Should be a unique id of the revision control
      //! system, so that the source state of the build is easily reproducable.
      //! Defaults to "".
      //----------------------------------------------------------------------------
      static const char* rev_id();
   };
}

#endif // openOR_coreDefs_hpp

