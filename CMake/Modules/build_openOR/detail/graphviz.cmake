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

# I am aware of the --grapghviz functionality of CMake,
# but it does not discriminate between direct and transitive deps
# which makes the resulting graph much too cluttered.

#-------------------------------------------------------------------------------------------------------------------------------

# Uses following externaly set variables
#   REQUIRED openOR_internal_targetlist_ALL            - a list of all targets build using this build system 
#   RERUIRED openOR_internal_target_${Target}_all_deps - a list of deps for any particular target
#   OPTIONAL openOR_internal_targetlist_[EXTERNAL|EXECUTABLE|HEADER_ONLY|MODULE|LIBRARY]
#                                                      - improves the resulting diagram by reflecting the Types of each Target

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_internal_init_graphviz)
   #option(openOR_create_graphviz "create dependency graph" OFF)
   if (openOR_create_graphviz)
      set(openOR_graphviz_filename "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}_dependencies.grv" CACHE FILEPATH
                                                                                                "File to save dependency graph")

      option(openOR_graphviz_cluster_groups "Cluster Modules acording to their group" OFF)

      option(openOR_graphviz_selected_targets_only "Create the dependency graph for selected targets instead of all" OFF)
      if (openOR_graphviz_selected_targets_only)
         set(openOR_graphviz_targets "MinimalExample;MinimalExampleRenderer" CACHE STRING
                                                                           "Targets for which the dependeny graph gets created")
      endif()

      set(openOR_graphviz_hide_targets "" CACHE STRING "Hide targets from the resulting Graph")
   endif()
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_internal_createGraphviz)

   if (openOR_create_graphviz)
      openOR_internal_dbgMessage("Creating dependency graph (${openOR_graphviz_filename}) ...")

      set(_msg_for "${CMAKE_PROJECT_NAME} v${${CMAKE_PROJECT_NAME}_VERSION} (${${CMAKE_PROJECT_NAME}_BuildID})")
      if (NOT openOR_graphviz_selected_targets_only)
         set(_targetlist_ALL ${openOR_internal_targetlist_ALL})
      else()
         string(REPLACE ";" ", " _tlist "${openOR_graphviz_targets}")
         set(_msg_for "${_tlist} (part of ${_msg_for})")

         openOR_internal_get_all_transitive(_trans ${openOR_graphviz_targets})
         set(_targetlist_ALL ${_trans})
      endif()


      file(WRITE ${openOR_graphviz_filename} "digraph ${CMAKE_PROJECT_NAME} { \n   ratio=0.5;\n")
      file(APPEND ${openOR_graphviz_filename} "   label=\"Dependency Graph for ${_msg_for}\"\n")
      openOR_internal_graphviz_remove_if_hidden(_targetlist_ALL)
      foreach(Target ${_targetlist_ALL})

         # find out what kind of Target we are dealing with
         openOR_internal_set_if_found(${Target} openOR_internal_targetlist_EXTERNAL _node_attribs
                                                                                       "shape=box,color=lightgrey,style=filled")
         openOR_internal_set_if_found(${Target} openOR_internal_targetlist_EXECUTABLE _node_attribs
                                                                                 "shape=hexagon,color=\"#F2DDB6\",style=filled")
         openOR_internal_set_if_found(${Target} openOR_internal_targetlist_HEADER_ONLY _node_attribs
                                                                           "shape=parallelogram,color=\"#87A0A4\",style=filled")
         openOR_internal_set_if_found(${Target} openOR_internal_targetlist_MODULE _node_attribs
                                                                               "shape=trapezium,color=\"#87A0A4\",style=filled")
         openOR_internal_set_if_found(${Target} openOR_internal_targetlist_LIBRARY _node_attribs
                                                                            "shape=invtrapezium,color=\"#87A0A4\",style=filled")

         file(APPEND ${openOR_graphviz_filename} "   ${Target} [${_node_attribs}];\n")

         set(_${Target}_all_deps ${openOR_internal_target_${Target}_all_deps})
         openOR_internal_graphviz_remove_if_hidden(_${Target}_all_deps)
         foreach(_dep ${_${Target}_all_deps})

            # find out what kind of dependency we are dealing with
            set(_dep_color "color=grey")                                                   # by default dependencies are private
            openOR_internal_set_if_found(${_dep} openOR_internal_target_${Target}_transitive_deps _dep_color "color=black")
            openOR_internal_set_if_found(${_dep} openOR_internal_target_${Target}_interface_transitive_deps _dep_color "color=red")

            set(_dep_style "style=filled")                                 # by default we have full implementation dependencies
            openOR_internal_set_if_found(${_dep} openOR_internal_target_${Target}_interface_deps _dep_style "style=dotted")

            set(_dep_attribs "${_dep_color},${_dep_style}")

            file(APPEND ${openOR_graphviz_filename} "      ${Target} -> ${_dep} [${_dep_attribs}];\n")
         endforeach()
      endforeach()

      openOR_internal_graphviz_append_clusters()

      file(APPEND ${openOR_graphviz_filename} "}")
   endif()
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_internal_graphviz_append_clusters)
   if (openOR_graphviz_cluster_groups)

      foreach(_Grp ${openOR_internal_grouplist})
         if (NOT ${_Grp} STREQUAL "BUILD-SYSTEM/ExternalLibraries")
            # TODO: Does not handle SubSubgroups
            string(FIND "${_Grp}" "/" _idx)
            if (NOT _idx EQUAL -1)
               math(EXPR _idxpp "${_idx} + 1")
               string(SUBSTRING "${_Grp}" ${_idxpp} -1 _SubGrp)
               string(SUBSTRING "${_Grp}" 0 ${_idx} _ParentGrp)

               openOR_internal_append_sorted(openOR_internal_grouplist ${_ParentGrp})
               openOR_internal_append_sorted(openOR_internal_grouplist_${_ParentGrp}_targets subgroup_${_SubGrp})
            endif()
         endif()
      endforeach()

      foreach(_Grp ${openOR_internal_grouplist})

         # cluster only existing targets
         unset(_group_targets)
         foreach(_Trgt ${openOR_internal_grouplist_${_Grp}_targets})
            list(FIND _targetlist_ALL "${_Trgt}" _idx)
            if(NOT _idx EQUAL -1)
               list(APPEND _group_targets ${_Trgt})
            endif()
            unset(_idx)

            string(FIND ${_Trgt} "subgroup_" _idx)
            if(NOT _idx EQUAL -1)
               list(APPEND _subgroup_targets_${_Grp} ${_Trgt})
            endif()
         endforeach()

         if (_subgroup_targets_${_Grp} OR _group_targets)
            if (_end)
               file(APPEND ${openOR_graphviz_filename} "   ")
            endif()
            if (${_Grp} STREQUAL "BUILD-SYSTEM/ExternalLibraries")
               set(_Grp "External")
            endif()
            file(APPEND ${openOR_graphviz_filename} "   subgraph \"cluster_${_Grp}\" { label=\"${_Grp}\";")
            if (_group_targets)
               file(APPEND ${openOR_graphviz_filename} " ${_group_targets}")
            endif()
            if(NOT _subgroup_targets_${_Grp})
               file(APPEND ${openOR_graphviz_filename} " }")
            else()
               list(GET _subgroup_targets_${_Grp} -1 _lastValue)
               string(REPLACE "subgroup_" "" _end "${_lastValue}")
            endif()
            file(APPEND ${openOR_graphviz_filename} "\n")
         endif()

         string(FIND "${_Grp}" "/" _idx)
         if (NOT _idx EQUAL -1)
            math(EXPR _idxpp "${_idx} + 1")
            string(SUBSTRING "${_Grp}" ${_idxpp} -1 _SubGrp)

            if(_end STREQUAL _SubGrp)
               file(APPEND ${openOR_graphviz_filename} "   }\n")
               unset(_subgroup_targets)
               unset(_end)
            endif()
         endif()

      endforeach()
   endif()
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_internal_get_all_transitive result)
   set(_args ${ARGN})
   list(GET _args 0 _first)
   if (NOT _first STREQUAL "RECURSE")
      set(_visited "" CACHE INTERNAL "Already visited during recursion")
   else()
      list(REMOVE_AT _args 0)
   endif()

   foreach(_target ${_args})
      list(FIND _visited ${_target} _already_visited)
      if(_already_visited EQUAL -1)
         list(APPEND _visited ${_target})
         set(_visited ${_visited} CACHE INTERNAL "Already visited during recursion")

         openOR_internal_get_all_transitive(_rec_result_${_target} RECURSE ${openOR_internal_target_${_target}_all_deps})
      endif()
   endforeach()

   if (NOT _first STREQUAL "RECURSE")
      list(APPEND ${result} ${_visited})
      set(${result} ${${result}} PARENT_SCOPE)

      unset(_visited CACHE)
   endif()
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_internal_graphviz_remove_if_hidden _List)
   if(${_List} AND openOR_graphviz_hide_targets)
      list(REMOVE_ITEM ${_List} ${openOR_graphviz_hide_targets})
   endif()

   set(${_List} ${${_List}} PARENT_SCOPE)
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_internal_set_if_found Value List Result ResultValue)
   list(FIND ${List} "${Value}" _index)
   if (NOT _index EQUAL -1)
      set(${Result} "${ResultValue}" PARENT_SCOPE)
   endif()
   unset(_index)
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------
