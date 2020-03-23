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

#ifndef CAVELIB

#include "VolumeVizAuditor.h"
#include <Inventor/STL/algorithm>
#include <Inventor/Xt/SoXtFileSelectionDialog.h>

#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/helpers/SbFileHelper.h>
#include <math.h>
#include <ScaleViz/SoScaleViz.h>

enum WhichAxis { PLUS_X, MINUS_X, PLUS_Y, MINUS_Y, PLUS_Z, MINUS_Z };
static WhichAxis m_camUpAxis = PLUS_Y;
static void
alignCamera(const WhichAxis viewAxis)
//
////////////////////////////////////////////////////////////////////////
{
  SoCamera *camera = m_viewer->getCamera();
  if (camera == NULL)
    return;

  // get center of rotation
  SbRotation camRot = camera->orientation.getValue();
  float radius = camera->focalDistance.getValue();
  SbMatrix mx;
  mx = camRot;
  SbVec3f forward(-mx[2][0], -mx[2][1], -mx[2][2]);
  SbVec3f center = camera->position.getValue()
    + radius * forward;

  // Avoid annoying warnings about truncation from double to float (VC++)
  const float pi = (float)M_PI;

  // Default is
  //   View Axis: PLUS_Z
  //   Up Axis  : PLUS_Y
  //
  // First rotate to requested view axis, then rotate to current up axis.
  //
  // If requested view axis is illegal with current up axis, do nothing!
  switch (viewAxis) {
  case PLUS_X: // +Y is default, +/- X is not allowed
    switch (m_camUpAxis) {
    case PLUS_Y:
      camRot.setValue(SbVec3f(0,1,0), pi/2);
      break;
    case MINUS_Y:
      camRot.setValue(SbVec3f(0,1,0), pi/2);
      camRot *= SbRotation(SbVec3f(1,0,0), pi);
      break;
    case PLUS_Z:
      camRot.setValue(SbVec3f(0,1,0), pi/2);
      camRot *= SbRotation(SbVec3f(1,0,0), pi/2);
      break;
    case MINUS_Z:
      camRot.setValue(SbVec3f(0,1,0), pi/2);
      camRot *= SbRotation(SbVec3f(1,0,0), -pi/2);
      break;
    default:
      break;
    }
    break;

  case MINUS_X: // +Y is default, +/- X is not allowed
    switch (m_camUpAxis) {
    case PLUS_Y:
      camRot.setValue(SbVec3f(0,1,0), -pi/2);
      break;
    case MINUS_Y:
      camRot.setValue(SbVec3f(0,1,0), -pi/2);
      camRot *= SbRotation(SbVec3f(1,0,0), pi);
      break;
    case PLUS_Z:
      camRot.setValue(SbVec3f(0,1,0), -pi/2);
      camRot *= SbRotation(SbVec3f(1,0,0), pi/2);
      break;
    case MINUS_Z:
      camRot.setValue(SbVec3f(0,1,0), -pi/2);
      camRot *= SbRotation(SbVec3f(1,0,0), -pi/2);
      break;
    default:
      break;
    }
    break;

  case PLUS_Y: // -Z is default, +/- Y up is not allowed
    switch (m_camUpAxis) {
    case PLUS_X:
      camRot.setValue(SbVec3f(1,0,0), -pi/2);
      camRot *= SbRotation(SbVec3f(0,1,0), -pi/2);
      break;
    case MINUS_X:
      camRot.setValue(SbVec3f(1,0,0), -pi/2);
      camRot *= SbRotation(SbVec3f(0,1,0), pi/2);
      break;
    case PLUS_Z:
      camRot.setValue(SbVec3f(1,0,0), -pi/2);
      camRot *= SbRotation(SbVec3f(0,1,0), pi);
      break;
    case MINUS_Z:
      camRot.setValue(SbVec3f(1,0,0), -pi/2);
      break;
    default:
      break;
    }
    break;

  case MINUS_Y: // +Z is default, +/- Y up is not allowed
    switch (m_camUpAxis) {
    case PLUS_X:
      camRot.setValue(SbVec3f(1,0,0), pi/2);
      camRot *= SbRotation(SbVec3f(0,1,0), pi/2);
      break;
    case MINUS_X:
      camRot.setValue(SbVec3f(1,0,0), pi/2);
      camRot *= SbRotation(SbVec3f(0,1,0), -pi/2);
      break;
    case PLUS_Z:
      camRot.setValue(SbVec3f(1,0,0), pi/2);
      break;
    case MINUS_Z:
      camRot.setValue(SbVec3f(1,0,0), pi/2);
      camRot *= SbRotation(SbVec3f(0,1,0), pi);
      break;
    default:
      break;
    }
    break;

  case PLUS_Z:  // +Y is default, + and - Z are not allowed
    switch (m_camUpAxis) {
    case PLUS_X:
      camRot.setValue(SbVec3f(0,0,1), -pi/2);
      break;
    case MINUS_X:
      camRot.setValue(SbVec3f(0,0,1), pi/2);
      break;
    case PLUS_Y:
      camRot.setValue(SbVec3f(0,1,0),0);
      break;
    case MINUS_Y:
      camRot.setValue(SbVec3f(0,0,1), pi);
      break;
    default:
      break;
    }
    break;

  case MINUS_Z:  // +Y is default, + and - Z are not allowed
    switch (m_camUpAxis) {
    case PLUS_X:
      camRot.setValue(SbVec3f(0,1,0), pi);
      camRot *= SbRotation(SbVec3f(0,0,1), -pi/2);
      break;
    case MINUS_X:
      camRot.setValue(SbVec3f(0,1,0), pi);
      camRot *= SbRotation(SbVec3f(0,0,1), pi/2);
      break;
    case PLUS_Y:
      camRot.setValue(SbVec3f(0,1,0), pi);
      break;
    case MINUS_Y:
      camRot.setValue(SbVec3f(0,1,0), pi);
      camRot *= SbRotation(SbVec3f(0,0,1), pi);
      break;
    default:
      break;
    }
    break;
  }

  // apply new rotation to the camera
  camera->orientation = camRot;

  // reposition camera to look at pt of interest
  mx = camRot;
  forward.setValue(-mx[2][0], -mx[2][1], -mx[2][2]);
  camera->position = center - radius * forward;
}

////////////////////////////////////////////////////////////////////////
VolumeVizAuditor::VolumeVizAuditor(): m_pCmapFileDialog(NULL),m_pDataFileDialog(NULL), m_pSettingsFileDialog(NULL),
                                      m_pClipmapFileDialog(0),m_pVolClippingFileDialog(0)
{
  userColorMapName = SbString("blueWhiteRed.txt");
  m_ROIManipNode=TRUE;
  m_isInteractive = true;
}

VolumeVizAuditor::~VolumeVizAuditor()
{
  delete m_pCmapFileDialog;
  delete m_pDataFileDialog;
  delete m_pSettingsFileDialog;
}

int
VolumeVizAuditor::findComplexityString(const SbString& str)
{
  int num = m_cplx->fieldSettings.getNum();
  for ( int i = 0; i < num; i++ )
  {
    if ( m_cplx->fieldSettings[i].find(str) != SbString::npos )
      return i;
  }
  return -1;
}

