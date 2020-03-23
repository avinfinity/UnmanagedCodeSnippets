#version 400

layout (location = 0) in vec3 vert;

out vec3 EntryPoint;

uniform mat4 mvpMatrix;

void main()
{
    EntryPoint = vert;
    
	gl_Position = mvpMatrix * vec4(vert,1.0);  
}
