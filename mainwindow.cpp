#include "mainwindow.h"
#include "QMenuBar"
#include "QFileDialog"
#include "QActionGroup"

MainWindow::MainWindow()
{
	createObjects();
	createConnections();
	setupUi();

}

void MainWindow::createObjects()
{

	mRootLO = new QVBoxLayout;
	mToolsMenu = menuBar()->addMenu("Tools");
	mViewMenu = menuBar()->addMenu("View");

	mViewModeMenu = mViewMenu->addMenu("View Mode");

	mCentralW = new QWidget;
	mCentralLO = new QGridLayout;
	 
	mCentralW->setLayout(mCentralLO);

	
	mVolumeInfo = new imt::volume::VolumeInfo();

	mVolumeInfo->loadVolume("C:\\Projects\\Wallthickness\\data\\separated_part_7.wtadat");

	mWte = new imt::volume::WallthicknessEstimator(*mVolumeInfo);
	mVolumeLoader = new imt::volume::VolumeLoaderThread(mVolumeInfo);
	mEstimatorTh = new imt::volume::WTEstimatorThread( mVolumeInfo , mWte );

	mSlicerPlane = new imt::volume::SlicerPlane();

	mVolumeRenderManager = new imt::volume::VolumeRenderManager();

	mVolumeRenderManager->setVolumeInfo(mVolumeInfo);

	mVolumeRenderManager->setWallthicknessEstimator(mWte);

	mProgressReporter = new imt::volume::ProgressReporter();

	mSliceRenderManager = new imt::volume::SliceRenderManager( mVolumeInfo );

	mRenderSurface = new imt::volume::RenderSurface(mVolumeInfo, mVolumeLoader, mVolumeRenderManager , mSliceRenderManager, 
		                                            mSlicerPlane , imt::volume::RenderSurface::VOLUME3D , 0 , mProgressReporter );

	mRenderSurfaceX = new imt::volume::RenderSurface(mVolumeInfo, mVolumeLoader, mVolumeRenderManager, mSliceRenderManager, mSlicerPlane, 
		                                             imt::volume::RenderSurface::SLICE_2DXY, mRenderSurface, mProgressReporter);
	
	mRenderSurfaceY = new imt::volume::RenderSurface(mVolumeInfo, mVolumeLoader, mVolumeRenderManager, mSliceRenderManager,
		                                             mSlicerPlane, imt::volume::RenderSurface::SLICE_2DYZ, mRenderSurface, mProgressReporter);

	mRenderSurfaceZ = new imt::volume::RenderSurface(mVolumeInfo, mVolumeLoader, mVolumeRenderManager, mSliceRenderManager,
		                                             mSlicerPlane, imt::volume::RenderSurface::SLICE_2DZX, mRenderSurface, mProgressReporter);

	mRenderSurfaceX->setCamera(mRenderSurface->camera());
	mRenderSurfaceY->setCamera(mRenderSurface->camera());
	mRenderSurfaceZ->setCamera(mRenderSurface->camera());

	//mCentralLO->addWidget(mRenderSurfaceZ , 0 , 0);
	//mCentralLO->addWidget(mRenderSurfaceY , 0 , 1);
	//mCentralLO->addWidget(mRenderSurfaceX , 1 , 0);
	mCentralLO->addWidget(mRenderSurface ); // , 1 , 1

	

	setCentralWidget(mCentralW);

	mLoadVolumeDataAct = new QAction("Load Volume", this);
	mRTWallThicknessAct = new QAction("Ray Trace", this);
	mSphereWallTicknessAct = new QAction("Distance Transform( Sphere )", this);
	mCoordinateInputAct = new QAction( "Coordinate Input" , this );
	mExitAppAct = new QAction( "Exit" , this );
	mViewHeatMapAct = new QAction("View Heat Map", this);
	mViewSideBySideAct = new QAction("Side by Side Ray/Sphere", this);
	mShowCusrorRegionAct = new QAction("Show Region under Cursor" , this);
	mViewSphereDemoAct = new QAction("Show Sphere Demo", this);
	mViewCrossSectionAct = new QAction("View Cross Section", this);
	mViewRayAct = new QAction("View Ray");
	mViewSphereAct = new QAction("View Sphere");
	mSegmentationBenchmarkAct = new QAction("Segmentation Benchmark" , this);

	mView3to1Act = new QAction("3 : 1");
	mViewFullAct = new QAction("Full");

	mViewModeActG = new QActionGroup(this);

	mViewHeatMapAct->setCheckable(true);
	mViewSideBySideAct->setCheckable(true);
	mShowCusrorRegionAct->setCheckable(true);
	mViewSphereDemoAct->setCheckable(true);
	mViewCrossSectionAct->setCheckable(true);

	mInfoW = new QWidget;
	mInfoSphereW = new QWidget;

	mDialog = new QDialog;

	mToolsMenu->addAction(mLoadVolumeDataAct);
	mToolsMenu->addAction(mRTWallThicknessAct);
	mToolsMenu->addAction(mSphereWallTicknessAct);
	mToolsMenu->addAction(mCoordinateInputAct);
	mToolsMenu->addAction(mExitAppAct);

	mViewMenu->addAction( mViewHeatMapAct );
	mViewMenu->addAction( mViewSideBySideAct );
	mViewMenu->addAction( mShowCusrorRegionAct );
	mViewMenu->addAction( mViewCrossSectionAct );
	mViewMenu->addAction( mViewRayAct );
	mViewMenu->addAction( mViewSphereAct );


	mViewModeMenu->addAction(mView3to1Act);
	mViewModeMenu->addAction(mViewFullAct);

	mView3to1Act->setCheckable(true);
	mViewFullAct->setCheckable(true);

	mViewModeActG->addAction(mView3to1Act);
	mViewModeActG->addAction(mViewFullAct);

	mWta.setupUi(mInfoW);
	mSphereWta.setupUi(mInfoSphereW);

	mInput.setupUi(mDialog);

	mRenderSurface->setInfoW(mInfoW);
	mRenderSurface->setSphereInfoW(mInfoSphereW);
	mRenderSurface->setProgressReporter(mProgressReporter);

	

	QFont f("Arial", 12, QFont::Bold);
	mWta.mMouseCoordLB->setFont(f);
	mWta.mMouseThicknessLB->setFont(f);
	mWta.mMouseThicknessValLB->setFont(f);
	mWta.mMouxXLB->setFont(f);
	mWta.mMouxYLB->setFont(f);
	mWta.mMouxZLB->setFont(f);
	mWta.mPinnedCoordLB->setFont(f);
	mWta.mPinnedStatusLB->setFont(f);
	mWta.mPinnedThicknessLB->setFont(f);
	mWta.mPinnedThicknessValLB->setFont(f);
	mWta.mPinnedXLB->setFont(f);
	mWta.mPinnedYLB->setFont(f);
	mWta.mPinnedZLB->setFont(f);


	mSphereWta.mMouseCoordLB->setFont(f);
	mSphereWta.mMouseThicknessLB->setFont(f);
	mSphereWta.mMouseThicknessValLB->setFont(f);
	mSphereWta.mMouxXLB->setFont(f);
	mSphereWta.mMouxYLB->setFont(f);
	mSphereWta.mMouxZLB->setFont(f);
	mSphereWta.mPinnedCoordLB->setFont(f);
	mSphereWta.mPinnedStatusLB->setFont(f);
	mSphereWta.mPinnedThicknessLB->setFont(f);
	mSphereWta.mPinnedThicknessValLB->setFont(f);
	mSphereWta.mPinnedXLB->setFont(f);
	mSphereWta.mPinnedYLB->setFont(f);
	mSphereWta.mPinnedZLB->setFont(f);

}


