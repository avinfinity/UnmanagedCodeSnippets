include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( "E:/projects/libraries/GMP-MPFR-latest/include" )
link_directories( "E:/projects/libraries/GMP-MPFR-latest/lib" )

add_executable( meshcrosssectiondemo meshcrosssectiondemo.cpp  ) #${VOLUME_RC_SRCS}  ${IMT_WALLTHICKNESS_RC_SRCS} 
add_executable( meshslicedemo meshslicedemo.cpp )
add_executable( implicitsurfacedemo implicitsurfacedemo.cpp )
add_executable( openinventordemo openinventordemo.cpp )


target_link_libraries( meshcrosssectiondemo ${QT_LIBRARIES} ${OpenCV_LIBS} volumebasic ) 

message( STATUS "vtk libraries : ${VTK_LIBRARIES}" )

target_link_libraries( meshslicedemo volumebasic ${QT_LIBRARIES} ${PCL_LIBRARIES} ${CUDA_LIBRARIES}  
                                     ${IMT_EMBREE_LIBRARY} ${VTK_LIBRARIES} tracer gpuslicer )

target_link_libraries( implicitsurfacedemo volumebasic ${QT_LIBRARIES} ${PCL_LIBRARIES} ${CUDA_LIBRARIES}  
                       ${IMT_EMBREE_LIBRARY} ${VTK_LIBRARIES} CGAL_Core-vc120-mt-4.9-I-900 CGAL_ImageIO-vc120-mt-4.9-I-900 
					   CGAL-vc120-mt-4.9-I-900 tracer libgmp-10 libmpfr-4 )
					   
					   
target_link_libraries( openinventordemo ${IMT_OPENINVENTOR_LIBRARIES} openinventorQt )	
	   