/*----------------------------- dialogCheckBox -----------------------------------------*/
void
VolumeVizAuditor::dialogCheckBox(SoDialogCheckBox* cpt)
{
  SbBool concerned = FALSE;
  SbBool state = cpt->state.getValue();

  if (cpt->auditorID.getValue() == "usedataext"){
    m_volData->useExtendedData.setValue(state);
  }

  else if (cpt->auditorID.getValue() == "complexity")
  {
    m_cplxSwitch->whichChild = state?SO_SWITCH_ALL:SO_SWITCH_NONE;

    if ( state )
    {
      m_isInteractive = true;
      int pos = findComplexityString("SoShadowGroup isActive");
      if ( m_shadowGroup->isActive.getValue() )
      {
        if ( pos == -1 )
          m_cplx->fieldSettings.set1Value(m_cplx->fieldSettings.getNum(), "SoShadowGroup isActive FALSE");
      }
      else
      {
        if ( pos != -1 )
          m_cplx->fieldSettings.deleteValues(pos, 1);
      }
    }
    else
      m_isInteractive = false;
  }
  else if (cpt->auditorID.getValue() == "shadows")
  {
    m_shadowGroup->isActive.setValue(state);

    if ( !state && m_isInteractive && m_cplx->fieldSettings.getNum() == 5 )
      m_cplx->fieldSettings.deleteValues(4, 1);

    if ( state && m_isInteractive )
      m_cplx->fieldSettings.set1Value(4, "SoShadowGroup isActive FALSE");
  }

  else if (cpt->auditorID.getValue() == "backgroundnode")
  {
    if(!state)
      m_backGroundSwitch->whichChild = SO_SWITCH_NONE;
    else
      m_backGroundSwitch->whichChild = SO_SWITCH_ALL;
  }

  else if (cpt->auditorID.getValue() == "invert")
  {
    m_invertTransparency = state;
		updateOpaqueRegion( m_transferFunction, m_minColorMap, m_maxColorMap, m_minOpaqueMap, m_maxOpaqueMap, m_invertTransparency );
		createHistoLegend( m_volHistoSwitch );
  }

  else if (cpt->auditorID.getValue() == "sliceinvert")
  {
    m_invertSliceTransparency = state;
		updateOpaqueRegion( m_sliceTransferFunction, m_minSliceColorMap, m_maxSliceColorMap, m_minSliceOpaqueMap, m_maxSliceOpaqueMap, m_invertSliceTransparency );
		createHistoLegend( m_volHistoSwitch, 1 );
  }

  else if (cpt->auditorID.getValue() == "colormapvisibility")
  {
    if(state)
      m_volColormapSwitch->whichChild = SO_SWITCH_ALL;
    else
      m_volColormapSwitch->whichChild = SO_SWITCH_NONE;
  }

  else if (cpt->auditorID.getValue() == "loadingcb")
  {
    if(state)
      m_loadingCBSwitch->whichChild = SO_SWITCH_ALL;
    else
      m_loadingCBSwitch->whichChild = SO_SWITCH_NONE;
  }

  //replace roi by roi manip and vice&versa
  else if (cpt->auditorID.getValue() == "replaceroi")
  {
    SoROI *roi = m_ROIManipNode ? m_ROIManip : m_volROI;
    // Search for the current SoROI / SoROIManip and replace it
    roi->ref(); // In order it is not deleted when replaced
    SoSearchAction searchAction;
    searchAction.setSearchingAll(TRUE);
    searchAction.setNode( roi );
    searchAction.apply( m_root );
    SoPath *path = searchAction.getPath();
    if ( !m_ROIManipNode ) {
      m_ROIManip->replaceNode( path );
      ((SoTabBoxDragger*)m_ROIManip->getDragger())->adjustScaleTabSize();
    }
    else
      m_ROIManip->replaceManip( path, m_volROI );
    m_ROIManipNode = !m_ROIManipNode;
    roi->unrefNoDelete(); // Balance the ref() above
    ROIChangedCB( (void*)m_ROIManip, NULL );
  }

  else if (cpt->auditorID.getValue() == "applyroitogeometry") {
    if (state)
      sliceGroup->insertChild( m_volROISwitch , 0 );
    else
      sliceGroup->removeChild( 0 );
  }

  else if (cpt->auditorID.getValue() == "draggervisibility")
    m_volROISwitch->whichChild = state ? SO_SWITCH_ALL : SO_SWITCH_NONE;

  else if (cpt->auditorID.getValue() == "constrained")
    m_ROIManip->constrained = state;

  else if (cpt->auditorID.getValue() == "controlling") {

    m_ROIManip->boxOn = !state;
    // Different color according to which box is dragged.
    SoMaterial *material = (SoMaterial*)(m_ROIManip->getDragger()->getPart("tabPlane1.scaleTabMaterial",0));
    if (state) {
      material->diffuseColor.setValue(1,0,0);
      material->emissiveColor.setValue(1,0,0);
    }
    else {
      material->diffuseColor.setValue(0,1,0);
      material->emissiveColor.setValue(0,1,0);
    }
  }

  else if (cpt->auditorID.getValue() == "orthousergba")
    m_orthoTab[m_currentSlice]->useRGBA = state;

  else if (cpt->auditorID.getValue() == "orthobump")
    m_orthoTab[m_currentSlice]->enableBumpMapping = state;

  else if (cpt->auditorID.getValue() == "orthooutlined")
    m_orthoQualityTab[m_currentSlice]->voxelOutline = state;

  else if (cpt->auditorID.getValue() == "obliqueusergba")
    m_obliSlice->useRGBA = state;

  else if (cpt->auditorID.getValue() == "obliquebump")
    m_obliSlice->enableBumpMapping = state;

  else if (cpt->auditorID.getValue() == "obliqueoutlined")
    m_obliSliceQuality->voxelOutline = state;

  else if (cpt->auditorID.getValue() == "orthovisibility"){
    m_switchOrthoTab[m_currentSlice]->whichChild = state ? SO_SWITCH_ALL : SO_SWITCH_NONE;
    concerned = TRUE;
    if(!state)
      m_orthoDataTab[m_currentSlice].timerSensor->unschedule();
    else if(m_orthoDataTab[m_currentSlice].animEnabled)
      m_orthoDataTab[m_currentSlice].timerSensor->schedule();
  }
  else if (cpt->auditorID.getValue() == "largeslicesupport")
  {
    // Activate/Deactivate large slice support for current orthoslice
    m_orthoTab[m_currentSlice]->largeSliceSupport.setValue(state);
    int pos = findComplexityString("SoOrthoSlice largeSliceSupport");
    if ( !state )
    {
      if ( pos != -1 )
        m_cplx->fieldSettings.deleteValues(pos, 1);
    }
    else
    {
      if ( pos == -1 )
        m_cplx->fieldSettings.set1Value(m_cplx->fieldSettings.getNum(), "SoOrthoSlice largeSliceSupport FALSE 1");
    }
  }
  else if (cpt->auditorID.getValue() == "VSlargeSliceSupport")
  {
    // Activate/Deactivate large slice support for current volumeSkin
    m_volSkin->largeSliceSupport.setValue(state);
    int pos = findComplexityString("SoVolumeSkin largeSliceSupport");
    if ( !state )
    {
      if ( pos != -1 )
        m_cplx->fieldSettings.deleteValues(pos, 1);
    }
    else
    {
      if ( pos == -1 )
        m_cplx->fieldSettings.set1Value(m_cplx->fieldSettings.getNum(), "SoVolumeSkin largeSliceSupport FALSE 1");
    }
  }

  else if (cpt->auditorID.getValue() == "allvisibility"){
    m_volRenderOrthSliceSwitch->whichChild = state ? SO_SWITCH_ALL : SO_SWITCH_NONE;
    concerned = TRUE;
    if(!state)
      for(int i = 0; i < m_lastNumSlices; i++)
        m_orthoDataTab[i].timerSensor->unschedule();
    else
    {
      SoDialogIntegerSlider* sliceNum = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("slicenumber"));
      SoDialogComboBox* controlledSlice = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("selectedslice"));
      int selectedSlice = controlledSlice->selectedItem.getValue();
      int sliceNumber = m_orthoTab[selectedSlice]->sliceNumber.getValue();
      sliceNum->value.setValue(sliceNumber);

      for(int i = 0; i < m_lastNumSlices; i++)
          m_orthoDataTab[i].timerSensor->schedule();
    }
  }

  else if (cpt->auditorID.getValue() == "obliquevisibility"){
    m_volRenderObliSliceSwitch->whichChild = state ? SO_SWITCH_ALL : SO_SWITCH_NONE;
    //concerned = TRUE;
  }

  else if (cpt->auditorID.getValue() == "orthodraggervisibility")
    m_draggerVolRenderSwitch->whichChild = state ? SO_SWITCH_ALL : SO_SWITCH_NONE;

  else if (cpt->auditorID.getValue() == "obliquedraggervisibility")
    m_draggerObliSwitch->whichChild = state ? SO_SWITCH_ALL : SO_SWITCH_NONE;

  else if (cpt->auditorID.getValue() == "orthoclipping"){
      m_orthoTab[m_currentSlice]->clipping = state;
      concerned = TRUE;
  }

  else if (cpt->auditorID.getValue() == "obliqueclipping"){
    m_obliClipPlane->on = state;
    concerned = TRUE;
  }

  else if (cpt->auditorID.getValue() == "orthoclippingside")
    m_orthoTab[m_currentSlice]->clippingSide = state ? SoOrthoSlice::BACK : SoOrthoSlice::FRONT;

  else if (cpt->auditorID.getValue() == "volgeometryvisibility")
    m_volRenderVolGeomSwitch->whichChild = state ? SO_SWITCH_ALL : SO_SWITCH_NONE;

  else if (cpt->auditorID.getValue() == "volgeometryclipping") {
    void setVolumeGeometryClipping( SbBool );
    setVolumeGeometryClipping( state ? TRUE : FALSE );
  }

  else if (cpt->auditorID.getValue() == "volgeometryinterpolation") {
    void setVolumeGeometryInterpolation( SbBool );
    setVolumeGeometryInterpolation( state ? TRUE : FALSE );
  }

  else if (cpt->auditorID.getValue() == "volgeometryoutlined") {
    void setVolumeGeometryOutline( SbBool );
    setVolumeGeometryOutline( state  );
  }

  else if (cpt->auditorID.getValue() == "skinbump") {
    m_volSkin->enableBumpMapping = state;
  }

  else if (cpt->auditorID.getValue() == "skinoutlined") {
    m_volSkinQuality->voxelOutline = state;
  }

  else if (cpt->auditorID.getValue() == "probe") {

    if(state)
    {
      m_switchForOrthoDragger->whichChild = SO_SWITCH_NONE;
    }
    else
    {
      m_switchForOrthoDragger->whichChild = SO_SWITCH_ALL;
    }

    static SbBool flag = FALSE;
    if ( !flag ) {
      m_profile = new volRendProfView(myWindow);
      flag = TRUE;
    }
    if ( m_profile->isVisibleProfile() ) {
      m_mouseMoveEvent->removeEventCallback( SoLocation2Event::getClassTypeId(), mouseMoveEventCB, m_root);
      m_mouseKeyEvent->removeEventCallback( SoMouseButtonEvent::getClassTypeId(), mouseKeyEventCB, m_root );
    if ( m_textSwitch )
      m_textSwitch->whichChild = SO_SWITCH_NONE;
      m_profile->hideProfile();
      // Note picking on the data overrides picking on the ROI dragger
      m_pick->style = SoPickStyle::UNPICKABLE;
      m_volROIPickStyle->style = SoPickStyle::SHAPE;
    }
    else {
      m_pick->style = SoPickStyle::SHAPE;
      m_volROIPickStyle->style = SoPickStyle::UNPICKABLE;
      m_profile->showProfile();
      m_mouseMoveEvent->addEventCallback( SoLocation2Event::getClassTypeId(), mouseMoveEventCB, m_root);
      m_mouseKeyEvent->addEventCallback( SoMouseButtonEvent::getClassTypeId(), mouseKeyEventCB, m_root );
      if( m_textSwitch )
        m_textSwitch->whichChild = SO_SWITCH_ALL;
    }
  }

  else if (cpt->auditorID.getValue() == "visibility")
    m_VolRenderSwitch->whichChild = state ? SO_SWITCH_ALL : SO_SWITCH_NONE;

  else if (cpt->auditorID.getValue() == "volumePriority")
    m_volumeGeometrySwitch->whichChild = state ? SO_SWITCH_ALL : SO_SWITCH_NONE;

  else if (cpt->auditorID.getValue() == "gpuvertgen")
    m_volRend->gpuVertexGen = state;

  else if (cpt->auditorID.getValue() == "subdivideTile")
    m_volRend->subdivideTile = state;

  else if (cpt->auditorID.getValue() == "earlyz")
    m_volRend->useEarlyZ = state;

  else if (cpt->auditorID.getValue() == "palettedtexture")
  {
    m_volData->usePalettedTexture = state;
    SoDialogCheckBox* compressedTex = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(SbString("compressedtexture"));
    if ( state )
      compressedTex->enable = FALSE;
    else
      compressedTex->enable = TRUE;
  }
  else if (cpt->auditorID.getValue() == "sharedpalettedtexture")
    m_volData->useSharedPalettedTexture = state;

  else if (cpt->auditorID.getValue() == "compressedtexture")
    m_volData->useCompressedTexture = state;

  else if (cpt->auditorID.getValue() == "showhisto") {

    m_showHistogram = state;
    if (m_showHistogram) {
      createHistoLegend( m_volHistoSwitch );
      m_volHistoSwitch->whichChild = SO_SWITCH_ALL;
    }
    else {
      m_volHistoSwitch->whichChild = SO_SWITCH_NONE;
    }
  }

  else if (cpt->auditorID.getValue() == "drawboxe") {

    if (state) {
      updateVolBBoxGeometry();
      m_volBBoxSwitch->whichChild.setValue (SO_SWITCH_ALL);
    }
    else {
      m_volBBoxSwitch->whichChild.setValue (SO_SWITCH_NONE);
    }
  }
  else if (cpt->auditorID.getValue() == "drawaxes") {

    if (state) {
      updateVolAxesGeometry();
      m_volAxesSwitch->whichChild.setValue (SO_SWITCH_ALL);
    }
    else {
      m_volAxesSwitch->whichChild.setValue (SO_SWITCH_NONE);
    }
  }
  else if (cpt->auditorID.getValue() == "floatingPointRendering") {
#ifndef __APPLE__
    if (state)
      m_viewer->setFloatingColorBuffer(TRUE, SoXtRenderArea::FLOAT_32_COLOR_BUFFER);
    else
      m_viewer->setFloatingColorBuffer(FALSE);
#endif
  }
  else if (cpt->auditorID.getValue() == "cam_lock_ROI") {
    m_camLockToROI = state;
  }

  else if (cpt->auditorID.getValue() == "datarangeactive") {
    m_dataRangeSwitch->whichChild = state ? SO_SWITCH_ALL : SO_SWITCH_NONE;
    if(state) {
      SoDialogRealSlider* dataRangeMin = (SoDialogRealSlider*)myTopLevelDialog->searchForAuditorId(SbString("datarangemin"));
      m_dataRange->min = dataRangeMin->value.getValue();
      SoDialogRealSlider* dataRangeMax = (SoDialogRealSlider*)myTopLevelDialog->searchForAuditorId(SbString("datarangemax"));
      m_dataRange->max = dataRangeMax->value.getValue();
    } else { //Reset isoslider to full data range
      double min=0., max=255.0;
      if (m_volData)
        m_volData->getMinMax(min,max);
      SoDialogRealSlider* isoVal = (SoDialogRealSlider*)myTopLevelDialog->searchForAuditorId(SbString("isovalue"));
      isoVal->max = (float)max;
      isoVal->min = (float)min;
      isoVal->value = (float)(max+min)/2.f;
      int numIso = m_volIsosurface->isovalues.getNum();
      for(int i = 0; i < numIso; i++)
        m_volIsosurface->isovalues.set1Value(i, (float)(max+min)/2.f);
    }
  } else if (cpt->auditorID.getValue() == "volumeClippingEnable") {
    m_volumeClippingSwitch->whichChild = state ? SO_SWITCH_ALL : SO_SWITCH_NONE;
  } else if (cpt->auditorID.getValue() == "volClippingMode") {
    getVolumeClippingGroup(m_selectedVolume)->clipOutside = state;
  } else if (cpt->auditorID.getValue() == "uniformGridClippingEnable") {
    m_uniformGridClippingSwitch->whichChild = state ? SO_SWITCH_ALL : SO_SWITCH_NONE;
  } else if (cpt->auditorID.getValue() == "gridDraggerVisibility") {
    changeGridDraggerVisibility(state);
  } else if (cpt->auditorID.getValue() == "gridVisibility") {
    SoSwitch *gridSwitch = (SoSwitch *)m_gridsGroup->getChild(m_selectedGrid);
    gridSwitch->whichChild = state ? SO_SWITCH_ALL : SO_SWITCH_NONE;
  } else if (cpt->auditorID.getValue() == "volClipDraggerVisibility") {
    changeVolClipDraggerVisibility(state);
  } else if(cpt->auditorID.getValue() == "volClipModelVisibility") {
    m_volumeClippingVisibleSwitch->whichChild = state?SO_SWITCH_ALL:SO_SWITCH_NONE;
  } else if(cpt->auditorID.getValue() == "jittering")
  {
    m_volQuality->jittering = state;
  } else if (cpt->auditorID.getValue() == "raycast")
  {
    m_volQuality->raycasting = state;
  } else if (cpt->auditorID.getValue() == "outlined")
  {
    m_volQuality->voxelOutline = state;
  }
  else if(cpt->auditorID.getValue() == "edgecoloring")
  {
    m_volQuality->edgeColoring = state;
  } 
  else if(cpt->auditorID.getValue() == "ambientocclusion")
  {
    m_volQuality->ambientOcclusion = state;
  } 
  else if(cpt->auditorID.getValue() == "deferredLighting")
  {
    m_volQuality->deferredLighting = state;
  } 
  else if(cpt->auditorID.getValue() == "boundopacity")
  {
    m_volQuality->boundaryOpacity = state;
  } else if(cpt->auditorID.getValue() == "edgeimage")
  {
    m_volQuality->edgeDetect2D = state;
    activeSurfaceScalar(true);
  } else if(cpt->auditorID.getValue() == "edgeluminance")
  {
    int bitmask = m_volQuality->edgeDetect2DMethod.getValue();
    m_volQuality->edgeDetect2DMethod = state? (bitmask|SoVolumeRenderingQuality::LUMINANCE):(bitmask&~SoVolumeRenderingQuality::LUMINANCE);
    activeSurfaceScalar(true);
  }
  else if(cpt->auditorID.getValue() == "edgegrad")
  {
    int bitmask = m_volQuality->edgeDetect2DMethod.getValue();
    m_volQuality->edgeDetect2DMethod = state? (bitmask|SoVolumeRenderingQuality::GRADIENT):(bitmask&~SoVolumeRenderingQuality::GRADIENT);
    activeSurfaceScalar(true);
  }
  else if(cpt->auditorID.getValue() == "edgedepth")
  {
    int bitmask = m_volQuality->edgeDetect2DMethod.getValue();
    m_volQuality->edgeDetect2DMethod = state? (bitmask|SoVolumeRenderingQuality::DEPTH):(bitmask&~SoVolumeRenderingQuality::DEPTH);
    activeSurfaceScalar(true);
  }

  //record dialog state for benchmark :
  if (concerned&&m_bench.isRecording()) m_bench.recordDialogState(  m_switchOrthoTab[m_currentSlice]->whichChild.getValue(),
                             m_volRenderObliSliceSwitch->whichChild.getValue(),
                             m_obliClipPlane->on.getValue(),
                             m_orthoTab[m_currentSlice]->clipping.getValue(),m_currentSlice);

}//#end of dialogCheckBox

/*----------------------------- dialogRadioButtons -----------------------------------------*/
void
VolumeVizAuditor::dialogRadioButtons (SoDialogRadioButtons *)
{
}

