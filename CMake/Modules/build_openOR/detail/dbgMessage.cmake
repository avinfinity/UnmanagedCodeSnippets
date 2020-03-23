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

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_internal_init_dbgMessage)
   option(openOR_debug_build_system_output "Output detailed debug messages")
   if (openOR_debug_build_system_output)
      set(openOR_debug_build_system_verbosity "1" CACHE STRING
               "Debug Verbosity Level (higher is more verbose, 1 through 3 is currently used)")
      openOR_internal_dbgMessage("Debug Verbosity = ${openOR_debug_build_system_verbosity}")
   endif()
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_internal_dbgMessage)
   if (openOR_debug_build_system_output)
      cmake_parse_arguments(argparse "" "VERBOSITY" "" ${ARGN})
      set(Message ${argparse_UNPARSED_ARGUMENTS})
      openOR_internal_ternary(Verbosity argparse_VERBOSITY "${argparse_VERBOSITY}" "1")

      if (NOT Verbosity GREATER openOR_debug_build_system_verbosity)
         message("[Debug] ${Message}")
      endif()
   endif()
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------
