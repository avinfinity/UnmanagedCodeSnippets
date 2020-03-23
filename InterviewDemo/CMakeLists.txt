

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

qt5_add_resources( TBRC_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/shaders.qrc ${CMAKE_CURRENT_SOURCE_DIR}/textures.qrc)

qt5_wrap_ui( TB_UI_SRCS mainwindow.ui )

add_executable( trackballdemo geometryengine.cpp main.cpp mainwindow.cpp mainwidget.cpp ${TBRC_SRCS} ${TB_UI_SRCS} )


target_link_libraries( trackballdemo ${QT_LIBRARIES})