/*----------------------------- dialogEditText -----------------------------------------*/
void VolumeVizAuditor::dialogEditText(SoDialogEditText* cpt)
{
  if (cpt->auditorID.getValue() == "readfilename")
    benchReadFileName = const_cast<char*>((cpt->editText.getValue()).toLatin1());
  else if (cpt->auditorID.getValue() == "writefilename")
    benchWriteFileName = const_cast<char*>((cpt->editText.getValue()).toLatin1());

  else if (cpt->auditorID.getValue() == "numloop")
  {
    m_numBenchLoop = cpt->editText.getValue().toInt();
  }

  else if (cpt->auditorID.getValue() == "colormapfilename") {
    userColorMapName = cpt->editText.getValue();
	volumeUserColorMapName = userColorMapName;
  }
  else if (cpt->auditorID.getValue() == "slicecolormapfilename") {
    userColorMapName = cpt->editText.getValue();
	sliceUserColorMapName = userColorMapName;
  }
  else if (cpt->auditorID.getValue() == "datafilename")
    userDataName = cpt->editText.getValue();

  else if (cpt->auditorID.getValue() == "bumpscale") {
    float value = cpt->editText.getValue().toFloat();
    m_orthoTab[m_currentSlice]->bumpScale = value;
  }

  else if (cpt->auditorID.getValue() == "skinbumpscale") {
    float value = cpt->editText.getValue().toFloat();
     m_volSkin->bumpScale = value;
  }
  
  else if (cpt->auditorID.getValue() == "oblibumpscale") {
    float value = cpt->editText.getValue().toFloat();
    m_obliSlice->bumpScale = value;
  }

   // Subvolume size/bounds
  else {
    int xmin,ymin,zmin,xmax,ymax,zmax;
    SoROI *roi = m_ROIManipNode ? m_ROIManip : m_volROI;
    roi->subVolume.getValue().getBounds(xmin,ymin,zmin,xmax,ymax,zmax);
    if (cpt->auditorID.getValue() == "roi_min_x") {
      xmin = cpt->editText.getValue().toInt();
      roi->subVolume.setValue(xmin,ymin,zmin,xmax,ymax,zmax);
    }
    else if (cpt->auditorID.getValue() == "roi_min_y") {
      ymin = cpt->editText.getValue().toInt();
      roi->subVolume.setValue(xmin,ymin,zmin,xmax,ymax,zmax);
    }
    else if (cpt->auditorID.getValue() == "roi_min_z") {
      zmin = cpt->editText.getValue().toInt();
      roi->subVolume.setValue(xmin,ymin,zmin,xmax,ymax,zmax);
    }
    else if (cpt->auditorID.getValue() == "roi_max_x") {
      xmax = cpt->editText.getValue().toInt();
      roi->subVolume.setValue(xmin,ymin,zmin,xmax,ymax,zmax);
    }
    else if (cpt->auditorID.getValue() == "roi_max_y") {
      ymax = cpt->editText.getValue().toInt();
      roi->subVolume.setValue(xmin,ymin,zmin,xmax,ymax,zmax);
    }
    else if (cpt->auditorID.getValue() == "roi_max_z") {
      zmax = cpt->editText.getValue().toInt();
      roi->subVolume.setValue(xmin,ymin,zmin,xmax,ymax,zmax);
    }
    updateROIdisplay( xmin, ymin, zmin, xmax, ymax, zmax );
    if (m_ROIManipNode)
      ((SoTabBoxDragger*)m_ROIManip->getDragger())->adjustScaleTabSize();
  }
}//#fin editText


/*----------------------------- dialogRealSlider -----------------------------------------*/
void
VolumeVizAuditor::dialogRealSlider(SoDialogRealSlider* cpt)
{
  bool updateIsoMinMax = false;
  float value = cpt->value.getValue();

  if (cpt->auditorID.getValue() == "globalalpha") {
    m_material->transparency.set1Value(m_currentMaterial, 1.0F - value);
    //Force the update of the material editor slider
    m_mtlEditor->attach(m_material, m_currentMaterial);
  }

  else if (cpt->auditorID.getValue() == "background") {

    int color1 = (int)value;
    int color2 = color1 + 1;
    if (color2 > cpt->max.getValue())
      color2  = (int)(cpt->max.getValue());
    float w = value - color1;
    SbVec3f color = (1-w) * (SbVec3f)(cpt->colors[color1].getValue()) + w * (SbVec3f)(cpt->colors[color2].getValue());
    color *= 0.5;
    m_viewer->setBackgroundColor(SbColor(color));
  }

  else if (cpt->auditorID.getValue() == "volgeometryoffset") {
    void setVolumeGeometryOffset( float );
    setVolumeGeometryOffset( value );
  }

  else if (cpt->auditorID.getValue() == "datarangemin") {
    m_dataRange->min = value;
    updateIsoMinMax = true;
  }

  else if (cpt->auditorID.getValue() == "datarangemax") {
    m_dataRange->max = value;
    updateIsoMinMax = true;
  }
  else if (cpt->auditorID.getValue() == "isovalue")   {
    m_volIsosurface->isovalues.set1Value(m_currentIso, value);
  }

  else if (cpt->auditorID.getValue() == "volumeScalingX"
        || cpt->auditorID.getValue() == "volumeScalingY"
        || cpt->auditorID.getValue() == "volumeScalingZ") {
    // Could be X, Y or Z slider - most processing is the same
    const char *labelStr = cpt->label.getValue().toLatin1();
    int index = *labelStr - 'X';
    m_volScaleFactor[index] = value;
    SbVec3f smin, smax;
    m_volDefaultSize.getBounds( smin, smax );
    for (int i = 0; i < 3; ++i) {
      float hrange = (float)(smax[i] - smin[i]) * 0.5f;
      float center = (float)(smin[i] + hrange);
      hrange /= m_volScaleFactor[i];
      smin[i] = center - hrange;
      smax[i] = center + hrange;
    }
    // Modify the volume (geometric) size
    m_volData->extent = SbBox3f(smin,smax);
    // Re-compute bounding box geometry
    updateVolBBoxGeometry();
    // Hack to work around ebug 1791
    m_ROIManip->box.touch();
    if (m_ROIManipNode)
      ((SoTabBoxDragger*)m_ROIManip->getDragger())->adjustScaleTabSize();
    // Hack to work around ebug 1770
    SoPreferences::setInt( "IVVR_SLICE_CACHE_OFF", 1 );
  }
  else if (cpt->auditorID.getValue() == "boundOpacityThr") {
    m_volQuality->boundaryOpacityThreshold = value;
  }
  else if (cpt->auditorID.getValue() == "boundOpacityInt") {
    m_volQuality->boundaryOpacityIntensity = value;
  }
  else if (cpt->auditorID.getValue() == "edgeThr") {
    m_volQuality->edgeThreshold = value;
  }
  else if (cpt->auditorID.getValue() == "opacityThr") {
    m_volRend->opacityThreshold = value;
  }
  else if (cpt->auditorID.getValue() == "edgeInnerThr") {
    m_volQuality->edgeDetect2DInnerThreshold = value;
  }
  else if (cpt->auditorID.getValue() == "edgeOutterThr") {
    m_volQuality->edgeDetect2DOuterThreshold = value;
  }
  else if ( cpt->auditorID.getValue() == "fauxShadingStrength" ) {
    m_transferFunction->fauxShadingStrength = value;
    updateOpaqueRegion(m_transferFunction, m_minColorMap, m_maxColorMap, m_minOpaqueMap, m_maxOpaqueMap, m_invertTransparency);
  }
  else if ( cpt->auditorID.getValue() == "sliceFauxShadingStrength" ) {
    m_sliceTransferFunction->fauxShadingStrength = value;
    updateOpaqueRegion(m_sliceTransferFunction, m_minSliceColorMap, m_maxSliceColorMap, m_minSliceOpaqueMap, m_maxSliceOpaqueMap, m_invertSliceTransparency );
  }
  else if ( cpt->auditorID.getValue() == "fauxShadingLength" ) {
    m_transferFunction->fauxShadingLength = value;
    updateOpaqueRegion(m_transferFunction, m_minColorMap, m_maxColorMap, m_minOpaqueMap, m_maxOpaqueMap, m_invertTransparency);
  }
  else if ( cpt->auditorID.getValue() == "sliceFauxShadingLength" ) {
    m_sliceTransferFunction->fauxShadingLength = value;
    updateOpaqueRegion(m_sliceTransferFunction, m_minSliceColorMap, m_maxSliceColorMap, m_minSliceOpaqueMap, m_maxSliceOpaqueMap, m_invertSliceTransparency );
  }
  else if ( cpt->auditorID.getValue() == "fauxShadingDarkenThreshold" ) {
    m_transferFunction->fauxShadingDarkenThreshold = value;
    updateOpaqueRegion(m_transferFunction, m_minColorMap, m_maxColorMap, m_minOpaqueMap, m_maxOpaqueMap, m_invertTransparency);
  }
  else if ( cpt->auditorID.getValue() == "sliceFauxShadingDarkenThreshold" ) {
    m_sliceTransferFunction->fauxShadingDarkenThreshold = value;
    updateOpaqueRegion(m_sliceTransferFunction, m_minSliceColorMap, m_maxSliceColorMap, m_minSliceOpaqueMap, m_maxSliceOpaqueMap, m_invertSliceTransparency );
  }
  else if (cpt->auditorID.getValue() == "segmentedthreshold")
  {
    m_volQuality->segmentedInterpolationThreshold = value;
  }

  if (updateIsoMinMax && m_dataRangeSwitch->whichChild.getValue() != SO_SWITCH_NONE) {
    SoDialogRealSlider* isoVal = (SoDialogRealSlider*)myTopLevelDialog->searchForAuditorId(SbString("isovalue"));
    isoVal->max = (float)m_dataRange->max.getValue();
    isoVal->min = (float)m_dataRange->min.getValue();
    isoVal->value = isoVal->min.getValue();
    int numIso = m_volIsosurface->isovalues.getNum();
    for(int i = 0; i < numIso; i++)
      m_volIsosurface->isovalues.set1Value(i, isoVal->min.getValue());
  }
}//#fin RealSlider


