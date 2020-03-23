#ifndef _GLOBAL_
#define _GLOBAL_

// data files --------------------------------------------------
#include "volRendAllHeader.h"

//menu list
const char *fileNamesMenu[] = {
  "3DHead   (ldm) 256x256x109x8",
  "Virus    (am)  50x50x50x8",
#ifdef TGSONLY
  "Aneurism (am)  128x128x41x8",
#endif
  "Bonsai   (am)  256x256x256x8",
  "Brill    (am)  122x122x122x8",
  "CThead8  (am)  128x128x128x8",
  "CThead16 (am)  128x128x128x16",
  "agipAVS (fld) 128x128x128x8",
  "SYN_64   (vol) 64x64x64x8",
  "3DHead   (vol) 256x256x109x8",
  "Engine   (vol) 256x256x110x8",
  "Lobster  (vol) 320x320x34x8",
  "Colt     (vol) 300x455x91x8",
  "User filename"
};

SoTranslation* m_loadingSignTrans;
SoTranslation* m_colorMapTrans;
SoTranslation* m_histoTrans;

SoSwitch* m_logoSwitch = NULL;
SoSwitch* m_imgSwitch = NULL;

SoImageBackground* m_courtesyLogo = NULL;
//actual filenames
const char *fileNames[] = {
    //"head128.vox",
  "3DHEAD.ldm",
  "virus.am",
#ifdef TGSONLY
  "aneurism.am",
#endif
  "bonsai.am",
  "brill.am",
  "CThead8.am",
  "CThead16.am",
  "agipAVS.fld",
  "SYN_64.VOL",
  "3DHEAD.VOL",
  "ENGINE.VOL",
  "LOBSTER.VOL",
  "COLT.VOL",
  "medicalFoot.ldm",
  "User filename"
};
const int numDataFiles = sizeof(fileNames) / sizeof(char*);
int m_defaultFile = 0;

const char *dataPaths[] = {
  "../../Data/",
  "../Data/",
  "./Data/",
  "../../../Segy/Data/",
  "./",
  "../../../src/Inventor/demos/ShadersBrowser/Models/",
  "../../../../src/Inventor/demos/ShadersBrowser/Models/",
  "../../../../src/Inventor/examples/data/",
  "../../../src/Inventor/examples/data/"
};
const int numDataPaths  = sizeof(dataPaths) / sizeof(char*);

char m_startDirectory[4096];

int m_minColorMap = 0;
int m_maxColorMap = 255;
int m_minOpaqueMap = 0;
int m_maxOpaqueMap = 255;
int m_minSliceColorMap = 0;
int m_maxSliceColorMap = 255;
int m_minSliceOpaqueMap = 0;
int m_maxSliceOpaqueMap = 255;
float *m_userColorMap = 0;
int m_colorMapSize = 0;

SbString userColorMapName;
SbString sliceUserColorMapName;
SbString volumeUserColorMapName;

SbString userDataName;

int m_width, m_height, m_depth;
float m_minWidth, m_minHeight, m_minDepth, m_maxWidth, m_maxHeight, m_maxDepth;

SbVec3f m_draggerNormal;
SbVec3f m_draggerPos;
SbVec3f m_draggerSlicePos;

SoVolumeData* m_volData = NULL;
SoTransferFunction* m_transferFunction = NULL;
SoTransferFunction* m_sliceTransferFunction = NULL;

SoSeparator *m_root;
SoPickStyle* m_pick;
SbBool m_newFile = TRUE;
SoSwitch *m_textSwitch = NULL;
SoEventCallback *m_mouseMoveEvent;
SoEventCallback *m_mouseKeyEvent;

SoSwitch* m_backGroundSwitch = NULL;

SoSwitch* m_cplxSwitch = NULL;
SoShadowGroup* m_shadowGroup = NULL;
SoInteractiveComplexity* m_cplx = NULL;

SoSwitch *m_VolRenderSwitch;
SoMaterial *m_material = NULL;
SoVolumeRender *m_volRend = NULL;
int m_currentIso = 0;
int m_currentMaterial = 0;
int m_posMaxValueHisto = 0; //position of the 2nd max in histogram
SoVolumeIsosurface *m_volIsosurface = NULL;
SoVolumeRenderingQuality *m_volQuality = NULL;
SoJackDragger *m_draggerVolRender, *m_draggerObliSlice;
SbBool m_leftMousePressed;
SbBool m_leftMouseReleased;
SbBool m_showFrameRate;
SbBool m_showHistogram;

// Volume skin
SoSwitch *m_volSkinSwitch = NULL;
SoVolumeRenderingQuality *m_volSkinQuality = NULL;
SoVolumeSkin *m_volSkin = NULL;

// Geometry properties
SoGeometryPriority* m_geometryProprity;
SoSwitch* m_volumeGeometrySwitch;

//Volume Clipping
SoSwitch *m_volumeClippingSwitch;
SoSwitch *m_volumeClippingVisibleSwitch;
SoTransform *m_volumeClippingTransform;
SoTransformerManip *m_volumeClippingManip;
int m_selectedVolume;

