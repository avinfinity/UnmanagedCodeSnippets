#include "volumerenderingopengl.h"


namespace imt 
{

	namespace volume 
	{



		VolumeRendererOpenGL::VolumeRendererOpenGL(unsigned short* volumeData, int width, int height, int depth) : mVolumeData(volumeData),
			mWidth(width), mHeight(height) , mDepth(depth)
		{
			initializeOpenGLFunctions();
		}

		void VolumeRendererOpenGL::initialize()
		{

		}

		void VolumeRendererOpenGL::setTransferFunction( TransferFunction* transferFunction )
		{

		}


		void VolumeRendererOpenGL::generateShaders()
		{
			mFrontVSSrc = "#version 400 \n"

				"layout(location = 0) in vec3 VerPos; \n"
				// have to use this variable!!!, or it will be very hard to debug for AMD video card
				"layout(location = 1) in vec3 VerClr; \n"
				"out vec3 EntryPoint; \n"
				"out vec4 ExitPointCoord; \n"
				"uniform mat4 MVP; \n"
				"void main() \n"
				"{ \n"
				"EntryPoint = VerClr; \n"
				"gl_Position = MVP * vec4(VerPos, 1.0); \n"
				"ExitPointCoord = gl_Position; \n"
				"} \n";


			mFrontFSSrc = 
			
			"#version 400  \n"
				
			"in vec3 EntryPoint;  \n"
			"in vec4 ExitPointCoord;  \n"

			"uniform sampler2D ExitPoints;  \n"
			"uniform sampler3D VolumeTex;  \n"
			"uniform sampler1D TransferFunc;  \n"
			"uniform float     StepSize;  \n"
			"uniform vec2      ScreenSize;  \n"
			"layout(location = 0) out vec4 FragColor;  \n"

			"void main()  \n"
			"{  \n"
				// ExitPointCoord
			"	vec3 exitPoint = texture(ExitPoints, gl_FragCoord.st / ScreenSize).xyz;  \n"
				// that will actually give you clip-space coordinates rather than
				// normalised device coordinates, since you're not performing the perspective
				// division which happens during the rasterisation process (between the vertex
				// shader and fragment shader
				// vec2 exitFragCoord = (ExitPointCoord.xy / ExitPointCoord.w + 1.0)/2.0;
				// vec3 exitPoint  = texture(ExitPoints, exitFragCoord).xyz;
			"	if (EntryPoint == exitPoint)  \n"
					//background need no raycasting
			"		discard;  \n"
			"	vec3 dir = exitPoint - EntryPoint;  \n"
			"	float len = length(dir);  \n" // the length from front to back is calculated and used to terminate the ray
			"	vec3 deltaDir = normalize(dir) * StepSize;  \n"
			"	float deltaDirLen = length(deltaDir);  \n"
			"	vec3 voxelCoord = EntryPoint;  \n"
			"	vec4 colorAcum = vec4(0.0);  \n" // The dest color
			"	float alphaAcum = 0.0;  \n"                // The  dest alpha for blending
				
			"	float intensity;  \n"
			"	float lengthAcum = 0.0;  \n"
			"	vec4 colorSample;   \n"// The src color 
			"	float alphaSample;   \n"// The src alpha
				// backgroundColor
			"	vec4 bgColor = vec4(1.0, 1.0, 1.0, 0.0);  \n"

			"	for (int i = 0; i < 1600; i++)  \n"
			"	{  \n"
			"		intensity = texture(VolumeTex, voxelCoord).x;  \n"

			"		colorSample = texture(TransferFunc, intensity);  \n"
					// modulate the value of colorSample.a
					// front-to-back integration
			"		if (colorSample.a > 0.0) {  \n"
						// accomodate for variable sampling rates (base interval defined by mod_compositing.frag)
			"			colorSample.a = 1.0 - pow(1.0 - colorSample.a, StepSize * 200.0f);  \n"
			"			colorAcum.rgb += (1.0 - colorAcum.a) * colorSample.rgb * colorSample.a;  \n"
			"			colorAcum.a += (1.0 - colorAcum.a) * colorSample.a;  \n"
			"		}  \n"
			"		voxelCoord += deltaDir;  \n"
			"		lengthAcum += deltaDirLen;  \n"
			"		if (lengthAcum >= len)  \n"
			"		{  \n"
			"			colorAcum.rgb = colorAcum.rgb * colorAcum.a + (1 - colorAcum.a) * bgColor.rgb;  \n"
			"			break;    \n"// terminate if opacity > 1 or the ray is outside the volume	
			"		}  \n"
			"		else if (colorAcum.a > 1.0)  \n"
			"		{  \n"
			"			colorAcum.a = 1.0;  \n"
			"			break;  \n"
			"		}  \n"
			"	} \n"
			"	FragColor = colorAcum;  \n"
				// for test
				// FragColor = vec4(EntryPoint, 1.0);
				// FragColor = vec4(exitPoint, 1.0);

			"} \n";

			mBackVSSrc = // for raycasting
			
			"#version 400  \n"
			"layout(location = 0) in vec3 VerPos;  \n"
			"layout(location = 1) in vec3 VerClr;  \n"

			"out vec3 Color;  \n"

			"uniform mat4 MVP;  \n"


			"void main()  \n"
			"{  \n"
		    "  Color = VerClr;  \n"
		    "  gl_Position = MVP * vec4(VerPos, 1.0);  \n"
			"}  \n"; 


			mBackFSSrc = // for raycasting
				"#version 400  \n"

				"in vec3 Color;  \n"
				"layout(location = 0) out vec4 FragColor;  \n"


				"void main()  \n"
				"{  \n"
				"	FragColor = vec4(Color, 1.0);  \n"
				"}  \n";
			
			
		}

		void VolumeRendererOpenGL::compileAndLinkShaders()
		{

		}






	}

}