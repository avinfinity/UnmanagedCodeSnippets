

cmake_minimum_required( VERSION 3.1)

cmake_policy(VERSION 3.1)

project(Wallthickness)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(utilities.cmake)



set_property( GLOBAL PROPERTY USE_FOLDERS ON)


set( EIGEN_INCLUDE_DIR "C:/projects/libraries/installs/Eigen/include/eigen3" CACHE PATH "set eigen include dir" )

SET( OpenCV_LIBRARY_MODE "SHARED" CACHE STRING "Select opencv library mode" )

SET_PROPERTY( CACHE OpenCV_LIBRARY_MODE PROPERTY STRINGS "STATIC" "SHARED")


find_package(OpenCV)

find_package( CUDA )

find_package( OpenCL )

include_directories( "C:/ProgramData/NVIDIA Corporation/CUDA Samples/v9.1/common/inc" )
link_directories( "C:/ProgramData/NVIDIA Corporation/CUDA Samples/v9.1/common/lib/x64" )

CUDA_ADD_LIBRARY( volumebasic_cu volumeRender_kernel.cu )	
CUDA_ADD_LIBRARY( gpuslicer planeslicer.cu ) 

set(LIBRARY_OUTPUT_PATH CACHE PATH "set library output path")
set(EXECUTABLE_OUTPUT_PATH CACHE PATH "set executable output path")


MESSAGE( STATUS " cuda libraries "${CUDA_LIBRARIES} )

target_link_libraries( volumebasic_cu ${CUDA_LIBRARIES} cuda )

find_package(Qt5 COMPONENTS Widgets OpenGL Core Gui DataVisualization Xml)

set(CMAKE_AUTOMOC ON)

find_package(OpenMP)



include_directories( ${CMAKE_CURRENT_BINARY_DIR} )



IF(OPENMP_FOUND)
   MESSAGE("OPENMP FOUND")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  # SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_C_FLAGS}" )
ENDIF()



 





if(OpenCV_FOUND)

message(STATUS "opencv include dirs : ${OpenCV_INCLUDE_DIRS}")

 include_directories(${OpenCV_INCLUDE_DIRS})

 else()

 message(STATUS "opencv not found")

endif(OpenCV_FOUND)


set( QT_INCLUDES
                ${Qt5Widgets_INCLUDE_DIRS}
                ${Qt5Core_INCLUDE_DIRS}
                ${Qt5MultimediaWidgets_INCLUDE_DIRS}
                ${Qt5OpenGL_INCLUDE_DIRS}
                ${Qt5Multimedia_INCLUDE_DIRS}
                ${Qt5Multimedia_INCLUDE_DIR}
		        ${Qt5Gui_INCLUDE_DIRS}
                ${Qt5Xml_INCLUDE_DIRS}
				${Qt5Svg_INCLUDE_DIRS}

 )
 
 
 
  set ( QT_LIBRARIES ${Qt5Widgets_LIBRARIES} 
                     ${Qt5Core_LIBRARIES}
                     ${Qt5OpenGL_LIBRARIES}
                     ${Qt5Gui_LIBRARIES}		
					 ${Qt5Xml_LIBRARIES}	
                     ${Qt5Svg_LIBRARIES}					   
                     ${Qt5Multimedia_LIBRARIES}
					 ${Qt5DataVisualization_LIBRARIES}
		)



		


  qt5_add_resources( VOLUME_RC_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/volumetric.qrc )

  qt5_wrap_ui( VOLUME_UI_SRCS wta.ui input.ui )
  
  set( CUDA_HELPER_DIR "C:/ProgramData/NVIDIA Corporation/CUDA Samples/v7.5/common" CACHE PATH "set cuda helper dir path" )
  
  set( IMT_FRAUNHOFER_BASE_DIR "G:/Projects/CaligoLibs/Libs/VolumeSegmentation/Release" CACHE PATH "set fraunhofer include dir" )
  set( IMT_FRAUNHOFER_LIBRARY_DIR "G:/Projects/CaligoLibs/Libs/VolumeSegmentation/Release" CACHE PATH "set fraunhofer library dir" )
  set( IMT_FRAUNHOFER_LIBRARIES  ZeissSegmentationInterface )
  
  include_directories( 
                       ${CMAKE_CURRENT_SOURCE_DIR} 
                       ${CMAKE_CURRENT_SOURCE_DIR}/pcl/common 
					   ${CUDA_TOOLKIT_ROOT_DIR}/include
					   ${CUDA_HELPER_DIR}/inc 
					   ${OpenCL_INCLUDE_DIR}
					   ${IMT_FRAUNHOFER_BASE_DIR}
					   ${IMT_FRAUNHOFER_BASE_DIR}/src/plugins/Application/Zeiss
					   ${IMT_FRAUNHOFER_BASE_DIR}/src/plugins/Application/backend/include
					   ${IMT_FRAUNHOFER_BASE_DIR}/src/plugins/Image
					   ${IMT_FRAUNHOFER_BASE_DIR}/src/plugins/Image/ImageIO/base/include
					   ${IMT_FRAUNHOFER_BASE_DIR}/src/plugins/Image/Regions/include
					   ${IMT_FRAUNHOFER_BASE_DIR}/src/plugins/Image/Utility/include
					   ${IMT_FRAUNHOFER_BASE_DIR}/src/plugins/Image/ImageData/include
					   ${IMT_FRAUNHOFER_BASE_DIR}/src/plugins/Infrastructure/Basic/include
					   ${IMT_FRAUNHOFER_BASE_DIR}/src/plugins/Application/Zeiss/ImageIO/include
					   ${IMT_FRAUNHOFER_BASE_DIR}/src/plugins/Application/Zeiss/backend/include
					   ${IMT_FRAUNHOFER_BASE_DIR}/src/core/include
					   
					   
					 )
					 
					 
