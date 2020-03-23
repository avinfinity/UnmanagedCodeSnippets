#ifndef _VOL_REND_
#define _VOL_REND_

#if defined(_WIN32)
#if !defined(_CRT_SECURE_NO_DEPRECATE)
# define _CRT_SECURE_NO_DEPRECATE
#endif
#endif

#include "volRendAllHeader.h"

#include <Inventor/nodes/SoFrontBufferGroup.h>
#define ROI_TAB_BOX 1

#define _SEGY_FILE    "WahaPub.sgy"
#define VOL_VIEWER_INFO "VolumeViewerInfo"

//--------------------------------------------------------------

#define PIO2 1.570796326794896619231f

#ifdef _WIN32
#define DISPLAY_ERROR(_str_) \
    SoDebugError::post("VolRend", _str_);
#else
#define DISPLAY_ERROR(_str_) fprintf( stderr, _str_ );
#endif

#define NUM_ENTRIES_CMAP 65536

#ifndef MIN
#define MIN(a, b) ((a)<(b)?(a):(b))
#endif

SoUniformGridClipping *createDefaultGrid(SbBool createDistanceTexture = FALSE);
void genDefaultClipMap(SoUniformGridClipping *grid);
SbString getDataFilePath( const SbString &filename );
void initDataNode(SoVolumeData * volData, const SbString &filepath);
void createHistoLegend( SoGroup* parent, int whichColormap = 0 );
//float *getColorTable( const SbString &colorFile );
int loadColorTable( const SbString &colorFile, int &numComps, float *&values );
void updateOpaqueRegion( SoTransferFunction *tf, int minCm, int maxCm, int minOp, int maxOp, SbBool invertFlag);
void viewerFPSCB( float fps, void *userData, SoXtViewer *viewer );
void keyEventCB( void *userData, SoEventCallback *node);
void motionSliceCallback(void *user_data, SoJackDragger *dragger);
void motionObliSliceCallback(void *user_data, SoJackDragger *dragger);

SoSeparator* createColorMapLegend( SoTransferFunction *pXferNode );
void updateVolBBoxGeometry();
void updateVolAxesGeometry();
#ifdef _WIN32
SbBool EventCB( void *userData, MSG *msg );
#endif
void viewerStartCB( void *userData, SoXtViewer *viewer );
void viewerFinishCB( void *userData, SoXtViewer *viewer );
void callbackCB(void *userData, SoAction *action);

void fpsCB(float fps, void *, SoXtViewer *viewer) ;

void ROIChangedCB( void *data, SoDragger *dragger); // to update size/bounds in UI
void updateROIdisplay( int xmin, int ymin, int zmin, int xmax, int ymax, int zmax );

void updateColorMapLegend( SoTransferFunction *pXferNode, SoTexture2 *pTexNode );
void disableOverrideCB(void *userData, SoAction *action);
void colorMapSensorCB( void *data, SoSensor *sensor );
void
mouseKeyEventCB(void *userData, SoEventCallback *eventCB);
void
mouseMoveEventCB(void *userData, SoEventCallback *eventCB);

void setDraggerSlicePos (const SbVec3f &pos);

  void initDialog(void);

enum slice_orientation {
	X, Y, Z
} ;

void *writeFile(SoGroup* sep, const SbString &filename);

// static int page_size = 64;
// static int isFrontBufRendering = 0;
// static int isInteracting = 0;

void createOrthoSlices(size_t, int orthoVis = SO_SWITCH_ALL);
void resetDraggerPos(int sliceNumber);
void updateSelectSliceMenu(int num);
void updateOrthoSliceMenu(int sliceID);

void updateSelectGridMenu(int num);
void createGrids(size_t num);
void addGridToSceneGraph(SoUniformGridClipping *grid, int unit);
void addClipVolumeToSceneGraph(SoVolumeClippingGroup *cg);
SoVolumeClippingGroup *getVolumeClippingGroup(int n);
SoTransform *getVolumeClippingGroupManip(int n);
void changeVolumeClippingGroupManip(int n, SoTransform *transform);

void switchOptimInterfaceToExpert(bool flag);

void updateSelectIsoMenu(int num);
void updateSelectMaterialMenu(int num);
void updateIsoSlider();
void updateIsoInterpolationMethod(int index);
void switchInterfaceToIsoMode(bool enable);
void activeEdgesAndJitter(bool enable);
void activeSurfaceScalar(bool enable);
void switchInterfaceToVolumeSkinMode(bool enable);
void fixedResCB(SoVolumeData::LDMResourceParameter::FixedResolutionReport& report, void*);


SoSeparator* createLoadingSign();
void loadCB(SbBool startLoading, void* userData );
void redrawLoadingSign(void *data, SoSensor *sensor) ;

#ifdef SPACEBALL
void motion3RotationCB(void *userData, SoEventCallback *cb);
void motion3TranslationCB(void *userData, SoEventCallback *cb);
void spaceballButtonCB(void *userData, SoEventCallback *cb);
#endif

#include "externGlobalVariable.h"


#endif


