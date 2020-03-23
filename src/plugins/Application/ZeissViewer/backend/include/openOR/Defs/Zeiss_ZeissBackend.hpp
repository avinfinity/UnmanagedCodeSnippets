//****************************************************************************
// (c) 1012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//! OPENOR_INTERFACE_FILE(Zeiss_ZeissBackend)
//****************************************************************************
/**
* @file
* @ingroup Zeiss_ZeissBackend
*/
#ifndef openOR_Defs_Zeiss_ZeissBackend_hpp
#define openOR_Defs_Zeiss_ZeissBackend_hpp


//----------------------------------------------------------------------------
//! \def OPENOR_ZEISS_ZEISSBACKEND_API
//! \brief Portable definition for cross DLL linkage
//!
//! Will handle the import/export problem on Windows.
//! \ingroup Zeiss_ZeissBackend
//----------------------------------------------------------------------------

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#   if !defined(ZeissBackend_STATIC) && !defined(OPENOR_BUILD_AS_STATIC)
#   ifdef ZeissBackend_EXPORTS
#      define OPENOR_ZEISS_ZEISSBACKEND_API __declspec(dllexport)
#   else
#      define OPENOR_ZEISS_ZEISSBACKEND_API __declspec(dllimport)
#   endif
#   else
#      define OPENOR_ZEISS_ZEISSBACKEND_API
#   endif
#else
#   define OPENOR_ZEISS_ZEISSBACKEND_API
#endif 

#endif
