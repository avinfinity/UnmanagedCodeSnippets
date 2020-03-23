#-------------------------------------------------------------------------------------------------------------------------------
# (c) 2012 by the openOR Team
#-------------------------------------------------------------------------------------------------------------------------------
# The contents of this file are available under the GPL v2.0 license
# or under the openOR comercial license. see 
#   /Doc/openOR_free_license.txt or
#   /Doc/openOR_comercial_license.txt 
# for Details.
#-------------------------------------------------------------------------------------------------------------------------------

include(${CMAKE_CURRENT_LIST_DIR}/dbgMessage.cmake)

#-------------------------------------------------------------------------------------------------------------------------------

# Needs to be a macro so that the included files can set their variables in the top level scope

macro(openOR_internal_init_platform)

   message(STATUS "   Platform                : ${CMAKE_SYSTEM_NAME}")
   message(STATUS "   Compiler                : ${CMAKE_CXX_COMPILER_ID}")

   # Add Handler for the current platforms special definitions (if available)
   include("${CMAKE_SOURCE_DIR}/Platform.${CMAKE_SYSTEM_NAME}.cmake" OPTIONAL RESULT_VARIABLE _current_platform)
   if (_current_platform)
      openOR_internal_dbgMessage("   \\---> Using platform specific settings from 'Platform.${CMAKE_SYSTEM_NAME}.cmake'.")
      list(APPEND _cpdefs ${_current_platform})
   else()
      openOR_internal_dbgMessage("   \\---> No platform specific settings found.")
   endif()
   unset(_current_platform)

   include("${CMAKE_SOURCE_DIR}/Platform.${CMAKE_CXX_COMPILER_ID}.cmake" OPTIONAL RESULT_VARIABLE _current_compiler)
   if (_current_compiler)
      list(APPEND _cpdefs ${_current_compiler})
      openOR_internal_dbgMessage("   \\---> Using platform specific settings from 'Platform.${CMAKE_CXX_COMPILER_ID}.cmake'.")
   else()
      openOR_internal_dbgMessage("   \\---> No platform specific settings found.")
   endif()
   unset(_current_compiler)

   if (_cpdefs)
      add_custom_target(Definitions_CurrentPlatform SOURCES ${_cpdefs})
      openOR_project_group(Definitions_CurrentPlatform "BUILD - SYSTEM")
   endif()
   unset(_cpdefs)

   # Add all available platform handlers 
   file(GLOB _Platform_Handlers "Platform.*.cmake")
   if (_Platform_Handlers)
      add_custom_target(Definitions_Platforms SOURCES ${_Platform_Handlers})
      openOR_project_group(Definitions_Platforms "BUILD - SYSTEM")
   endif()
   unset(_Platform_Handlers)

endmacro()

#-------------------------------------------------------------------------------------------------------------------------------
