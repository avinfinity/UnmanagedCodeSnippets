openOR_source(Zeiss_ImageIO
   INTERFACE ID Interface NAME "Interface"
   include/openOR/Defs/Zeiss_ImageIO.hpp
   include/openOR/Image/ZeissRawExporter.hpp
   include/openOR/Image/ZeissRawExtendedExporter.hpp
   include/openOR/Image/ZeissRawImporter.hpp
)

openOR_add_dependency(TRANSITIVE Zeiss_ImageIO openOR_core)
openOR_add_dependency(TRANSITIVE Zeiss_ImageIO Basic)
openOR_add_dependency(TRANSITIVE Zeiss_ImageIO Image_ImageIO)
openOR_add_dependency(TRANSITIVE Zeiss_ImageIO Image_ImageData)
openOR_add_dependency(TRANSITIVE Zeiss_ImageIO Image_Utility)
openOR_add_dependency(TRANSITIVE Zeiss_ImageIO Image_Regions)

openOR_create(HEADER_ONLY Zeiss_ImageIO GROUP "Zeiss")
