#-------------------------------------------------------------------------------------------------------------------------------
# (c) 2012 by the openOR Team
#-------------------------------------------------------------------------------------------------------------------------------
# The contents of this file are available under the GPL v2.0 license
# or under the openOR comercial license. see 
#   /Doc/openOR_free_license.txt or
#   /Doc/openOR_comercial_license.txt 
# for Details.
#-------------------------------------------------------------------------------------------------------------------------------

include(CMakeParseArguments)
set(openOR_internal_build_openOR_baseDir ${CMAKE_CURRENT_LIST_DIR})
include(${CMAKE_CURRENT_LIST_DIR}/build_openOR/detail/include.cmake)

openOR_internal_include(build_openOR/detail/internal.cmake)
openOR_internal_include(build_openOR/detail/dbgMessage.cmake)
openOR_internal_include(build_openOR/detail/repository.cmake)
openOR_internal_include(build_openOR/detail/platform.cmake)
openOR_internal_include(build_openOR/detail/error.cmake)

openOR_internal_include(build_openOR/search_external.cmake)
openOR_internal_include(build_openOR/tests.cmake)
openOR_internal_include(build_openOR/docs.cmake)
openOR_internal_include(build_openOR/create.cmake)
openOR_internal_include(build_openOR/source.cmake)
openOR_internal_include(build_openOR/dependencies.cmake)
openOR_internal_include(build_openOR/install.cmake)

#-------------------------------------------------------------------------------------------------------------------------------

# This is the main setup, needs to be a macro so that the scope of the variables set here is global

macro(openOR_project PName)
   cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
   cmake_policy(SET CMP0012 OLD)
   cmake_policy(SET CMP0039 OLD)
   cmake_policy(SET CMP0048 OLD)
   cmake_policy(SET CMP0054 NEW)

   message(STATUS "Building Project ${PName}")
   message(STATUS "   Source path             : ${CMAKE_SOURCE_DIR}")
   message(STATUS "   CMake Version           : ${CMAKE_VERSION}")

   option(openOR_build_no_module_groups "Skip generation of module groups (for Visual Studio Express)" OFF)
   if(NOT openOR_build_no_module_groups)
      set_property(GLOBAL PROPERTY USE_FOLDERS ON)
   endif()

   openOR_add_cmake_search_path(${openOR_internal_build_openOR_baseDir})
   openOR_internal_ensure_out_of_source()
   openOR_internal_init_dbgMessage()
   openOR_internal_dbgMessage("Initializing Build for ${PName} ...")

   project(${PName})

   openOR_internal_init_include()
   openOR_internal_init_tests()
   openOR_internal_init_docs()
   openOR_internal_init_install()
   openOR_internal_init_search_external()
   openOR_internal_init_platform()

   openOR_add_cmake_search_path("${CMAKE_SOURCE_DIR}/ThirdParty")
   openOR_add_cmake_search_path("${CMAKE_SOURCE_DIR}/External")
   openOR_add_cmake_search_path("${CMAKE_SOURCE_DIR}/ExternalDependencies")
   list(INSERT 0 CMAKE_MODULE_PATH ${openOR_internal_CMAKE_MODULE_PATH})

   set(openOR_internal_dll_path "lib")
   if (WIN32)
      # Windows needs the dlls placed in the same dir as the executables
      set(openOR_internal_dll_path "bin")
   endif()

   if(CMAKE_CFG_INTDIR STREQUAL ".")
      # makefile generators can not build Debug and Release from the same tree
      set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
      set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
      set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${openOR_internal_dll_path}")

      set(openOR_internal_maketime_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
   else()
      # IDE generators can, so we use different dirs to keep the results well separated.
      set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug/bin")
      set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug/lib")
      set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug/${openOR_internal_dll_path}")

      set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release/bin")
      set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release/lib")
      set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release/${openOR_internal_dll_path}")

      set(openOR_internal_maketime_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/bin")
   endif()

endmacro()

#-------------------------------------------------------------------------------------------------------------------------------

macro(openOR_add_subdirectory Dir)
   openOR_internal_add_subdirectory(${Dir})
   add_subdirectory(${Dir})
endmacro()

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_build)
   openOR_internal_create_subdirectory_target()

   set(openOR_internal_first_scan_complete TRUE CACHE INTERNAL "All targets have now been seen at least once.")
   openOR_internal_check_dependencies()

   # make sure that handle errors stays the last command in this function
   #openOR_internal_handle_errors()
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_set_version)
   set(${CMAKE_PROJECT_NAME}_VERSION "" CACHE INTERNAL "")
   string(REPLACE ";" "." Version "${ARGN}")
   openOR_internal_dbgMessage("Setting ${CMAKE_PROJECT_NAME}'s Version to '${Version}'")

   openOR_internal_get_date(Date)
   openOR_internal_get_repository_information(Repos)

   set(BuildID "Build")
   if (NOT Repos_REV STREQUAL "unknown")
      set(BuildID "${BuildID} ${Repos_REV}")
   endif()
   if(Date)
      set(BuildID "${BuildID} on ${Date}")
   endif()

   message(STATUS "   Build Date              : ${Date}")
   message(STATUS "   Last Cheked out revision: ${Repos_REV}")
   message(STATUS "   Build ID                : ${BuildID}")

   set(${CMAKE_PROJECT_NAME}_VERSION "${Version}" CACHE INTERNAL "Version of ${CMAKE_PROJECT_NAME}")
   set(${CMAKE_PROJECT_NAME}_BuildID "${BuildID}" CACHE INTERNAL "Version of ${CMAKE_PROJECT_NAME}")
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------