add_subdirectory( cuda )

 add_definitions( -DROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}" )					 
  
  
 set( IMT_BOOST_SEARCH_INLUDE_PATH "C:/local/boost_1_60_0" CACHE PATH "set boost search include path" )
 set( IMT_BOOST_SEARCH_LIBRARY_PATH "C:/local/boost_1_60_0/lib64-msvc-12.0" CACHE PATH "set boost library search path" )
 set( IMT_EMBREE_DIR "C:/Program Files/Intel/Embree v2.10.0 x64" CACHE PATH "set embree dir" )
 set( IMT_OPTIX_DIR "C:/ProgramData/NVIDIA Corporation/OptiX SDK 4.0.0" CACHE PATH "set optix dir" )
 set( IMT_OPENMESH_INCLUDE_DIR "C:/projects/libraries/installs/OpenMesh 6.2/include" CACHE PATH "set openmesh include dir")
 set( IMT_OPENMESH_LIBRARY_DIR "C:/projects/libraries/installs/OpenMesh 6.2/lib" CACHE PATH "set openmesh library dir")
 set( IMT_OPENMESH_LIBRARIES OpenMeshCore OpenMeshTools )
 set( IMT_CGAL_DIR "C:/projects/libraries/installs/CGAL" CACHE PATH "set cgal dir path" )
 set( IMT_IPP_DIR "C:/Program Files (x86)/Intel/Composer XE 2015/ipp" CACHE PATH "set ipp dir" )
 
 
 SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${IMT_BOOST_SEARCH_INLUDE_PATH}")
 SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${IMT_BOOST_SEARCH_LIBRARY_PATH}")
 
 find_package( Boost REQUIRED system filesystem thread date_time program_options )

 message( STATUS "boost library dir : ${Boost_LIBRARY_DIR}" )

 link_directories( ${Boost_LIBRARY_DIR} ${IMT_BOOST_SEARCH_LIBRARY_PATH} ${IMT_FRAUNHOFER_LIBRARY_DIR}  )
 
# find_package( FLANN )
  
 set( FLANN_INCLUDE_DIR  "E:/Projects/Libraries/Installs/flann/include" )
 set( FLANN_LIBRARY_DIR "E:/Projects/Libraries/Installs/flann/lib" )
 set( FLANN_LIBRARY flann_s.lib ) #flann_cuda_s.lib
 set( IMT_IPP_LIBRARIES  
 
 ippcoreem64tl.lib
ippiemergedem64t.lib
ippimergedem64t.lib
ippremergedem64t.lib
ipprmergedem64t.lib
ippsemergedem64t.lib
ippsmergedem64t.lib
ippsremergedem64t.lib
ippsrmergedem64t.lib
 
#ippcore_t.lib
#ippiemerged_.lib
#ippimerged.lib
#ippremerged_.lib
#ipprmerged.lib
#ippsemerged_.lib
#ippsmerged.lib
#ippsremerged_.lib
#ippsrmerged.lib
 )

 #ippcc.lib ippccmt.lib ippch.lib ippchmt.lib ippcore.lib
                        #ippcoremt.lib ippcore_tl.lib ippcv.lib ippcvmt.lib ippdc.lib ippdcmt.lib
                        #ippi.lib ippimt.lib ippi_tl.lib ipps.lib ippsmt.lib ippvm.lib ippvmmt.lib ipp_iw.lib


	

