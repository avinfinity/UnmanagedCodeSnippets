#-------------------------------------------------------------------------------------------------------------------------------
# (c) 2012 by the openOR Team
#-------------------------------------------------------------------------------------------------------------------------------
# The contents of this file are available under the GPL v2.0 license
# or under the openOR comercial license. see
#   /Doc/openOR_free_license.txt or
#   /Doc/openOR_comercial_license.txt
# for Details.
#-------------------------------------------------------------------------------------------------------------------------------

openOR_configure(openOR_core openor_config.h)
#openOR_configure(openOR_core include/openOR/OpenGL/OpenGL.hpp EXPORT "include/")

openOR_source(openOR_core
   INTERFACE ID 01a_Misc NAME "Interfaces"
      include/openOR/coreDefs.hpp
      include/openOR/Application.hpp
   INTERNAL ID 01b_Misc NAME "Internal Header"
      #doxygen_main.hpp
   IMPL ID 01c_Misc
      coreDefs.cpp
)

openOR_source(openOR_core
   INTERFACE ID 02a_Plugin NAME "Plugin"
      include/openOR/Plugin/AnyPtr.hpp
      include/openOR/Plugin/create.hpp  
      include/openOR/Plugin/Loader.hpp
      include/openOR/Plugin/Library.hpp
      include/openOR/Plugin/Config.hpp
      include/openOR/Plugin/Interface.hpp
      include/openOR/Plugin/interface_cast.hpp
      include/openOR/Plugin/Registration.hpp
      include/openOR/Plugin/CreateInterface.hpp
      include/openOR/Plugin/DisableAutoDelete.hpp
      include/openOR/Plugin/exception.hpp
)

openOR_source(openOR_core
   INTERFACE ID 02b_PluginDetail NAME "Plugin/Detail"
      include/openOR/Plugin/detail/AnyPtrT.hpp
      include/openOR/Plugin/detail/cast.hpp
      include/openOR/Plugin/detail/Wrapper.hpp
      include/openOR/Plugin/detail/WrapperBase.hpp
      include/openOR/Plugin/detail/Base.hpp
      include/openOR/Plugin/detail/AdapterBase.hpp
      include/openOR/Plugin/detail/reg.hpp
      include/openOR/Plugin/detail/interfacePtr.hpp
   IMPL ID 02c_PluginImpl NAME "Plugin/Implementation"
      Plugin/Loader.cpp
      Plugin/Library.cpp
      Plugin/Config.cpp
      Plugin/Interface.cpp   
   INTERNAL ID 02d_PluginImplDetailHeader NAME "Plugin/Implementation/Detail"
      #Plugin/LibHandle_dlopen.hpp
      Plugin/LibHandle_windows.hpp
      Plugin/autoOpenLibrary.hpp
      #include/openOR/Plugin/detail/scanForPlugins.hpp	
      include/openOR/Plugin/detail/TypePair.hpp
)

openOR_source(openOR_core
   IMPL ID 02e_PluginImplDetailImpl NAME "Plugin/Implementation/Detail"
      Plugin/autoOpenLibrary.cpp
      #Plugin/scanForPlugins.cpp	
)

openOR_source(openOR_core
   INTERFACE ID 03a_Logger NAME "Logger"
      include/openOR/Log/Logger.hpp
      include/openOR/Log/ModuleFilter.hpp
   IMPL ID 03b_LoggerImpl NAME "Logger/Implementation"
      Log/Logger.cpp
      Log/ModuleFilter.cpp
)

#openOR_source(openOR_core
#   INTERFACE ID 03c_Exception NAME "Exception"
#      include/openOR/Exception/Exception.hpp
#   IMPL ID 03d_ExceptionImpl NAME "Exception/Implementation"
#      Exception/Exception.cpp
#)

