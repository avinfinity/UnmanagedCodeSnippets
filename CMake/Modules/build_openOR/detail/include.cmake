#-------------------------------------------------------------------------------------------------------------------------------
# (c) 2012 by the openOR Team
#-------------------------------------------------------------------------------------------------------------------------------
# The contents of this file are available under the GPL v2.0 license
# or under the openOR comercial license. see 
#   /Doc/openOR_free_license.txt or
#   /Doc/openOR_comercial_license.txt 
# for Details.
#-------------------------------------------------------------------------------------------------------------------------------

include(${CMAKE_CURRENT_LIST_DIR}/internal.cmake)
set(openOR_internal_build_openOR_includeDir ${CMAKE_CURRENT_LIST_DIR})

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_internal_filter_list OUT Regex)
   foreach(_item ${ARGN})
      if(_item MATCHES "${Regex}")
         list(APPEND _list ${_item})
      endif()
   endforeach()
   set(${OUT} ${_list} PARENT_SCOPE)
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_internal_init_include)
   openOR_internal_append_sorted(openOR_internal_buildSystemFiles ${openOR_internal_build_openOR_includeDir}/internal.cmake)
   openOR_internal_append_sorted(openOR_internal_buildSystemFiles ${openOR_internal_build_openOR_includeDir}/include.cmake)
   openOR_internal_append_sorted(openOR_internal_buildSystemFiles ${openOR_internal_build_openOR_baseDir}/build_openOR.cmake)

   add_custom_target(BuildSystem SOURCES ${openOR_internal_buildSystemFiles})

   openOR_internal_filter_list(_build  ".*build_openOR/.*" ${openOR_internal_buildSystemFiles})
   openOR_internal_filter_list(_detail ".*detail.*" ${openOR_internal_buildSystemFiles})

   source_group("Build" FILES ${_build})
   source_group("Build/Detail" FILES ${_detail})

   set_property(TARGET BuildSystem PROPERTY FOLDER "BUILD - SYSTEM")
   set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "BUILD - SYSTEM")

   if(openOR_internal_buildSystemSubdirs)
      unset(openOR_internal_buildSystemSubdirs CACHE)
   endif()
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_internal_add_subdirectory Dir)
   string(REPLACE "${CMAKE_SOURCE_DIR}" "" RelPath "${CMAKE_CURRENT_SOURCE_DIR}")
   string(REGEX REPLACE "^/" "" RelPath "${RelPath}")

   set(_Dir "${RelPath}/${Dir}")
   string(REGEX REPLACE "^/" "" _Dir "${_Dir}")

   openOR_internal_append_sorted(openOR_internal_buildSystemSubdirs "${_Dir}")
   set(openOR_internal_buildSystemSubdirs ${openOR_internal_buildSystemSubdirs} CACHE INTERNAL "Subdirs")
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_internal_create_subdirectory_target)
   set(_Target "${CMAKE_PROJECT_NAME}_CMakeLists")

   openOR_internal_dbgMessage("Creating Target '${_Target}' containing all CMakeList files")
   foreach(Dir ${openOR_internal_buildSystemSubdirs})
      set(_Source "${Dir}/CMakeLists.txt")
      list(APPEND _Sources ${_Source})

      string(REPLACE "/" "\\" GroupName ${Dir})
      source_group("${GroupName}" FILES ${_Source})

      openOR_internal_dbgMessage(VERBOSITY 2 "   \\---> added Subdirectory '${_Source}' to group '${GroupName}'")
   endforeach()

   add_custom_target(${_Target} SOURCES ${_Sources})
   set_property(TARGET ${_Target} PROPERTY FOLDER "BUILD - SYSTEM")
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------

macro(openOR_internal_include relFile)
   include(${CMAKE_CURRENT_LIST_DIR}/${relFile})
   openOR_internal_append_sorted(openOR_internal_buildSystemFiles ${CMAKE_CURRENT_LIST_DIR}/${relFile})
endmacro()

#-------------------------------------------------------------------------------------------------------------------------------
