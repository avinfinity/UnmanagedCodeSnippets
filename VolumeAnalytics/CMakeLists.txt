
set(VC_ISPC_PATH  "D:/Projects/Libraries/Installs/ispc/ispc.exe")

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})


add_custom_command(OUTPUT volume_ispc.o
                   COMMAND ${VC_ISPC_PATH} --target=avx2 --arch=x86-64 --addressing=64 -O2  ${CMAKE_CURRENT_SOURCE_DIR}/volume.ispc -o volume_ispc.o -h volume_ispc.h
                   DEPENDS volume.ispc)


add_library( volumeanalytics SHARED volumeanalytics.cpp histogramprocessor.cpp  )

add_library( volumerenderingcpu  TransferFunction.cpp volumerenderercpu.cpp 
                                 volume_serial.cpp tasksys.cpp volume_ispc.o 
								 volumerenderingopengl.cpp )

add_executable( volumeanalyticsdemo volumeanalyticsdemo.cpp  )


target_link_libraries( volumeanalytics ${OpenCV_LIBS}  ${IMT_IPP_LIBRARIES} ZeissSegmentationInterface)

target_link_libraries( volumeanalyticsdemo volumeanalytics ${VTK_LIBRARIES}  )

set_property( TARGET volumeanalytics PROPERTY FOLDER "Libs" )