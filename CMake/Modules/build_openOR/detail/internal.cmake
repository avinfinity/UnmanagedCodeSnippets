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

macro(openOR_internal_ternary variable bool set1 set2)
   if(${bool})
      set(${variable} ${set1})
   else()
      set(${variable} ${set2})
   endif()
endmacro()

#-------------------------------------------------------------------------------------------------------------------------------

macro(openOR_internal_append_sorted list)
   openOR_internal_dbgMessage(VERBOSITY 3 "List ${list} = ${${list}} + ${ARGN}")
   list(APPEND ${list} ${ARGN})
   if (${list})
      list(SORT ${list})
      list(REMOVE_DUPLICATES ${list})
   endif()
endmacro()

#-------------------------------------------------------------------------------------------------------------------------------

macro(openOR_internal_append_unique list)
   openOR_internal_dbgMessage(VERBOSITY 3 "List ${list} = ${${list}} + ${ARGN}")
   list(APPEND ${list} ${ARGN})
   if (${list})
      list(REMOVE_DUPLICATES ${list})
   endif()
endmacro()

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_internal_ensure_out_of_source)
   if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
      message(FATAL_ERROR "The source and the build directory have to be different. Choose a different build directory.")
   endif()
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_internal_get_date Result)
   if(WIN32)
      execute_process(COMMAND "cmd" " /C date /T" OUTPUT_VARIABLE RawDate)
   elseif(UNIX)
      execute_process(COMMAND "date" "+%d/%m/%Y" OUTPUT_VARIABLE RawDate)
      execute_process(COMMAND "date" "+%H:%M" OUTPUT_VARIABLE RawTime)
      set(Time ${RawTime})
   else()
      set(RawDate "")
   endif()
   string(REGEX REPLACE "(..)[-/\\.](..)[-/\\.](....).*" "\\1/\\2/\\3" Date "${RawDate}")

   if(Time)
      set(Date "${Date} ${Time}")
   endif()
   string(REPLACE "\n" "" Date "${Date}")  # make sure no newlines survive, since they would cause much trouble
   set(${Result} "${Date}" PARENT_SCOPE)
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------
