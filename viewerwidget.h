#ifndef __VIEWERWIDGET_H__
#define __VIEWERWIDGET_H__

#include "QOpenGLWidget"
#include "QOpenGLFunctions"
#include "trackballcamera.h"
#include "volumeinfo.h"
#include "opencvincludes.h"
#include "oivvolumenode.h"
//#include "GLWallThicknessSlice.h"

class SoVolumeData;

class ViewerWidget : public QOpenGLWidget , public QOpenGLFunctions
{
	
	public:
	
	ViewerWidget();

	virtual void initializeGL();
	virtual void resizeGL(int w, int h);

	virtual void paintGL();

	void setVolInfo(imt::volume::VolumeInfo* volInfo);

	void keyPressEvent(QKeyEvent* e);
	void keyReleaseEvent(QKeyEvent* e);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent * event);

protected:

	void renderSlice(int planeType);
	void renderModel();
	void init();
	GLuint compileShaders(GLenum shaderType, const char *shaderSource);
	void updateImage();


protected:


	struct VertexData{

		Eigen::Vector3f mVertex , mVertexColor , mVertexNormal;
	};


	struct SliceVertexData
	{
		Eigen::Vector3f mVertex;
		Eigen::Vector2f mTexCoords;
	};


	GLuint mVAO[4] , mVBO[8];

	GLuint mThicknessVolumeTex, mThicknessDisplayTexXY, mThicknessDisplayTexYZ, mThicknessDisplayTexZX;

	GLint mSliceProgram , mModelProgram;

	std::vector< VertexData > mVertexData;
	

	TrackBallCamera mCameras[4] , *mCamera;

	imt::volume::VolumeInfo *mVolInfo;

	QString mVertexShaderSrc, mFragmentShaderSrc;
	QString mSliceVertexShaderSrc, mSliceFragmentShaderSrc;

	bool mIs3DModelInitialized;

	int mRenderMode;

	std::vector< SliceVertexData > mSliceVertices;

	bool mIsImageChanged;

	int mPlaneType;

	cv::Mat mSliceXY, mSliceYZ, mSliceZX;

	QMatrix4x4 mOrthoMatrix;

	//Zeiss::IMT::UI::METROTOM::GLWallThicknessSlice *mSlice;


	SoVolumeData *mVolumeData;

	imt::volume::OivVolumeNode *mVolumeNode;

	bool _IsCameraInitialized;

};



#endif