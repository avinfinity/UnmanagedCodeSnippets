openOR_source(Image_Regions
   INTERFACE ID ExportedHeaders NAME "Interface"
   include/openOR/Defs/Image_Regions.hpp
   include/openOR/Image/RegionOfInterest.hpp
   include/openOR/Image/ROIContainer.hpp
   
   IMPL ID Source NAME "Source"
   ROIContainer.cpp
)
 	
openOR_add_dependency(Image_Regions openOR_core)
openOR_add_dependency(Image_Regions Basic)

openOR_create(LIBRARY Image_Regions GROUP "Image")