SET( VC_VTK_VERSION "6.2" CACHE STRING "Select VTK Version" )

SET_PROPERTY( CACHE VC_VTK_VERSION PROPERTY STRINGS "6.1" "6.2" "8.1" )

find_package( VTK ${VC_VTK_VERSION} REQUIRED NO_MODULE )

include(${VTK_USE_FILE})


 # find_package( ITK )

  #if( ITK_FOUND )

  #include( ${ITK_USE_FILE} )

  #endif()
 


 message( STATUS "boost include dir : ${Boost_INCLUDE_DIR}" )
 
 set( PCL_LIBRARIES pcl_common pcl_features pcl_filters pcl_kdtree pcl_octree pcl_outofcore pcl_sample_consensus pcl_search pcl_segmentation pcl_surface )


	
		
 include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${QT_INCLUDES} ${EIGEN_INCLUDE_DIR}
                      ${Boost_INCLUDE_DIR} ${FLANN_INCLUDE_DIR} ${IMT_EMBREE_DIR}/include
					  ${IMT_OPENMESH_INCLUDE_DIR}
					  ${IMT_CGAL_DIR}/include 
					  ${IMT_IPP_DIR}/include
					  ${IMT_OPTIX_DIR}/include
					  ${CMAKE_CURRENT_SOURCE_DIR}/openCL/common/inc
					  "C:/projects/CT Analysis/NG.Metrotom/Zeiss.IMT.NG.Mterotom.WallthicknessNative"
					  "C:/projects/Production/VolumeProcessing/CTEvaluation"
					  "C:/projects/Production/Core/Shared Files/loXX/Include"
					  )
 
 link_directories( 
                   ${FLANN_LIBRARY_DIR} 
                   ${CUDA_TOOLKIT_ROOT_DIR}/lib/x64 
				   ${CUDA_HELPER_DIR}/lib/x64
                   ${IMT_EMBREE_DIR}/lib
				   ${IMT_OPTIX_DIR}/lib64
				   ${IMT_OPENMESH_LIBRARY_DIR}
				   ${IMT_CGAL_DIR}/lib 
				   ${IMT_IPP_DIR}/lib 
				   "C:/projects/Production/VolumeProcessing/bin/x64/Release"
				   "C:/projects/Production/Core/Shared Files/loXX/bin/x64/Release"
				 )
				   
set( IMT_CGAL_LIBRARIES CGAL_Core-vc120-mt-4.9-I-900 CGAL_ImageIO-vc120-mt-4.9-I-900 CGAL-vc120-mt-4.9-I-900 )
				   
set( IMT_EMBREE_LIBRARY embree )	

add_definitions( -D_USE_MATH_DEFINES )			   
 
 
qt5_add_resources( IMT_WALLTHICKNESS_RC_SRCS shaders/shaders.qrc )
 
  #add_subdirectory( pcl )
  add_subdirectory( PoissonRecon2 )
  
  add_subdirectory( spatialdatastructures )
  add_subdirectory( tracer )
  add_subdirectory( NeoInsights )
  add_subdirectory( OpenCL )
  add_subdirectory( maxflow )
  #add_subdirectory( ctevaluation ) 
  
  set( IMT_OPENINVENTOR_INCLUDE_DIR "C:/projects/Libs/Open Inventor/OIV9.6.2/include" CACHE PATH "set openinventor include dir" )
  set( IMT_OPENINVENTOR_LIBRARY_DIR "C:/projects/Libs/Open Inventor/OIV9.6.2/arch-Windows-x86_64-msvc12-Release/lib" CACHE PATH "set openinventor library dir" )
  #add_subdirectory( OpenMesh )
  set( IMT_CMATH_INCLUDE_DIR "G:/Projects/Libraries/Installs/CMath" CACHE PATH "set openinventor include dir" )
  set( IMT_CMATH_LIBRARY_DIR "G:/Projects/Libraries/Installs/CMath" CACHE PATH "set openinventor library dir" )

  set( IMT_PCP_INCLUDE_DIR "G:/Projects/Libraries/Installs/PCP" CACHE PATH "set openinventor include dir" )
  set( IMT_PCP_LIBRARY_DIR "G:/Projects/Libraries/Installs/PCP" CACHE PATH "set openinventor library dir" )

  set( IMT_NUMERICS_INCLUDE_DIR "G:/Projects/Libraries/Installs/Numerics" CACHE PATH "set openinventor include dir" )
  set( IMT_NUMERICS_LIBRARY_DIR "G:/Projects/Libraries/Installs/Numerics" CACHE PATH "set openinventor library dir" )
  
  include_directories( ${IMT_OPENINVENTOR_INCLUDE_DIR}
  "C:/projects/libraries/installs/MSVC14/x64/CGAL-4.9/auxiliary/gmp/include"
  "C:/projects/Production/CAD/Zeiss.IMT.CAD.Viewer/Include"
  ${IMT_CMATH_INCLUDE_DIR}  ${IMT_PCP_INCLUDE_DIR} ${IMT_NUMERICS_INCLUDE_DIR})


  link_directories( ${IMT_OPENINVENTOR_LIBRARY_DIR}
  ${IMT_IPP_DIR}/lib
  "C:/projects/libraries/installs/MSVC14/x64/CGAL-4.9/auxiliary/gmp/lib"
  "C:/projects/Production/CAD/Bin/x64/Release"
   
   ${IMT_CMATH_LIBRARY_DIR}
   ${IMT_PCP_LIBRARY_DIR}
   ${IMT_NUMERICS_LIBRARY_DIR}
   )



  set( IMT_OPENINVENTOR_LIBRARIES 
ImageViz.lib
Inventor.lib
InventorAlgorithms.lib
InventorDialogs.lib
InventorGL.lib
InventorGLWrapper.lib
IvGenericFileReader.lib
LDM.lib
VolumeViz.lib)


