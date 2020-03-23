#ifndef __VC_SLICERENDERMANAGER_H__
#define __VC_SLICERENDERMANAGER_H__


#include "QOpenGLFunctions_4_0_Core"
#include "QOpenGLFunctions_4_3_Core"
#include "qgl.h"
#include "eigenincludes.h"
#include "QObject"

#include "trackballcamera.h"
#include "volumeinfo.h"
#include "wallthicknessestimator.h"

namespace imt{
	
	namespace volume{
	
	class SliceRenderManager : public QObject , public QOpenGLFunctions_4_0_Core
	{
		
		Q_OBJECT;


		public:

			struct VertexData
			{
				Eigen::Vector3f mVertex, mVertexNormal, mVertexColor;
				float mWallThickness;
			};


        enum PLANETYPE{ XY = 0 , YZ , ZX };
		
		SliceRenderManager( imt::volume::VolumeInfo *volume , PLANETYPE planeType = XY );

		void setInfo( GLint surfaceSliceProgram , int nSurfaceVBOs , GLuint *surfaceVBOs , GLuint surfaceVAO , int nTris );

		void setOrthoProjectionMatrix( const QMatrix4x4& orthoProjMat );

		void setPlaneCoeffs(float coeff);

		void setSurface(std::vector< Eigen::Vector3f >& vertices, std::vector< unsigned int >& faces);

		void updateSurface();

		void setLimits(const Eigen::Vector3f& minLimits, const Eigen::Vector3f& maxLimits);

		void setCamera(TrackBallCamera *camera);

		void setPlaneType( int planeType , Eigen::Vector3f& planeCoeff , int w , int h );

		void setViewPort(int w, int h);

		void init();

		void render();

		void setNumTriangles(int numTris);

		bool isInitialized();

		void setDefaultFBO(GLint fbo);

		void setWallthicknessEstimator(imt::volume::WallthicknessEstimator *wte);



		void registerKeyPressEvent(QKeyEvent* e);
		void registerKeyReleaseEvent(QKeyEvent* e);
		void registerMousePressEvent(QMouseEvent *event);
		void registerMouseReleaseEvent(QMouseEvent *event);
		void registerMouseMoveEvent(QMouseEvent *event);
		void registerWheelEvent(QWheelEvent * event);

		void initializeShaders();

		GLuint compileShaders(GLenum shaderType, const char *shaderSource);


		public slots:

		void setChangedData();



	protected:

		void buildProjectionmatrix( const Eigen::Vector2f& xLimits , const Eigen::Vector2f& yLimits , const Eigen::Vector2f& zLimits );

		void computeSlice();
		void renderSlice();

	

		 

	protected:


		GLint mVolumeSliceProgram , mVolumeSliceTFProgram , mSurfaceSliceProgram;
		GLuint mSurfaceVBOs[3] , mSliceVBO[2] , mFeedbackBuffer;
		GLuint mSurfaceVAO[2] , mDefaultFBO ;
		unsigned int mNumSurfaceVBOs;
		QMatrix4x4 mOrthoProjMat;

		int mNumTriangles;

		TrackBallCamera *mCamera;
        int mPlaneType;

		Eigen::Vector3f mCoeff;

		int mWidth, mHeight;

		Eigen::Vector3f mMinLimits, mMaxLimits;

		bool mIsInitialized , mHasNewData;

		GLuint mFeedBackQuery;


		std::vector< Eigen::Vector3f > mSurfaceVertices;
		std::vector< VertexData > mVertexRenderData;
		std::vector< VertexData > mSliceVertexData;
		std::vector< unsigned int > mSurfaceIndices;

		imt::volume::VolumeInfo *mVolume;

		imt::volume::WallthicknessEstimator *mWte;


		



	};	
		
		
		
	}
	
}





#endif