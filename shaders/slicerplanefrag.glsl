//for raycasting
#version 400

layout(location = 0) out vec4 outColor;

uniform vec4 planeColor;

//uniform vec3 planeNormal;

void main()
{
  outColor = planeColor;
}