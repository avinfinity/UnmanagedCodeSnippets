#include "viewerwidget.h"
#include "iostream"
#include "QFile"
#include "QKeyEvent"
#include <VolumeViz/nodes/SoVolumeData.h>
#include <Inventor/lock/SoLockMgr.h>
#include <Inventor/SoInteraction.h>
#include <Inventor/nodekits/SoNodeKit.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/errors/SoGLError.h>
#include <Inventor/errors/SoMemoryError.h>
#include <Inventor/errors/SoReadError.h>
#include "optimizedzxoreader.h"
#include "rendertimer.h"

	void checkOpenGLError(const char* stmt, const char* function, const char* file, int line)
	{
		GLenum err = glGetError();
		if (err != GL_NO_ERROR)
		{
			qDebug() << "OpenGL error : " << (char *)gluErrorString(err) << "  at" << stmt //( char * )gluErrorString( 
				<< "called from" << function << "in file" << file << "line" << line;
			abort();
		}
	}


#define glSafeCall(stmt) do { \
stmt; \
checkOpenGLError(#stmt, Q_FUNC_INFO, __FILE__, __LINE__); \
} while (0)

ViewerWidget::ViewerWidget()
{
	mRenderMode = 0;

	mIsImageChanged = false;

	mPlaneType = 0;

	//mSlice = 0;
}

void ErrorCB(const SoError *pError, void* /*pData*/)
{
	// My own message log file
	const SbString &string = pError->getDebugString();
	//System::String^ message = Zeiss::IMT::Core::StringConverter<std::string>::Convert(string.toStdString());
	//Zeiss::IMT::NG::Metrotom::Data::Management::AppSession::Messages->Publish("ErrorMessage", message);

	std::cout << " error : " << string.toStdString() << std::endl;
}

void ViewerWidget::initializeGL()
{
	initializeOpenGLFunctions();


	qDebug() << " OpenGL Version : " << QGLFormat::openGLVersionFlags() << endl;

	qDebug() << "Context valid: " << context()->isValid();

	QString versionString(QLatin1String(reinterpret_cast< const char* >(glGetString(GL_VERSION))));

	qDebug() << "Driver Version String : " << versionString << endl;


	mIs3DModelInitialized = false;

	_IsCameraInitialized = false;

	//cv::Mat testIm = cv::imread("C:/Users/INASING1/Pictures/dog.jpg");

	//cv::resize(testIm, mSliceYZ , cv::Size(mVolInfo->mDepth, mVolInfo->mHeight));

	//std::cout << mVolInfo->mRayVertexColors.size() << " " << mVolInfo->mSphereVertexColors.size() << " " << mVolInfo->mVertexColors.size() << std::endl;

	mIsImageChanged = true;

	makeCurrent();

	SoDB::init();
	SoNodeKit::init();
	SoInteraction::init();
	SoVolumeRendering::init();

	SoDB::setDelaySensorTimeout(0.01f);

	// Reset error handlers to my own function
	SoError::setHandlerCallback(ErrorCB, NULL);
	SoDebugError::setHandlerCallback(ErrorCB, NULL);
	SoGLError::setHandlerCallback(ErrorCB, NULL);
	SoMemoryError::setHandlerCallback(ErrorCB, NULL);
	SoReadError::setHandlerCallback(ErrorCB, NULL);

	//mSlice = new Zeiss::IMT::UI::METROTOM::GLWallThicknessSlice();

	SoDB::setSystemTimer( new RenderTimer );


	mVolumeData = new SoVolumeData;

	mVolumeData->ref();

	std::string zxoFilePath = "C:/Data/ZxoData/separated_part_7.ZXO";

	imt::volume::OptimizedZXOReader *reader = new imt::volume::OptimizedZXOReader();

	mVolumeData->setReader(*reader, true);

	mVolumeData->fileName.setValue(zxoFilePath);//filePath // //zxoFilePath.toStdString() //"backup.zxo2"


	auto volSize = mVolumeData->getVolumeSize();

	auto minL = volSize.getMin();
	auto maxL = volSize.getMax();

	mCamera = new TrackBallCamera();

	QVector3D objectCenter;

	objectCenter[0] = (minL[0] + maxL[0]) * 0.5;
	objectCenter[1] = (minL[1] + maxL[1]) * 0.5;
	objectCenter[2] = (minL[2] + maxL[2]) * 0.5;

	std::cout << minL << " " << maxL << std::endl;


	float len = sqrtf(objectCenter[0] * objectCenter[0] +
		objectCenter[1] * objectCenter[1] +
		objectCenter[2] * objectCenter[2]);

	mCamera->setObjectCenter(objectCenter);
	mCamera->setRadius(len);

	mCamera->setViewPortDimension(width(), height());
	mCamera->init();

	//mCamera->init();

	mVolumeNode = new imt::volume::OivVolumeNode(mVolumeData, mCamera);



	init();

}

