//****************************************************************************
// (c) 1012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//! OPENOR_INTERFACE_FILE(Image_ImageIO)
//****************************************************************************
/**
 * @file
 * @ingroup Image
 */
#ifndef openOR_Defs_Image_Regions_hpp
#define openOR_Defs_Image_Regions_hpp

//----------------------------------------------------------------------------
//! \def OPENOR_IMAGE_REGIONS_API
//! \brief Portable definition for cross DLL linkage
//!
//! Will handle the import/export problem on Windows.
//! \ingroup Image
//----------------------------------------------------------------------------
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#   if !defined(Image_Regions_STATIC) && !defined(OPENOR_BUILD_AS_STATIC)
#      ifdef Image_Regions_EXPORTS
#         define OPENOR_IMAGE_REGIONS_API __declspec(dllexport)
#      else
#         define OPENOR_IMAGE_REGIONS_API __declspec(dllimport)
#      endif
#   else
#      define OPENOR_IMAGE_REGIONS_API
#   endif
#else
#   define OPENOR_IMAGE_REGIONS_API
#endif 

#endif

