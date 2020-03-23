// for raycasting

#version 400
in vec3 interpolatedVerts;

layout (location = 0) out vec3 outVerts; //flat 


void main()
{
    outVerts = interpolatedVerts;//vec3( 0 , 0 , 1 );//
}