set( IMT_CAD_LIBS Zeiss.IMT.CAD.Viewer.lib )

add_subdirectory( OpenInventor ) 
add_subdirectory( demos )
#add_subdirectory( InterviewDemo )
add_subdirectory( IsoSurfaceExtraction )

 
 add_library( volumebasic  rawvolumedataio.cpp wallthicknessestimator.cpp  volumeinfo.cpp volumeutility.cpp NeoInsightsLDMReader.cpp outofcoremarchingcubes.cpp
              display3droutines.cpp aabbtree.cpp lineiterator.cpp slicerendermanager.cpp volumeconverter.cpp NeoInsightsLDMWriter.cpp wallthicknessslicer.cpp
			  trackballcamera.cpp openglhelper.cpp volumerendermanager.cpp implicitsurfacegenerator.cpp volumetopooctree.cpp optimizedzxogenerator.cpp pointcloudsearchoctree.cpp mappedmemoryio.cpp
			  rendersurface.cpp mainwindow.cpp volumeloaderthread.cpp histogramfilter.cpp  volumesegmenter.cpp ZXOGenerator.cpp optimizedzxoreader.cpp tileweightheap.cpp
			  poissonmeshgenerator.cpp wtestimatorthread.cpp progressreporter.cpp slicerplane.cpp meshutility.cpp customtilemanager.cpp customnodefrontmanager.cpp customtilevisitor.cpp imagereslicer.cpp
			  vtkModifiedCutter.cxx vtkOptimizedCutter.cpp vtkContourHelperCustom.cxx segmentationbenchmark.cpp CustomTiledReader.cpp volumedata.cpp LRUCachedZXOReader.cpp
			  planeslicercpu.cpp vtkSinglePrecisionCutter.cpp cpuslicer.cpp tilecachemanager.cpp anglesearchtree.cpp overlappedvoxelsmarchingcubes.cpp ZxoWriter.cpp 
              vtkImageReslice2.cxx gridhierarchytree.cpp 
			  OnlineTiledReader.cpp OnlineTilesGenerator.cpp meshviewer.cpp simplemesh.cpp FullBlockVolumeDataProvider.cpp spheregeometry.cpp CPUBuffer.cpp
			   dualmarchingcubes.cpp marchingcubesmultimaterial.cpp multiresmarchingcubes.cpp multimaterialsurfaceextractor.cpp MultiResolutionMarchingCubes.cpp MultiResolutionMarchingCubes2.cpp
			  SobelGradientOperator.cpp MarchingCubesLookupTable.cpp halfedgemesh.cpp gradientbasedsurfaceextractor.cpp  ${VOLUME_UI_SRCS})
			  
			  #marchingcubes.cpp SphereCalculator.cpp VolumeCachedDataReader.cpp Uint16SCVTiledReader.cpp  FullInMemoryVolumeDataProvider.cpp TileCachedVolumeDataProvider.cpp 
		  #planeslicercuda.cpp 
			  
			  
			   link_directories("C:/projects/ZeissSeparation_update_2016-12-09/source code v03/build/vs2013/x64/Release/lib")


 add_subdirectory(VolumeAnalytics)

 
 add_executable( wallthicknessapp  wallthicknessapp.cpp volumevisualizer.cpp   ${VOLUME_RC_SRCS} ${IMT_WALLTHICKNESS_RC_SRCS} )

 add_executable( rawvolumedataiodemo rawvolumedataiodemo.cpp )
 
 add_executable( wallthicknessestimatordemo wallthicknessestimatordemo.cpp )
 
 add_executable( histogramfilterdemo histogramfilterdemo.cpp )
 
 add_executable( isosurfaceextractiondemo isosurfaceextractiondemo.cpp )
 
 add_executable( marchingcubescudademo marchingcubescudademo.cpp  )

 add_executable( wtadatasetgeneratordemo wtadatasetgeneratordemo.cpp  )

 add_executable( brepwallthicknessestimatordemo brepwallthicknessestimatordemo.cpp )
 
 add_executable( volumesegmenterdemo volumesegmenterdemo.cpp )
 
 add_executable( wallthickness2dslicedemo wallthickness2dslicedemo.cpp )
 
 add_executable( syntheticdatawallthicknessdemo syntheticdatawallthicknessdemo.cpp  )
 
 add_executable( meshcomparisondemo meshcomparisondemo.cpp )
 
 add_executable( functionaltestingfraunhofer functionaltestingfraunhofer.cpp )
 
 add_executable( customldmdemo customldmdemo.cpp  )

 add_executable( optimizedzxogeneratordemo optimizedzxogeneratordemo.cpp  )
 
 add_executable( oclmarchingcubesdemo oclmarchingcubesdemo.cpp )
 
 add_executable( xraydataloaderdemo xraydataloaderdemo.cpp  )
 
 add_executable( airfilteredvolumerenderingdemo airfilteredvolumerenderingdemo.cpp  )

 add_executable( volumerenderingcpudemo volumerenderingcpudemo.cpp )
 
 add_executable( variableisooptimization variableisooptimizationdemo.cpp  )
 
 add_executable( boxplaneintersectiondemo boxplaneintersectiondemo.cpp boxplaneintersection.cpp )

 add_executable( searchangleoptimizationdemo searchangleoptimizationdemo.cpp   )
 
 add_executable( multigpurenderingdemo multigpurenderingdemo.cpp )
 
 
 add_executable( oivrenderdemo oivrenderdemo.cpp viewerwidget.cpp optimizedzxoreader.cpp
 oivvolumenode.cpp trackballcamera.cpp volumetopooctree.cpp rendertimer.cpp )
 
 add_executable( ohtake_mminterface ohtake_mminterface.cpp GCoptimization.cpp LinkedBlockList.cpp )
 
 add_executable( variableisolabelingdemo  variableisolabelingdemo.cpp overlappedvoxelsegmentation.cpp )
 
 add_executable( overlappedsubvoxellebelingdemo overlappedsubvoxellebelingdemo.cpp )
 
 add_executable( mappedmemoryiodemo mappedmemoryiodemo.cpp )
 
 add_executable( volumeresizedemo volumeresizedemo.cpp ) 

 add_executable( dualmarchingcubesdemo dualmarchingcubesdemo.cpp volumesigneddistancefunction.cpp )
 
 add_executable( adaptivesegmentationdemo Segmentation/Operators.cpp #Segmentation/display3droutines.cpp 
                                          Segmentation/OverlapCanny.cxx Segmentation/overlappedvoxelsmarchingcubes.cpp 
										  Segmentation/utils.cpp rawvolumedataio.cpp volumeinfo.cpp)


 add_executable( multiresmarchingcubesdemo multiresmarchingcubesdemo.cpp  )

 add_executable( volumeisothresholdcomputationdemo volumethresholdcomputationdemo.cpp )

										  
										  
										  
