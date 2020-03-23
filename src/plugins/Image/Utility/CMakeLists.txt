

openOR_source(Image_Utility
   INTERFACE ID 02_ExportedHeaders NAME "Interface Headers"

   include/openOR/Defs/Image_Utility.hpp
   include/openOR/Image/DataWrapper.hpp
   #include/openOR/Image/VolumeOperation.hpp
   include/openOR/Image/HistogramCollector.hpp
   include/openOR/Image/SubvolumeHistogramCollector.hpp
   include/openOR/Image/VolumeCollector.hpp
   include/openOR/Image/HistogramAnalyser.hpp
   include/openOR/Image/HistogramMerger.hpp
   include/openOR/Image/HistogramProbabilityAnalyser.hpp
)

openOR_source(Image_Utility
   INTERFACE ID 01_Interfaces NAME "Interface"
   include/openOR/Image/DataCollector.hpp
)

openOR_source(Image_Utility
   IMPL ID 03_Impl NAME "Source"
   #VolumeOperation.cpp
   HistogramAnalyser.cpp
   HistogramMerger.cpp
   HistogramProbabilityAnalyser.cpp
)

#find_package(OpenCV QUIET)
#if (OpenCV_FOUND)

#   openOR_source(Image_Utility
#      INTERFACE ID 04a_ExportedHeaders NAME "Interface Headers (OpenCV)"
#      include/openOR/Image/Image2DProcessor.hpp

 #     IMPL ID 04b_Impl NAME "Source (OpenCV)"
  #    Image2DProcessor.cpp
  # )
#endif (OpenCV_FOUND)

openOR_add_dependency(Image_Utility openOR_core)
openOR_add_dependency(Image_Utility Basic)
openOR_add_dependency(Image_Utility Image_ImageData)
#openOR_add_dependency_if(OpenCV_FOUND Image_Utility OpenCV)
#openOR_add_dependency_if(OpenCV_FOUND Image_Utility Qt)

openOR_create(LIBRARY Image_Utility GROUP "Image/Processing")
