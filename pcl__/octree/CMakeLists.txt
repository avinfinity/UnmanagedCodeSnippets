    set( SUBSYS_NAME "octree" )
    
    set( srcs src/octree_inst.cpp)

    set(incs 
        "include/pcl/${SUBSYS_NAME}/boost.h"
        "include/pcl/${SUBSYS_NAME}/octree_base.h"
        "include/pcl/${SUBSYS_NAME}/octree_container.h"
        "include/pcl/${SUBSYS_NAME}/octree_impl.h"
        "include/pcl/${SUBSYS_NAME}/octree_nodes.h"
        "include/pcl/${SUBSYS_NAME}/octree_key.h"
        "include/pcl/${SUBSYS_NAME}/octree_pointcloud_density.h"
        "include/pcl/${SUBSYS_NAME}/octree_pointcloud_occupancy.h"
        "include/pcl/${SUBSYS_NAME}/octree_pointcloud_singlepoint.h"
        "include/pcl/${SUBSYS_NAME}/octree_pointcloud_pointvector.h"
        "include/pcl/${SUBSYS_NAME}/octree_pointcloud_changedetector.h"
        "include/pcl/${SUBSYS_NAME}/octree_pointcloud_voxelcentroid.h"
        "include/pcl/${SUBSYS_NAME}/octree_pointcloud.h"
        "include/pcl/${SUBSYS_NAME}/octree_iterator.h"
        "include/pcl/${SUBSYS_NAME}/octree_search.h"
        "include/pcl/${SUBSYS_NAME}/octree.h"
        "include/pcl/${SUBSYS_NAME}/octree2buf_base.h"
        "include/pcl/${SUBSYS_NAME}/octree_pointcloud_adjacency.h"
        "include/pcl/${SUBSYS_NAME}/octree_pointcloud_adjacency_container.h"
        )

    set(impl_incs    
        "include/pcl/${SUBSYS_NAME}/impl/octree_base.hpp"
        "include/pcl/${SUBSYS_NAME}/impl/octree_pointcloud.hpp"
        "include/pcl/${SUBSYS_NAME}/impl/octree2buf_base.hpp"
        "include/pcl/${SUBSYS_NAME}/impl/octree_iterator.hpp"
        "include/pcl/${SUBSYS_NAME}/impl/octree_search.hpp"
        "include/pcl/${SUBSYS_NAME}/impl/octree_pointcloud_voxelcentroid.hpp"
        "include/pcl/${SUBSYS_NAME}/impl/octree_pointcloud_adjacency.hpp"
        )

    set(LIB_NAME "pcl_octree")
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
    ADD_LIBRARY("${LIB_NAME}"  ${srcs} ${incs} ${impl_incs}) #"${SUBSYS_NAME}"

    if( APPLE )
    target_link_libraries("${LIB_NAME}" ${VC_APPLE_BASE_LIBRARIES})
    endif()

	set_property( TARGET  ${LIB_NAME} PROPERTY FOLDER "ThirdParty/PCL")