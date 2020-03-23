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

openOR_internal_include(detail/dbgMessage.cmake)

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_internal_init_install)
   option(openOR_install "Create installation target" OFF)
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_create_install)
   openOR_internal_dbgMessage("Creating installer ...")
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------
