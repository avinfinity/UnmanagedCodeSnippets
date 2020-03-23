


include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/shared/inc
                     ${CMAKE_CURRENT_SOURCE_DIR}/common/inc )

add_subdirectory( shared )

add_library( opencl_shared  shared/src/cmd_arg_reader.cpp shared/src/shrUtils.cpp common/src/oclUtils.cpp )

add_executable( oclmt_demo oclMultiThreads.cpp multithreading.cpp )



target_link_libraries( oclmt_demo ${OpenCL_LIBRARY} opencl_shared )



set_property( TARGET  oclmt_demo PROPERTY FOLDER "Demos/OpenCL")

