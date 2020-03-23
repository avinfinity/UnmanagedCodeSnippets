#ifndef __IMT_VOLUME_VOLUMERENDERINGOPENGL_H__
#define __IMT_VOLUME_VOLUMERENDERINGOPENGL_H__

#include "qopenglfunctions_4_4_compatibility.h"
#include "TransferFunction.h"
#include "eigenincludes.h"

namespace imt 
{
	namespace volume 
	{

		class VolumeRendererOpenGL : QOpenGLFunctions_4_4_Compatibility
		{

		public:

			VolumeRendererOpenGL( unsigned short* volumeData , int width , int height , int depth );

			void initialize();

			void setTransferFunction(TransferFunction* transferFunction);

		protected:

			void generateShaders();
			void compileAndLinkShaders();

		protected:

			TransferFunction* mTransferFunction;
		
			int mWidth, mHeight, mDepth;

			unsigned short* mVolumeData;

			unsigned int mVBO[2], mVAO;

			unsigned int mVolumeTexture;

			int mFrontRenderProgram, mBackRenderProgram;

			std::vector<Eigen::Vector3f> mCubeCorners;

			std::string mFrontVSSrc, mFrontFSSrc, mBackVSSrc, mBackFSSrc;
		};



	}

}



#endif