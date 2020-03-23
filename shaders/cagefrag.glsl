//for raycasting
#version 400

layout(location = 0) out vec3 outColor;

uniform vec3 color;

void main()
{
  outColor = color;
}