#version 400

layout(location = 0) in vec3 vert;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;

out vec3 surfaceNormal , surfaceColor , surfacePos;


uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat3 rT;
uniform float enableHeatMap;

void main()
{
    gl_Position = mvpMatrix * vec4( vert , 1.0 );
	
	surfaceNormal = rT * normal;
	

	
	surfaceColor = color;
	surfacePos = ( mvMatrix * vec4( vert , 1.0 ) ).xyz;
}
