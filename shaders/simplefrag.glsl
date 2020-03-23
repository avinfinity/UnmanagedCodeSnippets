#version 400

layout(location = 0) out vec4 outColor;

//uniform vec3 ambient;
uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform vec3 halfVector;   // surface orientation of shiniest spot
uniform float shininess;   // exponent of sharping highlights
uniform float strength;    //extra factor to adjust shininess 
uniform float enableHeatMap;

uniform vec4 surfaceColor;

in vec3 surfaceNormal , surfacePos;

void main()
{

  //code for lighting
  vec3 surfaceToLight = normalize( lightPosition - surfacePos );
  
  //calculate the cosine of the angle of incidence
  float brightness = abs( dot( surfaceNormal , surfaceToLight) / ( length(surfaceToLight) * length( surfaceNormal ) ) );
  
  brightness = clamp( brightness , 0 , 1 );

  vec3 scatteredLight = lightColor * brightness;
  
  vec3 inputColor = vec3( 1 , 1 , 1 );
	
  inputColor = surfaceColor.xyz;

  vec3 rgb = min( inputColor * scatteredLight  , vec3( 1.0 ) );  

  outColor.xyz = rgb;
  
  outColor.w = surfaceColor.w;

}

