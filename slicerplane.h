
#ifndef __IMT_SLICERPLANE_H__
#define __IMT_SLICERPLANE_H__

#include "QOpenGLFunctions_4_0_Core"
#include "QOpenGLFunctions_4_3_Core"
#include "qgl.h"
#include "eigenincludes.h"
#include "trackballcamera.h"
#include "progressreporter.h"
#include "QMouseEvent"

namespace imt{
	
	namespace volume{
		
		
	class SlicerPlane : public QOpenGLFunctions_4_0_Core
	{
		
		
		public:
		
		SlicerPlane();
		
		void setDimension( const Eigen::Vector3f& minLimits , const Eigen::Vector3f& maxLimits );

		void setCageVisible(bool visib);

		void setPlaneVisible(bool visib);

		void init();

		void initShaders();

		void render();

		void setPlaneType(int type , int h );

		void setCoeff( const Eigen::Vector3f& coeff );

		void setCamera(TrackBallCamera *camera);

		void setDfaultFBO(int defaultFBO);

		void setProgressReporter(imt::volume::ProgressReporter *progressReporter);

		void registerMouseMoveEvent(QMouseEvent* event);

		void registerMousePressEvent(QMouseEvent* event);

		void registerMouseReleaseEvent(QMouseEvent* event);

		Eigen::Vector3f& getPlaneSlicerCoeffs();


	protected:

		GLuint compileShaders(GLenum shaderType, const char *shaderSource);

		void updateVertexData();

		void renderCage();

		void renderPlane();


	protected:

		Eigen::Vector3f mMinLimits, mMaxLimits;

		Eigen::Vector3f mPlaneCoeffs;

		GLuint mVBOs[5] , mVAOs[2] , mBoundingBoxIndicesBuffer , mBoundingBoxVertexBuffer , 
			   mPlaneBoundaryBuffer , mPlaneIndicesBuffer , mPlaneVertexBuffer ;

		GLuint mDefaultFBO;

		GLint mWireBoxProgram , mSlicerPlaneProgram;

		bool mRenderPlane, mRenderCage , mCageChanged , mPlaneChanged;

		int mPlaneType;


		TrackBallCamera *mCamera;

		imt::volume::ProgressReporter *mProgressReporter;

		QPoint mCurrentPos, mPreviousPos;

		bool mRightButtonPressed;

		int mHeight;
		
		
		
	};	
		
		
		
	}
	
}





#endif