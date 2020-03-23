
#ifndef _EXTERN_GLOBAL_
#define _EXTERN_GLOBAL_


#include "volRendAllHeader.h"
#include "volRendProfView.h"

extern SoVolumeData* m_volData ;
extern SoTransferFunction* m_transferFunction ;

extern int m_defaultFile;
extern const char *fileNamesMenu[];
extern const char *fileNames[];
extern const int numDataFiles ;

extern SoTranslation* m_loadingSignTrans;
extern SoTranslation* m_colorMapTrans;
extern SoTranslation* m_histoTrans;

extern SoSwitch* m_logoSwitch;
extern SoSwitch* m_imgSwitch;
extern SoImageBackground* m_courtesyLogo;

extern const char *dataPaths[];
extern const int numDataPaths ;

extern char m_startDirectory[];

extern int m_minColorMap, m_maxColorMap, m_minOpaqueMap, m_maxOpaqueMap;
extern int m_minSliceColorMap, m_maxSliceColorMap, m_minSliceOpaqueMap, m_maxSliceOpaqueMap;

extern int m_colorMapSize;
extern float *m_userColorMap;

extern SbString userColorMapName, userDataName;
extern SbString sliceUserColorMapName, volumeUserColorMapName;

extern int m_width, m_height, m_depth;
extern float m_minWidth, m_minHeight, m_minDepth, m_maxWidth, m_maxHeight, m_maxDepth;

extern SbVec3f m_draggerNormal;
extern SbVec3f m_draggerPos;
extern SbVec3f m_draggerSlicePos;

extern float m_scaleFactor;

extern SoSeparator *m_root;
extern SoPickStyle* m_pick;
extern SbBool m_newFile ;
extern SoSwitch *m_textSwitch ;
extern SoEventCallback *m_mouseMoveEvent;
extern SoEventCallback *m_mouseKeyEvent;

extern SoSwitch* m_backGroundSwitch;
extern SoSwitch* m_cplxSwitch;
extern SoShadowGroup* m_shadowGroup;
extern SoInteractiveComplexity* m_cplx;


extern SoSwitch *m_VolRenderSwitch;
extern SoSwitch *m_volumeGeometrySwitch;
extern SoMaterial *m_material ;
extern SoVolumeRender *m_volRend ;

extern SoVolumeRenderingQuality *m_volQuality ;
extern SoVolumeIsosurface *m_volIsosurface ;
extern int m_posMaxValueHisto;
extern int m_currentIso ;
extern int m_currentMaterial ;
extern SoTransferFunction *m_transferFunction;
extern SoTransferFunction* m_sliceTransferFunction;
extern SoJackDragger *m_draggerVolRender, *m_draggerObliSlice;
extern SbBool m_leftMousePressed;
extern SbBool m_leftMouseReleased;
extern SbBool m_showFrameRate;
extern SbBool m_showHistogram;

// Volume skin
extern SoSwitch *m_volSkinSwitch;
extern SoVolumeRenderingQuality *m_volSkinQuality;
extern SoVolumeSkin *m_volSkin;

//Volume Clipping
extern SoSwitch *m_volumeClippingSwitch;
extern SoSwitch *m_volumeClippingVisibleSwitch;
extern SoTransform *m_volumeClippingTransform;
extern SoTransformerManip *m_volumeClippingManip;
extern int m_selectedVolume;

//Grid Clipping
extern SoSwitch *m_uniformGridClippingSwitch;
extern SoGroup *m_gridsGroup;
extern int m_selectedGrid;
extern const char *clipmapFilenames[];
extern std::vector<SbString> m_userGrid;
extern std::vector<int> m_predefinedGrid;

extern SoSwitch *m_draggerVolRenderSwitch,
          *m_draggerOrthSwitch,
          *m_draggerObliSwitch ;
extern SoSwitch *m_volRenderOrthoSliceZSwitch,
          *m_volRenderOrthoSliceYSwitch,
          *m_volRenderOrthoSliceXSwitch;
extern SoSwitch *m_volRenderObliSliceSwitch,
          *m_volRenderOrthSliceSwitch,
          *m_volRenderVolGeomSwitch ;
extern SoOrthoSlice *m_ZOrthoSlice,
              *m_YOrthoSlice,
             *m_XOrthoSlice;