add_executable( pointpicking3ddemo pointpicking3ddemo.cpp )
										  
										  
										  
 add_executable( pointpicking2ddemo pointpicking2ddemo.cpp  ) 		
 
 add_executable( multimaterialsurfaceextractiondemo multimaterialsurfaceextractiondemo.cpp )
 
 add_executable( volumeslicingdemo volumeslicingdemo.cpp) 

 add_executable( volumerenderingdemo volumerenderingdemo.cpp )
 
 add_executable( roidatademo roidatademo.cpp )
 
 add_executable( automaticalignmentdemo automaticalignmentdemo.cpp )


 target_link_libraries( volumebasic ${VTK_LIBRARIES} ${OpenCV_LIBS} PoissonRecon2 ${IMT_EMBREE_LIBRARY}  
  ZeissSegmentationInterface  ${ITK_LIBRARIES} GLU32 OpenGL32 
   ${CUDA_cudart_static_LIBRARY} ${CUDA_CUDA_LIBRARY}  ${IMT_IPP_LIBRARIES} #${IMT_OPENMESH_LIBRARIES} 

#  ippcore_t.lib
#ippiemerged_.lib
#ippimerged.lib
#ippremerged_.lib
#ipprmerged.lib
#ippsemerged_.lib
#ippsmerged.lib
#ippsremerged_.lib
#ippsrmerged.lib
  #gpuslicer
  )## ZeissSeparationInterface ${PCL_LIBRARIES} Zeiss.IMT.Core.Native.lib Zeiss.IMT.Numerics.Native.lib CMath.lib Zeiss.IMT.PointCloudProcessing.lib
 
 target_link_libraries(  wallthicknessapp  ${QT_LIBRARIES} ${OpenCV_LIBS} volumebasic )
 
 target_link_libraries( rawvolumedataiodemo  volumebasic  ${QT_LIBRARIES})
 
 target_link_libraries( wallthicknessestimatordemo volumebasic ${QT_LIBRARIES} )
 
 target_link_libraries( histogramfilterdemo ${VTK_LIBRARIES} ${OpenCV_LIBS} )#volumebasic ${QT_LIBRARIES}
 
 target_link_libraries( isosurfaceextractiondemo volumebasic ${QT_LIBRARIES} ${PCL_LIBRARIES}  ${Boost_DATE_TIME_LIBRARY_RELEASE})
 
 target_link_libraries( marchingcubescudademo volumebasic ${QT_LIBRARIES} ${PCL_LIBRARIES} ${CUDA_LIBRARIES}  )

 target_link_libraries( wtadatasetgeneratordemo volumebasic ${QT_LIBRARIES} ${PCL_LIBRARIES} ${CUDA_LIBRARIES} )

 target_link_libraries( brepwallthicknessestimatordemo volumebasic ${QT_LIBRARIES} ${PCL_LIBRARIES} ${CUDA_LIBRARIES}  ${IMT_EMBREE_LIBRARY})
 
 target_link_libraries( volumesegmenterdemo volumebasic ${QT_LIBRARIES}  ${CUDA_LIBRARIES}  ${IMT_EMBREE_LIBRARY}) #${PCL_LIBRARIES}
 
 target_link_libraries( wallthickness2dslicedemo volumebasic ${QT_LIBRARIES} ${PCL_LIBRARIES} ${CUDA_LIBRARIES}  ${IMT_EMBREE_LIBRARY} libgmp-10.lib )
 
 target_link_libraries( syntheticdatawallthicknessdemo volumebasic ${QT_LIBRARIES} ${PCL_LIBRARIES} ${CUDA_LIBRARIES} ${IMT_EMBREE_LIBRARY} )
 
 target_link_libraries( meshcomparisondemo volumebasic ${QT_LIBRARIES} ${PCL_LIBRARIES} ${CUDA_LIBRARIES} ${IMT_EMBREE_LIBRARY} )
 
 target_link_libraries( functionaltestingfraunhofer volumebasic ${QT_LIBRARIES} ${PCL_LIBRARIES} ${CUDA_LIBRARIES} ${IMT_EMBREE_LIBRARY} )

 target_link_libraries( customldmdemo volumebasic ${QT_LIBRARIES} ${IMT_OPENINVENTOR_LIBRARIES} ) #

 target_link_libraries( optimizedzxogeneratordemo volumebasic ${QT_LIBRARIES} ${IMT_OPENINVENTOR_LIBRARIES} )

 target_link_libraries( oclmarchingcubesdemo ${OpenCL_LIBRARY} opencl_shared volumebasic )
 
 target_link_libraries( xraydataloaderdemo volumebasic ${QT_LIBRARIES} ${IMT_OPENINVENTOR_LIBRARIES} )
 
 target_link_libraries( airfilteredvolumerenderingdemo volumebasic_cu ${QT_LIBRARIES} ${IMT_OPENINVENTOR_LIBRARIES} ${CUDA_LIBRARIES} cuda)

 target_link_libraries( volumerenderingdemo volumebasic ${QT_LIBRARIES} ${IMT_OPENINVENTOR_LIBRARIES} )
 
 target_link_libraries( variableisooptimization volumebasic ${QT_LIBRARIES} ${IMT_OPENINVENTOR_LIBRARIES} CTEvaluation loXX.lib )

 target_link_libraries( searchangleoptimizationdemo volumebasic ${QT_LIBRARIES} )
 
 target_link_libraries( oivrenderdemo ${IMT_OPENINVENTOR_LIBRARIES} ${QT_LIBRARIES} GLU32 OpenGL32 ${OpenCV_LIBS} ${IMT_CAD_LIBS})
 
 target_link_libraries( ohtake_mminterface ${OpenCV_LIBS} volumebasic ${ITK_LIBRARIES} ${QT_LIBRARIES})
 
 target_link_libraries( variableisolabelingdemo  ${ITK_LIBRARIES} volumebasic ${QT_LIBRARIES}  ${IMT_FRAUNHOFER_LIBRARIES}) #  ${OpenCV_LIBS}
 
 target_link_libraries( overlappedsubvoxellebelingdemo ${ITK_LIBRARIES} ZeissSegmentationInterface volumebasic ${QT_LIBRARIES} )
 
 target_link_libraries( mappedmemoryiodemo ZeissSegmentationInterface volumebasic ${QT_LIBRARIES} )
 
 target_link_libraries( volumeresizedemo volumebasic ${QT_LIBRARIES} )
 
 target_link_libraries( adaptivesegmentationdemo  ${QT_LIBRARIES} ZeissSegmentationInterface ${ITK_LIBRARIES})#volumebasic

 target_link_libraries( dualmarchingcubesdemo volumebasic ${QT_LIBRARIES}  ${CUDA_LIBRARIES}  ${IMT_EMBREE_LIBRARY} )

 target_link_libraries( multiresmarchingcubesdemo volumebasic ${QT_LIBRARIES}  ${CUDA_LIBRARIES}  ${IMT_EMBREE_LIBRARY} ZeissSegmentationInterface )
 
 target_link_libraries( volumeslicingdemo volumebasic ${QT_LIBRARIES}  ${CUDA_LIBRARIES}  ${IMT_EMBREE_LIBRARY} ZeissSegmentationInterface )

 target_link_libraries( volumerenderingcpudemo volumebasic volumerenderingcpu  ${QT_LIBRARIES}  ${CUDA_LIBRARIES}  ${IMT_EMBREE_LIBRARY} ZeissSegmentationInterface )

 target_link_libraries(multimaterialsurfaceextractiondemo volumebasic ${QT_LIBRARIES}  ${CUDA_LIBRARIES}  ${IMT_EMBREE_LIBRARY} ZeissSegmentationInterface )

 target_link_libraries(volumeisothresholdcomputationdemo volumebasic ${QT_LIBRARIES}  ${CUDA_LIBRARIES}  ${IMT_EMBREE_LIBRARY} ZeissSegmentationInterface )
 
 target_link_libraries( pointpicking2ddemo ZeissSegmentationInterface volumebasic ${QT_LIBRARIES} )
 target_link_libraries( pointpicking3ddemo ZeissSegmentationInterface volumebasic ${QT_LIBRARIES} )
 
 target_link_libraries( roidatademo volumebasic ${QT_LIBRARIES}  ${CUDA_LIBRARIES}  ${IMT_EMBREE_LIBRARY} ZeissSegmentationInterface )
 
 target_link_libraries( automaticalignmentdemo volumebasic ${QT_LIBRARIES}  ${CUDA_LIBRARIES}  ${IMT_EMBREE_LIBRARY} ZeissSegmentationInterface )
 