void ViewerWidget::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);

	mCamera->setViewPortDimension(w, h);

	update();
}

void ViewerWidget::paintGL()
{

	//if (!_IsCameraInitialized)
	//{
	//	mCamera->setViewPortDimension(width(), height());

	//	mCamera->init();

	//	_IsCameraInitialized = true;
	//}

	//glEnable(GL_DEPTH_TEST);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //);

	//if ( mSlice )
	//mSlice->Render();

	mVolumeNode->Render();

	return;

	//if (mRenderMode == 0)
	//{
	//	renderModel();
	//}
	//else
	//{
	//	renderModel();
	//	renderSlice(mRenderMode - 1);
	//}
}


void ViewerWidget::setVolInfo(imt::volume::VolumeInfo* volInfo)
{
	mVolInfo = volInfo;

}



void ViewerWidget::keyPressEvent(QKeyEvent* e)
{
	if ( e->key() == Qt::Key_0)
	{
		mRenderMode = 0;
	}

	if (e->key() == Qt::Key_1)
	{
		mRenderMode = 1;
	}

	if (e->key() == Qt::Key_2)
	{
		mRenderMode = 2;
	}

	if (e->key() == Qt::Key_3)
	{
		mRenderMode = 3;
	}
}

void ViewerWidget::keyReleaseEvent(QKeyEvent* e)
{
	
}

void ViewerWidget::mousePressEvent(QMouseEvent *event)
{
	mCameras[0].registerMousePress(event->pos(), event->buttons());

	mCameras->registerMouseRelease(event->pos());

	update();
}

void ViewerWidget::mouseReleaseEvent(QMouseEvent *event)
{
	mCameras[0].registerMouseRelease(event->pos());

	mCameras->registerMouseRelease(event->pos());

	update();
}

void ViewerWidget::mouseMoveEvent(QMouseEvent *event)
{
	mCameras[0].registerMouseMove(event->pos(), event->buttons());

	mCamera->registerMouseMove(event->pos(), event->buttons());

	update();
}


void ViewerWidget::wheelEvent(QWheelEvent * event)
{
	mCameras[0].registerMouseWheel(event->delta());

	mCameras->registerMouseWheel(event->delta());

	update();
}

void ViewerWidget::init()
{
	setMouseTracking(true);

}

GLuint ViewerWidget::compileShaders(GLenum shaderType, const char *shaderSource)
{
	GLuint shader = glCreateShader(shaderType);

	glShaderSource( shader, 1, &shaderSource, NULL );
	glCompileShader( shader );

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if ( status == GL_FALSE )
	{
		GLchar emsg[10024];
		
		glGetShaderInfoLog(shader, sizeof(emsg), 0, emsg);
		
		QString errMsg = QString("Error compiling GLSL shader (%s): %s\n") + emsg;

		qDebug() << " error in compiling shaders , file : " << __FILE__ << " : line : " << __LINE__ << errMsg << endl;

	}

	return shader;
}


void ViewerWidget::updateImage()
{
	
}


void ViewerWidget::renderSlice( int planeType )
{

}


void ViewerWidget::renderModel()
{

}