extern SoObliqueSlice *m_obliSlice ;
extern SoVolumeRenderingQuality *m_obliSliceQuality;
extern SoClipPlane *m_obliClipPlane;
extern SoLineSet *m_myLineSet;


#if   defined(ROI_HANDLE_BOX)
#define ROI_DRAGGER SoHandleBoxDragger
#elif defined(ROI_TAB_BOX)
#define ROI_DRAGGER SoTabBoxDragger
#else
#define ROI_DRAGGER SoTransformerDragger
#endif
extern ROI_DRAGGER *m_volROIDragger; //mmh
extern SoTimerSensor* m_loadingSphereSensor;

extern SoSwitch *m_volBBoxSwitch,
                *m_volAxesSwitch,
                *m_volHistoSwitch,
                *m_volColormapSwitch,
                *m_loadingCBSwitch,
                *m_volTextSwitch,
                *m_volROISwitch,
                *m_volROIDraggerSwitch,
                *m_volSubDraggerSwitch,
                *m_dataRangeSwitch;
extern SoDataRange *m_dataRange;
extern SoScale  *m_volROIScale;
extern SoROI    *m_volROI;
extern SoROIManip*  m_ROIManip;
extern SoPickStyle *m_volROIPickStyle;

extern slice_orientation m_sliceOri;
extern char buffer[256];

extern const int DEF_NUM_SAMPLES ;	// the normal default
extern SoGroup *sliceGroup;
extern VVizBenchmark m_bench;
extern int m_numBenchLoop;

extern const char* benchReadFileName;
extern const char* benchWriteFileName;
extern SbBool stopPushed;

extern std::vector< SoRef<SoVolumeRenderingQuality> > m_orthoQualityTab;
extern std::vector< SoRef<SoOrthoSlice> > m_orthoTab;
extern std::vector< OrthoSlicesData > m_orthoDataTab;
extern std::vector< SoRef<SoGroup> > m_groupTab;
extern std::vector< SoRef<SoSwitch> > m_switchOrthoTab;
extern SoRef<SoOrthoSlice> m_selectedOrthoSlice;
extern int m_lastNumSlices;
extern int m_currentSlice;

extern SbString m_filename;

extern bool m_LDMmode;
extern SoSwitch* m_switchForOrthoDragger;
extern int m_fixedRes;

extern SoRotation* m_rotation;

extern SoRotation* m_segyRotation;
extern SoSwitch*   m_segySwitch  ;


extern SbBool    m_invertTransparency, m_invertSliceTransparency;
extern int m_levelMax;
extern int m_tileSize;
#ifndef CAVELIB
extern Widget myWindow;
extern SoXtExaminerViewer *m_viewer ;
extern SoXtMaterialEditor *m_mtlEditor;
extern SoXtMaterialEditor *m_mtlSliceEditor;
extern SoXtDirectionalLightEditor *m_headlightEd;
extern SoTopLevelDialog *myTopLevelDialog;
extern volRendProfView *m_profile;
#else
extern void *myWindow;
#endif

extern SoLDMProximityVisitor* m_proxiVisitor;
extern SoLDMTileVisitor*      m_tileVisitor ;
extern SoMaterial* m_loadingSignMat;
extern SbThreadMutex* m_loadCBMutex;
extern SbBool m_changeMaterial;
extern SbColor m_loadSignColor;


//SpaceBall
#ifdef SPACEBALL
extern SbBool m_sb_available;
extern SoXtSpaceball *m_spaceball;
extern SbBool m_rotationMode;
extern SbBool m_translationMode;
extern SbBool m_colorMapMode;
extern SoTranslation *m_correct_trans;
extern SoTranslation *m_correct_trans_back;
#define SB_STEP 0.0004f
typedef struct _sb_struct{
  SoTranslation *sb_struct_trans;
  SoRotation *sb_struct_rot;
  SbBool sb_next_colorMap;
} sb_stuct;
extern sb_stuct *m_sb_struct;
#endif //spaceball

// Camera tracking
extern int m_camLockToROI;
extern SbVec3f m_ROIcenter;

// Volume scaling (e.g. scale time axis for seismic volume)
extern SbBox3f m_volDefaultSize; // Before scaling
extern SbVec3f m_volScaleFactor;

extern int page_size;
extern int isFrontBufRendering;
extern int isInteracting;

#endif