void MainWindow::createConnections()
{
	connect(mExitAppAct, SIGNAL(triggered(bool)), this, SLOT(close()));
	connect(mLoadVolumeDataAct, SIGNAL(triggered(bool)), this, SLOT(loadVolume()));
	connect(mRTWallThicknessAct, SIGNAL(triggered(bool)), this, SLOT(computeRayTraceWallthickness()));
	connect(mSphereWallTicknessAct, SIGNAL(triggered(bool)), this, SLOT(computeSphereMethodWallThickness()));
	connect(mCoordinateInputAct, SIGNAL(triggered(bool)), this, SLOT(enterCoordinates()));

	connect(mProgressReporter, SIGNAL(setUnderCursorDataS(Eigen::Vector3f, float)), this, SLOT(updateUnderCursorData(Eigen::Vector3f, float)));

	connect(mProgressReporter, SIGNAL(setPinnedDataS(Eigen::Vector3f, float)), this, SLOT(updatePinnedData(Eigen::Vector3f, float)));

	connect(mProgressReporter, SIGNAL(setUnderCursorDataSphereS(Eigen::Vector3f, float)), this, SLOT(updateUnderCursorDataSphere(Eigen::Vector3f, float)));

	connect(mProgressReporter, SIGNAL(setPinnedDataSphereS(Eigen::Vector3f, float)), this, SLOT(updatePinnedDataSphere(Eigen::Vector3f, float)));

	connect(mViewHeatMapAct, SIGNAL(toggled(bool)), mProgressReporter, SLOT(showHeatMap(bool))); 
	connect(mViewSideBySideAct, SIGNAL(toggled(bool)), mProgressReporter, SLOT(showSideBySideRaySphere(bool)));

	//*mViewRayAct, *mViewSphereAct
	connect(mViewRayAct, SIGNAL(toggled(bool)), mProgressReporter, SLOT(showRay(bool)));
	connect(mViewSphereAct, SIGNAL(toggled(bool)), mProgressReporter, SLOT(showRay(bool)));

	connect( mView3to1Act , SIGNAL(triggered()), this, SLOT(view3to1Mode()));
	connect( mViewFullAct, SIGNAL(triggered()), this, SLOT(viewFullMode()));

}


