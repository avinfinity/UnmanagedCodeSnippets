#version 400

layout(location = 0) in vec3 vert;
layout(location = 1) in vec3 normal;

uniform mat4 mvpMatrix;
uniform mat3 rT;

out vec3 surfaceNormal;

void main()
{
    gl_Position = mvpMatrix * vec4( vert , 1.0 );
	
	surfaceNormal = rT * normal;
}