#
#  Find openOR includes and library.
#
#  Optionally finds the openOR source-tree.
#
#  This module defines:
#  openOR_FOUND         - if openOR has been found
#  OPENOR_INCLUDE_DIR
#  OPENOR_CORE_LIB

find_path(OPENOR_INCLUDE_DIR openOR/coreDefs.hpp
   ${OPENORDIR}/include
)

if(CMAKE_BUILD_TYPE AND CMAKE_DEBUG_POSTFIX)
   string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UC)
   if (CMAKE_BUILD_TYPE_UC STREQUAL "DEBUG")
      set(_postfix ${CMAKE_DEBUG_POSTFIX})
   else (CMAKE_BUILD_TYPE_UC STREQUAL "DEBUG")
      set(_postfix "")
   endif (CMAKE_BUILD_TYPE_UC STREQUAL "DEBUG")
endif(CMAKE_BUILD_TYPE AND CMAKE_DEBUG_POSTFIX)

find_library(OPENOR_CORE_LIB  core${_postfix} 
   PATHS ${OPENORDIR}/lib
)

if(WIN32)
   find_file(OPENOR_CORE_DLL core${_postfix}.dll ${OPENORDIR}/bin)
else(WIN32)
   set(OPENOR_CORE_DLL ${OPENOR_CORE_LIB})
endif(WIN32)

if(NOT OPENOR_INCLUDE_DIR-NOTFOUND)
   message(STATUS "Found openOR")
   set(openOR_FOUND TRUE)
endif(NOT OPENOR_INCLUDE_DIR-NOTFOUND)