void MainWindow::handlePinnedPoint()
{

	Eigen::Vector3f pinnedPoint;

	pinnedPoint(0) = mInput.mXLE->text().toFloat();
	pinnedPoint(1) = mInput.mYLE->text().toFloat();
	pinnedPoint(2) = mInput.mZLE->text().toFloat();

	std::cout << " pinned point " << pinnedPoint.transpose() << std::endl;

	mProgressReporter->setPinnedInput(pinnedPoint);
}


void MainWindow::setupUi()
{

	viewFullMode();
}


void MainWindow::loadVolume()
{

	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Create Project"), "",
		tr("Volume Files (*.wtadat);;"
		"All Files (*)"));


	if (!fileName.isEmpty())
	{
		mVolumeLoader->setFilePath(fileName);
		
	}

	mVolumeLoader->start();
}


void MainWindow::computeRayTraceWallthickness()
{
	mEstimatorTh->setMethod(imt::volume::WTEstimatorThread::RAY);

	mEstimatorTh->start();
}

void MainWindow::computeSphereMethodWallThickness()
{
	mEstimatorTh->setMethod(imt::volume::WTEstimatorThread::SPHERE);

	mEstimatorTh->start();

}

void MainWindow::enterCoordinates()
{

	mDialog->exec();
}


void MainWindow::updateUnderCursorData(Eigen::Vector3f coordinates, float thickness)
{
	mWta.mMouxXLB->setText(QString::number(coordinates(0)));
	mWta.mMouxYLB->setText(QString::number(coordinates(1)));
	mWta.mMouxZLB->setText(QString::number(coordinates(2)));

	mWta.mMouseThicknessValLB->setText(QString::number(thickness));

}
void MainWindow::updatePinnedData(Eigen::Vector3f coordinates, float thickness)
{
	mWta.mPinnedXLB->setText(QString::number(coordinates(0)));
	mWta.mPinnedYLB->setText(QString::number(coordinates(1)));
	mWta.mPinnedZLB->setText(QString::number(coordinates(2)));

	mWta.mPinnedThicknessValLB->setText(QString::number(thickness));


}


void MainWindow::updateUnderCursorDataSphere(Eigen::Vector3f coordinates, float thickness)
{
	mSphereWta.mMouxXLB->setText(QString::number(coordinates(0)));
	mSphereWta.mMouxYLB->setText(QString::number(coordinates(1)));
	mSphereWta.mMouxZLB->setText(QString::number(coordinates(2)));

	mSphereWta.mMouseThicknessValLB->setText(QString::number(thickness));

}
void MainWindow::updatePinnedDataSphere(Eigen::Vector3f coordinates, float thickness)
{
	mSphereWta.mPinnedXLB->setText(QString::number(coordinates(0)));
	mSphereWta.mPinnedYLB->setText(QString::number(coordinates(1)));
	mSphereWta.mPinnedZLB->setText(QString::number(coordinates(2)));

	mSphereWta.mPinnedThicknessValLB->setText(QString::number(thickness));

}


void MainWindow::view3to1Mode()
{


	mRenderSurfaceX->show();
	mRenderSurfaceY->show();
	mRenderSurfaceZ->show();
	//mCentralLO->addWidget(mRenderSurface, 1, 1);

	//setCentralWidget(mCentralW);

}


void MainWindow::viewFullMode()
{

	mRenderSurfaceX->hide();
	mRenderSurfaceY->hide();
	mRenderSurfaceZ->hide();

	//setCentralWidget(mRenderSurface);
}

#include "mainwindow.moc"