set_property( TARGET wallthicknessapp PROPERTY FOLDER "Apps")
set_property( TARGET rawvolumedataiodemo PROPERTY FOLDER "Demos")
set_property( TARGET wallthicknessestimatordemo PROPERTY FOLDER "Demos")
set_property( TARGET histogramfilterdemo PROPERTY FOLDER "Demos")
set_property( TARGET isosurfaceextractiondemo PROPERTY FOLDER "Demos")
set_property( TARGET  marchingcubescudademo PROPERTY FOLDER "Demos")
set_property( TARGET  wtadatasetgeneratordemo PROPERTY FOLDER "Demos")
set_property( TARGET  brepwallthicknessestimatordemo PROPERTY FOLDER "Demos")
set_property( TARGET  volumesegmenterdemo PROPERTY FOLDER "Demos")
set_property( TARGET  implicitsurfacedemo PROPERTY FOLDER "Demos")
set_property( TARGET  meshcrosssectiondemo PROPERTY FOLDER "Demos")
set_property( TARGET  meshslicedemo PROPERTY FOLDER "Demos")
set_property( TARGET  wallthickness2dslicedemo PROPERTY FOLDER "Demos")
set_property( TARGET  syntheticdatawallthicknessdemo PROPERTY FOLDER "Demos")
set_property( TARGET  meshcomparisondemo PROPERTY FOLDER "Demos")
set_property( TARGET  variableisooptimization PROPERTY FOLDER "Demos" )
set_property( TARGET  customldmdemo PROPERTY FOLDER "Demos/VolumeRendering")
set_property( TARGET  oivrenderdemo PROPERTY FOLDER "Demos/VolumeRendering" )
set_property( TARGET  optimizedzxogeneratordemo PROPERTY FOLDER "Demos" )
set_property( TARGET  searchangleoptimizationdemo PROPERTY FOLDER "Demos"  )
set_property( TARGET  xraydataloaderdemo PROPERTY FOLDER "Demos/VolumeRendering" )
set_property( TARGET  airfilteredvolumerenderingdemo PROPERTY FOLDER "Demos/VolumeRendering" )
set_property( TARGET  volumerenderingdemo PROPERTY FOLDER "Demos/VolumeRendering" )
set_property( TARGET  oclmarchingcubesdemo PROPERTY FOLDER "Demos/OpenCL")
set_property( TARGET  variableisolabelingdemo PROPERTY FOLDER "Demos")
set_property( TARGET  overlappedsubvoxellebelingdemo PROPERTY FOLDER "Demos"  )
set_property( TARGET mappedmemoryiodemo PROPERTY FOLDER "Demos"  )
set_property( TARGET volumeresizedemo PROPERTY FOLDER "Demos" )
set_property( TARGET pointpicking2ddemo PROPERTY FOLDER "Demos" )
set_property( TARGET pointpicking3ddemo PROPERTY FOLDER "Demos" )
set_property( TARGET multimaterialsurfaceextractiondemo PROPERTY FOLDER "Demos" )
set_property( TARGET volumeisothresholdcomputationdemo PROPERTY FOLDER "Demos")
set_property( TARGET automaticalignmentdemo PROPERTY FOLDER "Demos")


