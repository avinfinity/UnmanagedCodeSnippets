/*=======================================================================
 *** THE CONTENT OF THIS WORK IS PROPRIETARY TO FEI S.A.S, (FEI S.A.S.),            ***
 ***              AND IS DISTRIBUTED UNDER A LICENSE AGREEMENT.                     ***
 ***                                                                                ***
 ***  REPRODUCTION, DISCLOSURE,  OR USE,  IN WHOLE OR IN PART,  OTHER THAN AS       ***
 ***  SPECIFIED  IN THE LICENSE ARE  NOT TO BE  UNDERTAKEN  EXCEPT WITH PRIOR       ***
 ***  WRITTEN AUTHORIZATION OF FEI S.A.S.                                           ***
 ***                                                                                ***
 ***                        RESTRICTED RIGHTS LEGEND                                ***
 ***  USE, DUPLICATION, OR DISCLOSURE BY THE GOVERNMENT OF THE CONTENT OF THIS      ***
 ***  WORK OR RELATED DOCUMENTATION IS SUBJECT TO RESTRICTIONS AS SET FORTH IN      ***
 ***  SUBPARAGRAPH (C)(1) OF THE COMMERCIAL COMPUTER SOFTWARE RESTRICTED RIGHT      ***
 ***  CLAUSE  AT FAR 52.227-19  OR SUBPARAGRAPH  (C)(1)(II)  OF  THE RIGHTS IN      ***
 ***  TECHNICAL DATA AND COMPUTER SOFTWARE CLAUSE AT DFARS 52.227-7013.             ***
 ***                                                                                ***
 ***                   COPYRIGHT (C) 1996-2014 BY FEI S.A.S,                        ***
 ***                        MERIGNAC, FRANCE                                        ***
 ***                      ALL RIGHTS RESERVED                                       ***
**=======================================================================*/
/*=======================================================================
** Author      : Pascal Estrade (Apr 2000)
** Modified by : Mike Heck (Aug 2000)
** Modified by : Thierry Dufour (Sep 2000)
** Modified by : Pascal Estrade (Dec 2000)
** Modified by : Jerome Hummel (Dec 2002)
**=======================================================================*/

#ifdef CAVELIB
#include "volRend.h"
#else
#include "VolumeVizAuditor.h"
#endif

#ifdef DVD_DEMO
#include <Inventor/lock/SoLockMgr.h>
#endif

#include "globalVariable.h"

#include "ResourceManagerAuditor.h"
#include <Inventor/sys/SoDynamicLibManager.h>
#include <Inventor/helpers/SbFileHelper.h>

#include <Inventor/nodes/SoShadowGroup.h>
#include <Inventor/nodes/SoInteractiveComplexity.h>

// Modified geometry for TabBoxDragger's box
// Volume rendering is usually done against a black background, so the
// default dragger box doesn't show up very well.  We just took these
// nodes from the standard data\draggerDefaults\tabBoxDragger.iv file
// that comes with the Open Inventor SDK and changed the color to the
// gray used by the transformer dragger (which looks semi-nice).  Below
// we'll call pDragger->setPart to substitute this for the default nodes.
/*
static const char *
tabBoxGeomBuffer = "\
#Inventor V2.0 ascii\n\
DEF tabBoxBoxGeom Separator { \
  renderCaching ON \
  PickStyle { style UNPICKABLE } \
  DrawStyle { style LINES } \
  MaterialBinding { value OVERALL } \
  Material { \
    diffuseColor	[ 0.5 0.5 0.5 ] \
    emissiveColor	[ 0.5 0.5 0.5 ] \
  } \
  Complexity { value .01  textureQuality  0 } \
  Cube {} \
}";
*/

SoGroup *myDialogGroup = NULL;

#ifndef CAVELIB
VolumeVizAuditor *myAuditor = NULL;

//Function called when main window is closed
void exitCB (void *, SoXtComponent *)
{
  for(int i = 0; i < m_lastNumSlices; i++) {
    if (m_orthoDataTab[i].timerSensor) {
      m_orthoDataTab[i].timerSensor->unschedule();
      delete m_orthoDataTab[i].timerSensor;
      m_orthoDataTab[i].timerSensor=NULL;
    }
  }
#if !defined(_WIN32) && !defined(__APPLE__)
  SoXt::canExit(true);
#endif
}

