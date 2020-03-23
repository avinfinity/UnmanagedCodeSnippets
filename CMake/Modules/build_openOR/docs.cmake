#-------------------------------------------------------------------------------------------------------------------------------
# (c) 2012 by the openOR Team
#-------------------------------------------------------------------------------------------------------------------------------
# The contents of this file are available under the GPL v2.0 license
# or under the openOR comercial license. see 
#   /Doc/openOR_free_license.txt or
#   /Doc/openOR_comercial_license.txt 
# for Details.
#-------------------------------------------------------------------------------------------------------------------------------

include(${CMAKE_CURRENT_LIST_DIR}/detail/include.cmake)
openOR_internal_include(detail/graphviz.cmake)

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_internal_init_docs)
   option(openOR_no_docs "Skip build Documentation" OFF)
   openOR_internal_init_graphviz()
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_build_docs)
   if (NOT openOR_no_docs)
      openOR_internal_dbgMessage("Creating API docs ...")
      
      # TODO
      
      openOR_internal_createGraphviz()
   endif()
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------