set_property( TARGET  volumebasic PROPERTY FOLDER "Libs")
set_property( TARGET  gpuslicer PROPERTY FOLDER "Libs")
set_property( TARGET  PoissonRecon2 PROPERTY FOLDER "Libs")

set_property( TARGET roidatademo PROPERTY FOLDER "Libs" )



#setup of frauhoffer

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src/core/include
C:/projects/ZeissSeparation_update_2016-12-09/source code v03/src/plugins/Application/ZeissViewer/SegmentationInterface )



#include(CMake/Modules/build_openOR.cmake)


#openOR_search_external(OpenGL OPTIONAL)
#if(OpenGL_FOUND)
   #set(QT_USE_QTOPENGL TRUE)
#endif()

#set(QT_USE_QTWEBKIT TRUE)
#openOR_search_external(Qt4 OPTIONAL VERSION 4.5.3 RESULT_VARS "QT")

#openOR_search_external(DCMTK OPTIONAL EXACT VERSION 3.6.0)
#if (NOT WIN32)
#   add_definitions(-DHAVE_CONFIG_H)            # DCMTK needs this flag to find its config
#endif (NOT WIN32)
#openOR_search_external(OpenCV OPTIONAL)
#openOR_search_external(PNG OPTIONAL)

#-------------------------------------------------------------------------------------------------------------------------------

message(STATUS "Building components ...")
#openOR_add_subdirectory(src)

#-------------------------------------------------------------------------------------------------------------------------------


#openOR_build()
#openOR_build_tests()
#openOR_build_docs()

#-------------------------------------------------------------------------------------------------------------------------------

#openOR_create_install(...)

set( IMT_ZEISS_LIBS  ZeissSegmentationInterface )#ZeissSeparationInterface  ZeissBackend

set( IMT_CT_ANALYSIS_BIN_DIR "C:/projects/CT Analysis/bin/x64/Release" CACHE PATH " set imt ct analysis binary directory " )

link_directories( ${IMT_CT_ANALYSIS_BIN_DIR} )

set( IMT_ZEISS_LIBS2 Zeiss.IMT.ExternalLibs.VolumeSegmentationCommonLibs )


add_executable( volumesegmentationdemo volumesegmentationdemo.cpp  )

target_link_libraries( volumesegmentationdemo volumebasic ${IMT_ZEISS_LIBS} ${QT_LIBRARIES}) #${IMT_ZEISS_LIBS2}

set_property( TARGET  volumesegmentationdemo PROPERTY FOLDER "Demos")