/*----------------------------- dialogComboBox -----------------------------------------*/
void VolumeVizAuditor::dialogComboBox(SoDialogComboBox* cpt)
{
  int selectedItem = cpt->selectedItem.getValue();
  if (cpt->auditorID.getValue() == "renderstyle")
  {
    //SoDialogComboBox *buttons = (SoDialogComboBox *)cpt;
    SoDialogComboBox* samplingAlignmentCombo = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("samplingalignment"));

    switch(selectedItem) {
      case 0:
        m_volSkinSwitch->whichChild = 0;  //Classical volume rendering
        samplingAlignmentCombo->enable = TRUE;
        m_volQuality->lighting = FALSE;
        m_volQuality->preIntegrated = FALSE;
        m_volQuality->voxelizedRendering = FALSE;
        switchInterfaceToIsoMode(false);
        activeEdgesAndJitter(true);
        switchInterfaceToVolumeSkinMode(false);
        activeSurfaceScalar(false);
        break;
      case 1: {
        m_volSkinSwitch->whichChild = 0;  //Preintegrated
        m_volQuality->lighting = FALSE;
        m_volQuality->preIntegrated = TRUE;
        m_volQuality->voxelizedRendering = FALSE;
        samplingAlignmentCombo->enable = TRUE;
        switchInterfaceToIsoMode(false);
        activeEdgesAndJitter(true);
        switchInterfaceToVolumeSkinMode(false);
        activeSurfaceScalar(false);
        break;
      }
      case 2:
        m_volSkinSwitch->whichChild = 0;  //Lighting
        m_volQuality->lighting = TRUE;
        m_volQuality->preIntegrated = FALSE;
        samplingAlignmentCombo->enable = TRUE;
        m_volQuality->voxelizedRendering = FALSE;
        switchInterfaceToIsoMode(false);
        activeEdgesAndJitter(true);
        switchInterfaceToVolumeSkinMode(false);
        activeSurfaceScalar(true);
        break;
      case 3:
        m_volSkinSwitch->whichChild = 0;   //Preintegrated+Lighting
        m_volQuality->lighting = TRUE;
        m_volQuality->preIntegrated = TRUE;
        m_volQuality->voxelizedRendering = FALSE;
        samplingAlignmentCombo->enable = TRUE;
        switchInterfaceToIsoMode(false);
        activeEdgesAndJitter(true);
        switchInterfaceToVolumeSkinMode(false);
        activeSurfaceScalar(true);
        break;
      case 4: {
        m_volSkinSwitch->whichChild = 1;    //Isosurface
        samplingAlignmentCombo->enable = TRUE;
        m_volQuality->voxelizedRendering = FALSE;
        switchInterfaceToIsoMode(true);
        activeEdgesAndJitter(true);
        switchInterfaceToVolumeSkinMode(false);
        activeSurfaceScalar(false);
        break;
      }
      case 5:
        m_volSkinSwitch->whichChild = 2;    // volSkin
        m_volSkin->useRGBA = false;
        samplingAlignmentCombo->enable = FALSE;
        switchInterfaceToIsoMode(false);
        activeEdgesAndJitter(false);
        switchInterfaceToVolumeSkinMode(true);
        activeSurfaceScalar(true);
        break;
      case 6:
        m_volSkinSwitch->whichChild = 2;     // volSkin  RGB
        m_volSkin->useRGBA = true;
        samplingAlignmentCombo->enable = FALSE;
        switchInterfaceToIsoMode(false);
        activeEdgesAndJitter(false);
        switchInterfaceToVolumeSkinMode(true);
        activeSurfaceScalar(true);
        break;
      case 7:
        {          
          SoDialogCheckBox* rc = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(SbString("raycast"));
          rc->state = true;
          m_volSkinSwitch->whichChild = 0;   //Voxelized
          m_volQuality->voxelizedRendering = TRUE;
          m_volQuality->preIntegrated = FALSE;
          m_volQuality->lighting = TRUE;
          m_volQuality->voxelOutline = TRUE;
          m_volQuality->raycasting = TRUE;
          samplingAlignmentCombo->enable = FALSE;
          switchInterfaceToIsoMode(false);
          activeEdgesAndJitter(true);
          switchInterfaceToVolumeSkinMode(false);
          activeSurfaceScalar(true);
        }
        break;
    }
  }
  else if (cpt->auditorID.getValue() == "interpolation")
  {
    switch (selectedItem) {
      case 0:
        m_volRend->interpolation = SoVolumeRender::NEAREST;
        m_volSkin->interpolation = SoVolumeSkin::NEAREST;
        break;
      case 1:
        m_volRend->interpolation = SoVolumeRender::LINEAR;
        m_volSkin->interpolation = SoVolumeSkin::LINEAR;
        break;
      case 2:
        if ( m_volSkinSwitch->whichChild.getValue() != 2 )
          SoError::post("Multisample interpolation is only supported by VolumeSkin");
        m_volRend->interpolation = SoVolumeRender::LINEAR;
        m_volSkin->interpolation = SoVolumeSkin::MULTISAMPLE_12;
        break;
      case 3:
        m_volRend->interpolation = SoVolumeRender::CUBIC;
        m_volSkin->interpolation = SoVolumeSkin::CUBIC;
        break;
    }
  }
  else if(cpt->auditorID.getValue() == "numberOfSlice") {
    if(m_bench.isRecording()){
      SoError::post("Benchmark: Slices must be created before starting recording.\nStop recording");
      m_bench.stop();
      m_bench.reset();
    }
    int numSliceToCreate = selectedItem+1;
    createOrthoSlices(numSliceToCreate);

    updateSelectSliceMenu(numSliceToCreate);
    updateOrthoSliceMenu(selectedItem);

  }
  else if(cpt->auditorID.getValue() == "selectedslice") {

    m_currentSlice = selectedItem;
    //reinit();
    SoDialogIntegerSlider* sliceNum = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("slicenumber"));
    if( m_orthoTab.empty() )
      return;
    int sliceNumber = m_orthoTab[m_currentSlice]->sliceNumber.getValue();

    // Orthoslice visibility
    SoDialogCheckBox* vis = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(SbString("orthovisibility"));
    SbBool visibility = (m_switchOrthoTab[m_currentSlice]->whichChild.getValue()==SO_SWITCH_ALL)?TRUE:FALSE;
    vis->state.setValue(visibility);

    // Orthoslice mode
    SoDialogCheckBox* visLargeSlice = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(SbString("largeslicesupport"));
    SbBool largeSliceSupport = m_orthoTab[m_currentSlice]->largeSliceSupport.getValue();
    visLargeSlice->state.setValue(largeSliceSupport);

    resetDraggerPos(sliceNumber);

    int sliceMax = 0;
    int axe = m_orthoTab[m_currentSlice]->axis.getValue();
    if(axe ==SoOrthoSlice::X)
      sliceMax    = m_width - 1;
    else if(axe ==SoOrthoSlice::Y)
      sliceMax    = m_height - 1;
    else
      sliceMax    = m_depth - 1;

    // Make slice number slider max value appropriate for this axis
    // and set the new slice number
    SoDialogIntegerSlider *pSlider = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId("slicenumber");
    pSlider->max.setValue( sliceMax );

    updateOrthoSliceMenu(selectedItem);

    sliceNum->value.setValue(sliceNumber);

    //benchmarking
    if (m_bench.isRecording()) m_bench.recordDialogState(  m_switchOrthoTab[m_currentSlice]->whichChild.getValue(),
                             m_volRenderObliSliceSwitch->whichChild.getValue(),
                             m_obliClipPlane->on.getValue(),
                             m_orthoTab[m_currentSlice]->clipping.getValue(),m_currentSlice);

  }
  else if (cpt->auditorID.getValue() == "obliqueinterpolation") {
    switch (selectedItem) {
      case 0: m_obliSlice->interpolation = SoObliqueSlice::NEAREST;   break;
      case 1: m_obliSlice->interpolation = SoObliqueSlice::LINEAR;    break;
      case 2: m_obliSlice->interpolation = SoObliqueSlice::TRILINEAR; break;
      case 3: m_obliSlice->interpolation = SoObliqueSlice::MULTISAMPLE_12; break;
    }
  }

  else if (cpt->auditorID.getValue() == "tilesize") {
    m_tileSize = 1 << (selectedItem + 5);
    m_volData->ldmResourceParameters.getValue()->tileDimension = SbVec3i32(m_tileSize, m_tileSize, m_tileSize);
  }
  else if (cpt->auditorID.getValue() == "subtilesize") {
    int subTileSize = 1 << (selectedItem + 3);
    m_volData->ldmResourceParameters.getValue()->subTileDimension = SbVec3i32(subTileSize, subTileSize, subTileSize);
  }
  else if (cpt->auditorID.getValue() == "renderStyle") {
    static float transparency;
    m_volSkinSwitch->whichChild = selectedItem;
    switch (selectedItem) {
      case 0: // volume render
        m_material->diffuseColor.setValue(1,1,1);
        m_material->emissiveColor.setValue(0,0,0);
        m_material->transparency = transparency;
        break;
      case 1: // volume skin
        m_material->diffuseColor.setValue(0.5f,0.5f,0.5f);
        m_material->emissiveColor.setValue(0.5f,0.5f,0.5f);
        transparency = m_material->transparency[0];
        m_material->transparency = 0;
        break;
    }
  }
  else if (cpt->auditorID.getValue() == "skinfacemode") {
    SbString item = cpt->items[selectedItem];
    if ( item == "FRONT" )
    {
      m_volSkin->faceMode = SoVolumeSkin::FRONT;
    }
    else if ( item == "BACK" )
    {
      m_volSkin->faceMode = SoVolumeSkin::BACK;
    }
    else if ( item == "FRONT_AND_BACK" )
    {
      m_volSkin->faceMode = SoVolumeSkin::FRONT_AND_BACK;
    }
  }

  else if (cpt->auditorID.getValue() == "roishape") {

    switch (selectedItem) {
      case 0: m_ROIManip->flags = SoROI::SUB_VOLUME   ; break;
      case 1: m_ROIManip->flags = SoROI::CROSS        ; break;
      case 2: m_ROIManip->flags = SoROI::FENCE        ; break;
      case 3: m_ROIManip->flags = SoROI::EXCLUSION_BOX; break;
    }
  }

  else if (cpt->auditorID.getValue() == "draggerorientation") {

    int sliceNumber, sliceMax;
    float x, y, z, xfrac, yfrac, zfrac;
    xfrac = yfrac = zfrac = 0.5;
    sliceNumber = m_orthoTab[m_currentSlice]->sliceNumber.getValue();
    switch (selectedItem) {
    case 0 : // X
      m_sliceOri = X;
      m_draggerNormal.setValue(1,0,0);
      m_orthoTab[m_currentSlice]->axis.setValue(SoOrthoSlice::X);
      sliceMax    = m_width - 1;
      xfrac = (float)sliceNumber / (float)(m_width - 1);

      //m_bench.setOrthoSlice(m_ZOrthoSlice);//jh

      break;

    case 1 : // Y
      m_sliceOri = Y;
      m_draggerNormal.setValue(0,1,0);
      m_orthoTab[m_currentSlice]->axis.setValue(SoOrthoSlice::Y);
      sliceMax    = m_height - 1;                             //mmh
      yfrac = (float)sliceNumber / (float)(m_height - 1);

      //m_bench.setOrthoSlice(m_YOrthoSlice);//jh
      break;

    case 2 : // Z
      m_sliceOri = Z;
      m_draggerNormal.setValue(0,0,1);
      m_orthoTab[m_currentSlice]->axis.setValue(SoOrthoSlice::Z);
      sliceMax    = m_depth - 1;                             //mmh
      zfrac = (float)sliceNumber / (float)(m_depth - 1);
      //m_bench.setOrthoSlice(m_ZOrthoSlice);//jh
      break;
    }
    x = m_minWidth  + xfrac * (m_maxWidth - m_minWidth);
    y = m_minHeight + yfrac * (m_maxHeight - m_minHeight);
    z = m_minDepth  + zfrac * (m_maxDepth - m_minDepth);
    m_draggerSlicePos.setValue( x, y, z );
    setDraggerSlicePos( m_draggerSlicePos );
    m_draggerVolRender->enableValueChangedCallbacks( FALSE );
    m_draggerVolRender->translation.setValue( x, y, z );
    m_draggerVolRender->enableValueChangedCallbacks( TRUE );

    // Make slice number slider max value appropriate for this axis
    // and set the new slice number
    SoDialogIntegerSlider *pSlider = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId("slicenumber");
    pSlider->max.setValue( sliceMax );
   // pSlider->value.setValue( sliceNumber);
  }

  else if (cpt->auditorID.getValue() == "obliquealphatype") {

    switch (selectedItem) {
    case 0:
      m_obliSlice->alphaUse = SoObliqueSlice::ALPHA_BINARY;
      break;
    case 1:
      m_obliSlice->alphaUse = SoObliqueSlice::ALPHA_AS_IS;
      break;
    case 2:
      m_obliSlice->alphaUse = SoObliqueSlice::ALPHA_OPAQUE;
      break;
    }
  }

  else if (cpt->auditorID.getValue() == "orthoalphatype") {

    switch (selectedItem) {
    case 0:
      m_orthoTab[m_currentSlice]->alphaUse = SoOrthoSlice::ALPHA_BINARY;
      break;
    case 1:
      m_orthoTab[m_currentSlice]->alphaUse = SoOrthoSlice::ALPHA_AS_IS;
      break;
    case 2:
      m_orthoTab[m_currentSlice]->alphaUse = SoOrthoSlice::ALPHA_OPAQUE;
      break;
    }
  }

  else if (cpt->auditorID.getValue() == "orthointerpolation")
  {
    switch ( selectedItem )
    {
    case 0:
      m_orthoTab[m_currentSlice]->interpolation = SoOrthoSlice::NEAREST;
      break;
    case 1:
      m_orthoTab[m_currentSlice]->interpolation = SoOrthoSlice::LINEAR;
      break;
    case 2:
      m_orthoTab[m_currentSlice]->interpolation = SoOrthoSlice::MULTISAMPLE_12;
      break;
    }
  }
  else if (cpt->auditorID.getValue() == "colormap") {
    doCmapChange(selectedItem,m_transferFunction);
  }//end if colorMap choice

  else if (cpt->auditorID.getValue() == "slicecolormap") {
    doCmapChange(selectedItem,m_sliceTransferFunction);
  }//end if colorMap choice

  else if (cpt->auditorID.getValue() == "data") {
    doDataChange( selectedItem ,SbString("")) ;
  }//end if Data choice

  else if (cpt->auditorID.getValue() == "composition") {
    switch(selectedItem) {
      case 0:
        m_volRend->renderMode = SoVolumeRender::VOLUME_RENDERING;
        break;
      case 1:
        m_volRend->renderMode = SoVolumeRender::MIN_INTENSITY_PROJECTION;
        break;
      case 2:
        m_volRend->renderMode = SoVolumeRender::MAX_INTENSITY_PROJECTION;
        break;
      case 3:
        m_volRend->renderMode = SoVolumeRender::SUM_INTENSITY_PROJECTION;
        break;
      case 4:
        m_volRend->renderMode = SoVolumeRender::AVERAGE_INTENSITY_PROJECTION;
        break;
      default:
        m_volRend->renderMode = SoVolumeRender::VOLUME_RENDERING;
        break;
     }
  }
  // Camera align, etc.
  else if (cpt->auditorID.getValue() == "cam_up_axis") {
    SoDialogPushButton* btnPX = (SoDialogPushButton*)myTopLevelDialog->searchForAuditorId(SbString("cam_align_px"));
    SoDialogPushButton* btnMX = (SoDialogPushButton*)myTopLevelDialog->searchForAuditorId(SbString("cam_align_mx"));
    SoDialogPushButton* btnPY = (SoDialogPushButton*)myTopLevelDialog->searchForAuditorId(SbString("cam_align_py"));
    SoDialogPushButton* btnMY = (SoDialogPushButton*)myTopLevelDialog->searchForAuditorId(SbString("cam_align_my"));
    SoDialogPushButton* btnPZ = (SoDialogPushButton*)myTopLevelDialog->searchForAuditorId(SbString("cam_align_pz"));
    SoDialogPushButton* btnMZ = (SoDialogPushButton*)myTopLevelDialog->searchForAuditorId(SbString("cam_align_mz"));
    btnPX->enable = TRUE;
    btnMX->enable = TRUE;
    btnPY->enable = TRUE;
    btnMY->enable = TRUE;
    btnPZ->enable = TRUE;
    btnMZ->enable = TRUE;
    switch (selectedItem) {
    case 0:
      m_camUpAxis = PLUS_X;
      btnPX->enable = FALSE;
      btnMX->enable = FALSE;
      break;
    case 1:
      m_camUpAxis = MINUS_X;
      btnPX->enable = FALSE;
      btnMX->enable = FALSE;
      break;
    case 2:
      m_camUpAxis = PLUS_Y;
      btnPY->enable = FALSE;
      btnMY->enable = FALSE;
      break;
    case 3:
      m_camUpAxis = MINUS_Y;
      btnPY->enable = FALSE;
      btnMY->enable = FALSE;
      break;
    case 4:
      m_camUpAxis = PLUS_Z;
      btnPZ->enable = FALSE;
      btnMZ->enable = FALSE;
      break;
    default:
      m_camUpAxis = MINUS_Z;
      btnPZ->enable = FALSE;
      btnMZ->enable = FALSE;
      break;
    }
  }
  else if(cpt->auditorID.getValue() == "numberOfIso")
  {
    int numIsoToCreate = selectedItem+1;
    int numIso = m_volIsosurface->isovalues.getNum();
    if(numIso > numIsoToCreate)
    {
      m_volIsosurface->isovalues.deleteValues(numIsoToCreate);
      m_currentIso = (m_currentIso < numIsoToCreate-1 ? m_currentIso : numIsoToCreate-1);
    }
    else
      for(int i = numIso; i < numIsoToCreate; i++)
        m_volIsosurface->isovalues.set1Value(i, 0);
    updateSelectIsoMenu(numIsoToCreate);
    updateIsoSlider();
  }
  else if(cpt->auditorID.getValue() == "selectediso")
  {
    m_currentIso = selectedItem;
    updateIsoSlider();
  }
  else if(cpt->auditorID.getValue() == "interpolationmethod")
  {
    updateIsoInterpolationMethod(selectedItem);
  }
  else if(cpt->auditorID.getValue() == "numberOfMaterial")
  {
    int numMaterialToCreate = selectedItem+1;
    int numMaterial = m_material->diffuseColor.getNum();
    if(numMaterial > numMaterialToCreate)
    {
      m_material->diffuseColor.deleteValues(numMaterialToCreate);
      m_material->transparency.deleteValues(numMaterialToCreate);
      m_material->specularColor.deleteValues(numMaterialToCreate);
      m_material->shininess.deleteValues(numMaterialToCreate);
      m_currentMaterial = (m_currentMaterial < selectedItem ? m_currentMaterial : selectedItem);
      m_mtlEditor->attach(m_material, m_currentMaterial);
    }
    else
      for(int i = numMaterial; i < numMaterialToCreate; i++)
      {
        m_material->diffuseColor.set1Value(i, SbColor(0.8f, 0.8f, 0.8f));
        m_material->transparency.set1Value(i, 0);
        m_material->specularColor.set1Value(i, SbColor(0.1f,0.1f,0.1f));
        m_material->shininess.set1Value(i, 0.5);
      }
    updateSelectMaterialMenu(numMaterialToCreate);
  }
  else if(cpt->auditorID.getValue() == "selectedmaterial")
  {
    m_currentMaterial = selectedItem;
    m_mtlEditor->attach(m_material, m_currentMaterial);
    SoDialogRealSlider* transparencySlider = (SoDialogRealSlider*)myTopLevelDialog->searchForAuditorId(SbString("globalalpha"));
    transparencySlider->value = 1.f - (float)m_material->transparency[m_currentMaterial];
  }

  // Camera align, etc.
  else if (cpt->auditorID.getValue() == "cam_up_axis") {
    switch (selectedItem) {
    case 0:
      m_camUpAxis = PLUS_X;
      break;
    case 1:
      m_camUpAxis = MINUS_X;
      break;
    case 2:
      m_camUpAxis = PLUS_Y;
      break;
    case 3:
      m_camUpAxis = MINUS_Y;
      break;
    case 4:
      m_camUpAxis = PLUS_Z;
      break;
    default:
      m_camUpAxis = MINUS_Z;
      break;
    }
  } else if(cpt->auditorID.getValue() == "clippingModel") {
    SoVolumeClippingGroup *cg = getVolumeClippingGroup(m_selectedVolume);
    switch (selectedItem) {
      case 0:
        cg->replaceChild(0, new SoSphere);
        break;
      case 1:
        cg->replaceChild(0, new SoCone);
        break;
      case 2:
        cg->replaceChild(0, new SoCylinder);
        break;
      case 3:
        doClipModelChange(getDataFilePath("$OIVHOME/src/Inventor/examples/data/torus.iv"));
        break;
    }
    m_volumeClippingVisibleSwitch->replaceChild(2, cg->getChild(0));
    //update the manip/transform node
    changeVolumeClippingGroupManip(m_selectedVolume, new SoTransformerManip);
  } else if(cpt->auditorID.getValue() == "gridModel") {
    doClipmapChange(selectedItem, 0);
  } else if(cpt->auditorID.getValue() == "numberOfGridsID") {
    int numGridsToCreate = selectedItem+1;
    createGrids(numGridsToCreate);
    updateSelectGridMenu(numGridsToCreate);
    m_selectedGrid = (m_selectedGrid>=numGridsToCreate)?numGridsToCreate-1:m_selectedGrid;

    SoDialogComboBox* model = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("gridModel"));
    model->selectedItem = m_predefinedGrid[m_selectedGrid];
    SoDialogEditText* modelName = (SoDialogEditText*)myTopLevelDialog->searchForAuditorId(SbString("clipmapFilename"));
    modelName->editText = m_userGrid[m_selectedGrid];
  } else if(cpt->auditorID.getValue() == "selectedGrid") {
    m_selectedGrid = selectedItem;

    SoDialogComboBox* model = (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("gridModel"));
    model->selectedItem = m_predefinedGrid[m_selectedGrid];
    SoDialogEditText* modelName = (SoDialogEditText*)myTopLevelDialog->searchForAuditorId(SbString("clipmapFilename"));
    modelName->editText = m_userGrid[m_selectedGrid];

    //Update gridDraggerVisibility check box
    SoDialogCheckBox* dragVis = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(SbString("gridDraggerVisibility"));
    SoGroup *group = (SoGroup *)m_gridsGroup->getChild(m_selectedGrid);
    SbBool visibility = (group->getChild(0)->getTypeId() != SoTransform::getClassTypeId());
    dragVis->state = visibility;

    //Update gridVisibility check box
    SoDialogCheckBox* gridVis = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(SbString("gridVisibility"));
    SoSwitch *gridSwitch = (SoSwitch *)m_gridsGroup->getChild(m_selectedGrid);
    visibility = gridSwitch->whichChild.getValue() == SO_SWITCH_ALL;
    gridVis->state = visibility;
  } else if(cpt->auditorID.getValue() == "numberOfClipVol") {
  }
  else if ( cpt->auditorID.getValue() == "optimMode" )
  {
    switch (selectedItem)
    {
    case 0:
      switchOptimInterfaceToExpert(false);
      m_volRend->gpuVertexGen = FALSE;
      m_volRend->subdivideTile = FALSE;
      m_volRend->useEarlyZ = FALSE;
      break;

    case 1:
      switchOptimInterfaceToExpert(false);
      m_volRend->gpuVertexGen = TRUE;
      m_volRend->subdivideTile = TRUE;
      m_volRend->useEarlyZ = TRUE;
      m_volRend->numEarlyZPasses = 30;
      break;

    case 2:
      switchOptimInterfaceToExpert(true);
      break;
    }
  }
  else if ( cpt->auditorID.getValue() == "gradientquality" )
  {
    switch (selectedItem)
    {
    case 0:
      m_volQuality->gradientQuality = SoVolumeRenderingQuality::LOW;
      m_cplx->fieldSettings.set1Value(2, SbString("SoVolumeRenderingQuality gradientQuality LOW LOW"));
      break;
    case 1:
      m_volQuality->gradientQuality = SoVolumeRenderingQuality::MEDIUM;
      m_cplx->fieldSettings.set1Value(2, SbString("SoVolumeRenderingQuality gradientQuality LOW MEDIUM"));
      break;
    case 2:
      m_volQuality->gradientQuality = SoVolumeRenderingQuality::HIGH;
      m_cplx->fieldSettings.set1Value(2, SbString("SoVolumeRenderingQuality gradientQuality LOW HIGH"));
      break;
    }
  }
  else if ( cpt->auditorID.getValue() == "lightmodel" )
  {
    switch (selectedItem)
    {
    case 0:
      m_volQuality->lightingModel = SoVolumeRenderingQuality::OIV6;
      break;
    case 1:
      m_volQuality->lightingModel = SoVolumeRenderingQuality::OPENGL;
      break;
    }
  }
  else if(cpt->auditorID.getValue() == "samplingalignment")
  {
    switch (selectedItem)
    {
    case 0:
      m_volRend->samplingAlignment = SoVolumeRender::VIEW_ALIGNED;
      break;
    case 1:
      m_volRend->samplingAlignment = SoVolumeRender::DATA_ALIGNED;
      break;
    case 2:
      m_volRend->samplingAlignment = SoVolumeRender::BOUNDARY_ALIGNED;
      break;
    case 3:
      m_volRend->samplingAlignment = SoVolumeRender::SMOOTH_BOUNDARY_ALIGNED;
      break;
    }
  } 
}//#fin ComboBox


