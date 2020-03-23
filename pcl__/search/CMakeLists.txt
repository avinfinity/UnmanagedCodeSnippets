set(SUBSYS_NAME search)

set(srcs
        src/search.cpp
        src/kdtree.cpp
        src/brute_force.cpp
        src/organized.cpp
        src/octree.cpp
        )

    set(incs
        "include/pcl/${SUBSYS_NAME}/search.h"
        "include/pcl/${SUBSYS_NAME}/kdtree.h"
        "include/pcl/${SUBSYS_NAME}/brute_force.h"
        "include/pcl/${SUBSYS_NAME}/organized.h"
        "include/pcl/${SUBSYS_NAME}/octree.h"
        "include/pcl/${SUBSYS_NAME}/flann_search.h"
        "include/pcl/${SUBSYS_NAME}/pcl_search.h"
        )

    set(impl_incs
        "include/pcl/${SUBSYS_NAME}/impl/search.hpp"
        "include/pcl/${SUBSYS_NAME}/impl/kdtree.hpp"
        "include/pcl/${SUBSYS_NAME}/impl/flann_search.hpp"
        "include/pcl/${SUBSYS_NAME}/impl/brute_force.hpp"
        "include/pcl/${SUBSYS_NAME}/impl/organized.hpp"
        )

    set(LIB_NAME "pcl_${SUBSYS_NAME}")
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
    ADD_LIBRARY("${LIB_NAME}"  ${srcs} ${incs} ${impl_incs}) #"${SUBSYS_NAME}"

    if(APPLE)
     target_link_libraries("${LIB_NAME}" pcl_common ${FLANN_LIBRARIES} pcl_octree pcl_kdtree ${VC_APPLE_BASE_LIBRARIES})
    else()
     target_link_libraries("${LIB_NAME}" pcl_common ${FLANN_LIBRARIES} pcl_octree pcl_kdtree)
    endif()


	set_property( TARGET  ${LIB_NAME} PROPERTY FOLDER "ThirdParty/PCL")

