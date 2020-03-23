#-------------------------------------------------------------------------------------------------------------------------------
# (c) 2012 by the openOR Team
#-------------------------------------------------------------------------------------------------------------------------------
# The contents of this file are available under the GPL v2.0 license
# or under the openOR comercial license. see 
#   /Doc/openOR_free_license.txt or
#   /Doc/openOR_comercial_license.txt 
# for Details.
#-------------------------------------------------------------------------------------------------------------------------------
cmake_policy(SET CMP0046 NEW)
include(CMakeParseArguments)
include(${CMAKE_CURRENT_LIST_DIR}/detail/include.cmake)

openOR_internal_include(detail/internal.cmake)
openOR_internal_include(detail/error.cmake)
openOR_internal_include(detail/dbgMessage.cmake)

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_create Type Target)
   cmake_parse_arguments(argparse "PUBLIC" "GROUP" "" ${ARGN})
   openOR_internal_ternary(PB argparse_PUBLIC "Public " "")
   message(STATUS "Create target ${Target} as a ${PB}${Type}")

   if(argparse_PUBLIC)
      set(PUBLIC TRUE)
   endif()

   openOR_internal_append_sorted(openOR_internal_targetlist_ALL ${Target})
   set(openOR_internal_targetlist_ALL ${openOR_internal_targetlist_ALL} CACHE INTERNAL "List of all modules in the current build")
   openOR_internal_append_sorted(openOR_internal_targetlist_${Type} ${Target})
   set(openOR_internal_targetlist_${Type} ${openOR_internal_targetlist_${Type}} CACHE INTERNAL "List of all ${Type} modules in the current build")

   # mark target seen
   set(openOR_internal_target_${Target}_seen TRUE CACHE INTERNAL "Target has been visited by CMake")
   set(openOR_internal_target_${Target}_incomplete_deps "")
   set(openOR_internal_target_${Target}_incomplete_interface_deps "")

   foreach(GID ${openOR_internal_sourcelist_gids_${Target}})
      set(SType ${openOR_internal_sourcelist_type_${Target}_${GID}})

      # create a list of all non Resource typed sources
      if (NOT (SType STREQUAL "RESOURCE"))
         # sorting the sources would change the group order in XCode
         openOR_internal_append_unique(Sources ${openOR_internal_sourcelist_${Target}_${GID}})
      else()
         openOR_internal_append_unique(Resources ${openOR_internal_sourcelist_${Target}_${GID}})
      endif()

      # create a list of Headers (for Qt moc-ing)
      if ((SType STREQUAL "EXPORTED") OR (SType STREQUAL "INTERNAL"))
         openOR_internal_append_unique(Headers ${openOR_internal_sourcelist_${Target}_${GID}})
      endif()

      # add source groups for clarity
      if (openOR_internal_sourcelist_${Target}_${GID})
         string(REPLACE "/" "\\" GroupName ${openOR_internal_sourcelist_name_${Target}_${GID}})
         source_group("${GroupName}" FILES ${openOR_internal_sourcelist_${Target}_${GID}})

         openOR_internal_dbgMessage("   \\---> adding group ${GID} '${openOR_internal_sourcelist_name_${Target}_${GID}}'")
         openOR_internal_dbgMessage(VERBOSITY 2 "                   passed Name: '${GroupName}'")
         foreach(Source ${openOR_internal_sourcelist_${Target}_${GID}})
            openOR_internal_dbgMessage("     |      ${prefix}${Source}")
         endforeach()
      endif()
   endforeach()

   # calculate dependencies
   foreach(Dep ${openOR_internal_dependencylist_${Target}_IMPLEMENTATION_ALLDEPS})
      openOR_internal_dbgMessage("   \\---> depends on ${Dep}")

      openOR_internal_gather_deps(tdep ${Dep})
      if(tdep_incomplete)
         openOR_internal_append_unique(openOR_internal_target_${Target}_incomplete_deps ${tdep_incomplete})
      endif()
      if(tdep_incomplete_interface)
         openOR_internal_append_unique(openOR_internal_target_${Target}_incomplete_interface_deps ${tdep_incomplete_interface})
      endif()

      if(tdep_include_dirs)
         openOR_internal_dbgMessage("     |      include dirs = ${tdep_include_dirs}")
         include_directories(${tdep_include_dirs})
      endif()

      if(tdep_link_dirs)
         openOR_internal_dbgMessage("     |      link dirs    = ${tdep_link_dirs}")
         link_directories(${tdep_link_dirs})
      endif()

      if(tdep_link_libs)
         openOR_internal_dbgMessage("     |      link libs    = ${tdep_link_libs}")
         openOR_internal_append_unique_link_targets(to_be_linked ${tdep_link_libs})
      endif()

      # This is not strictly supposed to be here, but there is no better place
      if(Dep STREQUAL "Qt4")
         openOR_internal_dbgMessage("     |   Generating MOC files ...")
         # Todo: check the new set_target_properties(${Target} PROPERTIES AUTOMOC TRUE) feature is better than this
         qt4_wrap_cpp(MOC_Sources ${Headers} OPTIONS -nw -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)
                      # the -nn options stops moc complaining about mocing non Qt files (because Xcode makes an error out of it)
                      # the -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED is a workaround for QTBUG-22829
         set(Sources ${Sources} ${MOC_Sources})
         source_group("Generated\\Qt MOCs" FILES ${MOC_Sources})

         foreach(moc ${MOC_Sources})
            openOR_internal_dbgMessage("     |     |   ${moc}")
         endforeach()
      endif()

   endforeach()

   foreach(Dep ${openOR_internal_dependencylist_${Target}_INTERFACE_ALLDEPS})
      openOR_internal_dbgMessage("   \\---> depends on interface of ${Dep}")

      openOR_internal_gather_deps(tdep ${Dep})
      if(tdep_incomplete)
         openOR_internal_append_unique(openOR_internal_target_${Target}_incomplete_deps ${tdep_incomplete})
      endif()
      if(tdep_incomplete_interface)
         openOR_internal_append_unique(openOR_internal_target_${Target}_incomplete_interface_deps ${tdep_incomplete_interface})
      endif()

      if(tdep_include_dirs)
         openOR_internal_dbgMessage("     |      include dirs = ${tdep_include_dirs}")
         include_directories(${tdep_include_dirs})
      endif()
   endforeach()

   # create a new target of the appropriate type
   if (Type STREQUAL "EXECUTABLE")
      add_executable(${Target} ${Sources})
      set_target_properties(${Target} PROPERTIES OUTPUT_NAME App_${Target})
   endif()
   if (Type STREQUAL "MODULE")
      add_library(${Target} MODULE ${Sources})
   endif()
   if (Type STREQUAL "LIBRARY")
      set(BuildMode "SHARED")
      option(openOR_build_standalone_publics "Build Public Targets in a way that they have linked in all their dependencies" ON)
      if(openOR_build_standalone_publics)
         if (PUBLIC)
            option(openOR_build_static_publics "Build Public Libraries statically" OFF)
            if (openOR_build_static_publics)
               add_definitions("-DOPENOR_BUILD_AS_STATIC") # TODO: stopgap for compatibility remove as soon as we have sth. better
               set(BuildMode "STATIC")
            endif()
         else()
            add_definitions("-DOPENOR_BUILD_AS_STATIC") # TODO: stopgap for compatibility remove as soon as we have sth. better
            set(BuildMode "STATIC")
         endif()
      endif()

      add_library(${Target} ${BuildMode} ${Sources})
      openOR_internal_dbgMessage("   \\---> Library build Mode: ${BuildMode}")
   endif()
   if (Type STREQUAL "HEADER_ONLY")
      add_custom_target(${Target} SOURCES ${Sources})
      set_target_properties(${Target} PROPERTIES LINKER_LANGUAGE CXX)

      # TODO: when the Target is Header-only all dependencies become transitive!
   endif()

   # include own interfaces
   include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

   # export own and own generated interfaces
   set(interfaces "${CMAKE_CURRENT_SOURCE_DIR}/include" "${openOR_internal_export_genreated_${Target}}")

   # add Target to a project folder
   if (argparse_GROUP)
      openOR_project_group(${Target} ${argparse_GROUP})
   endif()

   # we copy resource files to their proper location
   if (Resources)
      openOR_internal_handle_resources(${Target} ${Resources})
   endif()

   # we add the dependencies to the traget, ...
   if (openOR_internal_dependencylist_${Target}_IMPLEMENTATION_ALLDEPS)
      add_dependencies(${Target} ${openOR_internal_dependencylist_${Target}_IMPLEMENTATION_ALLDEPS})
   endif()
   target_link_libraries(${Target} ${to_be_linked})

   # ... figure out which dependencies need to be passed down to our users
   list(APPEND _all_deps ${openOR_internal_dependencylist_${Target}_IMPLEMENTATION_ALLDEPS})
   list(APPEND _all_deps ${openOR_internal_dependencylist_${Target}_INTERFACE_ALLDEPS})

   set(openOR_internal_target_${Target}_all_deps ${_all_deps} CACHE INTERNAL "All deps of ${Target}")
   set(openOR_internal_target_${Target}_transitive_deps ${openOR_internal_dependencylist_${Target}_IMPLEMENTATION_TRANSITIVE}
                                                                                  CACHE INTERNAL "Transitive deps of ${Target}")
   set(openOR_internal_target_${Target}_interface_transitive_deps ${openOR_internal_dependencylist_${Target}_INTERFACE_TRANSITIVE}
                                                                        CACHE INTERNAL "Interface transitive deps of ${Target}")
   set(openOR_internal_target_${Target}_interface_deps ${openOR_internal_dependencylist_${Target}_INTERFACE_ALLDEPS}
                                                                        CACHE INTERNAL "Interface transitive deps of ${Target}")
   set(openOR_internal_target_${Target}_install_deps ${openOR_internal_dependencylist_${Target}_IMPLEMENTATION_PRIVATE}
                                                                                     CACHE INTERNAL "Install deps of ${Target}")
   set(openOR_internal_target_${Target}_incomplete_deps ${openOR_internal_target_${Target}_incomplete_deps}
                                                                  CACHE INTERNAL "List of incompletely configured Dependencies")
   set(openOR_internal_target_${Target}_incomplete_interface_deps ${openOR_internal_target_${Target}_incomplete_interface_deps}
                                                                  CACHE INTERNAL "List of incompletely configured Dependencies")

   # and pass on additional data about this target
   set(openOR_internal_target_${Target}_interfaces ${interfaces} CACHE INTERNAL "Include dir of ${Target}")
   if (NOT Type STREQUAL "HEADER_ONLY")
      set(openOR_internal_target_${Target}_binaries ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "Binary dir of ${Target}")
      set(openOR_internal_target_${Target}_link ${Target} CACHE INTERNAL "What needs to be linked for target ${Target}")
   endif()

   if (NOT openOR_no_tests)
      # Todo: add test runner
   endif()