//Interface-----------------------------------
Widget buildInterface(Widget window)
{
  SoInput myInput;
  const char* ivFile = "volRendDialog.iv";
  if (! myInput.openFile(ivFile ))
    return NULL;
  myDialogGroup = SoDB::readAll( &myInput );
  if (! myDialogGroup)
    return NULL;
  myDialogGroup->ref();
  myTopLevelDialog = (SoTopLevelDialog *)myDialogGroup->getChild( 0 );

  myAuditor = new VolumeVizAuditor;

  myTopLevelDialog->addAuditor(myAuditor);
  SoDialogCustom *customNode = (SoDialogCustom *)myTopLevelDialog->searchForAuditorId(SbString("Viewer"));

  myTopLevelDialog->setSynchronizable(false);
  myTopLevelDialog->buildDialog( window, customNode != NULL );
  myTopLevelDialog->show();

  ResourceManagerAuditor *resourceManagerAuditor = new ResourceManagerAuditor(myTopLevelDialog);
  resourceManagerAuditor->setVolumeData(m_volData);
  myTopLevelDialog->addAuditor(resourceManagerAuditor);

  if (SoPreferences::getBool("VOLREND_remoteDialog", 0)) {
    myTopLevelDialog->setSynchronizable(false);
    myTopLevelDialog->buildDialog( window, TRUE );
  }
  else
  {
    myTopLevelDialog->setSynchronizable(false);
    myTopLevelDialog->buildDialog( window, FALSE );
  }
  myTopLevelDialog->show();

  // TODO: Make sure initial dialog values match actual data values.
  //       (data values could be changed by config file)
  int ival;
  SoDialogCheckBox *pChkBox;
  pChkBox = (SoDialogCheckBox *)myTopLevelDialog->searchForAuditorId("sliceequalresolution");
  if (pChkBox)
    pChkBox->state = SoLDMGlobalResourceParameters::getSliceEqualResolution();
  pChkBox = (SoDialogCheckBox *)myTopLevelDialog->searchForAuditorId("viewculling");
  if (pChkBox)
    pChkBox->state = SoLDMGlobalResourceParameters::getViewCulling();
  pChkBox = (SoDialogCheckBox *)myTopLevelDialog->searchForAuditorId("viewpointrefine");
  if (pChkBox)
    pChkBox->state = SoLDMGlobalResourceParameters::getViewpointRefinement();

  pChkBox = (SoDialogCheckBox *)myTopLevelDialog->searchForAuditorId("colormapvisibility");
  if (pChkBox) {
    ival = m_volColormapSwitch->whichChild.getValue();
    pChkBox->state = (ival == SO_SWITCH_ALL ? 1 : 0);
  }
  pChkBox = (SoDialogCheckBox *)myTopLevelDialog->searchForAuditorId("showhisto");
  if (pChkBox) {
    ival = m_volHistoSwitch->whichChild.getValue();
    pChkBox->state = (ival == SO_SWITCH_ALL ? 1 : 0);
  }
  pChkBox = (SoDialogCheckBox *)myTopLevelDialog->searchForAuditorId("drawboxe");
  if (pChkBox) {
    ival = m_volBBoxSwitch->whichChild.getValue();
    pChkBox->state = (ival == SO_SWITCH_ALL ? 1 : 0);
  }
  pChkBox = (SoDialogCheckBox *)myTopLevelDialog->searchForAuditorId("drawaxes");
  if (pChkBox) {
    ival = m_volAxesSwitch->whichChild.getValue();
    pChkBox->state = (ival == SO_SWITCH_ALL ? 1 : 0);
  }


  // By default all slider are disabled because we launch VolRend with resource set
  // on automatic. 
  SoDialogIntegerSlider *pIntSlider;
  pIntSlider = (SoDialogIntegerSlider *)myTopLevelDialog->searchForAuditorId("mainmemorymb");
  if (pIntSlider)
  {
    // Set max of the slider to the max default main memory allowed
    pIntSlider->enableNotify(FALSE);
    pIntSlider->max = (int32_t)SoLDMGlobalResourceParameters::getDefaultMaxMainMem();
    pIntSlider->enableNotify(TRUE);

    pIntSlider->value.enableNotify(FALSE);
    pIntSlider->value = 0;
    pIntSlider->value.enableNotify(TRUE);
    pIntSlider->enable = false;
    pIntSlider->toggleVisible = false;
  }

  pIntSlider = (SoDialogIntegerSlider *)myTopLevelDialog->searchForAuditorId("notifyrate");
  if (pIntSlider)
  {
    // Set max of the slider to the max default load rate
    pIntSlider->enableNotify(FALSE);
    pIntSlider->max = SoLDMGlobalResourceParameters::getLoadNotificationRate();
    pIntSlider->enableNotify(TRUE);

    pIntSlider->value.enableNotify(FALSE);
    pIntSlider->value = 0;
    pIntSlider->value.enableNotify(TRUE);
    pIntSlider->enable = false;
  }
  

  pIntSlider = (SoDialogIntegerSlider *)myTopLevelDialog->searchForAuditorId("numtexturesmb");
  if (pIntSlider) 
  {
    // Set max of the slider to the max default tex memory allowed
    pIntSlider->enableNotify(FALSE);
    pIntSlider->max = SoLDMGlobalResourceParameters::getMaxTexMemory();
    pIntSlider->enableNotify(TRUE);

    pIntSlider->value.enableNotify(FALSE);
    pIntSlider->value = 0;
    pIntSlider->value.enableNotify(TRUE);
    pIntSlider->enable = false;
  }

  pIntSlider = (SoDialogIntegerSlider *)myTopLevelDialog->searchForAuditorId("textureloadrate");
  if (pIntSlider) 
  {
    // Set max of the slider to the max default Texture load rate
    pIntSlider->enableNotify(FALSE);
    pIntSlider->max = SoLDMGlobalResourceParameters::getTex3LoadRate();
    pIntSlider->enableNotify(TRUE);

    pIntSlider->value.enableNotify(FALSE);
    pIntSlider->value = 0;
    pIntSlider->value.enableNotify(TRUE);
    pIntSlider->enable = false;
  }
  
  pIntSlider = (SoDialogIntegerSlider *)myTopLevelDialog->searchForAuditorId("slicetextureloadrate");
  if (pIntSlider) 
  {
    pIntSlider->enableNotify(FALSE);
    pIntSlider->max = SoLDMGlobalResourceParameters::getTex2LoadRate();
    pIntSlider->enableNotify(TRUE);

    pIntSlider->value.enableNotify(FALSE);
    pIntSlider->value = 0;
    pIntSlider->value.enableNotify(TRUE);
    pIntSlider->enable = false;
  }

  // Update number of slices on geometry tab
  SoDialogComboBox *pCombo = NULL;
  pCombo = (SoDialogComboBox *)myTopLevelDialog->searchForAuditorId("numberOfSlice");
  if (pCombo) {
    int value = SoPreferences::getInt( "VOLREND_numOrthoSlice", 1 );
    pCombo->selectedItem = value ? value-1 : value;
  }

  // Update intial colormap name
  // Note: For historical reasons the UI and the SoTransferFunction enum are not
  //       in the same order...
  // SoTransferFunction:
  //   None, Gray, Temperature, Physics, Standard, Glow, Blue_Red, Seismic, Blue_White_Red, Intensity, Label 256
  // UI:
  //   Seismic, Standard, Glow, BlueRed, Physics, Temperature, Grey, BlueWhiteRed, Intensity, Label 256
  static int mapTFtoUI[11] = { 0, 6, 5, 4, 1, 2, 3, 0, 7, 8, 9 };
  static int maplen = sizeof(mapTFtoUI) / sizeof(int);

  pCombo = (SoDialogComboBox *)myTopLevelDialog->searchForAuditorId("colormap");
  if (pCombo){
    int value = (int)m_transferFunction->predefColorMap.getValue();
    value = (value < maplen) ? mapTFtoUI[value] : 0;
    pCombo->selectedItem = value;
  }
  pCombo = (SoDialogComboBox *)myTopLevelDialog->searchForAuditorId("slicecolormap");
  if (pCombo){
    int value = (int)m_sliceTransferFunction->predefColorMap.getValue();
    value = (value < maplen) ? mapTFtoUI[value] : 0;
    pCombo->selectedItem = value;
  }

  return customNode ? customNode->getWidget() : window;
}
#endif // CAVELIB

void oneShotSensorCB(void *data, SoSensor *)
{
  //The ideal texture front hasn't been reached yet, let's render again
  VVizBenchmark* bench = (VVizBenchmark*)data;
  bench->play(FALSE,bench->m_numLoop);
}

void updateGlobalAlphaCB(void *, const SoMaterial *)
{
  SoDialogRealSlider* transparencySlider = (SoDialogRealSlider*)myTopLevelDialog->searchForAuditorId(SbString("globalalpha"));
  transparencySlider->value =  1.f - (float)m_material->transparency[m_currentMaterial];
}

//main----------------------------------------
#ifndef CAVELIB

#if !defined(_DEBUG)
#include <Inventor/SoWinApp.h>
#endif

int
main( int argc, char **argv)

#else  // CAVELIB
SoSeparator*
volumeVizInit( int argc, char **argv )