void VolumeVizAuditor::doClipmapChange(int num, const SbString& username)
{
  SbString filename;
  //bool useDist = (strcmp(clipmapFilenames[num], "Distance") == 0) && !username;
  if ( !username.isEmpty())
    filename = username;
  else
    filename = getDataFilePath(clipmapFilenames[num]);

  if(!username.isEmpty() && !filename.isEmpty() )
  {
    m_predefinedGrid[m_selectedGrid] = -1;
    SbString basename = SbFileHelper::getBaseName(username);
    m_userGrid[m_selectedGrid] = SbString(basename);
  }
  else
  {
    m_predefinedGrid[m_selectedGrid] = num;
    m_userGrid[m_selectedGrid] = SbString("");
  }

  SoSwitch *gridSwitch = (SoSwitch *)m_gridsGroup->getChild(m_selectedGrid);
  SoSearchAction *sa = new SoSearchAction;
  sa->setType(SoUniformGridClipping::getClassTypeId(), FALSE);
  sa->setSearchingAll(TRUE);
  sa->apply(gridSwitch);
  SoPath *path = sa->getPath();
  if(path) {
    SoUniformGridClipping *grid = (SoUniformGridClipping *)path->getTail();
    if(!filename.isEmpty())
      grid->filename = filename;
    else {
      SoUniformGridClipping *newGrid = createDefaultGrid(TRUE);
      newGrid->extent = grid->extent;
      gridSwitch->replaceChild(path->getIndexFromTail(0), newGrid);
    }
  }
  delete sa;
}

void
VolumeVizAuditor::clipmapFileDialogCB(void *data, SoXtFileSelectionDialog *dialog)
{
  char *filename = dialog->getFileName();
  if (filename && *filename != '\0') {

    ((VolumeVizAuditor *)data)->userClipmapName = filename;

    SoDialogEditText *editText =
      (SoDialogEditText *)myTopLevelDialog->searchForAuditorId("clipmapFilename");
    if (editText) {
      editText->editText = filename;
    }
    SoDialogComboBox *combo =
      (SoDialogComboBox *)myTopLevelDialog->searchForAuditorId("gridModel");
    int numItems = combo->items.getNum();
    combo->selectedItem = numItems - 1; // User defined file should be last item

    // Not possible to programmatically trigger combobox,
    // so call work function as if it was triggered.
    // Note: Need full file path in order to allow opening a data file
    // in an arbitrary directory (not just the predefined directories).
    char *filepath = dialog->getFilePath();
    ((VolumeVizAuditor *)data)->doClipmapChange( numItems - 1, filepath );
  }
}

void VolumeVizAuditor::changeGridDraggerVisibility(SbBool visible)
{
  SoGroup *group = (SoGroup *)m_gridsGroup->getChild(m_selectedGrid);
  SoSearchAction *sa = new SoSearchAction;
  sa->setNode(group->getChild(0));
  sa->setSearchingAll(TRUE);
  sa->apply(m_root);
  SoPath *path = sa->getPath();
  if(visible) {
    SoJackManip *manip = new SoJackManip;
    manip->replaceNode(path);
  } else {
    SoJackManip *manip = (SoJackManip *)group->getChild(0);
    SoTransform *xform = new SoTransform;
    xform->setName("GridTransform");
    manip->ref();
    manip->replaceManip(path, xform);
    manip->unref();
  }
  delete sa;
}

void VolumeVizAuditor::changeVolClipDraggerVisibility(SbBool visible)
{
  SoSearchAction *sa = new SoSearchAction;
  sa->setNode(getVolumeClippingGroupManip(m_selectedVolume));
  sa->setSearchingAll(TRUE);
  sa->apply(m_root);
  SoPath *path = sa->getPath();

  if(!visible) {
    SoTransform *xform = new SoTransform;
    xform->setName("VolClipTransform");
    SoTransformerManip *manip = (SoTransformerManip *)getVolumeClippingGroupManip(m_selectedVolume);
    manip->ref();
    manip->replaceManip(path, xform);
    manip->unrefNoDelete();
  } else {
    SoTransformerManip *manip = new SoTransformerManip;
    manip->replaceNode(path);
  }
  delete sa;
}

void
VolumeVizAuditor::volumeClippingFileDialogCB(void *data, SoXtFileSelectionDialog *dialog)
{
  char *filename = dialog->getFileName();
  if (filename && *filename != '\0') {

    ((VolumeVizAuditor *)data)->userClipmapName = filename;

    SoDialogEditText *editText =
      (SoDialogEditText *)myTopLevelDialog->searchForAuditorId("clipModelFilename");
    if (editText) {
      editText->editText = filename;
    }

    char *filepath = dialog->getFilePath();
    ((VolumeVizAuditor *)data)->doClipModelChange(filepath );
  }
}

void VolumeVizAuditor::doClipModelChange(const SbString& username)
{
  // Open the input file
  SoInput mySceneInput;
  if (!mySceneInput.openFile(username)) {
    return;
  }
  // Read the whole file into the database
  SoSeparator *myGraph = SoDB::readAll(&mySceneInput);
  if (myGraph == NULL) {
    return;
  }
  mySceneInput.closeFile();

  SoVolumeClippingGroup *cg = getVolumeClippingGroup(m_selectedVolume);
  SoNode *oldClippingModel = cg->getChild(0);
  m_volumeClippingVisibleSwitch->replaceChild(oldClippingModel, myGraph);
  cg->replaceChild(0, myGraph);
  changeVolumeClippingGroupManip(m_selectedVolume, new SoTransformerManip);
}

