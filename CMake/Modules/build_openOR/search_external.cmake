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
include(${CMAKE_CURRENT_LIST_DIR}/detail/include.cmake)

openOR_internal_include(detail/internal.cmake)

#-------------------------------------------------------------------------------------------------------------------------------

macro(openOR_internal_init_search_external)
   openOR_add_cmake_search_path("${CMAKE_SOURCE_DIR}/ThirdParty")
   openOR_add_cmake_search_path("${CMAKE_SOURCE_DIR}/External")
   openOR_add_cmake_search_path("${CMAKE_SOURCE_DIR}/ExternalDependencies")
endmacro()

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_search_external Library)
   cmake_parse_arguments(argparse "REQUIRED;OPTIONAL;EXACT;MINIMUM" "VERSION;TARGET;RESULT_VARS" "" ${ARGN})

   openOR_internal_ternary(RO argparse_OPTIONAL "optional" "required")
   openOR_internal_ternary(EM argparse_EXACT "exact" "minimum")

   openOR_internal_ternary(VS argparse_VERSION "with ${EM} Version ${argparse_VERSION}" "")
   openOR_internal_ternary(LibraryTARGET argparse_TARGET      "${argparse_TARGET}"      "${Library}")
   openOR_internal_ternary(LibraryRESULT argparse_RESULT_VARS "${argparse_RESULT_VARS}" "")

   message(STATUS "Searching for ${RO} external Library ${Library} ${VS} ...")

   set(openOR_internal_target_${LibraryTARGET}_seen TRUE CACHE INTERNAL "Target has been visited by CMake")

   if(argparse_OPTIONAL)
      option(openOR_use_${Library} "Build parts that depend on ${Library}" ON)
   endif()

   if (openOR_use_${Library} OR NOT argparse_OPTIONAL)
      include(${Library} OPTIONAL RESULT_VARIABLE handler)

      if(handler)
         openOR_internal_dbgMessage(VERBOSITY 1 "   \\---> Using custom finder")
         openOR_find_external(${ARGN})

         add_custom_target(${LibraryTARGET} SOURCES ${handler}) # TODO: only add SOURCES if the handler file
                                                                # is in a sub dir of ${CMAKE_SOURCE_DIR}
      else()
         # try searching by traditional means
         openOR_internal_dbgMessage(VERBOSITY 1 "   \\---> Using default package finding")

         find_package(${Library} QUIET)

         openOR_internal_try_include_usefile(${${Library}_USE_FILE})
         openOR_internal_try_include_usefile(${${LibraryRESULT}_USE_FILE})

         if(${Library}_FOUND OR ${LibraryRESULT}_FOUND)
            set(includeDirs "${${Library}_INCLUDE_DIRS}" "${${LibraryRESULT}_INCLUDE_DIRS}")
            set(linkDirs    "${${Library}_LIBRARY_DIRS}" "${${LibraryRESULT}_LIBRARY_DIRS}")
            set(libs        "${${Library}_LIBRARIES}" "${${LibraryRESULT}_LIBRARIES}")

            set(openOR_internal_target_${LibraryTARGET}_interfaces ${includeDirs} CACHE INTERNAL "Include dir of ${Library}")
            set(openOR_internal_target_${LibraryTARGET}_binaries   ${linkDirs}    CACHE INTERNAL "Binary dir of ${Library}")
            set(openOR_internal_target_${LibraryTARGET}_link       ${libs}
                                                                 CACHE INTERNAL "What needs to be linked for target ${Library}")
         endif()

         add_custom_target(${LibraryTARGET})
      endif()

      if(${Library}_FOUND OR ${LibraryRESULT}_FOUND)
         message(STATUS "Using external Library: ${Library}")
      else()
         if(argparse_OPTIONAL)
            message(STATUS "${Library} not found - Building without ${Library} support.")
         else()
            message(FATAL_ERROR "${Library} not found. Cannot build.")
         endif()
      endif()

      openOR_project_group(${LibraryTARGET} "BUILD - SYSTEM/External Libraries")

      openOR_internal_append_sorted(openOR_internal_targetlist_ALL ${LibraryTARGET})
      set(openOR_internal_targetlist_ALL ${openOR_internal_targetlist_ALL} CACHE INTERNAL
                                                                                     "List of all modules in the current build")

      if(openOR_internal_target_${LibraryTARGET}_interfaces)
         openOR_internal_dbgMessage("     |      include dirs = ${openOR_internal_target_${LibraryTARGET}_interfaces}")
      endif()

      if(openOR_internal_target_${LibraryTARGET}_binaries)
         openOR_internal_dbgMessage("     |      link dirs    = ${openOR_internal_target_${LibraryTARGET}_binaries}")
      endif()

      if(openOR_internal_target_${LibraryTARGET}_link)
         openOR_internal_dbgMessage("     |      link libs    = ${openOR_internal_target_${LibraryTARGET}_link}")
      endif()
   else()
      message(STATUS "Building without ${Library} support. Set option 'openOR_use_${Library}' to use it")
   endif()

endfunction()

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_mark_external Library)
   openOR_internal_dbgMessage("Marking ${Library} to be a external Library")

   set(openOR_internal_target_${Library}_seen TRUE CACHE INTERNAL "Target has been visited by CMake")
   add_custom_target(${Library})
   openOR_project_group(${Library} "BUILD - SYSTEM/External Libraries")

   openOR_internal_append_sorted(openOR_internal_targetlist_ALL ${Library})
   set(openOR_internal_targetlist_ALL ${openOR_internal_targetlist_ALL} CACHE INTERNAL
                                                                                     "List of all modules in the current build")
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------
# \warning: This cannot be called from a function since that messes up scope
macro(openOR_add_cmake_search_path Path)
   get_filename_component(openOR_build_system_path "${Path}" ABSOLUTE)
   openOR_internal_dbgMessage("Adding CMake search Path '${openOR_build_system_path}'")
   list(APPEND openOR_internal_CMAKE_MODULE_PATH ${openOR_build_system_path})
   set(openOR_internal_CMAKE_MODULE_PATH ${openOR_internal_CMAKE_MODULE_PATH} CACHE INTERNAL "Search Path")

   set(CMAKE_MODULE_PATH ${openOR_internal_CMAKE_MODULE_PATH} ${CMAKE_MODULE_PATH})
endmacro()

#-------------------------------------------------------------------------------------------------------------------------------

macro(openOR_internal_try_include_usefile)
   if(${ARGC} EQUAL 1)
      include(${ARGN} OPTIONAL RESULT_VARIABLE usefile)
      if(usefile)
         openOR_internal_dbgMessage(VERBOSITY 1 "      \\---> used package specific use file '${ARGN}'")
      endif()
   endif()
endmacro()

#-------------------------------------------------------------------------------------------------------------------------------
