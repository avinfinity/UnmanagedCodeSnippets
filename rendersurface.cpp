/*
 * Copyright 2015 <copyright holder> <email>
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 */

#include "QApplication"
#include "rendersurface.h"
#include "QDockWidget"
#include "QDir"
#include "iostream"
#include "QDebug"
#include "openglhelper.h"
#include "QKeyEvent"

namespace imt{
  
  namespace volume{

	  RenderSurface::RenderSurface( imt::volume::VolumeInfo *volumeInfo, imt::volume::VolumeLoaderThread *volumeLoader, 
		                            imt::volume::VolumeRenderManager *volumeRenderManager , imt::volume::SliceRenderManager *sliceRenderManager, imt::volume::SlicerPlane *slicerPlane , RENDER_MODE renderMode, 
									RenderSurface *sharedRenderSurface, imt::volume::ProgressReporter *progressReporter) : QOpenGLWidget(sharedRenderSurface), mSliceRenderManager(sliceRenderManager),
									mSlicerPlane(slicerPlane), mProgressReporter( progressReporter )
{

	mCamera = new TrackBallCamera();

	mVolumeInfo = volumeInfo;

	mVolumeRenderManager = volumeRenderManager;

	mVolumeLoader = volumeLoader;

	mRenderVolume = false;

	mUpdateTexture = false;

	mInfoW = 0;

	mRenderMode = renderMode;

	if ( mProgressReporter )
	{
		connect( mProgressReporter , SIGNAL( updateDisplayS() ), this , SLOT( updateDisplay() ) );
	}

	mSliceRenderManager = 0;

	if ( mRenderMode < 3 )
	{
		mSliceRenderManager = new SliceRenderManager( mVolumeInfo  );

		mSliceRenderManager->setWallthicknessEstimator(mVolumeRenderManager->wallthicknessEstimator());

		connect(mVolumeInfo, SIGNAL(surfaceDataChangedS()), mSliceRenderManager, SLOT(setChangedData()));
	}

	if (mRenderMode == VOLUME3D)
	{

		mSegmentationBenchMark = new imt::volume::SegmentationBenchmark( mVolumeInfo );

		connect(mVolumeInfo, SIGNAL(surfaceDataChangedS()), mVolumeRenderManager, SLOT(setChangedData()));
	}

  
    createGuiObjects();
    setupGui();
    createConnections();
}





	  TrackBallCamera *RenderSurface::camera()
	  {
		  return mCamera;

	  }



	  void RenderSurface::setCamera(TrackBallCamera *camera)
	  {

		  mCamera = camera;
	  }




void RenderSurface::createGuiObjects()
{

}


void RenderSurface::setupGui()
{
}


void RenderSurface::createConnections()
{
	if (mRenderMode == VOLUME3D)
	{
		connect(mVolumeLoader, SIGNAL(finished()), this, SLOT(updateVolume()));
	}


	
}





void RenderSurface::initializeGL()
{

	
	setMouseTracking(true);
//  initializeOpenGLFunctions();   
 
    qDebug() << " OpenGL Version : " << QGLFormat::openGLVersionFlags() << endl;

    qDebug() << "Context valid: " << context()->isValid();

    QString versionString( QLatin1String(reinterpret_cast< const char* >( glGetString( GL_VERSION ) )) );

    qDebug() << "Driver Version String : "<< versionString << endl;

    GL_CHECK( glEnable(GL_DEPTH_TEST) );
    
    QSurfaceFormat f = format();
    
    GL_CHECK( glEnable(GL_CULL_FACE) );

	

	if (mRenderMode == VOLUME3D)
	{

		mVolumeRenderManager->setCamera(mCamera);

		if ( mSlicerPlane )
		{
			mSlicerPlane->init();

			mSlicerPlane->setCamera(mCamera);
		}

		mVolumeRenderManager->init();

		mSegmentationBenchMark->init();

		std::cout << " initializing slice render manager " << std::endl;

	}
	else
	{
		mSliceRenderManager->setCamera(mCamera);

		mSliceRenderManager->init();
	}


	//std::cout << " default fbo in init : " << defaultFramebufferObject() << std::endl;
	if (mRenderMode == VOLUME3D)
	{
		mVolumeRenderManager->setDefaultFrameBuffer(defaultFramebufferObject());
	}
	

	if (mSlicerPlane )
	{
		mSlicerPlane->setDfaultFBO(defaultFramebufferObject());
	}

	mUpdateTexture = true;

	if ( mSliceRenderManager )
	{
		mSliceRenderManager->setDefaultFBO(defaultFramebufferObject());
	}

   qDebug()<<" initialization completed "<< endl;
}


void RenderSurface::setInfoW(QWidget *infoW)
{
	mInfoW = infoW;

	mInfoW->setParent(this);

	mInfoW->move(width() - mInfoW->width(), height() - mInfoW->height());
}


void RenderSurface::setSphereInfoW( QWidget *sphereInfoW )
{
	mSphereInfoW = sphereInfoW;

	mSphereInfoW->setParent(this);

	mSphereInfoW->setVisible(false);

}


void RenderSurface::setProgressReporter(imt::volume::ProgressReporter *progressReporter)
{

	mProgressReporter = progressReporter;

	mVolumeRenderManager->setProgressReporter(mProgressReporter);
}

 
void RenderSurface::resizeGL( int w , int h )
{
	if ( mCamera )
	mCamera->setViewPortDimension(w, h);

	update();

	if ( mInfoW )
	mInfoW->move(width() - mInfoW->width(), height() - mInfoW->height());
}
  
void RenderSurface::paintGL()
{

  GL_CHECK( glClearColor(0.1f, 0.1f, 0.1f, 1.0f) );
  GL_CHECK( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) ); //);

