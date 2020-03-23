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

openOR_source(ZeissSeparationInterface
   INTERFACE ID interface NAME "Interface"
      ZeissSeparationInterface.hpp
)

openOR_source(ZeissSeparationInterface
   IMPL ID src NAME "Sources"
      ZeissSeparationInterface.cpp
)

#-------------------------------------------------------------------------------------------------------------------------------
# Dependencies

openOR_add_dependency(ZeissSeparationInterface ZeissBackend)
openOR_add_dependency(ZeissSeparationInterface Image_Regions)
openOR_add_dependency(ZeissSeparationInterface INTERFACE Basic)

#openOR_add_dependency(ZeissSeparationInterface Boost)

#-------------------------------------------------------------------------------------------------------------------------------
# Build

openOR_create(LIBRARY ZeissSeparationInterface PUBLIC GROUP "Zeiss")

if (WIN32)
  set_target_properties(ZeissSeparationInterface PROPERTIES LINK_FLAGS /LARGEADDRESSAWARE )
endif (WIN32)
