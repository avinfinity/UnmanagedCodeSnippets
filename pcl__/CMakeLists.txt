if( APPLE )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++11 -stdlib=libc++" )
endif()
include( ${VTK_USE_FILE} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/common/include
                     ${CMAKE_CURRENT_SOURCE_DIR}/kdtree/include
                     ${CMAKE_CURRENT_SOURCE_DIR}/octree/include
                     ${CMAKE_CURRENT_SOURCE_DIR}/search/include)

add_subdirectory( common )
add_subdirectory( kdtree )
add_subdirectory( octree )
add_subdirectory( search )
add_subdirectory( outofcore )
add_subdirectory( segmentation )
add_subdirectory( surface )
add_subdirectory( filters )
add_subdirectory( features )
add_subdirectory( sample_consensus )
