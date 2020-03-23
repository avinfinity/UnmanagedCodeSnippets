
macro( vcMove binaryname movedirectory )


# if( WIN32 )

# get_target_property( BINARY_NAME ${binaryname}  LOCATION )

add_custom_command( TARGET ${binaryname}  POST_BUILD
                      COMMAND ${CMAKE_COMMAND} -E make_directory ${movedirectory}
                      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${binaryname}> ${movedirectory}  )
					  
					  
# endif()					  
# 					  
endmacro( vcMove )