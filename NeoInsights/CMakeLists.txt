
message( STATUS "imt library dirs : ${IMT_LIBRARY_DIRS}" )

set( IMT_INCLUDE_DIRS "C:/projects/CT Analysis/VolumeProcessing" 
                      "C:/projects/CT Analysis/Core/Shared Files/loXX/Include"
					  "C:/projects/Libs/IPP/6.1.5.054/em64t/include"
					  "C:/projects/CT Analysis/NG.Metrotom/Zeiss.IMT.NG.Metrotom.SurfaceTriangulation")
                      
set( IMT_LIBRARY_DIRS "C:/projects/CT Analysis/VolumeProcessing/bin/x64/Release"
                      "C:/projects/CT Analysis/Core/Shared Files/loXX/bin/x64/Release"
					  "C:/projects/Libs/IPP/6.1.5.054/em64t/stublib"
					  "C:/projects/Libs/IPP/6.1.5.054/em64t/lib"
					  
                      )					  

include_directories( ${IMT_INCLUDE_DIRS} )
link_directories( ${IMT_LIBRARY_DIRS} )

set( IMT_IPP_LIBRARIES  ippacemergedem64t.lib
ippacmergedem64t.lib
#ippacmergedem64t_t.lib
ippccemergedem64t.lib
ippccmergedem64t.lib
#ippccmergedem64t_t.lib
ippchemergedem64t.lib
ippchmergedem64t.lib
#ippchmergedem64t_t.lib
ippcoreem64tl.lib
#ippcoreem64t_t.lib
ippcvemergedem64t.lib
ippcvmergedem64t.lib
#ippcvmergedem64t_t.lib
ippdcemergedem64t.lib
ippdcmergedem64t.lib
#ippdcmergedem64t_t.lib
ippdiemergedem64t.lib
ippdimergedem64t.lib
#ippdimergedem64t_t.lib
ippgenemergedem64t.lib
ippgenmergedem64t.lib
#ippgenmergedem64t_t.lib
ippiemergedem64t.lib
ippimergedem64t.lib
#ippimergedem64t_t.lib
ippjemergedem64t.lib
ippjmergedem64t.lib
#ippjmergedem64t_t.lib
ippmemergedem64t.lib
ippmmergedem64t.lib
#ippmmergedem64t_t.lib
ippremergedem64t.lib
ipprmergedem64t.lib
#ipprmergedem64t_t.lib
ippscemergedem64t.lib
ippscmergedem64t.lib
#ippscmergedem64t_t.lib
ippsemergedem64t.lib
ippsmergedem64t.lib
#ippsmergedem64t_t.lib
ippsremergedem64t.lib
ippsrmergedem64t.lib
#ippsrmergedem64t_t.lib
ippvcemergedem64t.lib
ippvcmergedem64t.lib
#ippvcmergedem64t_t.lib
ippvmemergedem64t.lib
ippvmmergedem64t.lib
#ippvmmergedem64t_t.lib
#libiomp5mt.lib
libirc.lib
#libircmt.lib
#libm.lib
libmmds.lib
#libmmt.lib
svml_disp.lib
#svml_dispmt.lib 
)


add_executable( surfacetriangulatordemo surfacetriangulatordemo.cpp  )

set( IMT_LIBRARIES loXX.lib CTEvaluation.lib)

add_subdirectory( CTEvaluation )

#target_link_libraries( surfacetriangulatordemo surfacetriangulatordemo.cpp )





