#ifndef __IMT_SEGMENTATIONBENCHMARK_H__
#define __IMT_SEGMENTATIONBENCHMARK_H__

#include "QOpenGLFunctions_4_0_Core"
#include "QOpenGLFunctions_4_3_Core"
#include "qgl.h"
#include "eigenincludes.h"
#include "trackballcamera.h"
#include "embreeincludes.h"
#include "progressreporter.h"
#include "pclincludes.h"
#include "volumeinfo.h"
#include "QObject"
#include "wallthicknessestimator.h"
#include "embreeincludes.h"
#include "qgl.h"
#include "trackballcamera.h"

namespace imt{
	
	namespace volume{
		
		
		class SegmentationBenchmark : public QObject, public QOpenGLFunctions_4_0_Core
		{
	
      
        public:


			struct VertexData{

				Eigen::Vector3f mPosition, mNormal, mColor;

			};


		struct EmbreeVertex
		{
			float x, y, z, a;
		};

		struct EmbreeTriangle { int v0, v1, v2; };

    	  
		SegmentationBenchmark( imt::volume::VolumeInfo *volumeInfo );
		
		void init();
		
		void render();

		void updateBenchMarkData();

		void setCamera(TrackBallCamera *camera);



		protected:

		void renderReferenceData();
		void renderActualData();

		void buildTracer();

		void compareMeshes();

		void initializeShaders();

		GLuint compileShaders(GLenum shaderType, const char *shaderSource);

		
	protected:

		GLuint mVBOs[4] , mVAOs[2];

		imt::volume::VolumeInfo *mVolumeInfo;


		RTCScene mScene;

		unsigned int mGeomID;

		RTCDevice mDevice;

		GLint mSurfaceDisplayProgram;

		GLuint mDefaultFBO;

		bool mHasNewReferenceData , mHasNewActualData;

		std::vector< VertexData > mRefVertData , mActualVertData;
		std::vector< unsigned int > mRefIndices, mActualIndices;

		TrackBallCamera *mCamera;

		
		
	};	
		
		
		
	}
	
	
}




#endif