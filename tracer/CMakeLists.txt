



add_library( tracer bvh.cpp camera.cpp hapalaaccelerator.cpp implicitaaccelerator.cpp 
                    implicitbaccelerator.cpp mesh.cpp 
                    sparseaccelerator.cpp stackaccelerator.cpp tracer.cpp  )
                    
add_executable( tracer_main main.cpp )

target_link_libraries( tracer ${VTK_LIBRARIES} )

target_link_libraries( tracer_main tracer )

if( WIN32 )
  set_property( TARGET tracer PROPERTY FOLDER "ThirdParty")
  set_property( TARGET tracer_main PROPERTY FOLDER "Demos/Tracer")
endif()