#endif // CAVELIB
{
#ifdef DVD_DEMO
#include <lock/demoUnlock.cxx>
#endif

  SoPreferences::setString("SO_DRAGGER_DIR",SbString("$OIVHOME/src/VolumeViz/VolRend/"));

  //This optims doesn't work with the jack dragger and grid clipping shader
  SoPreferences::setInt( "OIV_USE_COMPACT_PATHS", 0 );

  // Initialize Open Inventor
#ifndef CAVELIB
  myWindow = SoXt::init("");
  if (myWindow == NULL) exit(1);
#endif

  int readArgC = 0;
  int launchSizeW = 0;
  int launchSizeH = 0;
  SbString fileToLoad;
  if (argc > 1) {
  for (readArgC = 1; readArgC < argc; readArgC++) {
    if ( readArgC + 2 < argc && strcmp(argv[readArgC], "-size") == 0 ) {
      launchSizeW = atoi(argv[++readArgC]);
      launchSizeH = atoi(argv[++readArgC]);
    }
    else {
      fileToLoad = argv[readArgC];
    }
  }
  }


#if !defined(WIN32) && !defined(__APPLE__)
  //char *argv_[3];
  //int argc_ = 1;
  Widget toplevel;
  if (SoPreferences::getBool("VOLREND_remoteDialog", 0)) {
    //argv_[0] = (char *) "volRend";

    Display* display = XtOpenDisplay(SoXt::getAppContext(), "localhost:1.0", "volRend", "helloworldclass", NULL, 0, &argc, argv);
    fprintf(stderr, "display = %p\n", display);
    XtDisplayInitialize(SoXt::getAppContext(), display, "volRend", "helloworldclass", NULL, 0, &argc, argv);
    int n =  0;
    Arg  arglist[10] ;
    XtSetArg(arglist[n], XmNwidth, 400);  n++;
    XtSetArg(arglist[n], XmNheight, 600);  n++;

    toplevel = XtAppCreateShell(argv[0], NULL, applicationShellWidgetClass, display, arglist, n);
    fprintf(stderr, "toplevel = %p\n", toplevel);
    XtRealizeWidget(toplevel);

    int c = SoPreferences::getInt("VOLREND_waitTime", 1);
    fprintf(stderr, "Waiting %d for realizeWidget\n", c);
    sleep(c);
    fprintf(stderr, "OK\n");
  }
#endif

  //SoConsole::setMaxNumMessages(1000);
  // Initialize VolumeRendering extension
  SoVolumeRendering::init();

  SbString home = SoPreferences::getString("OIVHOME","");
  if ( !home.isEmpty() )
  {
    SoInput::addDirectoryFirst(home);
    SoInput::addDirectoryFirst(home+SbString("/src/VolumeViz/VolRend/"));
    SoInput::addDirectoryFirst(home+SbString("/src/VolumeViz/Data/"));
  }
  SoInput::addDirectoryFirst(SbString("Data/"));

  //Try to guess OIVHOME
  SbString dllPath = SoDynamicLibManager::getLibraryFromSymbol((void*)SoNode::initClasses);
  dllPath = SbFileHelper::toUnixPath(SbFileHelper::getDirName(dllPath));
  if ( !dllPath.isEmpty() )
  {
    SoInput::addDirectoryFirst(dllPath);
    SoInput::addDirectoryFirst(dllPath+SbString("/../../"));
    SoInput::addDirectoryFirst(dllPath+SbString("/../../src/VolumeViz/VolRend/"));
    SoInput::addDirectoryFirst(dllPath+SbString("/../../src/VolumeViz/Data/"));
    SoInput::addDirectoryFirst(".");
  }

  //Set a loading CB to know when the system loads in LDM mode
  SoLDMGlobalResourceParameters::setLoadCB( loadCB, NULL );

  // Global rendering options
  int value;
  value = SoPreferences::getBool( "VOLREND_viewCulling", 1 );
  SoLDMGlobalResourceParameters::setViewCulling( value );

  value = SoPreferences::getBool( "VOLREND_viewPointRefine", 1 );
  SoLDMGlobalResourceParameters::setViewpointRefinement( value );

  value = SoPreferences::getBool( "VOLREND_sliceEqualRes", 0 );
  SoLDMGlobalResourceParameters::setSliceEqualResolution( value );

  value = SoPreferences::getBool( "VOLREND_moveLowRes", 0 );
  SoLDMGlobalResourceParameters::setMoveLowResolution( value );

  value = SoPreferences::getBool( "VOLREND_tileOutline", 0 );
  SoLDMGlobalResourceParameters::setVisualFeedbackParam( SoLDMGlobalResourceParameters::DRAW_TILE_OUTLINE, value );

  value = SoPreferences::getBool( "VOLREND_dataOutline", 0 );
  SoLDMGlobalResourceParameters::setVisualFeedbackParam( SoLDMGlobalResourceParameters::DRAW_TOPOLOGY, value );

  SoLDMGlobalResourceParameters::setScreenResolutionCulling(TRUE);

#ifndef CAVELIB
  // Initialize DialogViz extension
  SoDialogViz::init();
#endif

  m_loadCBMutex = new SbThreadMutex(TRUE);

  m_loadingSphereSensor = new SoTimerSensor( redrawLoadingSign, NULL);
  m_loadingSphereSensor->setInterval(SbTime(0.5));
  m_loadingSphereSensor->schedule();

  m_draggerNormal.setValue(0,0,1);
  m_scaleFactor = 0.4f;

  // Scene graph root
  m_root = new SoSeparator();
  m_root->ref() ;
  m_root->setName( "SCENE_ROOT" ); // for debugging

  m_shadowGroup = new SoShadowGroup;
  m_shadowGroup->method = SoShadowGroup::VARIANCE_SHADOW_MAP;
  m_shadowGroup->isActive.setValue(FALSE);
  m_root->addChild(m_shadowGroup);

  //Special rotation for segy file. trace Lenght is mapped on X and must be down (-Y).
  m_segySwitch   = new SoSwitch;
  m_segySwitch->setName("SegyRotate");
  m_segySwitch->whichChild = SO_SWITCH_NONE;
  m_segyRotation = new SoRotation;
  m_segyRotation->rotation.setValue(SbVec3f(0,0,1), (float)(PI/2));
  m_segySwitch->addChild(m_segyRotation);
  m_shadowGroup->addChild(m_segySwitch);

#ifdef SPACEBALL
  //SpaceBall
  if ( SoXtSpaceball::exists()) {
    SoEventCallback *spaceballCB = new SoEventCallback;
    m_sb_struct = new sb_stuct;
    m_sb_struct->sb_struct_rot = new SoRotation;
    m_sb_struct->sb_struct_trans = new SoTranslation;
    m_sb_struct->sb_next_colorMap = FALSE;
    m_correct_trans = new SoTranslation;
    m_correct_trans->translation.setValue(SbVec3f(0.0,0.0,0.0));
    m_correct_trans_back = new SoTranslation;
    m_correct_trans_back->translation.setValue(SbVec3f(0.0,0.0,0.0));
    shadowGroup->addChild(m_correct_trans);
    shadowGroup->addChild(m_sb_struct->sb_struct_trans);
    shadowGroup->addChild(m_sb_struct->sb_struct_rot);
    shadowGroup->addChild(m_correct_trans_back);
    shadowGroup->addChild(spaceballCB);

    // Set up SpaceBall event callbacks
    spaceballCB->addEventCallback( SoSpaceballButtonEvent::getClassTypeId(),
      spaceballButtonCB,
      m_sb_struct);
    spaceballCB->addEventCallback( SoMotion3Event::getClassTypeId(),
      motion3TranslationCB,
      m_sb_struct);
    spaceballCB->addEventCallback( SoMotion3Event::getClassTypeId(),
      motion3RotationCB,
      m_sb_struct);
    m_sb_available  = TRUE;
  }
  else {
    m_sb_available = FALSE;
  }
#endif


  //check if user has an LDMLicense, if not change default file :
  m_defaultFile = 0;
  SbString neededVersion = SO_COMPACT_VERSION_DOTTED_STRING;
  if(SoDB::LicenseCheck("VolumeVizLDM",neededVersion.toFloat(), SoLockManager::GetUnlockString())<0)
  {
    m_defaultFile = 1;
  }

  // Reader
  // Find a valid data path
  SbString filepath;
  if ( !fileToLoad.isEmpty() )
    filepath = getDataFilePath( fileToLoad );
  if (filepath.isEmpty())
  {
    filepath = SoPreferences::getString( "VOLREND_filename","");
    if (!filepath.isEmpty())
      filepath = getDataFilePath( filepath );
  }
  if ( filepath.isEmpty())
    filepath = getDataFilePath(fileNames[m_defaultFile]);

  // Volume Data
  m_volData = new SoVolumeData();
  m_volData->setName("volumeData");

  m_proxiVisitor = new SoLDMProximityVisitor;
  m_tileVisitor  = new SoLDMTileVisitor;

 // m_volData->getLdmManagerAccess().setTileVisitor(m_proxiVisitor);

  //for demo only if we need to display a particular logo
  m_courtesyLogo = new SoImageBackground;
  m_courtesyLogo->style = SoImageBackground::LOWER_RIGHT;

  m_logoSwitch = new SoSwitch;
  m_logoSwitch->whichChild = SO_SWITCH_NONE;
  m_logoSwitch->addChild(m_courtesyLogo);
  m_imgSwitch = new SoSwitch;

  initDataNode(m_volData, filepath);

  m_volData->usePalettedTexture = TRUE;

  m_rotation = new SoRotation;
  m_rotation->rotation.setValue(SbVec3f(0,1,0), 0);
  m_shadowGroup->addChild( m_rotation);
  m_shadowGroup->addChild( m_volData );

  // data range
  m_dataRangeSwitch = new SoSwitch;
  m_dataRangeSwitch->whichChild = SO_SWITCH_NONE;
  m_shadowGroup->addChild( m_dataRangeSwitch );
  m_dataRange = new SoDataRange;
  m_dataRangeSwitch->addChild( m_dataRange );

  // Get range of voxel data values in volume
  // (indirectly this forces the reader to be called to load the volume)
  double min, max;
  m_volData->getMinMax(min,max);

  // Get the voxel dimensions of the volume
  SbVec3i32 dimensions;
  SoVolumeData::DataType dataType = (SoVolumeData::DataType)m_volData->data.getDataType();
  dimensions = m_volData->data.getSize();
  int volWidth, volHeight, volDepth;
  dimensions.getValue(volWidth, volHeight, volDepth);

  // Get the geometric size of the volume
  m_width  = volWidth;
  m_height = volHeight;
  m_depth  = volDepth;
  SbBox3f volSize = m_volData->extent.getValue();
  volSize.getBounds(m_minWidth, m_minHeight, m_minDepth, m_maxWidth, m_maxHeight, m_maxDepth);
  float xSize, ySize, zSize;
  volSize.getSize( xSize, ySize, zSize );
  m_volDefaultSize = volSize; // Before any volume scaling

  float avgSize = (xSize + ySize + zSize) / 3;
  m_scaleFactor = 0.4f * avgSize;

  m_minColorMap   = SoPreferences::getInt( "VOLREND_minColorMap" , 0 );
  m_maxColorMap   = SoPreferences::getInt( "VOLREND_maxColorMap" , 255 );
  m_minOpaqueMap  = SoPreferences::getInt( "VOLREND_minOpaqueMap", m_minColorMap );
  m_maxOpaqueMap  = SoPreferences::getInt( "VOLREND_maxOpaqueMap", m_maxColorMap );

  // Slice transfer function
  m_sliceTransferFunction = new SoTransferFunction();
  int sliceColorMap = SoPreferences::getInt( "VOLREND_sliceColorMap", (int)SoTransferFunction::TEMPERATURE );
  if (sliceColorMap == 0) {
    const char *filename = SoPreferences::getValue( "VOLREND_sliceColorMapFile" );
    if (!filename || *filename == '\0')
      filename = "blueWhiteRed.txt";
    //m_userColorMap = getColorTable( filename );
    int numComps = 0;
    /*int numEntries =*/ loadColorTable( filename, numComps, m_userColorMap );
    if (m_userColorMap) {
      m_sliceTransferFunction->predefColorMap = SoTransferFunction::NONE;
      m_sliceTransferFunction->colorMapType = SoTransferFunction::RGBA;
      m_sliceTransferFunction->colorMap.setValues( 0, 256 * 4, m_userColorMap );
      userColorMapName = filename;
	  sliceUserColorMapName = userColorMapName;
    }
  }
  else
    m_sliceTransferFunction->predefColorMap = (SoTransferFunction::PredefColorMap)sliceColorMap;

  updateOpaqueRegion( m_sliceTransferFunction, m_minColorMap, m_maxColorMap, m_minOpaqueMap, m_maxOpaqueMap, m_invertTransparency );
  m_sliceTransferFunction->setName("SliceTransferFunction");
  m_shadowGroup->addChild( m_sliceTransferFunction );

#ifdef XDEBUG
  // ***** Hack for modeling transform test *****
  //
  // Add a stationary sphere at the origin so we can see
  // that the volume is offset by the modeling transform
  SoSeparator *pSphSep = new SoSeparator;
  SoSphere    *pSphere = new SoSphere;
  pSphere->radius = 0.5;
  pSphSep->addChild( new SoMaterial );
  pSphSep->addChild( pSphere );

  SoTransform *pHackXform  = new SoTransform;
  SoTransform *pHackXform2 = new SoTransform;
  SoTransform *pHackXform3 = new SoTransform;
  pHackXform->setName( "HackXform" );

  // Select some transforms to apply...
  pHackXform->translation = SbVec3f( -1,-1.5,-2 );
  //pHackXform2->rotation = SbRotation(SbVec3f(3,2,1),1.570796326794896f);
  pHackXform2->rotation = SbRotation(SbVec3f(0,1,0),0.78539816339744f);
  //pHackXform->rotation = SbRotation(SbVec3f(0,0,1),0.78539816339744f);
  //pHackXform->rotation = SbRotation(SbVec3f(1,1.5f,2),0.78539816339744f);
  //pHackXform2->scaleFactor = SbVec3f( 2,2,2 );
  pHackXform3->translation = SbVec3f( 1,1.5,2 );

  SoGroup *pXformGroup = new SoGroup;   // Hack for modeling transform test
  pXformGroup->setName( "XformGroup" );
  pXformGroup->addChild( pSphSep );
  pXformGroup->addChild( pHackXform );
  pXformGroup->addChild( pHackXform2 );
  pXformGroup->addChild( pHackXform3 );
  m_shadowGroup->addChild( pXformGroup );
#endif // _DEBUG

  // Ortho Slices
  // build dragger for volume rendering slice.
  //SoGroup *sliceGroup = new SoGroup;
  sliceGroup = new SoGroup;
  sliceGroup->setName("SlicesGroup");
  m_shadowGroup->addChild(sliceGroup);

  m_draggerVolRender = new SoJackDragger();
  m_draggerVolRender->setName("JackDraggerForOrthoSlice");
  m_draggerVolRender->setPart("rotator.rotator",NULL);
  m_draggerVolRender->setPart("rotator.rotatorActive",NULL);
  m_draggerVolRender->setPart("translator.xzTranslator.translator",NULL);
  m_draggerVolRender->setPart("translator.yzTranslator.translator",NULL);
  m_draggerVolRender->setPart("translator.xyTranslator.translator",NULL);
  m_draggerVolRender->setPart("translator.yzTranslator.translatorActive",NULL);
  m_draggerVolRender->setPart("translator.xzTranslator.translatorActive",NULL);
  m_draggerVolRender->setPart("translator.xyTranslator.translatorActive",NULL);

  m_draggerVolRender->addMotionCallback((SoDraggerCB *)&motionSliceCallback,NULL);
  m_draggerVolRender->rotation = SbRotation(SbVec3f(0,1,0),m_draggerNormal);
  m_draggerVolRender->scaleFactor.setValue(m_scaleFactor,m_scaleFactor,m_scaleFactor);
  //jh-february 2003 for benchmarking
  m_draggerVolRender->addStartCallback((SoDraggerCB *)VVizBenchmark::sliceStartCallback, 0);
  m_draggerVolRender->addFinishCallback((SoDraggerCB *)VVizBenchmark::sliceFinishCallback, &m_bench);
  m_draggerVolRender->addMotionCallback((SoDraggerCB *)VVizBenchmark::sliceMotionCallback, &m_bench);

  m_draggerVolRenderSwitch = new SoSwitch();
  m_draggerVolRenderSwitch->setName("SwitchForOrthoDragger");
  m_draggerVolRenderSwitch->whichChild = SO_SWITCH_NONE;
  m_draggerVolRenderSwitch->addChild(m_draggerVolRender);

  m_uniformGridClippingSwitch = new SoSwitch;
  m_uniformGridClippingSwitch->setName("SwitchForUniformGridClipping");

  m_volRenderOrthSliceSwitch = new SoSwitch ;
  m_volRenderOrthSliceSwitch->setName("SwitchForOrtho");
  m_volRenderOrthSliceSwitch->whichChild = SO_SWITCH_NONE;
  m_volRenderOrthSliceSwitch->addChild(m_draggerVolRenderSwitch);

  //jh
  //createOrthoSlices(1, SO_SWITCH_ALL);


  // Oblique Slice
  SbVec3f obliPos = volSize.getMin()+(volSize.getMax()-volSize.getMin())/2.0f;
  m_volRenderObliSliceSwitch = new SoSwitch ;
  m_volRenderObliSliceSwitch->setName("SwitchForOblique");
  m_volRenderObliSliceSwitch->addChild(m_uniformGridClippingSwitch);
  m_volRenderObliSliceSwitch->whichChild = SO_SWITCH_NONE;

  m_volRenderObliSliceSwitch->addChild(m_obliSliceQuality = new SoVolumeRenderingQuality);
  m_obliSliceQuality->interpolateOnMove = TRUE;
  m_obliSliceQuality->lightingModel = SoVolumeRenderingQuality::OPENGL;
  m_obliSliceQuality->edgeDetect2DMethod = SoVolumeRenderingQuality::LUMINANCE|SoVolumeRenderingQuality::DEPTH;
  m_obliSliceQuality->surfaceScalarExponent = 8;

  m_obliSlice = new SoObliqueSlice ;
  m_obliSlice->interpolation.setValue(SoObliqueSlice::LINEAR);
  m_obliSlice->plane = SbPlane(m_draggerNormal, obliPos);
  m_volRenderObliSliceSwitch->addChild(m_obliSlice) ;

  m_draggerObliSwitch = new SoSwitch();
  m_draggerObliSwitch->setName("SwitchForObliqueDragger");
  m_draggerObliSwitch->whichChild = SO_SWITCH_ALL;
  m_volRenderObliSliceSwitch->addChild(m_draggerObliSwitch);
  m_draggerObliSlice = new SoJackDragger();
  m_draggerObliSlice->setName("JackForOblique");

  m_draggerObliSlice->addMotionCallback((SoDraggerCB *)&motionObliSliceCallback,NULL);
  m_draggerObliSwitch->addChild(m_draggerObliSlice);
  m_draggerObliSlice->rotation = SbRotation(SbVec3f(0,1,0),m_draggerNormal);
  m_draggerObliSlice->scaleFactor.setValue(m_scaleFactor,m_scaleFactor,m_scaleFactor);
  m_draggerObliSlice->translation.setValue(obliPos);
  //jh-february 2003 for benchmarking
  m_draggerObliSlice->addStartCallback((SoDraggerCB *)VVizBenchmark::sliceStartCallback, 0);
  m_draggerObliSlice->addFinishCallback((SoDraggerCB *)VVizBenchmark::sliceFinishCallback, &m_bench);
  m_draggerObliSlice->addMotionCallback((SoDraggerCB *)VVizBenchmark::obliSliceMotionCallback, &m_bench);


  m_obliClipPlane = new SoClipPlane;
  m_obliClipPlane->setName("ObliqueClipPlane");
  m_obliClipPlane->on = FALSE;
  m_obliClipPlane->plane.connectFrom((SoField*)&(m_obliSlice->plane));
  m_volRenderObliSliceSwitch->addChild(m_obliClipPlane);

  SoMaterial* sliceMaterial = new SoMaterial;
  sliceMaterial->ambientColor.setValue(0,0,0);
  sliceMaterial->diffuseColor.setValue(1.0f,1.0f,1.0f);
  sliceMaterial->emissiveColor.setValue(0.f,0.f,0.f);
  sliceMaterial->transparency.setValue(0.f);
  sliceMaterial->specularColor.setValue(0.1f,0.1f,0.1f);
  sliceMaterial->shininess.setValue(1.0f);

#ifndef CAVELIB
  m_mtlSliceEditor = new SoXtMaterialEditor();
  m_mtlSliceEditor->setTitle("Material of ortho and oblique slice");
  m_mtlSliceEditor->attach(sliceMaterial);
#endif

  // Volume geometry
  SoSeparator* createVolumeGeometryGraph();
  m_volRenderVolGeomSwitch = new SoSwitch;
  m_volRenderVolGeomSwitch->setName( "SwitchForVolumeGeometry" );
  m_volRenderVolGeomSwitch->whichChild = SO_SWITCH_NONE;

  m_volRenderVolGeomSwitch->addChild( createVolumeGeometryGraph() );


  sliceGroup->addChild(sliceMaterial);
  sliceGroup->addChild(m_volRenderObliSliceSwitch);
  sliceGroup->addChild(m_volRenderOrthSliceSwitch);
  sliceGroup->addChild(m_volRenderVolGeomSwitch);

  // Lighting / Material
  // The ambientColor only seems to have effect when lighting is
  // enabled, but we need a high value to get "reasonable" appearance.
  const float defaultAlpha = 0.3f;
  float alpha = SoPreferences::getFloat( "VOLREND_globalAlpha", defaultAlpha );
  if (alpha < 0 || alpha > 1)
    alpha = defaultAlpha;
  m_material = new SoMaterial();
  m_material->ambientColor.setValue(0.3f,0.3f,0.3f);
  m_material->diffuseColor.setValue(0.9f,0.9f,0.9f);
  m_material->transparency.setValue( 1.0f - alpha );
  m_material->specularColor.setValue(0.1f,0.1f,0.1f);
  m_material->shininess.setValue(0.1f);
  m_shadowGroup->addChild(m_material);

#ifndef CAVELIB
  m_mtlEditor = new SoXtMaterialEditor();
  m_mtlEditor->setTitle("Material of Volume Rendering");
  m_mtlEditor->attach(m_material);

  // When the material editor comes up it does not display the
  // actual values in this material.  Probably a bug, but this
  // should force it to be correct.
  m_mtlEditor->setMaterial( *m_material );
#endif //CAVELIB

  // Volume render
  m_VolRenderSwitch = new SoSwitch;
  m_VolRenderSwitch->setName("SwitchForVolumeRenderNodes");
  SoGroup *pVolRendGrp = new SoGroup;
  pVolRendGrp->setName( "VolRendGrp" );
  m_volRend = new SoVolumeRender();
  m_volRend->interpolation.setValue(SoVolumeRender::LINEAR);
  m_volRend->renderMode.setValue(SoVolumeRender::VOLUME_RENDERING);
  m_volRend->numSlicesControl.setValue(SoVolumeRender::AUTOMATIC);

  m_geometryProprity = new SoGeometryPriority();
  m_geometryProprity->priority.setValue(0.9f);
  m_volumeGeometrySwitch = new SoSwitch;
  m_volumeGeometrySwitch->whichChild = SO_SWITCH_NONE;

  // Add the geometry to the switch
  m_volumeGeometrySwitch->addChild(m_geometryProprity);
  pVolRendGrp->addChild(m_volumeGeometrySwitch);

  //Switch for volume clipping
  m_predefinedGrid.resize(3);
  m_userGrid.resize(3);
  m_selectedVolume = 0;
  m_volumeClippingVisibleSwitch = new SoSwitch; //Make the clipping object in/visible
  m_volumeClippingVisibleSwitch->setName("volumeClippingVisibleSwitch");
  m_volumeClippingVisibleSwitch->whichChild = SO_SWITCH_NONE;
  m_volumeClippingSwitch = new SoSwitch;
  m_volumeClippingSwitch->setName("volumeClippingSwitch");
  m_volumeClippingTransform = new SoTransform;
  m_volumeClippingTransform->ref(); //Avoid to delete it when changing models...

  SoVolumeClippingGroup *volClippingGroup = new SoVolumeClippingGroup;
  volClippingGroup->addChild(new SoSphere);
  addClipVolumeToSceneGraph(volClippingGroup);
  pVolRendGrp->addChild(m_volumeClippingSwitch);

  //Don't apply manip transformation on the volume
  pVolRendGrp->addChild(new SoResetTransform);
  pVolRendGrp->addChild(m_segySwitch);

  //Scale the clipping volume according to the size of the volume data
  m_volumeClippingTransform->scaleFactor = SbVec3f(avgSize/4, avgSize/4, avgSize/4);
  m_volumeClippingTransform->translation = SbVec3f(m_minWidth+xSize/2, m_minHeight+ySize/2, m_minDepth+zSize/2);

  //Switch for uniform grid clipping
  m_selectedGrid = 0;
  SoGroup *gridClippingGroup = new SoGroup;
  m_gridsGroup = new SoGroup;
  SoTextureUnit *texUnit2 = new SoTextureUnit;
  texUnit2->unit = 2;

  SoUniformGridClipping *grid = createDefaultGrid();

  pVolRendGrp->addChild(m_uniformGridClippingSwitch);
  m_uniformGridClippingSwitch->addChild(gridClippingGroup);
  m_uniformGridClippingSwitch->setName("uniformGridClippingSwitch");
  m_gridsGroup->setName("uniformGridClippingGroup");
  gridClippingGroup->addChild(m_gridsGroup);
  addGridToSceneGraph(grid, 2);

  gridClippingGroup->addChild(new SoResetTransform);

  //Scale the grid according to the size of the volume data
  grid->extent = SbBox3f(m_minWidth, m_minHeight, m_minDepth, m_maxWidth, m_minHeight+ySize/2, m_maxDepth);

  // Node specific rendering options
  value = SoPreferences::getInt( "VOLREND_samplingAlignement", 1 );
  m_volRend->samplingAlignment = value;

  value = SoPreferences::getInt( "VOLREND_numSlicesControl", 2 ); // AUTOMATIC
  m_volRend->numSlicesControl.setValue( (SoVolumeRender::NumSlicesControl)value );

  value = SoPreferences::getInt( "VOLREND_numSlices", 200 );
  if (m_volRend->numSlicesControl.getValue() != SoVolumeRender::AUTOMATIC)
    m_volRend->numSlices = value;

  // Axes, Bounding box and Region-of-Interest
  m_volBBoxSwitch = new SoSwitch;
  m_volAxesSwitch = new SoSwitch;
  m_volROISwitch  = new SoSwitch;
  m_volBBoxSwitch->setName( "BBoxSwitch" );   //debug
  m_volAxesSwitch->setName( "AxesSwitch" );   //debug
  m_volROISwitch ->setName( "ROISwitch"  );   //debug

  int showBBox = SoPreferences::getBool( "VOLREND_showBBox", FALSE );
  int showAxes = SoPreferences::getBool( "VOLREND_showAxes", FALSE );
  if (showBBox)
    m_volBBoxSwitch->whichChild = SO_SWITCH_ALL;
  if (showAxes)
    m_volAxesSwitch->whichChild = SO_SWITCH_ALL;

  SoAntiSquish *pAnti = new SoAntiSquish;
  pAnti->recalcAlways = TRUE;

  m_volROIScale = new SoScale;
  m_volROIScale->setName("ScaleForROI");
  SbBox3f volBox = m_volData->extent.getValue();
  float sizeX, sizeY, sizeZ;
  volBox.getSize( sizeX, sizeY, sizeZ );
  float maxSize = sizeX;
  if (maxSize < sizeY) maxSize = sizeY;
  if (maxSize < sizeZ) maxSize = sizeZ;
  m_volROIScale->scaleFactor.setValue( sizeX/maxSize, sizeY/maxSize, sizeZ/maxSize );

  SoSeparator *pTabGroup = new SoSeparator;
  pTabGroup->setName("GroupForROIAttributes");
  pTabGroup->addChild( m_volROIScale );
  m_volROIPickStyle = new SoPickStyle;
  pTabGroup->addChild( m_volROIPickStyle ); // does not prevent picking the volume.     mmh
  //pTabGroup->addChild( pAnti );

  m_volROIDraggerSwitch = new SoSwitch;
  m_volROIDraggerSwitch->setName("SwitchForROI");
  m_volROIDraggerSwitch->whichChild = SO_SWITCH_NONE;
  m_volROIDraggerSwitch->addChild( m_volROIPickStyle );

  m_volROI = new SoROI;
  m_volROI->setName("ROI");
  m_volROI->box.setValue( 0,0,0, volWidth-1, volHeight-1, volDepth-1 );

  m_volROISwitch->addChild( m_volROIDraggerSwitch );

  m_ROIManip = new SoROIManip();
  m_ROIManip->box.setValue( SbVec3i32(0,0,0), dimensions - SbVec3i32(1,1,1) );
  m_ROIManip->subVolume.setValue( SbVec3i32(0,0,0), dimensions - SbVec3i32(1,1,1) );
  m_ROIManip->constrained = TRUE;

  m_ROIManip->setName("ROIManip");
  m_volROISwitch->addChild( m_ROIManip );
  m_ROIManip->boxOn = true;

  SoMaterial *material = (SoMaterial*)(m_ROIManip->getDragger()->getPart("tabPlane1.scaleTabMaterial",0));
  material->diffuseColor.setValue(0,1,0);
  material->emissiveColor.setValue(0,1,0);
  // NOTE: Use the insertChild instead of addChild to have ROI affect
  // the slices (in addition to volume), else they are not affected.
  pVolRendGrp->addChild( m_volROISwitch  );
  sliceGroup->insertChild( m_volROISwitch , 0 );

  // Picking
  m_pick = new SoPickStyle;
  m_pick->style = SoPickStyle::UNPICKABLE;
  pVolRendGrp->addChild( m_pick );

  // Moved the bbox and axes up under shadowGroup, instead of VolRendGrp,
  // so they can still be visible even when the volume is turned off.
  // Note: BBox and axes must not be pickable or they will interfere
  //       with picking the slices and volume
  SoPickStyle *pUnpickable = new SoPickStyle;
  pUnpickable->style = SoPickStyle::UNPICKABLE;
  m_shadowGroup->addChild( pUnpickable );
  m_shadowGroup->addChild( m_volAxesSwitch );
  m_shadowGroup->addChild( m_volBBoxSwitch );
  m_shadowGroup->addChild( new SoPickStyle );

  // To get profile and value on volume !
  // NOTE: These nodes must be under shadowGroup, not VolRendGrp, so we can
  //       do picking on slices even when the volume is not visible.
  // This part is used for the picking in VolumeRendering.
  // Add an event callback to catch mouse moves.
  SoEventCallback * b = new SoEventCallback;
  m_shadowGroup->insertChild( b, 1 );

  m_mouseMoveEvent = new SoEventCallback();
  m_shadowGroup->addChild(m_mouseMoveEvent);
  // Add an event callback to catch mouse key event.
  m_mouseKeyEvent = new SoEventCallback();
  m_shadowGroup->addChild(m_mouseKeyEvent);

  // Also use this node to see key press events for frame rate
  // (we need the top level window handle to display frame rate)
  m_mouseKeyEvent->addEventCallback(
    SoKeyboardEvent::getClassTypeId(), keyEventCB, (void*)myWindow );

  // Setup to allow rendering volume in front buffer (mmh Dec-2000)
  //SoFrontBufferGroup *pFBGroup = new SoFrontBufferGroup;
  SoGroup *pFBGroup = new SoGroup;
  pFBGroup->setName( "FBufGroup" );

  //SoInteractiveComplexity node for sampling
  SoInteractiveComplexity* samplingComplexity = new SoInteractiveComplexity;
  samplingComplexity->refinementDelay.setValue(0.0);
  samplingComplexity->fieldSettings.set1Value(0, "SoComplexity value 0.2 1.0");

  SoSwitch* m_samplingComplexitySw = new SoSwitch;
  m_samplingComplexitySw->addChild(samplingComplexity);
  
  pFBGroup->addChild(m_samplingComplexitySw);
  pFBGroup->addChild(new SoComplexity);

  // This switch allows to choose volumeSkin or volumeRender for display
  m_volSkinSwitch = new SoSwitch;
  m_volSkinSwitch->whichChild = 0;

  //VolumeRenderingQuality
  SoGroup *qualityGroup = new SoGroup;
  qualityGroup->addChild(m_volQuality = new SoVolumeRenderingQuality);
  m_volQuality->interpolateOnMove = TRUE;
  m_volQuality->lightingModel = SoVolumeRenderingQuality::OPENGL;
  m_volQuality->edgeDetect2DMethod = SoVolumeRenderingQuality::LUMINANCE|SoVolumeRenderingQuality::DEPTH;
  m_volQuality->surfaceScalarExponent = 8;
  qualityGroup->addChild(m_volRend);
  m_volSkinSwitch->addChild(qualityGroup);

  //Isosurface
  SoGroup *isoGroup = new SoGroup;
  isoGroup->addChild(m_volQuality);
  isoGroup->addChild(m_volIsosurface = new SoVolumeIsosurface);
  m_volIsosurface->isovalues.set1Value( 0, (float)((max-min)/2) );
  isoGroup->addChild(m_volRend);
  m_volSkinSwitch->addChild(isoGroup);

  value = SoPreferences::getBool( "VOLREND_raycasting", TRUE );
  m_volQuality->raycasting = value;
  m_volIsosurface->raycasting.connectFrom(&m_volQuality->raycasting);

  //Skin
  SoGroup *skinGroup = new SoGroup;
  m_volSkinQuality = new SoVolumeRenderingQuality;
  m_volSkinQuality->interpolateOnMove = TRUE;
  m_volSkinQuality->lightingModel = SoVolumeRenderingQuality::OPENGL;
  m_volSkinQuality->edgeDetect2DMethod = SoVolumeRenderingQuality::LUMINANCE|SoVolumeRenderingQuality::DEPTH;
  m_volSkinQuality->surfaceScalarExponent = 8;
  skinGroup->addChild(m_volSkinQuality);
  skinGroup->addChild(m_volSkin = new SoVolumeSkin);
  m_volSkinSwitch->addChild( skinGroup );

  pFBGroup->addChild( m_volSkinSwitch );
  pVolRendGrp->addChild( pFBGroup );

  m_VolRenderSwitch->addChild( pVolRendGrp ) ;
  m_VolRenderSwitch->whichChild = SO_SWITCH_ALL;

  // Transfer function for volume

  m_transferFunction = new SoTransferFunction();
  int colorMapIndex = SoPreferences::getInt( "VOLREND_colorMap", (int)SoTransferFunction::SEISMIC );
  // Check for user defined colormap
  if (colorMapIndex == 0) {
    const char *filename = SoPreferences::getValue( "VOLREND_colorMapFile" );
    if (!filename || *filename == '\0')
      filename = "blueWhiteRed.txt";
    //m_userColorMap = getColorTable( filename );
    int numComps = 0;
    /*int numEntries =*/ loadColorTable( filename, numComps, m_userColorMap );
    if (m_userColorMap) {
      m_transferFunction->predefColorMap = SoTransferFunction::NONE;
      m_transferFunction->colorMapType = SoTransferFunction::RGBA;
      m_transferFunction->colorMap.setValues( 0, 256 * 4, m_userColorMap );
      userColorMapName = filename;
	  volumeUserColorMapName = filename;
    }
    else
      colorMapIndex = (int)SoTransferFunction::SEISMIC;
  }
  // Use predef colormap if no user colormap
  if (colorMapIndex > 0)
    m_transferFunction->predefColorMap = (SoTransferFunction::PredefColorMap)colorMapIndex;
  updateOpaqueRegion( m_transferFunction, m_minColorMap, m_maxColorMap, m_minOpaqueMap, m_maxOpaqueMap, m_invertTransparency );
  m_transferFunction->setName("TransferFunction");
  m_shadowGroup->addChild( m_transferFunction );

  m_shadowGroup->addChild(m_VolRenderSwitch);

  // Info
  SoSeparator          *p2DSep = new SoAnnotation;
  SoOrthographicCamera *p2Dcam = new SoOrthographicCamera;
  p2Dcam->nearDistance = 0.5f;
  p2Dcam->farDistance  = 1.5f;

    SoLightModel *pLightModel = new SoLightModel;
  pLightModel->model = SoLightModel::BASE_COLOR;
  SoBaseColor  *pBaseColor = new SoBaseColor;
  pBaseColor->rgb.set1Value( 0, SbColor(1,1,1) );

  SoFont  *pFont = new SoFont;
  SoText2 *pText = new SoText2;
  SoTranslation *pTran = new SoTranslation;
  pTran->translation = SbVec3f( -.99f, -.95f, 0.49f );
  pFont->size = (float)SoPreferences::getInt("VOLREND_fontSize", 12);
  int numBytes = m_volData->getDataSize();
  float totbytes = (float)(volWidth/1024.f) * (float)volHeight * (float)volDepth * (float)numBytes;
  int64_t totalBytes = int64_t(totbytes);
  const char* dataTypeStr = NULL;
  if (SoVolumeData::isDataFloat(dataType))
    dataTypeStr = "float";
  else if (SoVolumeData::isDataSigned(dataType))
    dataTypeStr = "signed integer";
  else
    dataTypeStr = "unsigned integer";
  sprintf( buffer,
    "Volume: %d x %d x %d x %d bits %s  =   %10.1f MB",
    volWidth, volHeight, volDepth, numBytes*8, dataTypeStr, (float)totalBytes/1024. );
  pText->string.setValue(buffer);
  pText->setName(VOL_VIEWER_INFO);

  p2DSep->setName ("_2DSep");
  p2DSep->addChild(pUnpickable);	// Shouldn't interfere with picking volume
  p2DSep->addChild(p2Dcam);
  p2DSep->addChild(pLightModel);
  p2DSep->addChild(pBaseColor);

  //colormap
  int showColormap = SoPreferences::getBool( "VOLREND_showColormap", TRUE );
  m_volColormapSwitch = new SoSwitch;
  m_volColormapSwitch->whichChild = showColormap ? SO_SWITCH_ALL : SO_SWITCH_NONE;

  SoSeparator *pLegendSep = createColorMapLegend( m_transferFunction );
  if (pLegendSep)
    m_volColormapSwitch->addChild( pLegendSep );
  p2DSep->addChild( m_volColormapSwitch );

  //loadingCB
  m_loadingCBSwitch = new SoSwitch;

  SoSeparator *pLoadCB = createLoadingSign();
  if (pLoadCB)
    m_loadingCBSwitch->addChild( pLoadCB );
  m_loadingCBSwitch->whichChild = SO_SWITCH_ALL;

  p2DSep->addChild( m_loadingCBSwitch );

  //histogram
  m_showHistogram = SoPreferences::getBool( "VOLREND_showHistogram", TRUE );
  m_volHistoSwitch = new SoSwitch;
  m_volHistoSwitch->whichChild = m_showHistogram ? SO_SWITCH_ALL : SO_SWITCH_NONE;
  createHistoLegend( m_volHistoSwitch );
  p2DSep->addChild( m_volHistoSwitch );

  // Annotation text
  int showText = SoPreferences::getBool( "VOLREND_showText", TRUE );
  m_volTextSwitch = new SoSwitch;
  m_volTextSwitch->whichChild = showText ? SO_SWITCH_ALL : SO_SWITCH_NONE;
  m_volTextSwitch->addChild(pTran);
  m_volTextSwitch->addChild(pFont);
  m_volTextSwitch->addChild(pText);
  p2DSep->addChild( m_volTextSwitch );

#ifndef CAVELIB
  // Use perspective camera by default
  {
    int usePerspCam = 1;
    const char *camstr = SoPreferences::getValue( "VOLREND_cameraType" );
    if (camstr) {
      if (*camstr == '0' || *camstr == 'o' || *camstr == 'O')
        usePerspCam = 0;
    }
    if (usePerspCam)
      m_root->insertChild(new SoPerspectiveCamera, 0 );
    else
      m_root->insertChild(new SoOrthographicCamera, 0);
  }
#endif // CAVELIB

  SoSeparator * mainRoot = new SoSeparator;

  //SoInteractiveComplexity node
  m_cplxSwitch = new SoSwitch;
  m_cplx = new SoInteractiveComplexity;
  m_cplxSwitch->addChild(m_cplx);
  m_cplx->refinementDelay.setValue(0.0);
  m_cplx->fieldSettings.set1Value(0, "");
  m_cplx->fieldSettings.set1Value(1, "SoVolumeRender lowScreenResolutionScale 2 1 -1");
  m_cplx->fieldSettings.set1Value(2, "SoShadowGroup quality 0.5 1 100");

  //
  //
  m_cplxSwitch->whichChild = SO_SWITCH_ALL;
  m_samplingComplexitySw->whichChild.connectFrom(&m_cplxSwitch->whichChild);

  mainRoot->addChild(m_cplxSwitch);

  mainRoot->addChild(new SoComplexity);

  // Create background nodes
  SoGradientBackground* pGradient = new SoGradientBackground;
  SoImageBackground* pImgBackg = new SoImageBackground;
  pImgBackg->filename = "logoVSGcorner.png";
  pImgBackg->style = SoImageBackground::LOWER_RIGHT;
  m_backGroundSwitch = new SoSwitch;
  m_backGroundSwitch->whichChild = SO_SWITCH_ALL;
  m_backGroundSwitch->addChild(pGradient);
  m_imgSwitch->addChild(pImgBackg);
  m_backGroundSwitch->addChild(m_imgSwitch);
  m_backGroundSwitch->addChild(m_logoSwitch);

  mainRoot->addChild(m_backGroundSwitch);
  mainRoot->addChild(m_root);
  mainRoot->addChild(p2DSep);

  mainRoot->renderCaching = SoSeparator::OFF;
  m_root->renderCaching = SoSeparator::OFF;

#ifndef CAVELIB
  m_headlightEd = new SoXtDirectionalLightEditor();
  m_headlightEd->setTitle( "Light Direction Editor" );

  // Set up viewer
  m_sliceOri = Z;
  Widget parent = buildInterface(myWindow);
#if !defined(WIN32) && !defined(__APPLE__)
  if (SoPreferences::getBool("VOLREND_remoteDialog", 0)) {
    m_viewer = new SoXtExaminerViewer(toplevel);
  }
  else
#endif
    m_viewer = new SoXtExaminerViewer(parent);
#if !defined(__APPLE__) && !defined(SOQT)
  m_viewer->setWindowCloseCallback(exitCB, NULL);
#endif
#ifdef SLB
  m_viewer->setAnimationEnabled( false );
#endif /* SLB */
  m_viewer->setTitle("OIV Volume Rendering");

  m_viewer->setTransparencyType(SoGLRenderAction::DELAYED_BLEND);
//  m_viewer->setTransparencyType(SoGLRenderAction::BLEND);
  m_viewer->getGLRenderAction()->setDelayedObjDepthWrite(TRUE);
#if defined(_WIN32) && !defined(SOQT)
  m_viewer->setEventCallback( EventCB, (void*)m_viewer );
#endif

#ifndef __APPLE__
  m_viewer->setFloatingColorBuffer(FALSE, SoXtRenderArea::FLOAT_32_COLOR_BUFFER);
#endif

  updateSelectSliceMenu(1);
  updateSelectIsoMenu(1);
  updateSelectMaterialMenu(1);

  //Workaround for bug under Linux : when combo box is disabled in the iv, cbox->enable = TRUE
  //doesn't enable it properly.
  SoDialogComboBox* cbox = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("selectediso"));
  cbox->enable = FALSE;
  cbox = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("numberOfIso"));
  cbox->enable = FALSE;
  cbox = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("numberOfMaterial"));
  cbox->enable = FALSE;
  cbox = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("selectedmaterial"));
  cbox->enable = FALSE;
  cbox = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("interpolationmethod"));
  cbox->enable = FALSE;
  cbox = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("skinfacemode"));
  cbox->enable = FALSE;
  SoDialogRealSlider* rslider = (SoDialogRealSlider*)myTopLevelDialog->searchForAuditorId(SbString("isovalue"));
  rslider->enable = FALSE;
  rslider = (SoDialogRealSlider*)myTopLevelDialog->searchForAuditorId(SbString("segmentedthreshold"));
  rslider->enable = FALSE;

  // set all optim by default
  if ( m_volRend )
  {
    switchOptimInterfaceToExpert(false);
    m_volRend->gpuVertexGen = TRUE;
    m_volRend->subdivideTile = TRUE;
    m_volRend->useEarlyZ = TRUE;
    m_volRend->numEarlyZPasses = 30;
  }


  //Callback for transparency slider and SoMaterial
  m_mtlEditor->addMaterialChangedCallback(updateGlobalAlphaCB);

  //Set isoval slider
  SoDialogRealSlider* isoVal = (SoDialogRealSlider*)myTopLevelDialog->searchForAuditorId(SbString("isovalue"));
  isoVal->max = (float)max;
  isoVal->min = (float)min;
  isoVal->value = (float)((int)m_volIsosurface->isovalues[0]);

  // Callbacks for user interaction
  m_viewer->addStartCallback( viewerStartCB, NULL );
  m_viewer->addFinishCallback( viewerFinishCB, NULL );

