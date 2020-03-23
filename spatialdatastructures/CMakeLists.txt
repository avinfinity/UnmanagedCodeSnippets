
add_library( spatialdatastructure hapalaaccelerator.cpp dynamicbvh.cpp  editablevertex.h ) #vbomanager.cpp octree.cpp   octree.h 

if( APPLE )
target_link_libraries( spatialdatastructure ${TBB_LIBRARIES}  tinyobj )
else()
  target_link_libraries( spatialdatastructure ${TBB_LIBRARIES}  tinyobj )#${OPEN_MESH_LIBRARIES}

endif()
if( WIN32 )
set_property( TARGET spatialdatastructure PROPERTY FOLDER "Libs")
endif()