/*----------------------------- dialogPushButton -----------------------------------------*/
void VolumeVizAuditor::dialogPushButton(SoDialogPushButton* cpt)
{
   if (cpt->auditorID.getValue() == "play")
   {
      m_bench.play();
   }
   if (cpt->auditorID.getValue() == "record")
   {
      m_bench.record();
   }
   if (cpt->auditorID.getValue() == "stop")
   {
      m_bench.stop();
   }
   if (cpt->auditorID.getValue() == "write")
   {
      m_bench.writeTrajectory((char *)benchWriteFileName);
   }
   if (cpt->auditorID.getValue() == "read")
   {
      m_bench.readTrajectory((char *)benchReadFileName);
   }
   if (cpt->auditorID.getValue() == "loop")
   {
    // for(int i=0;i<;i++)
       m_bench.play(FALSE,m_numBenchLoop);
   }
   if (cpt->auditorID.getValue() == "rewind")
   {
      m_bench.rewind();
   }

   else if (cpt->auditorID.getValue() == "material")
		m_mtlEditor->show();

   else if (cpt->auditorID.getValue() == "slicematerial")
     m_mtlSliceEditor->show();

   else if (cpt->auditorID.getValue() == "light")
		m_headlightEd->show();

   else if (cpt->auditorID.getValue() == "resetroi") {
    // Reset subvolume and ROI boxes
    SbVec3i32 volMax(m_width-1,m_height-1,m_depth-1);
    SbVec3i32 halfVol = volMax / 2;
    m_volROI->subVolume.setValue( SbVec3i32(0,0,0), halfVol );
    m_volROI->box.setValue( SbVec3i32(0,0,0), volMax );
    m_ROIManip->subVolume.setValue( SbVec3i32(0,0,0), halfVol );
    m_ROIManip->box.setValue( SbVec3i32(0,0,0), volMax );
#ifdef ROI_TAB_BOX
    ((SoTabBoxDragger*)m_ROIManip->getDragger())->adjustScaleTabSize();
#endif
    ROIChangedCB( (void*)m_ROIManip, NULL );
   }

   else if (cpt->auditorID.getValue() == "exit") {
     SoScaleViz::finish();
     exit(-1);
   }

   else if (cpt->auditorID.getValue() == "alternate") {

      //jh: I don't know if it should be called right before the write action
      //or before the creation of the scenegraph. Currently in main function
      SoVolumeRendering::setWriteAlternateRep(TRUE);
      writeFile(m_root, (char*)"alternateRep.iv");
      SoVolumeRendering::setWriteAlternateRep(FALSE);
   }
  else if (cpt->auditorID.getValue() == (char *)"colormapBrowse") {
    if (m_pCmapFileDialog == NULL) {
      m_pCmapFileDialog = new SoXtFileSelectionDialog;
      m_pCmapFileDialog->setFileDirectory((char*)"../Data/");
      m_pCmapFileDialog->setTitle((char*)"Volume Colormap file");
    }
    m_pCmapFileDialog->setTitle((char*)"Volume Colormap file");
    m_data.m_transferFunc = m_transferFunction;
    m_pCmapFileDialog->setAcceptCallback( VolumeVizAuditor::cmapFileDialogCB, this );
    m_pCmapFileDialog->show();
  }
  else if (cpt->auditorID.getValue() == "slicecolormapBrowse") {
    if (m_pCmapFileDialog == NULL) {
      m_pCmapFileDialog = new SoXtFileSelectionDialog;
      m_pCmapFileDialog->setFileDirectory((char*)"../Data/");
      m_pCmapFileDialog->setTitle((char*)"Slice Colormap file");
    }
    m_pCmapFileDialog->setTitle((char*)"Slice Colormap file");
    m_data.m_transferFunc = m_sliceTransferFunction;
    m_pCmapFileDialog->setAcceptCallback( VolumeVizAuditor::cmapFileDialogCB, this );
    m_pCmapFileDialog->show();
  }
  else if (cpt->auditorID.getValue() == "dataBrowse") {
    if (m_pDataFileDialog == NULL) {
      m_pDataFileDialog = new SoXtFileSelectionDialog;
      m_pDataFileDialog->setFileDirectory((char*)"../Data/");
      m_pDataFileDialog->setTitle((char*)"Volume data file");
    }
    m_pDataFileDialog->setAcceptCallback( VolumeVizAuditor::dataFileDialogCB, this );
    m_pDataFileDialog->show();
  }
  else if (cpt->auditorID.getValue() == "settingsBrowse") {
    if (m_pSettingsFileDialog == NULL) {
      m_pSettingsFileDialog = new SoXtFileSelectionDialog;
      m_pSettingsFileDialog->setFileDirectory((char*)"");
      m_pSettingsFileDialog->setFileName((char*)"oiv.cfg");
      m_pSettingsFileDialog->setTitle((char*)"Settings file");
      m_pSettingsFileDialog->setMode(SoXtFileSelectionDialog::SAVE_FILE);
    }
    m_pSettingsFileDialog->setAcceptCallback( VolumeVizAuditor::settingsFileDialogCB, this );
    m_pSettingsFileDialog->show();
  }
  // Camera align, view, etc.
  else if (cpt->auditorID.getValue() == "cam_align_px") {
    alignCamera( PLUS_X );
  }
  else if (cpt->auditorID.getValue() == "cam_align_mx") {
    alignCamera( MINUS_X );
  }
  else if (cpt->auditorID.getValue() == "cam_align_py") {
    alignCamera( PLUS_Y );
  }
  else if (cpt->auditorID.getValue() == "cam_align_my") {
    alignCamera( MINUS_Y );
  }
  else if (cpt->auditorID.getValue() == "cam_align_pz") {
    alignCamera( PLUS_Z );
  }
  else if (cpt->auditorID.getValue() == "cam_align_mz") {
    alignCamera( MINUS_Z );
  }
  else if (cpt->auditorID.getValue() == "cam_view_ROI") {
    int state = m_volROISwitch->whichChild.getValue();
    if (state == SO_SWITCH_ALL) {
      // ROI is active
      // Note: Don't use m_volROI here - its bbox is always empty!
      SoCamera *pCam = m_viewer->getCamera();
      pCam->viewAll( m_ROIManip, m_viewer->getViewportRegion() );
    }
    else
      m_viewer->viewAll();  // ROI is not active and may not be meaningful
  }
  else if (cpt->auditorID.getValue() == "animbutton")
  {
    m_orthoDataTab[m_currentSlice].animEnabled = !m_orthoDataTab[m_currentSlice].animEnabled;

    if(m_orthoDataTab[m_currentSlice].animEnabled &&
       m_switchOrthoTab[m_currentSlice]->whichChild.getValue() == SO_SWITCH_ALL &&
       m_volRenderOrthSliceSwitch->whichChild.getValue() == SO_SWITCH_ALL)
      m_orthoDataTab[m_currentSlice].timerSensor->schedule();
//    else
//      m_orthoDataTab[m_currentSlice].timerSensor->unschedule();
    updateOrthoSliceMenu(m_currentSlice);
  }

  // Camera align, view, etc.
  else if (cpt->auditorID.getValue() == "cam_align_px") {
    alignCamera( PLUS_X );
  }
  else if (cpt->auditorID.getValue() == "cam_align_mx") {
    alignCamera( MINUS_X );
  }
  else if (cpt->auditorID.getValue() == "cam_align_py") {
    alignCamera( PLUS_Y );
  }
  else if (cpt->auditorID.getValue() == "cam_align_my") {
    alignCamera( MINUS_Y );
  }
  else if (cpt->auditorID.getValue() == "cam_align_pz") {
    alignCamera( PLUS_Z );
  }
  else if (cpt->auditorID.getValue() == "cam_align_mz") {
    alignCamera( MINUS_Z );
  }
  else if (cpt->auditorID.getValue() == "cam_view_ROI") {
    int state = m_volROISwitch->whichChild.getValue();
    if (state == SO_SWITCH_ALL) {
      // ROI is active
      // Note: Don't use m_volROI here - its bbox is always empty!
      SoCamera *pCam = m_viewer->getCamera();
      pCam->viewAll( m_ROIManip, m_viewer->getViewportRegion() );
    }
    else
      m_viewer->viewAll();  // ROI is not active and may not be meaningful
  }
  else if (cpt->auditorID.getValue() == "volumeScalingReset") {
    m_volScaleFactor.setValue( 1, 1, 1 );
    ((SoDialogRealSlider*)myTopLevelDialog->searchForAuditorId(SbString("volumeScalingX")))->value = 1;
    ((SoDialogRealSlider*)myTopLevelDialog->searchForAuditorId(SbString("volumeScalingY")))->value = 1;
    ((SoDialogRealSlider*)myTopLevelDialog->searchForAuditorId(SbString("volumeScalingZ")))->value = 1;
    // Modify the volume (geometric) size
    m_volData->extent = m_volDefaultSize;
    // Re-compute bounding box geometry
    updateVolBBoxGeometry();
    // Hack to work around ebug 1791
    m_ROIManip->box.touch();
    if (m_ROIManipNode)
      ((SoTabBoxDragger*)m_ROIManip->getDragger())->adjustScaleTabSize();
    // Hack to work around ebug 1770
    SoPreferences::setInt( "IVVR_SLICE_CACHE_OFF", 1 );
  }
  else if (cpt->auditorID.getValue() == "clipmapBrowse") {
    if (m_pClipmapFileDialog == NULL) {
      m_pClipmapFileDialog = new SoXtFileSelectionDialog;
      m_pClipmapFileDialog->setFileDirectory((char*)"../Data/clipmaps");
      m_pClipmapFileDialog->setTitle((char*)"Uniform grid file (*.jpg, *.png ...)");
    }
    m_pClipmapFileDialog->setAcceptCallback( VolumeVizAuditor::clipmapFileDialogCB, this );
    m_pClipmapFileDialog->show();
  } else if (cpt->auditorID.getValue() == "clipModelBrowse") {
    if (m_pClipmapFileDialog == NULL) {
      m_pVolClippingFileDialog = new SoXtFileSelectionDialog;
      m_pVolClippingFileDialog->setFileDirectory((char*)"../Data/");
      m_pVolClippingFileDialog->setTitle((char*)"Choose an iv file");
    }
    m_pVolClippingFileDialog->setAcceptCallback( VolumeVizAuditor::volumeClippingFileDialogCB, this );
    m_pVolClippingFileDialog->show();
  }
}

/*----------------------------- dialogIntegerSlider -----------------------------------------*/
void VolumeVizAuditor::dialogIntegerSlider(SoDialogIntegerSlider* cpt)
{
  int value = cpt->value.getValue();

  if (cpt->auditorID.getValue() == "zpass")  {
    m_volRend->numEarlyZPasses = value;
  } if (cpt->auditorID.getValue() == "slicenumber") {

    float x,y,z,xfrac,yfrac,zfrac;
    cpt->value.setValue( value );
    m_draggerSlicePos.getValue( x, y, z );
    m_orthoTab[m_currentSlice]->sliceNumber.setValue( value );
    int axe = m_orthoTab[m_currentSlice]->axis.getValue();
    if(axe==SoOrthoSlice::X) {
      cpt->max.setValue(m_width-1);
      // m_width = number of slices
      // m_min/maxWidth = min/max X values of volume geometry
      xfrac = (float)value / (float)(m_width - 1);
      x = m_minWidth + xfrac * (m_maxWidth - m_minWidth);
      //m_bench.setOrthoSlice(m_XOrthoSlice);//jh
    }
    else if (axe==SoOrthoSlice::Y){
      cpt->max.setValue(m_height-1);
      yfrac = (float)value / (float)(m_height - 1);
        y = m_minHeight + yfrac * (m_maxHeight - m_minHeight);
        //m_bench.setOrthoSlice(m_YOrthoSlice);//jh
    }
    else{
      cpt->max.setValue(m_depth-1);
      zfrac = (float)value / (float)(m_depth - 1);
      z = m_minDepth + zfrac * (m_maxDepth - m_minDepth);
      //m_bench.setOrthoSlice(m_ZOrthoSlice);//jh
    }
    m_draggerSlicePos.setValue( x, y, z );
    m_draggerVolRender->enableValueChangedCallbacks( FALSE );
    m_draggerVolRender->translation.setValue( x, y, z );
    m_draggerVolRender->enableValueChangedCallbacks( TRUE );

    VVizBenchmark::sliceMotionCallback(&m_bench,m_draggerVolRender);
  }

  else if (cpt->auditorID.getValue() == "surfacescalar")
  {
    m_volQuality->surfaceScalarExponent = (float)value;
  }

  else if (cpt->auditorID.getValue() == "normalizegradient")
  {
    m_volQuality->unnormalizedGradientExponent = (float)value;
  }

  else if (cpt->auditorID.getValue() == "numberslices")
  {
    m_volRend->numSlices = value;

    int minNumSlice = std::min(value/3, 400);
    int increment = std::max(200, value/3);
    m_cplx->fieldSettings.set1Value(0, "SoVolumeRender numSlices " + SbString(minNumSlice) + " " + SbString(value) + " " + SbString(increment));

  }
  else if (cpt->auditorID.getValue() == "colormap min") {

		if (value > m_maxColorMap ) {
#if defined(_DEBUG)
			DISPLAY_ERROR("Max < Min: impossible");
#endif
		}
		else {
			// Remap colormap, then re-apply min/max opaque values
			m_minColorMap = value;
			updateOpaqueRegion( m_transferFunction, m_minColorMap, m_maxColorMap, m_minOpaqueMap, m_maxOpaqueMap, m_invertTransparency );
			createHistoLegend( m_volHistoSwitch );
		}
  }

  else if (cpt->auditorID.getValue() == "colormap max") {

		if (value < m_minColorMap) {
#if defined(_DEBUG)
			DISPLAY_ERROR("Max < Min: impossible");
#endif
		}
		else {
			// Remap colormap, then re-apply min/max opaque values
			m_maxColorMap = value;
			updateOpaqueRegion( m_transferFunction, m_minColorMap, m_maxColorMap, m_minOpaqueMap, m_maxOpaqueMap, m_invertTransparency );
			createHistoLegend( m_volHistoSwitch );
		}
  }

  else if (cpt->auditorID.getValue() == "opaque min") {

		if (value > m_maxOpaqueMap) {
#if defined(_DEBUG)
			DISPLAY_ERROR("Max < Min: impossible");
#endif
		}
		else {
			// Remap colormap, then re-apply min/max opaque values
			m_minOpaqueMap = value;
			updateOpaqueRegion( m_transferFunction, m_minColorMap, m_maxColorMap, m_minOpaqueMap, m_maxOpaqueMap, m_invertTransparency );
			createHistoLegend( m_volHistoSwitch );
		}
  }

  else if (cpt->auditorID.getValue() == "opaque max") {

		if (value < m_minOpaqueMap) {
#if defined(_DEBUG)
      DISPLAY_ERROR("Max < Min: impossible");
#endif
		}
		else {
			// Remap colormap, then re-apply min/max opaque values
			m_maxOpaqueMap = value;
			updateOpaqueRegion( m_transferFunction, m_minColorMap, m_maxColorMap, m_minOpaqueMap, m_maxOpaqueMap, m_invertTransparency );
			createHistoLegend( m_volHistoSwitch );
		}
  }

  else if (cpt->auditorID.getValue() == "slicecolormap min") {

		if (value > m_maxSliceColorMap ) {
#if defined(_DEBUG)
			DISPLAY_ERROR("Max < Min: impossible");
#endif
		}
		else {
			// Remap colormap, then re-apply min/max opaque values
			m_minSliceColorMap = value;
			updateOpaqueRegion( m_sliceTransferFunction, m_minSliceColorMap, m_maxSliceColorMap, m_minSliceOpaqueMap, m_maxSliceOpaqueMap, m_invertSliceTransparency );
			createHistoLegend( m_volHistoSwitch, 1 );
		}
  }

  else if (cpt->auditorID.getValue() == "slicecolormap max") {

		if (value < m_minSliceColorMap) {
#if defined(_DEBUG)
			DISPLAY_ERROR("Max < Min: impossible");
#endif
		}
		else {
			// Remap colormap, then re-apply min/max opaque values
			m_maxSliceColorMap = value;
			updateOpaqueRegion( m_sliceTransferFunction, m_minSliceColorMap, m_maxSliceColorMap, m_minSliceOpaqueMap, m_maxSliceOpaqueMap, m_invertSliceTransparency );
			createHistoLegend( m_volHistoSwitch, 1 );
		}
  }

  else if (cpt->auditorID.getValue() == "sliceopaque min") {

		if (value > m_maxSliceOpaqueMap) {
#if defined(_DEBUG)
			DISPLAY_ERROR("Max < Min: impossible");
#endif
		}
		else {
			// Remap colormap, then re-apply min/max opaque values
			m_minSliceOpaqueMap = value;
			updateOpaqueRegion( m_sliceTransferFunction, m_minSliceColorMap, m_maxSliceColorMap, m_minSliceOpaqueMap, m_maxSliceOpaqueMap, m_invertSliceTransparency );
			createHistoLegend( m_volHistoSwitch, 1 );
		}
  }

  else if (cpt->auditorID.getValue() == "sliceopaque max") {

		if (value < m_minSliceOpaqueMap) {
#if defined(_DEBUG)
      DISPLAY_ERROR("Max < Min: impossible");
#endif
		}
		else {
			// Remap colormap, then re-apply min/max opaque values
			m_maxSliceOpaqueMap = value;
			updateOpaqueRegion( m_sliceTransferFunction, m_minSliceColorMap, m_maxSliceColorMap, m_minSliceOpaqueMap, m_maxSliceOpaqueMap, m_invertSliceTransparency );
			createHistoLegend( m_volHistoSwitch, 1 );
		}
  } else  if (cpt->auditorID.getValue() == "isovalue")   {
    m_volIsosurface->isovalues.set1Value(m_currentIso, (float)value);
  } else if (cpt->auditorID.getValue() == "animspeed") {
     m_orthoDataTab[m_currentSlice].fps = value;
     if(value==0) {
       m_orthoDataTab[m_currentSlice].timerSensor->unschedule();
       return;
     } else {
      m_orthoDataTab[m_currentSlice].timerSensor->setInterval(SbTime (1./(float(value))));
      if(m_orthoDataTab[m_currentSlice].animEnabled)
        m_orthoDataTab[m_currentSlice].timerSensor->schedule();
     }
  } else if (cpt->auditorID.getValue() == "volClippingNumPasses") {
    getVolumeClippingGroup(m_selectedVolume)->numPasses = value;
  }
}

