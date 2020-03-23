cmake_policy(SET CMP0046 NEW)
openOR_source(OpenGL_DataInterpreter
   INTERFACE ID 01_Interfaces NAME "Interface"

	include/openOR/OpenGL/DataInterpreter.hpp
)

openOR_add_dependency(TRANSITIVE OpenGL_DataInterpreter openOR_core)
openOR_add_dependency(TRANSITIVE OpenGL_DataInterpreter OpenGL)
 	
openOR_create(HEADER_ONLY OpenGL_DataInterpreter GROUP "OpenGL")