#ifdef SPACEBALL
  //SpaceBall
  if (! m_sb_available) {
#if defined(_DEBUG)
    fprintf(stderr, "Sorry, no Space Ball or Magellan Space Mouse on this display!\n");
#endif
  }
  else {
    m_spaceball = new SoXtSpaceball;
    m_viewer->registerDevice(m_spaceball);
    m_spaceball->setRotationScaleFactor(SB_STEP);
    m_spaceball->setTranslationScaleFactor(SB_STEP);
    m_rotationMode    = 1;
    m_translationMode = 0;
    m_colorMapMode = 0;
  }
#endif

  m_viewer->setSceneGraph(mainRoot);
  m_viewer->setIconTitle("3dtgs.ico")   ;

  // force viewer size only if arg requested it.
  if ( launchSizeW> 0 && launchSizeH>0)
    m_viewer->setSize(SbVec2s(launchSizeW, launchSizeH));
#ifdef XDEBUG
  {
    SoWriteAction wa;
    wa.getOutput()->openFile( "debug.iv" );
    wa.apply( mainRoot );
    wa.getOutput()->closeFile();
  }
#endif

  //jh-february 4 2003-add event callback for benchmarking:
  // Set up the benchmarking
  m_bench.setViewer( m_viewer );
  m_bench.setOrthoSlice(m_ZOrthoSlice);
  m_bench.setObliqueSlice(m_obliSlice);


  b->addEventCallback( SoKeyboardEvent::getClassTypeId(), VVizBenchmark::keyPressCB, &m_bench );

  m_viewer->viewAll();
  //m_viewer->setFramesPerSecondCallback(fpsCB) ;
