#version 330

layout(location = 0) out vec3 outColor;

uniform vec3 lineColor;

in vec3 outputColor;

void main()
{
  outColor = outputColor; 
}
