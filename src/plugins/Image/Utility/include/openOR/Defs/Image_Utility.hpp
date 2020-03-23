//****************************************************************************
// (c) 2008 - 2010 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//! OPENOR_INTERFACE_FILE(Image_Utility)
//****************************************************************************

#ifndef openOR_Defs_Image_Utility_hpp
#define openOR_Defs_Image_Utility_hpp

//----------------------------------------------------------------------------
//! \def OPENOR_IMAGE_UTILITY_API
//! \brief Portable definition for cross DLL linkage
//!
//! Will handle the import/export problem on Windows.
//! \ingroup Image_Utility
//----------------------------------------------------------------------------
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#   ifdef Image_Utility_EXPORTS
#      if !defined(Image_Utility_STATIC) && !defined(OPENOR_BUILD_AS_STATIC)
#         define OPENOR_IMAGE_UTILITY_API __declspec(dllexport)
#      else
#         define OPENOR_IMAGE_UTILITY_API __declspec(dllimport)
#      endif
#   else
#      define OPENOR_IMAGE_UTILITY_API
#   endif
#else
#   define OPENOR_IMAGE_UTILITY_API
#endif 

#endif

