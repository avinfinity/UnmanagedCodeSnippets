#-------------------------------------------------------------------------------------------------------------------------------
# (c) 2012 by the openOR Team
#-------------------------------------------------------------------------------------------------------------------------------
# The contents of this file are available under the GPL v2.0 license
# or under the openOR comercial license. see
#   /Doc/openOR_free_license.txt or
#   /Doc/openOR_comercial_license.txt
# for Details.
#-------------------------------------------------------------------------------------------------------------------------------


#-------------------------------------------------------------------------------------------------------------------------------
# Sources and Resources

openOR_source(ZeissSegmentationInterface
   INTERFACE ID interface NAME "Interface"
      ZeissSegmentationInterface.hpp
)

openOR_source(ZeissSegmentationInterface
   IMPL ID src NAME "Sources"
      ZeissSegmentationInterface.cpp
)

#-------------------------------------------------------------------------------------------------------------------------------
# Dependencies

openOR_add_dependency(ZeissSegmentationInterface ZeissBackend)
openOR_add_dependency(ZeissSegmentationInterface INTERFACE Basic)
#openOR_add_dependency(ZeissSegmentationInterface Boost)

#-------------------------------------------------------------------------------------------------------------------------------
# Build

openOR_create(LIBRARY ZeissSegmentationInterface PUBLIC GROUP "Zeiss")

if (WIN32)
  set_target_properties(ZeissSegmentationInterface PROPERTIES LINK_FLAGS /LARGEADDRESSAWARE )
endif (WIN32)