/*----------------------------- fileSelection ---------------------------------------*/

void
VolumeVizAuditor::cmapFileDialogCB(void *data, SoXtFileSelectionDialog *dialog)
{
  char *filename = dialog->getFileName();
  if (filename && *filename != '\0') {
    // Note: Need full file path in order to allow opening a colormap
    // in an arbitrary directory (not just the predefined directories).
    ((VolumeVizAuditor *)data)->userColorMapName = dialog->getFilePath();

    SoDialogEditText *editText;
    SoDialogComboBox *combo;
    char* dlgName = dialog->getTitle();
    int r = strcmp("Volume Colormap file", dlgName);
    if(r == 0){
      editText = (SoDialogEditText *)myTopLevelDialog->searchForAuditorId("colormapfilename");
      combo =    (SoDialogComboBox *)myTopLevelDialog->searchForAuditorId("colormap");
    }
    else{
      editText = (SoDialogEditText *)myTopLevelDialog->searchForAuditorId("slicecolormapfilename");
      combo =    (SoDialogComboBox *)myTopLevelDialog->searchForAuditorId("slicecolormap");
    }
    if (editText) {
      editText->editText = filename;
    }
    int numItems = combo->items.getNum();
    combo->selectedItem = numItems - 1; // User defined file should be last item

    // Not possible to programmatically trigger combobox,
    // so call work function as if it was triggered
    ((VolumeVizAuditor *)data)->doCmapChange( numItems - 1,((VolumeVizAuditor *)data)->m_data.m_transferFunc);
  }
}

void
VolumeVizAuditor::dataFileDialogCB(void *data, SoXtFileSelectionDialog *dialog)
{
  char *filename = dialog->getFileName();
  if (filename && *filename != '\0') {

    ((VolumeVizAuditor *)data)->userDataName = filename;

    SoDialogEditText *editText =
      (SoDialogEditText *)myTopLevelDialog->searchForAuditorId("datafilename");
    if (editText) {
      editText->editText = filename;
    }
    SoDialogComboBox *combo =
      (SoDialogComboBox *)myTopLevelDialog->searchForAuditorId("data");
    int numItems = combo->items.getNum();
    combo->selectedItem = numItems - 1; // User defined file should be last item

    // Not possible to programmatically trigger combobox,
    // so call work function as if it was triggered.
    // Note: Need full file path in order to allow opening a data file
    // in an arbitrary directory (not just the predefined directories).
    char *filepath = dialog->getFilePath();
    ((VolumeVizAuditor *)data)->doDataChange( numItems - 1, filepath );
  }
}

void
VolumeVizAuditor::settingsFileDialogCB(void *data, SoXtFileSelectionDialog *dialog)
{
  char *filename = dialog->getFileName();
  if (filename && *filename != '\0') {

    ((VolumeVizAuditor *)data)->userDataName = filename;

    SoDialogEditText *editText =
      (SoDialogEditText *)myTopLevelDialog->searchForAuditorId("settingsfilename");
    if (editText) {
      editText->editText = filename;
    }

    char *filepath = dialog->getFilePath();
    FILE* settingsFile = fopen(filepath, "w");

    if(SoPreferences::getValue("VOLREND_cameraType"))
      fprintf(settingsFile, "VOLREND_cameraType %s\n",
	      SoPreferences::getValue("VOLREND_cameraType"));

    if(SoPreferences::getValue("VOLREND_cavePanIncr"))
      fprintf(settingsFile, "VOLREND_cavePanIncr %s\n",
	      SoPreferences::getValue("VOLREND_cavePanIncr"));

    if(SoPreferences::getValue("VOLREND_caveRotIncr"))
      fprintf(settingsFile, "VOLREND_caveRotIncr %s\n",
	      SoPreferences::getValue("VOLREND_caveRotIncr"));

    if(SoPreferences::getValue("VOLREND_caveScenePos"))
      fprintf(settingsFile, "VOLREND_caveScenePos %s\n",
	      SoPreferences::getValue("VOLREND_caveScenePos"));

    if(SoPreferences::getValue("VOLREND_caveSceneSize"))
      fprintf(settingsFile, "VOLREND_caveSceneSize %s\n",
	      SoPreferences::getValue("VOLREND_caveSceneSize"));

    fprintf(settingsFile, "VOLREND_colorMap %d\n",
	    m_transferFunction->predefColorMap.getValue());

    if(!volumeUserColorMapName.isNull())
      fprintf(settingsFile, "VOLREND_colorMapFile %s\n",
	      volumeUserColorMapName.toLatin1());

    if(SoPreferences::getValue("VOLREND_dataLogo"))
      fprintf(settingsFile, "VOLREND_dataLogo %s\n",
	      SoPreferences::getValue("VOLREND_dataLogo"));

    fprintf(settingsFile, "VOLREND_dataOutline %d\n",
	    SoLDMGlobalResourceParameters::
	    getVisualFeedbackParam( SoLDMGlobalResourceParameters::
				    DRAW_TOPOLOGY));

    fprintf(settingsFile, "VOLREND_filename %s\n",
	    m_volData->fileName.getValue().toLatin1());

    if(SoPreferences::getValue("VOLREND_fontSize"))
      fprintf(settingsFile, "VOLREND_fontSize %s\n",
	      SoPreferences::getValue("VOLREND_fontSize"));

    fprintf(settingsFile, "VOLREND_globalAlpha %f\n",
	    1 - m_material->transparency[0]);

    fprintf(settingsFile, "VOLREND_mainMemSize %d\n",
	    m_volData->ldmResourceParameters.getValue()->
	    maxMainMemory.getValue());

    fprintf(settingsFile, "VOLREND_maxColorMap %d\n", m_maxColorMap);

    fprintf(settingsFile, "VOLREND_maxOpaqueMap %d\n", m_maxOpaqueMap);

    fprintf(settingsFile, "VOLREND_minColorMap %d\n", m_minColorMap);

    fprintf(settingsFile, "VOLREND_minOpaqueMap %d\n", m_minOpaqueMap);
    //TODO fprintf(settingsFile, "VOLREND_moveLowRes %d\n", 0);

    fprintf(settingsFile, "VOLREND_numOrthoSlice %d\n", m_lastNumSlices);
    //m_orthoTab[0]->sliceNumber.getValue();

    fprintf(settingsFile, "VOLREND_numSlices %d\n",
	    m_volRend->numSlices.getValue());

    fprintf(settingsFile, "VOLREND_numSlicesControl %d\n",
	    m_volRend->numSlicesControl.getValue());

    //TODO fprintf(settingsFile, "VOLREND_remoteDialog %d\n", 0);
    //TODO fprintf(settingsFile, "VOLREND_remoteDialog %d\n", 0);
    //TODO fprintf(settingsFile, "VOLREND_remoteDialog %d\n", 0);

    fprintf(settingsFile, "VOLREND_showAxes %d\n",
	    (m_volAxesSwitch->whichChild.getValue()==SO_SWITCH_ALL));

    fprintf(settingsFile, "VOLREND_showBBox %d\n",
	    (m_volBBoxSwitch->whichChild.getValue()==SO_SWITCH_ALL));

    fprintf(settingsFile, "VOLREND_showColormap %d\n",
	    (m_volColormapSwitch->whichChild.getValue()==SO_SWITCH_ALL) );

    fprintf(settingsFile, "VOLREND_showHistogram %d\n",
	    (m_volHistoSwitch->whichChild.getValue()==SO_SWITCH_ALL));

    fprintf(settingsFile, "VOLREND_showText %d\n",
	    (m_volTextSwitch->whichChild.getValue()==SO_SWITCH_ALL));

    fprintf(settingsFile, "VOLREND_sliceColorMap %d\n",
	    m_sliceTransferFunction->predefColorMap.getValue());

    if(!sliceUserColorMapName.isNull())
      fprintf(settingsFile, "VOLREND_sliceColorMapFile %s\n",
	      sliceUserColorMapName.toLatin1());

    fprintf(settingsFile, "VOLREND_sliceEqualRes %d\n",
	    SoLDMGlobalResourceParameters::getSliceEqualResolution());

    fprintf(settingsFile, "VOLREND_sliceNumTex %d\n",
	    m_volData->ldmResourceParameters.getValue()->
	    max2DTextures.getValue());

    fprintf(settingsFile, "VOLREND_sliceTexLoadRate %d\n",
	    m_volData->ldmResourceParameters.getValue()->
	    tex2LoadRate.getValue());

    fprintf(settingsFile, "VOLREND_texLoadRate %d\n",
	    m_volData->ldmResourceParameters.getValue()->
	    tex3LoadRate.getValue());

    fprintf(settingsFile, "VOLREND_texMemSize %d\n",
	    m_volData->ldmResourceParameters.getValue()->
	    maxTexMemory.getValue());

    fprintf(settingsFile, "VOLREND_tileOutline %d\n",
	    SoLDMGlobalResourceParameters::
	    getVisualFeedbackParam(SoLDMGlobalResourceParameters::
				   DRAW_TILE_OUTLINE));

    if(SoPreferences::getValue("VOLREND_useLogo"))
      fprintf(settingsFile, "VOLREND_useLogo %s\n",
	      SoPreferences::getValue("VOLREND_useLogo"));

    fprintf(settingsFile, "VOLREND_raycasting %d\n",
	    m_volQuality->raycasting.getValue());

    fprintf(settingsFile, "VOLREND_samplingAlignment %d\n",
	    m_volRend->samplingAlignment.getValue());

    fprintf(settingsFile, "VOLREND_viewCulling %d\n",
	    SoLDMGlobalResourceParameters::getViewCulling());

    fprintf(settingsFile, "VOLREND_viewPointRefine %d\n",
	    SoLDMGlobalResourceParameters::getViewpointRefinement());

    if(SoPreferences::getValue("VOLREND_waitTime")) //
      fprintf(settingsFile, "VOLREND_waitTime %s\n",
	      SoPreferences::getValue("VOLREND_waitTime"));

    fclose(settingsFile);
  }
}

/*----------------------------- Colormap change ---------------------------------------*/

