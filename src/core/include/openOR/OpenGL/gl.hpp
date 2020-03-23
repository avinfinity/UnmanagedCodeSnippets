//****************************************************************************
// (c) 2008-2010 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#if (defined(_WIN32) || defined(__WIN32__) || defined(WIN32)) && !defined(APIENTRY) && !defined(__CYGWIN__)
   #define	WIN32_LEAN_AND_MEAN 1
   #include <windows.h>
#endif


#if (defined(__APPLE__) && defined(__GNUC__)) || defined(__MACOSX__)
   #include <gl.h>
#else
   #include <GL/gl.h>
#endif


