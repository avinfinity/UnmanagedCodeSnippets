openOR_source(Image_ImageIO
   INTERFACE ID 02_ExportedHeaders NAME "Header"
   include/openOR/Defs/Image_ImageIO.hpp
   include/openOR/Image/Filename.hpp
   #include/openOR/Image/RawExporter.hpp
   #include/openOR/Image/RawImporter.hpp
   
   IMPL ID 03_Impl NAME "Source"
   Filename.cpp
)

openOR_source(Image_ImageIO
   INTERFACE ID 01_Interface NAME "Interface"
   include/openOR/Image/FileImporter.hpp
   include/openOR/Image/FileExporter.hpp
   #include/openOR/Image/NetworkImporter.hpp
   #include/openOR/Image/NetworkExporter.hpp
)

openOR_source(Image_ImageIO
   IMPL ID 04_Factory NAME "Plugin Config"
   #Image_ImageIO_plugin_factory.cpp
)
	
openOR_add_dependency(Image_ImageIO openOR_core)
openOR_add_dependency(Image_ImageIO Basic)
openOR_add_dependency(Image_ImageIO Image_ImageData)
openOR_add_dependency(TRANSITIVE Image_ImageIO Image_Utility)

openOR_create(LIBRARY Image_ImageIO GROUP "Image/IO")