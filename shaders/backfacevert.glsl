// for raycasting
#version 400

layout(location = 0) in vec3 vert;

out vec3 interpolatedVerts;

uniform mat4 mvpMatrix;

void main()
{
    interpolatedVerts = vert;
    gl_Position = mvpMatrix * vec4( vert , 1.0 );
}
