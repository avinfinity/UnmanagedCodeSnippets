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
 * @ingroup Basic
 */
#ifndef openOR_Defs_Basic_hpp
#define openOR_Defs_Basic_hpp

//----------------------------------------------------------------------------
//! \def OPENOR_BASIC_API
//! \brief Portable definition for cross DLL linkage
//!
//! Will handle the import/export problem on Windows.
//! \ingroup Basic
//----------------------------------------------------------------------------
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#   if !defined(Basic_STATIC) && !defined(OPENOR_BUILD_AS_STATIC)
#      ifdef Basic_EXPORTS
#         define OPENOR_BASIC_API __declspec(dllexport)
#      else
#         define OPENOR_BASIC_API __declspec(dllimport)
#      endif
#   else
#      define OPENOR_BASIC_API
#   endif
#else
#   define OPENOR_BASIC_API
#endif 

#endif
