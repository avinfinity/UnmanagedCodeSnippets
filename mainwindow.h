#ifndef __IMT__MAINWINDOW_H__
#define __IMT__MAINWINDOW_H__

#include "QMainWindow"
#include "QVBoxLayout"
#include "QMenu"
#include "QAction"
#include "rendersurface.h"
#include "QObject"
#include "volumeloaderthread.h"
#include "volumeinfo.h"
#include "wtestimatorthread.h"
#include "progressreporter.h"
#include "QDialog"
#include "ui_wta.h"
#include "ui_input.h"
#include "volumerendermanager.h"
#include "slicerendermanager.h"
#include "slicerplane.h"
#include "wallthicknessestimator.h"

class MainWindow : public QMainWindow
{

	Q_OBJECT
	
	public:
	
	MainWindow();
	
	void createObjects();
	void createConnections();
	void setupUi();


	protected slots:

	void loadVolume();
	void computeRayTraceWallthickness();
	void computeSphereMethodWallThickness();
	void enterCoordinates();
	
	void updateUnderCursorData(Eigen::Vector3f coordinates, float thickness);
	void updatePinnedData(Eigen::Vector3f coordinates, float thickness);

	void updateUnderCursorDataSphere(Eigen::Vector3f coordinates, float thickness);
	void updatePinnedDataSphere(Eigen::Vector3f coordinates, float thickness);

	void handlePinnedPoint();

	void view3to1Mode();
	void viewFullMode();

protected:

	QVBoxLayout *mRootLO;

	QMenu *mToolsMenu , *mViewMenu , *mViewModeMenu;

	QWidget *mCentralW;

	QGridLayout *mCentralLO;

	QAction  *mLoadVolumeDataAct, *mRTWallThicknessAct, *mSphereWallTicknessAct , *mCoordinateInputAct , *mExitAppAct;

	QAction *mViewHeatMapAct , *mViewSideBySideAct , *mShowCusrorRegionAct , *mViewSphereDemoAct , *mViewCrossSectionAct , *mViewRayAct , *mViewSphereAct;

	QAction *mSegmentationBenchmarkAct;

	QAction *mView3to1Act, *mViewFullAct;

	QActionGroup *mViewModeActG;

	imt::volume::RenderSurface *mRenderSurface, *mRenderSurfaceX, *mRenderSurfaceY, *mRenderSurfaceZ;

	imt::volume::SlicerPlane *mSlicerPlane;

	imt::volume::WallthicknessEstimator *mWte;
	imt::volume::VolumeLoaderThread *mVolumeLoader;
	imt::volume::VolumeInfo *mVolumeInfo;
	imt::volume::WTEstimatorThread *mEstimatorTh;
	imt::volume::ProgressReporter *mProgressReporter;
	imt::volume::VolumeRenderManager *mVolumeRenderManager;
	imt::volume::SliceRenderManager *mSliceRenderManager;

	QWidget *mInfoW , *mInfoSphereW;
	QDialog *mDialog;

	Ui::WtaWidget mWta , mSphereWta;
	Ui::InputDialog mInput;



	
	
	
	
};

#endif