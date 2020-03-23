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

#-------------------------------------------------------------------------------------------------------------------------------

# Usage Warning: You can define each Source type once per call. 
function(openOR_source Target)
   set (SOURCE_TYPES "INTERFACE;EXPORTED;INTERNAL;IMPL;RESOURCE")
   set (source_type_INTERFACE_default_name "Interfaces")
   set (source_type_EXPORTED_default_name  "Exported")
   set (source_type_INTERNAL_default_name  "Header")
   set (source_type_IMPL_default_name      "Source")
   set (source_type_RESOURCE_default_name  "Resources")

   cmake_parse_arguments(argparse "" "PATH" "${SOURCE_TYPES}" ${ARGN})

   openOR_internal_ternary(prefix argparse_PATH "${argparse_PATH}/" "")

   openOR_internal_dbgMessage(VERBOSITY 3 "Adding sources to target ${Target}")
   foreach(Type ${SOURCE_TYPES})
      cmake_parse_arguments(listparse "" "NAME;ID" "" ${argparse_${Type}})

      set(SOURCES ${listparse_UNPARSED_ARGUMENTS})
      openOR_internal_ternary(GN listparse_NAME "${listparse_NAME}" "${source_type_${Type}_default_name}")
      openOR_internal_ternary(GID listparse_ID "${listparse_ID}" "${Type}")

      openOR_internal_append_to_source_group(${Target} "${GID}" "${Type}" NAME "${GN}" SOURCES ${SOURCES})
   endforeach()
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_configure Target Name)

   openOR_internal_dbgMessage("Configuring file ${Name} for target ${Target}")
   cmake_parse_arguments(argparse "" "ADD_TO_GROUP_ID;OUTPUT;EXPORT" "" ${ARGN})

   # init some names to sensible defaults (TODO: This could be moved to some init routine)
   set(DGID "zzzzz_openOR_internal_generated")
   set(openOR_internal_sourcelist_name_${Target}_zzzzz_openOR_internal_generated_from "Generated/Template")
   set(openOR_internal_sourcelist_type_${Target}_zzzzz_openOR_internal_generated_from "TEMPLATES")

   openOR_internal_ternary(SGID        argparse_ADD_TO_GROUP_ID "${argparse_ADD_TO_GROUP_ID}" "zzzzz_openOR_internal_generated_from")
   openOR_internal_ternary(Destination argparse_OUTPUT "${argparse_OUTPUT}" "${Name}")
   openOR_internal_ternary(ExportDir   argparse_EXPORT "${argparse_EXPORT}" "")

   # generate the file from its template
   configure_file(${Name}.in ${CMAKE_BINARY_DIR}/generated/${Destination} ESCAPE_QUOTES)

   # make it visible inside this module
   include_directories("${CMAKE_BINARY_DIR}/generated/${ExportDir}")

   # make it visible to depending modules
   if(ExportDir)
      openOR_internal_append_sorted(openOR_internal_export_genreated_${Target} "${CMAKE_BINARY_DIR}/generated/${ExportDir}")
      set(openOR_internal_export_genreated_${Target} ${openOR_internal_export_genreated_${Target}} PARENT_SCOPE)
   endif()

   # Add both source and destination to this modules Sourcelist to make it visible for the user
   openOR_internal_append_to_source_group(${Target} ${SGID} "${openOR_internal_sourcelist_type_${Target}_${SGID}}"
                                                            NAME "${openOR_internal_sourcelist_name_${Target}_${SGID}}"
                                                            SOURCES "${Name}.in")
   openOR_internal_append_to_source_group(${Target} ${DGID} "GENERATED"
                                                            NAME "Generated"
                                                            SOURCES "${CMAKE_BINARY_DIR}/generated/${Destination}")
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------

macro(openOR_internal_append_to_source_group Target GID GroupType)
   cmake_parse_arguments(argparse "" "NAME" "SOURCES" ${ARGN})
   set(_Sources ${argparse_SOURCES})
   set(_GroupName ${argparse_NAME})

   # a list for each individual group ID (for IDE source grouping)
   openOR_internal_append_sorted(openOR_internal_sourcelist_${Target}_${GID} ${_Sources})
   set(openOR_internal_sourcelist_${Target}_${GID} ${openOR_internal_sourcelist_${Target}_${GID}} PARENT_SCOPE)

   # output the group name and type for any given id
   set(openOR_internal_sourcelist_name_${Target}_${GID} "${_GroupName}" PARENT_SCOPE)
   set(openOR_internal_sourcelist_type_${Target}_${GID} "${GroupType}" PARENT_SCOPE)

   # and finaly output a list of all group IDs
   openOR_internal_append_sorted(openOR_internal_sourcelist_gids_${Target} ${GID})
   set(openOR_internal_sourcelist_gids_${Target} ${openOR_internal_sourcelist_gids_${Target}} PARENT_SCOPE)

   unset(_Sources)
   unset(_GroupName)
endmacro()