openOR_source(openOR_core
   INTERFACE ID 04a_Utility NAME "Utility"
      include/openOR/Utility/Singleton.hpp
      include/openOR/Utility/conceptcheck.hpp
      include/openOR/Utility/Types.hpp
      include/openOR/Utility/if_const.hpp
      #include/openOR/Utility/forward_const.hpp  
      include/openOR/Utility/is_base_of.hpp
      #include/openOR/Utility/main.hpp
   IMPL ID 04b_UtilityImpl NAME "Utility/Implementation"
      #Utility/main.cpp
)

#openOR_source(openOR_core
#   INTERFACE ID 05a_Signal NAME "Signal"
#      include/openOR/Signal/signals.hpp
#      include/openOR/Signal/BroadcastSignaller.hpp
#   IMPL ID 05b_SignalImpl NAME "Signal/Implementation"
#      Signal/signals.cpp
#      Signal/BroadcastSignaller.cpp	
#)

openOR_source(openOR_core
   INTERFACE ID 06_OpenGL NAME "OpenGL"
      include/openOR/OpenGL/gl.hpp
#      include/openOR/OpenGL/glu.hpp
#      include/openOR/OpenGL/glext.hpp
)

openOR_source(openOR_core
   INTERFACE ID 07a_Math NAME "Math"
      #include/openOR/Math/types.hpp
      include/openOR/Math/matrix_types.hpp
      include/openOR/Math/vector_types.hpp
      include/openOR/Math/math.hpp	
      include/openOR/Math/traits.hpp	
      include/openOR/Math/matrix.hpp	
      include/openOR/Math/matrixconcept.hpp	
      include/openOR/Math/matrixfunctions.hpp
      include/openOR/Math/matrixsetaxis.hpp
      include/openOR/Math/determinant.hpp	
      include/openOR/Math/utilities.hpp
      include/openOR/Math/vector.hpp	
      include/openOR/Math/vectorconcept.hpp	
      include/openOR/Math/vectorfunctions.hpp	
      include/openOR/Math/ublasvector.hpp	
      include/openOR/Math/ublasmatrix.hpp	
      include/openOR/Math/constants.hpp
      include/openOR/Math/create.hpp	
      include/openOR/Math/access.hpp   
      include/openOR/Math/comparison.hpp
      include/openOR/Math/GraphicalProjection.hpp  
      Math/GraphicalProjection.cpp
)

openOR_source(openOR_core
   INTERFACE ID 07b_MathDetail NAME "Math/Detail"
      include/openOR/Math/detail/determinant_impl.hpp
      include/openOR/Math/detail/inverse_impl.hpp
)

#openOR_source(openOR_core
#   INTERFACE ID 08_Geometry NAME "Geometry"
#      include/openOR/Geometry/geometry.hpp
#      include/openOR/Geometry/utilities.hpp	
#      include/openOR/Geometry/traits.hpp	
#      include/openOR/Geometry/Point.hpp	
#      include/openOR/Geometry/Line.hpp	
#      include/openOR/Geometry/Segment.hpp	
#      include/openOR/Geometry/Plane.hpp	
#      include/openOR/Geometry/Triangle.hpp
#      include/openOR/Geometry/NoIntersection.hpp
#      include/openOR/Geometry/Point2Functions.hpp	
#      include/openOR/Geometry/Point3Functions.hpp	
#      include/openOR/Geometry/Line2Functions.hpp	
#      include/openOR/Geometry/Line3Functions.hpp	
#      include/openOR/Geometry/Plane3Functions.hpp	
#      include/openOR/Geometry/Segment3Functions.hpp	
#      include/openOR/Geometry/Triangle3Functions.hpp	
#      include/openOR/Geometry/AxisAlignedBoundingBox3.hpp	
#      include/openOR/Geometry/kernel.hpp	
#)

#openOR_add_dependency(TRANSITIVE openOR_core Boost)

#openOR_mark_external(dl)
#openOR_add_dependency_if(UNIX openOR_core dl)

openOR_create(LIBRARY openOR_core GROUP "Core")
