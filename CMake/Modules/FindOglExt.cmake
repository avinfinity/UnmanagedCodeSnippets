# CMakeLists.txt                                 Copyright (C) 2008 Fabio Fracassi (fracassi@ipk.fraunhofer.de)
#                                             
#
# This file is part of OglExt, a free OpenGL extension library.
#
# This program is free software; you can redistribute it and/or modify it under the terms  of  the  GNU  Lesser
# General Public License as published by the Free Software Foundation; either version 2.1 of  the  License,  or
# (at your option) any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;  without  even  the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser  General  Public
# License for more details.
#
# You should have received a copy of the GNU Lesser General Public License along with  this  library;  if  not,
# write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


# TODO: make this right and use the UseOglExt.cmake files.

if ( NOT OglExt_DIR )
   find_library(OglExt_LIB NAMES OglExt OglExtd
                           PATHS
                              # User set Explicit
                              $ENV{OGLEXT_DIR}

                              # Unix standard locations
                              $ENV{HOME}
                              /usr/local
                              /usr

                              # Windows standard locations
                              $ENV{UserProfile}
                              $ENV{ProgramFiles}
							  ${TOOLROOTDIR}

                              # Search the Path
                              $ENV{PATH}

                           PATH_SUFFIXES
                              /lib
                              /OglExt/lib
               )

   if (OglExt_LIB)
      # Extract the OglExt Root path
      get_filename_component(OglExt_LIBRARY ${OglExt_LIB} NAME)
      string(REPLACE "/lib/${OglExt_LIBRARY}" "" OglExt_DIR_ ${OglExt_LIB})
   else (OglExt_LIB)
      set(OglExt_DIR_ "OglExt_DIR-NOTFOUND")
   endif (OglExt_LIB)
   
   set(OglExt_DIR "${OglExt_DIR_}" CACHE PATH "Path to OglExt")
  
endif ( NOT OglExt_DIR)

find_package(OpenGL)
if(OPENGL_FOUND)
	if(APPLE)
		set(OGL_INCLUDE_DIR "${OPENGL_INCLUDE_DIR}/Headers/" "${OPENGL_glu_LIBRARY}/Headers/")
	else(APPLE)
		set(OGL_INCLUDE_DIR ${OPENGL_INCLUDE_DIR})
	endif(APPLE)
   #message(STATUS "Found OpenGL in '${OGL_INCLUDE_DIR}'")
endif(OPENGL_FOUND)

unset(OglExt_LIB CACHE)
find_library(OglExt_LIB    NAMES OglExt   PATHS "${OglExt_DIR}/lib")
find_library(OglExt_d_LIB  NAMES OglExtd  PATHS "${OglExt_DIR}/lib")
find_library(OglExt_s_LIB  NAMES OglExts  PATHS "${OglExt_DIR}/lib")
find_library(OglExt_sd_LIB NAMES OglExtsd PATHS "${OglExt_DIR}/lib")

set(OglExt_FOUND FALSE)
if (OglExt_LIB)
   set(OglExt_FOUND TRUE)

   set( OglExt_LIBRARY_DIR "${OglExt_DIR}/lib" ${OPENGL_LIBRARY_DIR} )
   set( OglExt_INCLUDE_DIR "${OglExt_DIR}/include" ${OGL_INCLUDE_DIR} )
   set( OglExt_DLL_DIR     "${OglExt_DIR}/bin" )

   # CMake defaults to the plural version nowerdays
   set( OglExt_LIBRARY_DIRS "${OglExt_LIBRARY_DIR}" )
   set( OglExt_INCLUDE_DIRS "${OglExt_INCLUDE_DIR}" )

   set(_OglExt_LIB optimized ${OglExt_LIB})
   if (OglExt_d_LIB)
      list(APPEND _OglExt_LIB debug ${OglExt_d_LIB})
   endif()
   
   set(_OglExt_s_LIB "")
   if (OglExt_s_LIB)
      set(_OglExt_s_LIB optimized ${OglExt_s_LIB})
   endif()
   if (OglExt_sd_LIB)
      list(APPEND _OglExt_LIB debug ${OglExt_sd_LIB})
   endif()

   set( OglExt_LIBRARIES ${_OglExt_LIB} ${OPENGL_LIBRARIES})
   set( OglExt_STATIC_LIBRARIES ${_OglExt_s_LIB} ${OPENGL_LIBRARIES} )
   set( OglExt_DLL "${OglExt_DIR}/lib/OglExt.dll" )
endif (OglExt_LIB)

# EOF
