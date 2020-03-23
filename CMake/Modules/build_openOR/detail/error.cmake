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
include(${CMAKE_CURRENT_LIST_DIR}/internal.cmake)

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_internal_raise_error Type)
   set(Msg "${ARGN}")
   openOR_internal_dbgMessage("!! ERROR (${Type}): ${Msg}")

   # generate a unique id for this error
   if(NOT openOR_internal_error_current_id)
      set(openOR_internal_error_current_id 0)
   endif()
   math(EXPR openOR_internal_error_current_id ${openOR_internal_error_current_id}+1)
   set(openOR_internal_error_current_id ${openOR_internal_error_current_id} CACHE INTERNAL "ID of the last raised Error")
   openOR_internal_dbgMessage(VERBOSITY 2 " \\-> Error ID = ${openOR_internal_error_current_id}")

   openOR_internal_append_unique(openor_internal_error_queue "ID${openOR_internal_error_current_id}")

   set(openor_internal_error_queue ${openor_internal_error_queue} CACHE INTERNAL "A List of all currently active error ids")
   set(openor_internal_error_queue_ID${openOR_internal_error_current_id}_type ${Type}
                                             CACHE INTERNAL "The Type of the Error with ID ${openOR_internal_error_current_id}")
   set(openor_internal_error_queue_ID${openOR_internal_error_current_id}_msg "${Msg}"
                                          CACHE INTERNAL "The Message of the Error with ID ${openOR_internal_error_current_id}")
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------

function(openOR_internal_handle_errors)

   # Todo: Configurable handling (per Type, error message "intensity", i.e. WARNING, SEND_ERROR, ...)
   foreach(ErrorID ${openor_internal_error_queue})
      set(Type ${openor_internal_error_queue_${ErrorID}_type})
      set(Msg ${openor_internal_error_queue_${ErrorID}_msg})

      message(SEND_ERROR "${Msg}")

      unset(openor_internal_error_queue_${ErrorID}_type CACHE)
      unset(openor_internal_error_queue_${ErrorID}_msg CACHE)
   endforeach()

   # reset list
   unset(openor_internal_error_queue CACHE)
   unset(openOR_internal_error_current_id CACHE)
endfunction()

#-------------------------------------------------------------------------------------------------------------------------------
