openOR_source(ZeissBackend
   INTERFACE ID Interface NAME "Interface"
      include/openOR/Defs/Zeiss_ZeissBackend.hpp
      include/openOR/ZeissBackend.hpp
      include/openOR/ScanSurfaceSegmentor.hpp
      include/openOR/MTScanSurfaceSegmentor.hpp
      include/openOR/ProbabilitySegmentation.hpp

   IMPL ID ZImpl NAME "Source"
      ZeissBackend.cpp
      ScanSurfaceSegmentor.cpp
      MTScanSurfaceSegmentor.cpp
      ProbabilitySegmentation.cpp
)

openOR_source(ZeissBackend
   IMPL ID ZZFactory NAME "Plugin Config"
      ZeissBackend_plugin_factory.cpp
)

openOR_add_dependency(ZeissBackend openOR_core)
openOR_add_dependency(TRANSITIVE ZeissBackend Image_ImageData)
openOR_add_dependency(ZeissBackend Zeiss_ImageIO)
openOR_add_dependency(TRANSITIVE ZeissBackend Image_Utility) # should be INTERFACE_TRANSITIVE?
openOR_add_dependency(ZeissBackend Image_Regions)

openOR_create(LIBRARY ZeissBackend GROUP "Zeiss")

if (WIN32)
  set_target_properties(ZeissBackend PROPERTIES LINK_FLAGS /LARGEADDRESSAWARE )
endif (WIN32)
