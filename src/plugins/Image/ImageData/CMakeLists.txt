openOR_source(Image_ImageData
   INTERFACE ID 02_ExportedHeaders NAME "Interface Headers"
   include/openOR/Defs/Image_ImageData.hpp
   include/openOR/Image/InfoData.hpp
   include/openOR/Image/InfoDevice.hpp
   include/openOR/Image/InfoProjection.hpp
   include/openOR/Image/InfoPatient.hpp
   include/openOR/Image/Image1DData.hpp
   include/openOR/Image/Image2DData.hpp
   include/openOR/Image/Image3DData.hpp
   include/openOR/Image/Image3DSizeDescriptor.hpp
)

openOR_source(Image_ImageData
   INTERFACE ID 01_Interfaces NAME "Interface"
   include/openOR/Image/Image1DRawData.hpp
   include/openOR/Image/Image2DRawData.hpp
   include/openOR/Image/Image3DRawData.hpp
   include/openOR/Image/Image1DSize.hpp
   include/openOR/Image/Image2DSize.hpp
   include/openOR/Image/Image3DSize.hpp
)

openOR_source(Image_ImageData
   IMPL ID 03_Impl NAME "Source"
   InfoDevice.cpp
   InfoPatient.cpp
   InfoProjection.cpp
)

openOR_source(Image_ImageData
   IMPL ID 04_PluginConfig NAME "Plugin Config"
   #Image_ImageData_plugin_factory.cpp
)

openOR_add_dependency(Image_ImageData openOR_core)
openOR_add_dependency(TRANSITIVE Image_ImageData Basic)
openOR_add_dependency(TRANSITIVE Image_ImageData OpenGL_DataInterpreter)
 	
openOR_create(LIBRARY Image_ImageData GROUP "Image/Container")
