#ifndef __IMT_VOLUMERENDERMANAGER_H__
#define __IMT_VOLUMERENDERMANAGER_H__

#include "QOpenGLFunctions_4_0_Core"
#include "QOpenGLFunctions_4_3_Core"
#include "qgl.h"
#include "eigenincludes.h"
#include "trackballcamera.h"
#include "embreeincludes.h"
#include "progressreporter.h"
//#include "pclincludes.h"
#include "volumeinfo.h"
#include "QObject"
#include "wallthicknessestimator.h"

namespace imt{
	
	namespace volume{


		struct VertexData
		{
			Eigen::Vector3f mPoint, mNormal;
			Eigen::Vector3f mColor;

		};
		
	
	class VolumeRenderManager : public QObject , public QOpenGLFunctions_4_0_Core
	{

		Q_OBJECT
		
		
	public:

		struct EmbreeVertex
		{
			float x, y, z, a;
		};

		struct EmbreeTriangle { int v0, v1, v2; };
		
		enum RENDER_MODE{ SURFACE , VOLUME };

		VolumeRenderManager( int defaultFBO = 0 );
		
		void init();
		void render();

		void renderCrosssection(int planeType, Eigen::Vector3f planeCoeff, int rw, int rh);

		void loadVolume(  unsigned char* volumeData, int w, int h, int d , Eigen::Vector3f origin , Eigen::Vector3f voxelStep );
		void loadVolume(  unsigned short* volumeData, int w, int h, int d, Eigen::Vector3f origin, Eigen::Vector3f voxelStep );
		void loadSurface( std::vector< Eigen::Vector3f >& vertices, std::vector< Eigen::Vector3f >& colors,
			              std::vector< Eigen::Vector3f >& normals , std::vector< unsigned int >& faceIndices);

		void loadSurface( std::vector< Eigen::Vector3f >& vertices, std::vector< Eigen::Vector3f >& rayColors, 
			              std::vector< Eigen::Vector3f >& sphereColors , std::vector< Eigen::Vector3f >& normals ,
						  std::vector< unsigned int >& faceIndices );


		void setProgressReporter(imt::volume::ProgressReporter *progressReporter);

		void setVolumeInfo(VolumeInfo *volumeInfo);

		void setWallthicknessEstimator(imt::volume::WallthicknessEstimator *wte);


		void setCamera( TrackBallCamera *camera );

		void setDefaultFrameBuffer(GLuint defaultFBO);

		void setRenderMode(RENDER_MODE mode);

		void enablePicking();
		void disablePicking();

		void enableHeatMap();
		void disableHeatMap();

		GLint surfaceSliceProgram();
		GLuint surfaceVAO();
		GLuint *surfaceVBO();

		unsigned int numTriangles();

		GLint sliceRenderProgram();

		bool isInitialized();

		imt::volume::WallthicknessEstimator *wallthicknessEstimator();

		public slots:


		void setChangedData();

		

	protected:

		void initializeShaders();

		GLuint compileShaders(GLenum shaderType, const char *shaderSource);

		void renderIntersections();
		void renderVolume();
		void renderSurface();

		void renderISO();

		void buildTracer();

		int findIncidentTriangle(Eigen::Vector3f point);

		void renderRay();
		void renderSphere();
		void renderCrossSection();

		Eigen::Matrix4f computeOrthoProjectionMatrix(int plane, Eigen::Vector3f minLimits, Eigen::Vector3f maxLimits);

		void setPlaneType(int planeType, Eigen::Vector3f& planeCoeff, int w, int h);

		void buildOrthodProjectionMatrix();

		
	protected:

		GLuint mVolumeTex , mColorTex , mIntersectionsTex;
		GLuint mVolumeVBO[ 2 ] , mSurfaceVBO[3] , mVolumeVAO , mSurfaceVAO , mSphereVAO;
		GLuint mIntersectionsFBO;
		GLuint mDefaultFBO;

		GLint mVolumeVizProgram, mVolumeVizBackFaceProgram,
			  mSurfaceProgram, mSphereProgram, mRayProgram,
			  mMeshCrossSectionProgram;

		std::vector< VertexData > mSurfaceVertices , mSurfaceVerticesSphere;
		std::vector< GLuint > mSurfaceIndices;
		std::vector< std::vector< unsigned int > >  mIncidentTriangles;
		std::vector< unsigned char > mUsedTriangleMask;
		std::vector< float > mPlaneCoeffs;
		std::vector< Eigen::Vector3f > mFaceNormals;

		bool mHasNewData , mSurfaceDataChanged , mSphereSurfaceDataChanged;

		std::vector< Eigen::Vector3f > mVertices;
		std::vector< GLuint > mIndices;


		TrackBallCamera *mCamera;

		Eigen::Vector3f mVolumeOrigin, mVolumeStep;

		int mWidth, mHeight, mDepth;

		int mRenderWidth, mRenderHeight;

		QMatrix4x4 mOrthoProjMatData;

		Eigen::Vector3f mCoeff;

		imt::volume::WallthicknessEstimator *mWte;

		

		RENDER_MODE mRenderMode;

		imt::volume::ProgressReporter *mProgressReporter;

		VolumeInfo *mVolumeInfo;

		RTCScene mScene;

		unsigned int mGeomID;

		bool mPickingEnabled , mHeatMapViewingEnabled , mTracerBuilt;

		bool mViewPortEnabled;


		int mPlaneType;

		Eigen::Vector3f mPlaneCoeff;

		bool mIsInitialized;

		int mSliceWindowWidth, mSliceWindowHeight;

		RTCDevice mDevice;

		//pcl::PointCloud<pcl::PointXYZ>::Ptr mCloud;
		//pcl::KdTreeFLANN<pcl::PointXYZ> mKdtree;



	};	
		
		
		
		
		
	}
	
}




#endif