//Grid clipping
SoSwitch *m_uniformGridClippingSwitch;
SoGroup *m_gridsGroup;
int m_selectedGrid;
const char *clipmapFilenames[] = {
  "clipmaps/perlin1.png",
  "Distance",
  "clipmaps/figures.png",
  "User filename"
};
std::vector<SbString> m_userGrid;
std::vector<int> m_predefinedGrid;

SoSwitch *m_draggerVolRenderSwitch,
         *m_draggerOrthSwitch,
         *m_draggerObliSwitch;
SoSwitch *m_volRenderOrthoSliceZSwitch,
         *m_volRenderOrthoSliceYSwitch,
         *m_volRenderOrthoSliceXSwitch;
SoSwitch *m_volRenderObliSliceSwitch,
         *m_volRenderOrthSliceSwitch,
         *m_volRenderVolGeomSwitch;
SoOrthoSlice *m_ZOrthoSlice,
             *m_YOrthoSlice,
             *m_XOrthoSlice;
SoVolumeRenderingQuality *m_obliSliceQuality;
SoObliqueSlice *m_obliSlice;
SoClipPlane *m_obliClipPlane;

SoLineSet *m_myLineSet;


#if defined(ROI_HANDLE_BOX)
#  define ROI_DRAGGER SoHandleBoxDragger
#elif defined(ROI_TAB_BOX)
#  define ROI_DRAGGER SoTabBoxDragger
#else
#  define ROI_DRAGGER SoTransformerDragger
#endif

SoTimerSensor* m_loadingSphereSensor;

SoSwitch *m_volBBoxSwitch,
  *m_volAxesSwitch,
  *m_volHistoSwitch,
  *m_volColormapSwitch,
  *m_loadingCBSwitch,
  *m_volTextSwitch,
  *m_volROISwitch,
  *m_volROIDraggerSwitch,
  *m_volSubDraggerSwitch,
  *m_dataRangeSwitch;
SoDataRange *m_dataRange;
SoScale *m_volROIScale;
SoROI *m_volROI;
SoROIManip *m_ROIManip;
SoPickStyle *m_volROIPickStyle;

SoGroup *sliceGroup = NULL;

const int DEF_NUM_SAMPLES = 10; // the normal default
float m_scaleFactor;

slice_orientation m_sliceOri;

char buffer[256];
VVizBenchmark m_bench;
const char* benchReadFileName = "../benchmarking/oneOrthoSlice.dat";
const char* benchWriteFileName = "VVizBenchmark.dat";
SbBool stopPushed = FALSE;
int m_numBenchLoop = 3;

std::vector< SoRef<SoVolumeRenderingQuality> > m_orthoQualityTab;
std::vector< SoRef<SoOrthoSlice> > m_orthoTab;
std::vector< OrthoSlicesData > m_orthoDataTab;
std::vector< SoRef<SoGroup> > m_groupTab;
std::vector< SoRef<SoSwitch> > m_switchOrthoTab;
SoRef<SoOrthoSlice> m_selectedOrthoSlice;
int m_lastNumSlices=0;
int m_currentSlice=0;
bool m_LDMmode = true;

SoSwitch* m_switchForOrthoDragger = NULL;
//file being currently open:
SbString m_filename;

//remap inside or outside min max
SbBool m_invertTransparency = FALSE;
SbBool m_invertSliceTransparency = FALSE;
int m_fixedRes = 1;

//for segy files
SoRotation* m_rotation = NULL;
SoRotation* m_segyRotation = NULL;
SoSwitch*   m_segySwitch   = NULL;

int m_levelMax = 0;
int m_tileSize = 64;
#ifndef CAVELIB
Widget myWindow;
SoXtExaminerViewer *m_viewer = NULL;
SoXtMaterialEditor *m_mtlEditor;
SoXtMaterialEditor *m_mtlSliceEditor;
SoXtDirectionalLightEditor *m_headlightEd;

volRendProfView *m_profile;
SoTopLevelDialog *myTopLevelDialog = NULL;
#else
void *myWindow = NULL;
#endif

SoMaterial* m_loadingSignMat = NULL;
SbThreadMutex* m_loadCBMutex;
SbBool m_changeMaterial = FALSE;
SbColor m_loadSignColor;

//visitor
SoLDMProximityVisitor* m_proxiVisitor = NULL;
SoLDMTileVisitor*      m_tileVisitor  = NULL;

#ifdef SPACEBALL
//SpaceBall
SoXtSpaceball *m_spaceball;
SbBool m_rotationMode;
SbBool m_translationMode;
SbBool m_colorMapMode;
SoTranslation *m_correct_trans;
SoTranslation *m_correct_trans_back;
sb_stuct *m_sb_struct;
SbBool m_sb_available;
#endif //spaceball

// Camera tracking
int m_camLockToROI = 0;
SbVec3f m_ROIcenter(0,0,0);

int page_size = 64;
int isFrontBufRendering = 0;
int isInteracting = 0;

// Volume scaling (e.g. scale time axis for seismic volume)
// Default is no scaling.  Applied to volume size (geometric extent)
SbBox3f m_volDefaultSize; // Before scaling
SbVec3f m_volScaleFactor(1,1,1);
#endif


