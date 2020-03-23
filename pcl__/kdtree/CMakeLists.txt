
set( SUBSYS_NAME "kdtree" )

    set(srcs 
        src/kdtree_flann.cpp
        )

    set(incs 
        "include/pcl/${SUBSYS_NAME}/kdtree.h"
        "include/pcl/${SUBSYS_NAME}/io.h"
        "include/pcl/${SUBSYS_NAME}/flann.h"
        "include/pcl/${SUBSYS_NAME}/kdtree_flann.h"
        )

    set(impl_incs 
        "include/pcl/${SUBSYS_NAME}/impl/io.hpp"
        "include/pcl/${SUBSYS_NAME}/impl/kdtree_flann.hpp"
        )

    set( LIB_NAME "pcl_kdtree" )
    
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
    
    ADD_LIBRARY("${LIB_NAME}"  ${srcs} ${incs} ${impl_incs}) #"${SUBSYS_NAME}"


    
    if( UNIX AND NOT APPLE)
    target_link_libraries("${LIB_NAME}" pcl_common ${FLANN_LIBRARIES} lz4)
    endif()

    if( APPLE )
    #message( STATUS " lz4 library dir : ${VC_LZ4_LIB_DIR} " )
    LINK_DIRECTORIES( /opt/local/lib )
    target_link_libraries("${LIB_NAME}" pcl_common ${FLANN_LIBRARIES} ${VC_APPLE_BASE_LIBRARIES})
    endif()

    if( WIN32 )
    target_link_libraries("${LIB_NAME}" pcl_common ${FLANN_LIBRARY})
    endif()

	set_property( TARGET  ${LIB_NAME} PROPERTY FOLDER "ThirdParty/PCL")