void
VolumeVizAuditor::doCmapChange( int selectedItem, SoTransferFunction* transferFunc)
{
  int numEntries = m_colorMapSize;

  switch (selectedItem) {
    case 0:
      transferFunc->predefColorMap = SoTransferFunction::SEISMIC;
      break;
    case 1:
      transferFunc->predefColorMap = SoTransferFunction::STANDARD;
      break;
    case 2:
      transferFunc->predefColorMap = SoTransferFunction::GLOW;
      break;
    case 3:
      transferFunc->predefColorMap = SoTransferFunction::BLUE_RED;
      break;
    case 4:
      transferFunc->predefColorMap = SoTransferFunction::PHYSICS;
      break;
    case 5:
      transferFunc->predefColorMap = SoTransferFunction::TEMPERATURE;
      break;
    case 6:
      transferFunc->predefColorMap = SoTransferFunction::GREY;
      break;
    case 7:
      transferFunc->predefColorMap = SoTransferFunction::BLUE_WHITE_RED;
      break;
    case 8:
      transferFunc->predefColorMap = SoTransferFunction::INTENSITY;
      break;
    case 9:
      transferFunc->predefColorMap = SoTransferFunction::LABEL_256;
      break;
    case 10:
      userColorMapName = getDataFilePath("bonsai.txt");
      break;
    case 11:
      userColorMapName = getDataFilePath("Amplitude_BlueBrownRed_VolRend.am");
      break;
    case 12:
      transferFunc->predefColorMap = SoTransferFunction::VOLREN_GREEN;
      break;
    case 13:
      transferFunc->predefColorMap = SoTransferFunction::VOLREN_RED;
      break;
    case 14:
      transferFunc->predefColorMap = SoTransferFunction::AIRWAY;
      break;
    case 15:
      transferFunc->predefColorMap = SoTransferFunction::AIRWAY_SURFACES;
      break;
  }

  if ( selectedItem > 15  || selectedItem == 10 || selectedItem == 11 )
  {
    // Load user defined colormap
    const char* filename = userColorMapName.toLatin1();
    const char* empty = "";
    if(*filename == *empty)filename = "blueWhiteRed.txt";
    //delete [] m_userColorMap;
    int numComps = 0;

    numEntries = loadColorTable( filename, numComps, m_userColorMap );

    if (numEntries == 0)
      return;

    // Load values into transfer function node
    // Note: setNum() is required in case the new color map has
    // fewer values than the current one (we compute the length
    // of the colormap based on the number of values in this field).
    transferFunc->predefColorMap = SoTransferFunction::NONE;
    switch (numComps)
    {
    case 1: // Alpha
      transferFunc->colorMapType = SoTransferFunction::ALPHA;
      transferFunc->colorMap.setNum( numEntries );
      transferFunc->colorMap.setValues( 0, numEntries, m_userColorMap );
      break;
    case 2: // Luminance_alpha
      transferFunc->colorMapType = SoTransferFunction::LUM_ALPHA;
      transferFunc->colorMap.setNum( numEntries * 2 );
      transferFunc->colorMap.setValues( 0, numEntries * 2, m_userColorMap );
      break;
    case 4: // RGBA
      transferFunc->colorMapType = SoTransferFunction::RGBA;
      transferFunc->colorMap.setNum( numEntries * 4 );
      transferFunc->colorMap.setValues( 0, numEntries * 4, m_userColorMap );
      break;
    default:
      // Error, unsupported format
      break;
    }
  }

  int minColorMap, maxColorMap, minOpaqueMap, maxOpaqueMap, invertFlag;
  if (transferFunc == m_transferFunction) {
    minColorMap  = m_minColorMap;
    maxColorMap  = m_maxColorMap;
    minOpaqueMap = m_minOpaqueMap;
    maxOpaqueMap = m_maxOpaqueMap;
    invertFlag   = m_invertTransparency;
  }
  else if (transferFunc == m_sliceTransferFunction) {
    minColorMap  = m_minSliceColorMap;
    maxColorMap  = m_maxSliceColorMap;
    minOpaqueMap = m_minSliceOpaqueMap;
    maxOpaqueMap = m_maxSliceOpaqueMap;
    invertFlag   = m_invertSliceTransparency;
  }
  updateOpaqueRegion( transferFunc, minColorMap, maxColorMap, minOpaqueMap, maxOpaqueMap, invertFlag );
}

/*----------------------------- Data change ---------------------------------------*/

void
VolumeVizAuditor::doDataChange( int selectedItem, const SbString& droppedFile )
{
  SbString filename = droppedFile;
  if( filename.isEmpty())
  {
    if (selectedItem < numDataFiles-1)
      filename = fileNames[selectedItem];
    else
    {
      filename = userDataName.toLatin1();
      if( filename.isEmpty() )
        filename = "WahaPub.sgy";
    }
  }
  // Find a valid data path
  SbString filepath = getDataFilePath( filename );
  if ( filepath.isEmpty() )
  {
    char buf[1024];
    sprintf( buf, "Unable to locate data file '%s'.\n", filename.toLatin1() );
#if defined(_DEBUG)
    DISPLAY_ERROR( buf );
#endif
    return;
  }

  // Tell the user what we're doing
  SoText2 *pMsgText = (SoText2*)SoNode::getByName( VOL_VIEWER_INFO );
  if (pMsgText)
  {
    SbString msg = "Loading " + filepath;
    pMsgText->string.setValue( msg );
    int whichChild = m_VolRenderSwitch->whichChild.getValue();
    m_VolRenderSwitch->whichChild.setValue (SO_SWITCH_NONE);
    m_viewer->render();
    m_VolRenderSwitch->whichChild.setValue (whichChild);
  }

  m_newFile = TRUE;
  if( m_textSwitch )
    m_textSwitch->whichChild = SO_SWITCH_NONE;

  initDataNode(m_volData, filepath);

  // Get new volume dimensions and size
  SbVec3i32 dimension;
  SbDataType dataType;
  dimension = m_volData->data.getSize(); //getVolumeData( dimension, values, dataType );
  dataType = (SbDataType::DataType)m_volData->data.getDataType();
  int volWidth, volHeight, volDepth;
  dimension.getValue(volWidth, volHeight, volDepth);
  m_width  = volWidth;
  m_height = volHeight;
  m_depth  = volDepth;
  SbBox3f size;
  size = m_volData->extent.getValue();
  size.getBounds(m_minWidth, m_minHeight, m_minDepth, m_maxWidth, m_maxHeight, m_maxDepth);
  m_volDefaultSize = size; // Before any volume scaling

  //Scale the clipping volume according to the size of the volume data
  float sx, sy, sz;
  size.getSize(sx, sy, sz);
  float scaleFactor = (sx+sy+sz)/3.0f;
  m_volumeClippingTransform->scaleFactor = SbVec3f(scaleFactor/4, scaleFactor/4, scaleFactor/4);
  m_volumeClippingTransform->translation = SbVec3f(m_minWidth+sx/2, m_minHeight+sy/2, m_minDepth+sz/2);

 //Scale the grid clipping & update the manip/transform node & keep only one grid
  m_predefinedGrid.reserve(1);
  m_userGrid.reserve(1);
  SoSwitch *gridSwitch = (SoSwitch *)m_gridsGroup->getChild(0);
  SoUniformGridClipping *grid = (SoUniformGridClipping *)gridSwitch->getChild(2);
  grid->ref();
  grid->extent = SbBox3f(m_minWidth, m_minHeight, m_minDepth, m_maxWidth, m_minHeight+sy/2, m_maxDepth);
  m_gridsGroup->removeAllChildren(); //Keep the old grid
  addGridToSceneGraph(grid, 2);
  grid->unref();
  updateSelectGridMenu(1);
  m_selectedGrid = 0;

  //Update Oblique slice & dragger
  SbVec3f obliPos = size.getMin()+(size.getMax()-size.getMin())/2.0f;
  m_obliSlice->plane = SbPlane(SbVec3f(0, 0, 1), obliPos);
  m_draggerObliSlice->rotation = SbRotation(SbVec3f(0,1,0),m_draggerNormal);
  m_draggerObliSlice->scaleFactor = SbVec3f(scaleFactor*0.4f, scaleFactor*0.4f, scaleFactor*0.4f);
  m_draggerObliSlice->translation.setValue(obliPos);

  //Update gridDraggerVisibility check box
  SoDialogCheckBox* dragVis = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(SbString("gridDraggerVisibility"));
  dragVis->state = TRUE;
  SoDialogCheckBox* gridVis = (SoDialogCheckBox*)myTopLevelDialog->searchForAuditorId(SbString("gridVisibility"));
  gridVis->state = TRUE;

  //grid->size = SbBox3f(m_minWidth, m_minHeight, m_minDepth, m_maxWidth, m_minHeight+sy/2, m_maxDepth);
//SpaceBall
#ifdef SPACEBALL
if ( m_sb_available) {
  float step = ((m_maxWidth - m_minWidth) + (m_maxHeight - m_minHeight) + (m_maxDepth - m_minDepth))/3;
  m_spaceball->setRotationScaleFactor( SB_STEP * 2 );
  m_spaceball->setTranslationScaleFactor((SB_STEP * (step / 2)));
}
#endif

  // Update OrthoSlice draggers
  if (m_switchForOrthoDragger!=NULL)
  {
    for (int i=0; i<m_switchForOrthoDragger->getNumChildren();i++)
    {
      SoOrthoSliceDragger* sliceDragger = dynamic_cast<SoOrthoSliceDragger*>(m_switchForOrthoDragger->getChild(i));
      if ( sliceDragger != NULL)
      {
        sliceDragger->volumeExtent.setValue(m_volData->extent.getValue());
        sliceDragger->volumeDimension.setValue(m_volData->data.getSize());
      }
    }
  }

  //    m_XOrthoSlice->sliceNumber.setValue((int)m_width/2);
  //    m_YOrthoSlice->sliceNumber.setValue((int)m_height/2);
  //    m_ZOrthoSlice->sliceNumber.setValue((int)m_depth/2);

  // Get min and max values in data
  double min=0, max=255;
  if (m_showHistogram)
  {
    m_volData->getMinMax(min,max);
  }

  // Update color map
  m_minColorMap = 0;
  m_maxColorMap = 255;
  m_minOpaqueMap = m_minColorMap;
  m_maxOpaqueMap = m_maxColorMap;
  m_transferFunction->minValue = m_minColorMap;
  m_transferFunction->maxValue = m_maxColorMap;
  //and update dialog for colormap
  SoDialogIntegerSlider* colorMapMax = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("colormap max"));
  colorMapMax->max.setValue(255);
  colorMapMax->min.setValue(0);
  colorMapMax->value.setValue(255);
  SoDialogIntegerSlider* colorMapMin = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("colormap min"));
  colorMapMin->max.setValue(255);
  colorMapMin->min.setValue(0);
  colorMapMin->value.setValue(0);
  SoDialogIntegerSlider* opaqueMax = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("opaque max"));
  opaqueMax->max.setValue(255);
  opaqueMax->min.setValue(0);
  opaqueMax->value.setValue(255);
  SoDialogIntegerSlider* opaqueMin = (SoDialogIntegerSlider*)myTopLevelDialog->searchForAuditorId(SbString("opaque min"));
  opaqueMin->max.setValue(255);
  opaqueMin->min.setValue(0);
  opaqueMin->value.setValue(0);

  // update dialog for data range
  SoDialogRealSlider* dataRangeMin = (SoDialogRealSlider*)myTopLevelDialog->searchForAuditorId(SbString("datarangemin"));
  dataRangeMin->min.setValue((float)min);
  dataRangeMin->max.setValue((float)max);
  dataRangeMin->value.setValue((float)min);
  SoDialogRealSlider* dataRangeMax = (SoDialogRealSlider*)myTopLevelDialog->searchForAuditorId(SbString("datarangemax"));
  dataRangeMax->min.setValue((float)min);
  dataRangeMax->max.setValue((float)max);
  dataRangeMax->value.setValue((float)max);
  m_dataRange->min = min;
  m_dataRange->max = max;

  // Reset subvolume and ROI boxes
  // Arbitrarily set the subvolume to 1/4 the volume so effect is obvious
  SbVec3i32 volMax(m_width-1,m_height-1,m_depth-1);
  SbVec3i32 halfVol = volMax / 2;
  m_ROIManip->box.setValue( SbVec3i32(0,0,0), halfVol );
  m_ROIManip->subVolume.setValue( SbVec3i32(0,0,0), volMax );
  ROIChangedCB( (void*)m_ROIManip, NULL );

  //m_volROISwitch->whichChild = SO_SWITCH_ALL;


  #ifdef ROI_TAB_BOX
  ((SoTabBoxDragger*)m_ROIManip->getDragger())->adjustScaleTabSize();
  #endif

  // Re-create bbox, axes, legend, etc.
  updateVolBBoxGeometry();
  updateVolAxesGeometry();
  createHistoLegend( m_volHistoSwitch );

  SoDialogRealSlider* isoVal = (SoDialogRealSlider*)myTopLevelDialog->searchForAuditorId(SbString("isovalue"));
  isoVal->max = (float)max;
  isoVal->min = (float)min;
  isoVal->value = (float)m_posMaxValueHisto;
  int numIso = m_volIsosurface->isovalues.getNum();
  for(int i = 0; i < numIso; i++)
    m_volIsosurface->isovalues.set1Value(i, (float)m_posMaxValueHisto);

  // Update the volume info annotation text
  if (pMsgText) {
  int numBytes = m_volData->getDataSize();

  float totbytes = (float)(volWidth/1024.f) * (float)volHeight * (float)volDepth * (float)numBytes;
  int64_t totalBytes = int64_t(totbytes);
  const char* dataTypeStr = 0;
  if (!dataType.isInteger()) {
    dataTypeStr = "float";
    isoVal->max = 256; // VViz only support 8bits texture
    isoVal->value = (float)m_posMaxValueHisto;
  } else if (dataType.isSigned())
    dataTypeStr = "signed integer";
  else
    dataTypeStr = "unsigned integer";
  sprintf( buffer,
  "Volume: %d x %d x %d x %d bits %s = %10.1f MB",
  volWidth, volHeight, volDepth, numBytes*8, dataTypeStr,  (float)totalBytes/1024. );
  pMsgText->string.setValue(buffer);
  }

  //there is some important stuff like slicing in here
  //initDialog();
  SbVec3i32 mydimension = m_volData->data.getSize();

  //TEX
  int m_tileSize = 64;

  // compute level max
  int maxDim = mydimension[0];
  int levelMax = 0;
  if (mydimension[1] > maxDim) maxDim = mydimension[1];
  if (mydimension[2] > maxDim) maxDim = mydimension[2];
  for (levelMax = 0; (1<<levelMax)*m_tileSize < maxDim; levelMax++) ;
  if(m_volData->getReader() )
  {
    SoDialogComboBox* minres =
      (SoDialogComboBox*)myTopLevelDialog->searchForAuditorId(SbString("resthreshold"));
    minres->selectedItem = levelMax-1;
  }
  m_levelMax = levelMax;

  m_viewer->resetToHomePosition();

#ifdef SPACEBALL
  if ( m_sb_available) {
    //reset the rotation and the translation
    m_sb_struct->sb_struct_trans->translation.setValue(SbVec3f(0.0,0.0,0.0));
    m_sb_struct->sb_struct_rot->rotation.setValue(SbRotation(0.0,0.0,0.0,1.0));
    //center the object if its bounding box is not in the middle of the world
    m_correct_trans->translation.setValue(m_volData->extent.getValue().getCenter());
    m_correct_trans_back->translation.setValue((-1)*(m_volData->extent.getValue().getCenter()));
  }
#endif

  m_viewer->viewAll();

#ifdef SPACEBALL
  if (m_sb_available) {
    m_viewer->saveHomePosition();
  }
#endif

  // Update dragger position used for camera tracking
  if (m_ROIManipNode) {
    SoTabBoxDragger *pDragger = (SoTabBoxDragger *)m_ROIManip->getDragger();
    m_ROIcenter = pDragger->translation.getValue();
  }
}

#endif // CAVELIB


