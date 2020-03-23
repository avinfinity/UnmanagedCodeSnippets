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
#cmake_policy(SET CMP0054 OLD)
function(openOR_internal_get_repository_information Result)
   cmake_policy(SET CMP0054 OLD)
   #find_package(Subversion)
   if(SUBVERSION_FOUND)
      openOR_internal_dbgMessage(VERBOSITY 2 "Found Subversion Executable")
      openOR_internal_dbgMessage(VERBOSITY 3 "        \\--> '${Subversion_SVN_EXECUTABLE}'")

      openOR_internal_run_info_cmd(Subversion "${Subversion_SVN_EXECUTABLE}" "" "${CMAKE_SOURCE_DIR}" Project)
   endif()

   #find_package(Git)
   if(GIT_FOUND)
      openOR_internal_dbgMessage(VERBOSITY 2 "Found git Executable")
      openOR_internal_dbgMessage(VERBOSITY 3 "        \\--> '${GIT_EXECUTABLE}'")

      openOR_internal_run_info_cmd(Git "${GIT_EXECUTABLE}" "svn" "${CMAKE_SOURCE_DIR}" Project)
   endif()


   if(Project_IS_WORKING_COPY)
      string(REGEX REPLACE "\\\\" "/" Project_WC_INFO "${Project_WC_INFO}")
      openOR_internal_parse_svn_information(Project "${Project_WC_INFO}")
      
      openOR_internal_dbgMessage("Found ${Project_IS_WORKING_COPY} Working Copy at rev. ${Project_WC_REVISION}")
      openOR_internal_dbgMessage(VERBOSITY 2" Full Working Copy Info: \n${Project_WC_INFO}")
      set(${Result}_REV "r${Project_WC_REVISION}" PARENT_SCOPE)
   else()
      set(${Result}_REV "unknown" PARENT_SCOPE)
   endif()

endfunction()

#-------------------------------------------------------------------------------------------------------------------------------

# Taken from CMakes FindSubversion module
# we cannot use Subversion_WC_INFO directly since we don't want it to report errors via SEND_ERROR
# and we want to reuse it for git SVN

function(openOR_internal_run_info_cmd ReposKind ReposCmd CmdArgs WorkingDir prefix)
   # the commands should be executed with the C locale, otherwise
   # the message (which are parsed) may be translated
   set(_SAVED_LC_ALL "$ENV{LC_ALL}")
   set(ENV{LC_ALL} C)

   execute_process( COMMAND "${ReposCmd}" ${CmdArgs} info
                     WORKING_DIRECTORY "${WorkingDir}"
                     OUTPUT_VARIABLE ${prefix}_WC_INFO
                     ERROR_VARIABLE _info_error
                     RESULT_VARIABLE _info_result
                     OUTPUT_STRIP_TRAILING_WHITESPACE
                  )

   if ("${_info_result}" STREQUAL "0")   # for some reason integral comparison doesn't work
      openOR_internal_dbgMessage(VERBOSITY 2 "${ReposKind} working copy found at ${WorkingDir}")
      openOR_internal_dbgMessage(VERBOSITY 3 "     \\--> full info is \n${${prefix}_WC_INFO}")

      set(${prefix}_IS_WORKING_COPY "${ReposKind}" PARENT_SCOPE)
      set(${prefix}_WC_INFO "${${prefix}_WC_INFO}" PARENT_SCOPE)
   else()
      openOR_internal_dbgMessage(VERBOSITY 2 "No ${ReposKind} working copy found at ${WorkingDir}")
      openOR_internal_dbgMessage(VERBOSITY 3 "     \\--> Command '${ReposCmd}' reported '${_info_error}'.")

      if(NOT ${prefix}_IS_WORKING_COPY)
         set(${prefix}_IS_WORKING_COPY FALSE PARENT_SCOPE)
      endif()
   endif()

   # restore the previous LC_ALL
   set(ENV{LC_ALL} ${_SAVED_LC_ALL})
endfunction()

macro(openOR_internal_parse_svn_information prefix _Input)
   string(REGEX REPLACE "^(.*\n)?URL: ([^\n]+).*"                    "\\2" ${prefix}_WC_URL                    "${_Input}")
   string(REGEX REPLACE "^(.*\n)?Repository Root: ([^\n]+).*"        "\\2" ${prefix}_WC_ROOT                   "${_Input}")
   string(REGEX REPLACE "^(.*\n)?Revision: ([^\n]+).*"               "\\2" ${prefix}_WC_REVISION               "${_Input}")
   string(REGEX REPLACE "^(.*\n)?Last Changed Author: ([^\n]+).*"    "\\2" ${prefix}_WC_LAST_CHANGED_AUTHOR    "${_Input}")
   string(REGEX REPLACE "^(.*\n)?Last Changed Rev: ([^\n]+).*"       "\\2" ${prefix}_WC_LAST_CHANGED_REV       "${_Input}")
   string(REGEX REPLACE "^(.*\n)?Last Changed Date: ([^\n]+).*"      "\\2" ${prefix}_WC_LAST_CHANGED_DATE      "${_Input}")
endmacro()

#-------------------------------------------------------------------------------------------------------------------------------
