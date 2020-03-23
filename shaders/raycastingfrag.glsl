#version 400


layout (location = 0) out vec4 FragColor;



in vec3 EntryPoint;

uniform sampler1D TransferFunc;
uniform sampler2D ExitPoints;
uniform sampler3D VolumeTex;

uniform float     StepSize;
uniform vec2      ScreenSize;

uniform float w , h , d;



void main()
{
    // that will actually give you clip-space coordinates rather than
    // normalised device coordinates, since you're not performing the perspective
    // division which happens during the rasterisation process (between the vertex
    // shader and fragment shader
	vec3 exitPoint = texture(ExitPoints, gl_FragCoord.st/ScreenSize).xyz;
    
	if ( EntryPoint == exitPoint ) //background need no raycasting
     	return;
		
    vec3 dir = exitPoint - EntryPoint;
	
	vec4 entryExitColor = vec4( abs(dir.x / w) , abs(dir.y / h) , abs(dir.z / d) , 1 );
	
    float len = length(dir); // the length from front to back is calculated and used to terminate the ray
    vec3 deltaDir = normalize(dir) * StepSize;
    float deltaDirLen = length(deltaDir);
    vec3 voxelCoord = EntryPoint / vec3( w , h , d );
    vec4 colorAcum = vec4(0.0); // The dest color
    float alphaAcum = 0.0;                // The  dest alpha for blending
    float intensity = 0;
    float lengthAcum = 0.0;
    vec4 colorSample; // The src color 
    float alphaSample; // The src alpha
    // backgroundColor
    vec4 bgColor = vec4(0.0, 0.0, 0.0, 0.0);
	
	//float diag = sqrt( w * w  + h * h + d * d );
	
	
	
	FragColor = entryExitColor;
	
	return;
	
	intensity =  texture(VolumeTex, voxelCoord).x;
	
	bool found = false;

    for(int i = 0; i < 50; i++) //1600
    {
    	//scaler value
    	intensity =  texture(VolumeTex, voxelCoord).x;
    	colorSample = texture(TransferFunc, intensity);
    	// modulate the value of colorSample.a
    	// front-to-back integration
    	if (intensity > 0.10 )  //0.46997 //colorSample.a > 0.0 && 
		{
    	    // accomodate for variable sampling rates (base interval defined by mod_compositing.frag)
    	    colorSample.a = 1.0 - pow(1.0 - colorSample.a, StepSize*200.0f); //
    	    colorAcum.rgb += (1.0 - colorAcum.a) * colorSample.rgb * colorSample.a;
    	    colorAcum.a += (1.0 - colorAcum.a) * colorSample.a;
			
			found = true;
    	}
		else
		{
		  continue;
		}
		//if( intensity < 0.118 )
		// break;
		
    	voxelCoord += deltaDir;
    	lengthAcum += deltaDirLen;
    	if (lengthAcum >= len )
    	{	
    	    colorAcum.rgb = colorAcum.rgb * colorAcum.a + (1 - colorAcum.a) * bgColor.rgb ;		
    	    break;  // terminate if opacity > 1 or the ray is outside the volume	
    	}	
    	else if (colorAcum.a > 1.0)
    	{
    	    colorAcum.a = 1.0;
    	    break;
    	}
    } 
    
	if( found )
	  FragColor = vec4( 1 , 0 , 0 , 1 );//colorAcum;//vec4( voxelCoord , 1 );//vec4( intensity , intensity , intensity , 1.0 );//
	else
	  FragColor = vec4( 0 , 0 , 0 , 0 );
	
	//intensity =  texture( VolumeTex , vec3( 0.3 , 0.3 , 0.5 ) ).x;
    
	// for test
	//if( d > 100 )
    //FragColor = vec4( intensity , 1 , 0 , 1.0 );//vec4( EntryPoint , 1.0 );//vec4( voxelCoord , 1.0 );//
    
	
	// FragColor = vec4(exitPoint, 1.0);
   
}
