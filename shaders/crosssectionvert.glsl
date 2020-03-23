#version 330

layout(location = 0) in vec3 vert;
layout(location = 1) in vec3 normal;
layout(location = 3) in vec3 color;
layout(location = 4) in float wallThickness;

out vec4 vPosition;
out vec3 vDirection;
out vec3 vColor;
out float thickness;

void main()
{	
   vPosition = vec4( vert , 1 );

   vDirection = normal; 
   
   thickness = wallThickness;
   
   vColor = color;
   
}