endfunction()

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_internal_gather_deps OUT_BASE Dep)

   # it doesn't make sense to gather dependncies if the dependency has not been fully constructed yet.
   if(NOT openOR_internal_target_${Dep}_seen)
      set(${OUT_BASE}_incomplete ${Dep} PARENT_SCOPE)

      if(openOR_internal_first_scan_complete)
         # all targets have been visited, but the one that is requested here is still unknown.
         # This is a problem, so we report it.
         openOR_internal_raise_error(DEPENDENCY_MISSING "The Module '${Dep}' does not exist")
      endif()
      return()
   endif()

   if(openOR_internal_target_${Dep}_seen AND openOR_internal_target_${Dep}_incomplete_deps)
      set(${OUT_BASE}_incomplete ${Dep} PARENT_SCOPE)
   endif()
   if(openOR_internal_target_${Dep}_seen AND openOR_internal_target_${Dep}_incomplete_interface_deps)
      set(${OUT_BASE}_incomplete_interface ${Dep} PARENT_SCOPE)
   endif()

   set(${OUT_BASE}_include_dirs ${openOR_internal_target_${Dep}_interfaces})
   set(${OUT_BASE}_link_dirs    ${openOR_internal_target_${Dep}_binaries})
   set(${OUT_BASE}_link_libs    ${openOR_internal_target_${Dep}_link})
   set(${OUT_BASE}_incomplete   "")
   set(${OUT_BASE}_incomplete_interface   "")

   foreach(tdep ${openOR_internal_target_${Dep}_transitive_deps})
      # recurse into dependencies. This might be slow!
      openOR_internal_dbgMessage(VERBOSITY 2 "     Recursivly searching for transitive dep ${tdep}")
      openOR_internal_gather_deps(rdeps_${tdep} ${tdep})
      if(rdeps_${tdep}_incomplete)
         openOR_internal_append_unique(${OUT_BASE}_incomplete ${rdeps_${tdep}_incomplete})
      endif()

      # depencies should be unique but not sorted as the include/link order can be significant
      openOR_internal_append_unique(${OUT_BASE}_include_dirs ${rdeps_${tdep}_include_dirs})
      openOR_internal_append_unique(${OUT_BASE}_link_dirs    ${rdeps_${tdep}_link_dirs})
      openOR_internal_append_unique_link_targets(${OUT_BASE}_link_libs    ${rdeps_${tdep}_link_libs})

      openOR_internal_dbgMessage(VERBOSITY 3 "            \\---> rdepends on ${tdep}")
      if(rdeps_${tdep}_include_dirs)
         openOR_internal_dbgMessage(VERBOSITY 3 "              |      include dirs = ${rdeps_${tdep}_include_dirs}")
      endif()
      if(rdeps_${tdep}_link_dirs)
         openOR_internal_dbgMessage(VERBOSITY 3 "              |      link dirs    = ${rdeps_${tdep}_link_dirs}")
      endif()
      if(rdeps_${tdep}_link_libs)
         openOR_internal_dbgMessage(VERBOSITY 3 "              |      link libs    = ${rdeps_${tdep}_link_libs}")
      endif()
   endforeach()

   foreach(tdep ${openOR_internal_target_${Dep}_interface_transitive_deps})
      # recurse into dependencies. This might be slow!
      openOR_internal_dbgMessage(VERBOSITY 2 "     Recursivly searching for interface transitive dep ${tdep}")
      openOR_internal_gather_deps(rdeps_${tdep} ${tdep})
      if(rdeps_${tdep}_incomplete_interface)
         openOR_internal_append_unique(${OUT_BASE}_incomplete_interface ${rdeps_${tdep}_incomplete_interface})
      endif()

      # depencies should be unique but not sorted as the include/link order can be significant
      openOR_internal_append_unique(${OUT_BASE}_include_dirs ${rdeps_${tdep}_include_dirs})

      openOR_internal_dbgMessage(VERBOSITY 3 "            \\---> rdepends on ${tdep}")
      if(rdeps_${tdep}_include_dirs)
         openOR_internal_dbgMessage(VERBOSITY 3 "              |      include dirs = ${rdeps_${tdep}_include_dirs}")
      endif()
   endforeach()

   foreach(idep ${openOR_internal_target_${Target}_install_deps})

   endforeach()

   set(${OUT_BASE}_include_dirs ${${OUT_BASE}_include_dirs} PARENT_SCOPE)
   set(${OUT_BASE}_link_dirs    ${${OUT_BASE}_link_dirs}    PARENT_SCOPE)
   set(${OUT_BASE}_link_libs    ${${OUT_BASE}_link_libs}    PARENT_SCOPE)
   set(${OUT_BASE}_incomplete   ${${OUT_BASE}_incomplete}   PARENT_SCOPE)
   set(${OUT_BASE}_incomplete_interface   ${${OUT_BASE}_incomplete_interface}   PARENT_SCOPE)
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------

