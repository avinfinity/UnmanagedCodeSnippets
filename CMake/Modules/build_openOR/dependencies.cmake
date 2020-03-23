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
openOR_internal_include(detail/dbgMessage.cmake)
openOR_internal_include(detail/error.cmake)

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_add_dependency)
   cmake_parse_arguments(argparse  "" "INTERFACE_TRANSITIVE;TRANSITIVE;PRIVATE" "INTERFACE;IMPLEMENTATION" ${ARGN})

   # get Target related arguments
   if(argparse_PRIVATE)
      set(TargetType PRIVATE)
      set(Target ${argparse_PRIVATE})
   elseif (argparse_TRANSITIVE)
      set(TargetType TRANSITIVE)
      set(Target ${argparse_TRANSITIVE})
   elseif(argparse_INTERFACE_TRANSITIVE)
      set(TargetType INTERFACE_TRANSITIVE)
      set(Target ${argparse_INTERFACE_TRANSITIVE})
   else()
      # the implicit default is PRIVATE
      set(TargetType PRIVATE)
      list(GET argparse_UNPARSED_ARGUMENTS 0 Target)
      list(REMOVE_AT argparse_UNPARSED_ARGUMENTS 0)
   endif()

   # get Dependees related arguments
   if (argparse_IMPLEMENTATION)
      set(IMPLEMENTATION_depends ${argparse_IMPLEMENTATION})
   endif()
   if(argparse_INTERFACE)
      set(INTERFACE_depends ${argparse_INTERFACE})
   endif()
   openOR_internal_append_unique(IMPLEMENTATION_depends ${argparse_UNPARSED_ARGUMENTS})

   if(TargetType STREQUAL "INTERFACE_TRANSITIVE" AND (IMPLEMENTATION_depends))
      message(WARNING "The Target '${Target}' tries to bind a full implementation dependency as INTERFACE_TRANSITIVE.\n"
                      "This is an extremly bad Idea. Headers that need that kind of dependencies do not belong into "
                      "interface category. Move them to EXPORTED_HEADERS and use a TRANSITIVE dependency.\n"
                      "Full implementation dependencies:\n${IMPLEMENTATION_depends}")
   endif()

   openOR_internal_ternary(TP   argparse_TRANSITIVE     "transitive " "")
   openOR_internal_ternary(TP   argparse_INTERFACE_TRANSITIVE "interface " "${TP}")
   openOR_internal_dbgMessage(VERBOSITY 2 "Adding ${TP}dependency to target ${Target}")
   openOR_internal_dbgMessage(VERBOSITY 2 "   \\--> Full Implementation dependencies: ${IMPLEMENTATION_depends}")
   openOR_internal_dbgMessage(VERBOSITY 2 "   \\--> Interface only dependencies     : ${INTERFACE_depends}")
   if (argparse_UNPARSED_ARGUMENTS)
      openOR_internal_dbgMessage(VERBOSITY 3 "   \\--> !! Implicitly Full dependencies : ${argparse_UNPARSED_ARGUMENTS}")
   endif()

   openOR_internal_set_dependencies("${Target}" "IMPLEMENTATION" "${TargetType}" ${IMPLEMENTATION_depends})
   openOR_internal_set_dependencies("${Target}" "INTERFACE" "${TargetType}" ${INTERFACE_depends})
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------

macro(openOR_add_dependency_if Condition)
   if (${Condition})
      openOR_add_dependency(${ARGN})
   endif()
endmacro()

#-------------------------------------------------------------------------------------------------------------------------------

macro(openOR_internal_set_dependencies Target DepType TargetType)
   list(APPEND openOR_internal_dependencylist_${Target}_${DepType}_${TargetType} ${ARGN})
   set(openOR_internal_dependencylist_${Target}_${DepType}_${TargetType}
         ${openOR_internal_dependencylist_${Target}_${DepType}_${TargetType}} PARENT_SCOPE)

   list(APPEND openOR_internal_dependencylist_${Target}_${DepType}_ALLDEPS ${ARGN})
   set(openOR_internal_dependencylist_${Target}_${DepType}_ALLDEPS
         ${openOR_internal_dependencylist_${Target}_${DepType}_ALLDEPS} PARENT_SCOPE)
endmacro()

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_internal_check_dependencies)

   option(openOR_debug_disable_dependency_checking "Dissable checking dependencies for completeness" OFF)
   if (openOR_debug_disable_dependency_checking)
      return()
   endif()

   set(incomplete "")
   set(incomplete_interface "")
   foreach(Target ${openOR_internal_targetlist_ALL})
      openOR_internal_append_unique(incomplete ${openOR_internal_target_${Target}_incomplete_deps})
      openOR_internal_append_unique(incomplete_interface ${openOR_internal_target_${Target}_incomplete_interface_deps})
   endforeach()

   if (incomplete)
      openOR_internal_raise_error(DEPENDENCY_INCOMPLETE  "Dependency resolution incomplete for the targets below:\n"
                                                         "${incomplete}\n"
                                                         "Rerun CMake configure until this error disappears.")
   endif()
   if (incomplete_interface)
      openOR_internal_raise_error(DEPENDENCY_INCOMPLETE  "Interface dependency resolution incomplete for the targets below:\n"
                                                         "${incomplete_interface}\n"
                                                         "Rerun CMake configure until this error disappears.")
   endif()

endfunction()

#-------------------------------------------------------------------------------------------------------------------------------
