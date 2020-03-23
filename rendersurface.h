#ifndef __IMT_RENDERSURFACE_H__
#define __IMT_RENDERSURFACE_H__

#include "QOpenGLWidget"
#include "QOpenGLFunctions_3_3_Core"

#include "trackballcamera.h"
#include "volumerendermanager.h"
#include "volumeloaderthread.h"
#include "volumeinfo.h"
#include "QWidget"
#include "progressreporter.h"
#include "slicerendermanager.h"
#include "slicerplane.h"
#include "segmentationbenchmark.h"


namespace imt{
	
	namespace volume{


class RenderSurface : public QOpenGLWidget //, public  QOpenGLFunctions_3_3_Core
{
  Q_OBJECT
  
protected:



  
  
protected:

   void createGuiObjects();
   void setupGui();
   void createConnections();

   void renderVolume();
   void renderSlice();



 public:


	 enum RENDER_MODE{ SLICE_2DXY = 0, SLICE_2DYZ, SLICE_2DZX, VOLUME3D };

	 RenderSurface( imt::volume::VolumeInfo *volumeInfo, imt::volume::VolumeLoaderThread *volumeLoader, imt::volume::VolumeRenderManager *volumeRenderManager = 0, imt::volume::SliceRenderManager *sliceRenderManager = 0,
		            imt::volume::SlicerPlane *slicerPlane = 0, RENDER_MODE renderMode = RenderSurface::VOLUME3D, RenderSurface *sharedRenderSurface = 0, imt::volume::ProgressReporter *progressReporter = 0);
   
   virtual void initializeGL();
   virtual void resizeGL( int w , int h );
   virtual void paintGL(); 
   
   void keyPressEvent(QKeyEvent* e);
   void keyReleaseEvent(QKeyEvent* e);
   void mousePressEvent(QMouseEvent *event);
   void mouseReleaseEvent( QMouseEvent *event );
   void mouseMoveEvent(QMouseEvent *event); 
   void wheelEvent ( QWheelEvent * event );

   void setInfoW(QWidget *infoW);
   void setSphereInfoW(QWidget *sphereInfoW);

   void setProgressReporter(imt::volume::ProgressReporter *progressReporter);


   TrackBallCamera *camera();

   void setCamera(TrackBallCamera *camera);
   
   
   ~RenderSurface();


   protected slots:

   void updateVolume();

   void updateDisplay();



protected:

	TrackBallCamera *mCamera;
	
	VolumeRenderManager *mVolumeRenderManager;
	SliceRenderManager *mSliceRenderManager;
	SlicerPlane *mSlicerPlane;
	SegmentationBenchmark *mSegmentationBenchMark;

	VolumeLoaderThread *mVolumeLoader;
	VolumeInfo *mVolumeInfo;

	QWidget *mInfoW , *mSphereInfoW;

	imt::volume::ProgressReporter *mProgressReporter;


	bool mRenderVolume , mUpdateTexture;

	RENDER_MODE mRenderMode;
 

};
	
		
		
	}
	
	
}



#endif