//m_viewer->setSize(SbVec2s(512,512));
  m_viewer->saveHomePosition(); // not automatic when we have our own camera
  m_viewer->setTitle("OIV Volume Rendering");
  m_viewer->show();

  m_showFrameRate = FALSE;


#endif // CAVELIB

#if 0
  SoAlarmSensor* playSensor = new SoAlarmSensor(oneShotSensorCB, &m_bench);
  //This piece of code allow to launch volRend with benchmark and data playing from a console
  if(argv[1]&&argv[2]){
    m_bench.readTrajectory( argv[2] );
    //name of resulting benchmarking is trajectoryName.out:
    char* temp = argv[2];
    char* temp2 = m_bench.outputResult;
    int i=0;
    while(temp[i]!='\0')//.dat
    {
      i++;
    }
    strcpy(temp2,temp);
    temp2[i-4]='.';
    temp2[i-3]='o';
    temp2[i-2]='u';

    if(argv[3])
    {
      m_bench.outputResult = argv[3];
    }
    if(argv[4])
    {
      m_bench.numLoops = atoi(argv[4]);
    }
    SbTime time((double)1);
    playSensor->setTimeFromNow(time);
    playSensor->schedule();
  }
#endif
  //m_viewer->setTransparencyType(SoGLRenderAction::SORTED_OBJECT_BLEND);
  //SoLDMGlobalResourceParameters::setWriteAlternateRep(FALSE);
  //writeFile(m_root,SbString("volrend.iv"));
  //m_volROISwitch->whichChild = SO_SWITCH_ALL;
  //jh
  m_switchForOrthoDragger = new SoSwitch;
  m_switchForOrthoDragger->whichChild = SO_SWITCH_ALL;
  m_volRenderOrthSliceSwitch->addChild(m_switchForOrthoDragger);

  m_volRenderOrthSliceSwitch->addChild(m_uniformGridClippingSwitch);
  size_t numSlices = SoPreferences::getInt( "VOLREND_numOrthoSlice", 1 );
  createOrthoSlices( numSlices, SO_SWITCH_ALL );
  if (numSlices > 1) {
    m_orthoTab[1]->axis = SoOrthoSlice::Y;
  }
  if (numSlices > 2) {
    m_orthoTab[2]->axis = SoOrthoSlice::X;
  }

  SoDialogIntegerSlider* colorMapMax = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("colormap max"));
  m_transferFunction->maxValue.connectFrom(&colorMapMax->value);
  SoDialogIntegerSlider* colorMapMin = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("colormap min"));
  m_transferFunction->minValue.connectFrom(&colorMapMin->value);
  SoDialogIntegerSlider* sliceColorMapMin = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("slicecolormap min"));
  m_sliceTransferFunction->minValue.connectFrom(&sliceColorMapMin->value);
  SoDialogIntegerSlider* sliceColorMapMax = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("slicecolormap max"));
  m_sliceTransferFunction->maxValue.connectFrom(&sliceColorMapMax->value);

  //  m_viewer->setBackgroundColor(SbColor(1,1,0.5));
  initDialog();

  SoDialogIntegerSlider* volClippingSlider = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("volClippingNumPasses"));
  volClippingSlider->max = SoVolumeClippingGroup::getMaxNumPasses();

  // Update BBox/Axes (doesn't do anything if not enabled)
  updateVolBBoxGeometry();
  updateVolAxesGeometry();