  //if ( mRenderMode == VOLUME3D )
  //{
	 // if (mProgressReporter->mDisplayMode == imt::volume::ProgressReporter::WALL_THICKNESS)
	 // {

  //std::cout << "  rendering volume " << std::endl;
		  renderVolume();

		 // if (mSlicerPlane)
		 // {
			//  mSlicerPlane->render();
		 // }
	  //}
	  //else if ( mProgressReporter->mDisplayMode == imt::volume::ProgressReporter::SEGMENTATION_BENCHMARK )
	  //{
		 // mSegmentationBenchMark->render();
	  //}


  //}
  //else
  //{
	 // mSliceRenderManager->setDefaultFBO(defaultFramebufferObject());

	 // renderSlice();
  //}

}

void RenderSurface::renderVolume()
{
	if (mUpdateTexture)
	{
		mVolumeRenderManager->loadVolume(mVolumeInfo->mVolumeData, mVolumeInfo->mWidth,
			mVolumeInfo->mHeight, mVolumeInfo->mDepth,
			mVolumeInfo->mVolumeOrigin, mVolumeInfo->mVoxelStep);

		mUpdateTexture = false;

		mVolumeRenderManager->setDefaultFrameBuffer(defaultFramebufferObject());

		if (mSlicerPlane)
		{
			mSlicerPlane->setDfaultFBO(defaultFramebufferObject());
		}

		if (mSliceRenderManager)
		{
			mSliceRenderManager->setDefaultFBO(defaultFramebufferObject());
		}

	}


#if 0

	if (mVolumeInfo->mWallthicknessDataChanged)
	{
		mVolumeInfo->mWallthicknessDataChanged = false;

		//mVolumeRenderManager->loadSurface(mVolumeInfo->mVertices, mVolumeInfo->mVertexColors, mVolumeInfo->mVertexNormals, mVolumeInfo->mFaceIndices);

		mVolumeRenderManager->loadSurface(mVolumeInfo->mVertices, mVolumeInfo->mRayVertexColors, mVolumeInfo->mSphereVertexColors, mVolumeInfo->mVertexNormals, mVolumeInfo->mFaceIndices);

		if (mSlicerPlane)
		{

			std::cout << " setting cage visible : " << mVolumeInfo->mVoxelStep.transpose() << " " << mVolumeInfo->mWidth << " " << mVolumeInfo->mHeight << " " << mVolumeInfo->mDepth << std::endl;

			Eigen::Vector3f farEnd;

			farEnd(0) = mVolumeInfo->mWidth * mVolumeInfo->mVoxelStep(0);
			farEnd(1) = mVolumeInfo->mHeight * mVolumeInfo->mVoxelStep(1);
			farEnd(2) = mVolumeInfo->mDepth * mVolumeInfo->mVoxelStep(2);

			mSlicerPlane->setDimension(mVolumeInfo->mVolumeOrigin, farEnd);

			mSlicerPlane->setCageVisible(true);

		}

		if (mSliceRenderManager)
		{
			mSliceRenderManager->setNumTriangles(mVolumeInfo->mFaceIndices.size() / 3);
		}
		
	}



	if (mProgressReporter && mProgressReporter->mShowSideBySideRS)
	{
		if (!mSphereInfoW->isVisible())
		{
			mSphereInfoW->setVisible(true);

			mInfoW->move(width() / 2 - mInfoW->width(), height() - mInfoW->height());

			mSphereInfoW->move(width() - mInfoW->width(), height() - mInfoW->height());
		}
	}
	else
	{
		if (mSphereInfoW->isVisible())
		{
			mSphereInfoW->setVisible(false);

			mInfoW->move(width() - mInfoW->width(), height() - mInfoW->height());
		}

	}
#endif



	if ( mRenderMode == VOLUME3D )
	{
		//std::cout << " rendering volume " << std::endl;

		mVolumeRenderManager->render();
	}
	 
}

void RenderSurface::renderSlice()
{
	if ( mSliceRenderManager && mSlicerPlane && mRenderMode < 3)
	{
		mSliceRenderManager->setPlaneType(mRenderMode, mSlicerPlane->getPlaneSlicerCoeffs(), width(), height()); //

		if (mSliceRenderManager->isInitialized())
		{

		  mSliceRenderManager->render();
		}
		else
		{
			std::cout << " slice render manager not initialized " << std::endl;
		}
		      
	}


}



void RenderSurface::updateVolume()
{
	mRenderVolume = true;

	mUpdateTexture = true;

	//mVolumeRenderManager->setVolumeInfo(mVolumeInfo);

}


void RenderSurface::updateDisplay()
{
	//mProgressReporter->updateDisplay();
	//std::cout << " updating display : " << mRenderMode << std::endl;
	update();
}


void RenderSurface::keyPressEvent(QKeyEvent* e)
{
//  if( mCurrentRenderContext )
//  {
//    mCurrentRenderContext->registerKeyPress( e->key() );
  
	if ( mRenderMode == VOLUME3D )
	mCamera->registerKeyPress(e->key());

	mSliceRenderManager->registerKeyPressEvent(e);

    update();
//  }
}


void RenderSurface::keyReleaseEvent(QKeyEvent* e)
{
//  if( mCurrentRenderContext )
//  {
//    mCurrentRenderContext->registerKeyRelease( e->key() );
	if (mRenderMode == VOLUME3D)
	mCamera->registerKeyRelease(e->key());

	mSliceRenderManager->registerKeyReleaseEvent(e);

    update();
//  }
}

void RenderSurface::mouseMoveEvent(QMouseEvent* event)
{
//  if( mCurrentRenderContext )
//  {
//    mCurrentRenderContext->registerMouseMove( event->pos() , event->buttons() );
	if (mRenderMode == VOLUME3D)
	mCamera->registerMouseMove(event->pos(), event->buttons());

	mSliceRenderManager->registerMouseMoveEvent(event);

	if (mSlicerPlane)
	{
		mSlicerPlane->registerMouseMoveEvent(event);

		mProgressReporter->updateDisplay();
	}

    update();
//  }
}

void RenderSurface::mousePressEvent(QMouseEvent* event)
{
	if (mRenderMode == VOLUME3D)
     mCamera->registerMousePress( event->pos() , event->buttons() );

	 mSliceRenderManager->registerMousePressEvent(event);

	 if ( event->buttons() == Qt::RightButton )
	 {
		 if ( mSlicerPlane )
		 {
			 mSlicerPlane->setPlaneType( mRenderMode , height() );

			 mSlicerPlane->registerMousePressEvent(event);

			 mProgressReporter->updateDisplay();

		 }
	 }
  
     update();
}

void RenderSurface::mouseReleaseEvent(QMouseEvent* event)
{
  //if( mCurrentRenderContext )
  //{

	if (mRenderMode == VOLUME3D)
  mCamera->registerMouseRelease( event->pos()  );

  mSliceRenderManager->registerMouseReleaseEvent(event);

  if (mSlicerPlane)
  {
	  mSlicerPlane->registerMouseReleaseEvent(event);
  }
  
  update();
  //}
}

void RenderSurface::wheelEvent(QWheelEvent* event)
{
  //if( mCurrentRenderContext )
  //{

	if (mRenderMode == VOLUME3D)
   mCamera->registerMouseWheel( event->delta() );

   mSliceRenderManager->registerWheelEvent(event);
    
   update();
  //}
}




RenderSurface::~RenderSurface()
{

}

  }


}

#include "rendersurface.moc"
