#ignore_oiv_runtest()

set(include_SOURCES 
ResourceManagerAuditor.h  externGlobalVariable.h  volRend.h
VVizBenchmark.h           globalVariable.h        volRendAllHeader.h
VolumeVizAuditor.h        resource.h              volRendProfView.h)

set(resources_SOURCES volRendDialog.iv translate1Dragger.iv)
add_executable(volRend
 mainVolVizdemo.cxx
 volRend.cxx
 volRendProfView.cxx
 volumeGeometry.cxx
 VolumeVizAuditor.cxx
 VVizBenchmark.cxx
 volCavelibFuncs.cxx
 ${include_SOURCES}
 ${resources_SOURCES}
)

target_link_libraries(volRend ${DialogVizGui_LIBRARIES})
if(NOT WIN32 AND NOT APPLE)
target_link_libraries(volRend ${X11_X11_LIB})
endif(NOT WIN32 AND NOT APPLE)

#create_vcproj_hierarchy("Resource Files" ./${resources_SOURCES})