#ifndef CAVELIB
  //Attach the default SodirectionalLight to the light editor
  SoSearchAction *action = new SoSearchAction;
  action->setType( SoDirectionalLight::getClassTypeId() );
  action->apply( m_root );
  SoPath *path = action->getPath();
  //If a light was found, connect it
  if ( path )
  {
    SoNode *tail = path->getTail();
    SoDirectionalLight* directionalLight = (SoDirectionalLight *) tail;
    m_headlightEd->attach( new SoPath( tail ) );
  }
  delete action;
#endif

#ifdef WIN32
  GetCurrentDirectory(sizeof(m_startDirectory), m_startDirectory);
#else
  getcwd(m_startDirectory, sizeof(m_startDirectory));
#endif

#ifndef CAVELIB
  SoXt::show(myWindow);

#if defined(SOQT)
  SoXt::mainLoop();
#else
#ifndef _WIN32
  SoXt::mainLoop();
#else
  //allow window to accept dropped files:
  DragAcceptFiles((HWND)parent, TRUE);

  //do our own mainloop so we can catch DROPFILES message
  XtAppContext context = NULL;
  XEvent event;
  while (SoXt::nextEvent(context, &event)) {
    if(event.message == WM_DROPFILES)
    {
      char buffer[255];
      HDROP hDrop = (HDROP)event.wParam;
      DragQueryFile(hDrop, 0, buffer, 255);
      myAuditor->doDataChange(0, buffer);
    }
    else
      SoXt::dispatchEvent(&event);
  }

#endif
#endif
  m_root->removeAllChildren();
  m_root->unref();

  delete m_viewer;

  SoDialogViz::finish();
  SoVolumeRendering::finish();
  SoXt::finish();

  delete m_proxiVisitor;
  delete m_tileVisitor;
  return 0;
#else // CAVELIB
  return mainRoot;
#endif //CAVELIB
}