macro(openOR_internal_append_unique_link_targets list)
   openOR_internal_dbgMessage(VERBOSITY 3 "List ${list} = ${${list}} + ${ARGN}")
   list(APPEND ${list} ${ARGN})


   # TODO: unique that works with cmakes optimized/debug
endmacro()

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_internal_handle_resources Target)
   openOR_internal_dbgMessage("   \\---> handling Resource files")
   set(_Resources ${ARGN})
   set(_ResourceBaseDir "${Target}") # Todo: make this externaly configurable

   # copy resources to the binary dir, so that build executables find their resources
   set(_srcPath "${CMAKE_CURRENT_SOURCE_DIR}")
   set(_dstPath "${openOR_internal_maketime_RUNTIME_OUTPUT_DIRECTORY}/${_ResourceBaseDir}")

   foreach(File ${_Resources})
      set(_src ${_srcPath}/${File})
      set(_dst ${_dstPath}/${File})

      openOR_internal_dbgMessage(VERBOSITY 2 "       |      'Copying ${_src} to ${_dst}'")
      add_custom_command(
         TARGET ${Target} POST_BUILD
         COMMAND ${CMAKE_COMMAND}
         ARGS    -E copy_if_different ${_src} ${_dst}
         COMMENT "Copying ${_src} to ${_dst}"
      )
   endforeach()

   # Todo: add installation handling

endfunction()

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_project_group Target Group)
   openOR_internal_dbgMessage("   \\---> added to group '${Group}'")

   set_property(TARGET ${Target} PROPERTY FOLDER "${Group}")

   string(REPLACE " " "" Group "${Group}")

   openOR_internal_append_sorted(openOR_internal_grouplist ${Group})
   openOR_internal_append_sorted(openOR_internal_grouplist_${Group}_targets ${Target})

   set(openOR_internal_grouplist ${openOR_internal_grouplist} CACHE INTERNAL "All target groups in the Build")
   set(openOR_internal_grouplist_${Group}_targets ${openOR_internal_grouplist_${Group}_targets}
                                                                        CACHE INTERNAL "All targets that are in Group ${Group}")
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------
