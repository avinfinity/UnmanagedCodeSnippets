
set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS}; -gencode arch=compute_30,code=compute_30;--use_fast_math ;  --expt-relaxed-constexpr -lineinfo   ) # --restrict; cudaSafeCall(cudaGetLastError());  -g;


include_directories("G:/Projects/CaligoLibs/Libs/IPP/6.1.5.054/em64t")


link_directories( 
"G:/Projects/CaligoLibs/Libs/IPP/6.1.5.054/em64t/lib"
#"C:/projects/Wallthickness/build/vs2013/x64/RelWithDebInfo")
"C:/projects/Libs/VolumeSegmentation/vs2013/Release"

  ${IMT_OPENINVENTOR_LIBRARY_DIR})

cuda_add_library( ctevaluation_cu SHARED ctprofileevaluation.cu ctprofileevaluationkernels.cu ctprofileevaluation.cuh )  

target_link_libraries( ctevaluation_cu legacy_stdio_definitions )
  
add_library( ctevaluation CTPointCloudEvaluation.cpp ZiCTHistogram.cpp CTProfilsEvaluation.cpp 
array.cpp Vector.cpp svdsolver.cpp    Matrix.cpp CTPointCloudEvaluationCUDA.cpp ctprofileevaluationcpu.cpp ctpointcloudevaluationsp.cpp ctprofilsevaluationsp.cpp )


target_link_libraries(ctevaluation ctevaluation_cu )

add_executable( ctevaluationdmemo  ctevaluationdmemo.cpp  )

target_link_libraries( ctevaluationdmemo ${VTK_LIBRARIES}    ${QT_LIBRARIES} ctevaluation  volumebasic legacy_stdio_definitions ctevaluation_cu 
ippcoreem64tl.lib
ippiemergedem64t.lib
ippimergedem64t.lib
ippremergedem64t.lib
ipprmergedem64t.lib
ippsemergedem64t.lib
ippsmergedem64t.lib
ippsremergedem64t.lib
ippsrmergedem64t.lib) #${IMT_LIBRARIES} ${IMT_IPP_LIBRARIES}


#add_library( ctevaluation  ) #array.cpp  CTProfilsEvaluation.cpp  #CTPointCloudEvaluation.cpp
                             #Matrix.cpp svdsolver.cpp #linearsolver.cpp  lusolver.cpp
						     #Vector.cpp vmclasses.cpp ZiCTHistogram.cpp)



#set_property( TARGET  ctevaluation PROPERTY FOLDER "NeoInsights/Libs")

#vcMove( ctevaluationdmemo ${IMT_BINARY_DIR}  )