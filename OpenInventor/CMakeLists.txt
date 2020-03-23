#project(QtViewerComponents)

#OIV_QT_INCLUDE_DIRS()

include_directories(
 # ${CMAKE_SOURCE_DIR}/Inventor/gui/ViewerComponents
 # ${CMAKE_BINARY_DIR}/Inventor/gui/ViewerComponents
 
 ${CMAKE_CURRENT_SOURCE_DIR}
 ${CMAKE_CURRENT_BINARY_DIR}
)

add_subdirectory( ViewerComponents )
add_subdirectory( VolRend )

set(source_files
  RenderAreaExaminer.cxx
  RenderAreaExaminer.h
  RenderAreaInteractive.cxx
  RenderAreaInteractive.h
  RenderArea.cxx
  RenderArea.h
  QEventToSoEvent.cxx
  QEventToSoEvent.h
  QtTimer.h
  QtTimer.cxx
  #QtCustomViewer.cxx
)

#OIV_QT_WRAP_CPP ( MOC_RENDERAREA_SOURCES RenderAreaInteractive.h QtTimer.h )

#add_oiv_library(${PROJECT_NAME} STATIC ${source_files} ${MOC_RENDERAREA_SOURCES})
#set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "D")

#target_link_libraries(${PROJECT_NAME} ViewerComponents ${OpenInventor_LIBRARIES} ${QT_LIBRARIES})

add_library( openinventorQt ${source_files} )


add_executable(  oiqtcustomviewer oiqtcustomviewer.cpp )

target_link_libraries( openinventorQt ${QT_LIBRARIES} oiviewercomponents ${IMT_OPENINVENTOR_LIBRARIES} )

target_link_libraries( oiqtcustomviewer  openinventorQt ${QT_LIBRARIES} oiviewercomponents ${IMT_OPENINVENTOR_LIBRARIES} )