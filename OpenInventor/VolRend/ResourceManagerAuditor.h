#ifndef CAVELIB

//header files
#include <DialogViz/SoDialogVizAll.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <LDM/SoLDMNodeFrontManager.h>
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class ResourceManagerAuditor : public SoDialogAuditor
{
  void dialogIntegerSlider(SoDialogIntegerSlider* cpt);
  void dialogCheckBox(SoDialogCheckBox* cpt);
  void dialogComboBox(SoDialogComboBox* cpt);

public:
  void setVolumeData(SoVolumeData *volumeData);
  ResourceManagerAuditor(SoTopLevelDialog* topDialog){m_top = topDialog;}

private:
  SoVolumeData *m_volumeData;
  SoTopLevelDialog* m_top;

  //void updateDialog();
};

void
ResourceManagerAuditor::setVolumeData(SoVolumeData *volumeData)
{
  m_volumeData = volumeData;
}

void
ResourceManagerAuditor::dialogComboBox(SoDialogComboBox* cpt)
{
  int selectedItem = cpt->selectedItem.getValue();

  if(cpt->auditorID.getValue() == "requestedres")
  {
    m_fixedRes = selectedItem;
    if(m_volumeData->ldmResourceParameters.getValue()->fixedResolution.getValue())
      m_volumeData->ldmResourceParameters.getValue()->enableFixedResolutionMode(m_fixedRes,fixedResCB);
    m_volumeData->touch();
  }

  if (cpt->auditorID.getValue() == "resthreshold")
    m_volumeData->ldmResourceParameters.getValue()->minResolutionThreshold = selectedItem;

  if (cpt->auditorID.getValue() == "maxresthreshold")
    m_volumeData->ldmResourceParameters.getValue()->maxResolutionThreshold = selectedItem;

  if (cpt->auditorID.getValue() == "loadingpolicy")
  {
    switch(selectedItem)
    {
    case 0 :
      m_volumeData->ldmResourceParameters.getValue()->loadPolicy = SoVolumeData::LDMResourceParameter::NO_USER_INTERACTION;
      break;
    case 1 :
      m_volumeData->ldmResourceParameters.getValue()->loadPolicy = SoVolumeData::LDMResourceParameter::ALWAYS;
      break;
    case 2 :
      m_volumeData->ldmResourceParameters.getValue()->loadPolicy = SoVolumeData::LDMResourceParameter::NEVER;
      break;
    default:
      break;
    }
  }
}

void
ResourceManagerAuditor::dialogIntegerSlider(SoDialogIntegerSlider* cpt)
{
  // main memory control parameters
 int value = cpt->value.getValue();

  if (cpt->auditorID.getValue() == "numIO") {
    SoLDMGlobalResourceParameters::setNumIO( value );
  }

  if (cpt->auditorID.getValue() == "notifyrate") {
	  m_volumeData->ldmResourceParameters.getValue()->loadNotificationRate = value;
  }

  else if (cpt->auditorID.getValue() == "mainmemorymb")
  {
    m_volumeData->ldmResourceParameters.getValue()->maxMainMemory = value;
    //make it reevaluate if drawing feedback:
    if(SoLDMGlobalResourceParameters::getVisualFeedbackParam(SoLDMGlobalResourceParameters::DRAW_TOPOLOGY))
      m_volumeData->getLdmManagerAccess().getNodeFrontManager()->geomChangeNotify(TRUE);
  }

  // texture 3 memory control parameters
  else if (cpt->auditorID.getValue() == "numtexturesmb" && value > 0)
    m_volumeData->ldmResourceParameters.getValue()->maxTexMemory = value;

  else if (cpt->auditorID.getValue() == "textureloadrate")
    m_volumeData->ldmResourceParameters.getValue()->tex3LoadRate = value;

  else if (cpt->auditorID.getValue() == "slicetextureloadrate")
    m_volumeData->ldmResourceParameters.getValue()->tex2LoadRate = value;
  else
    return;
}

void
ResourceManagerAuditor::dialogCheckBox(SoDialogCheckBox* cpt)
{
  SbBool state = cpt->state.getValue();

  if (cpt->auditorID.getValue() == "fixedresmode")
  {
    if (state)
      m_volumeData->ldmResourceParameters.getValue()->enableFixedResolutionMode(m_fixedRes,fixedResCB);
    else
      m_volumeData->ldmResourceParameters.getValue()->fixedResolution = FALSE;
  }

   if (cpt->auditorID.getValue() == "octree")
    SoLDMGlobalResourceParameters::setVisualFeedbackParam( SoLDMGlobalResourceParameters::SHOW_TILES_VALUATED, state );
   else if (cpt->auditorID.getValue() == "rayentry")
     SoPreferences::setBool("IVVR_SHOW_RAY_ENTRY",state);
   else if (cpt->auditorID.getValue() == "rayexit")
     SoPreferences::setBool("IVVR_SHOW_RAY_EXIT",state);
  else if (cpt->auditorID.getValue() == "drawtiles")
    SoLDMGlobalResourceParameters::setVisualFeedbackParam( SoLDMGlobalResourceParameters::DRAW_TILES, state );

  else if (cpt->auditorID.getValue() == "slicestex")
    SoLDMGlobalResourceParameters::setVisualFeedbackParam( SoLDMGlobalResourceParameters::DRAW_SLICES_TEX, state );

  else if (cpt->auditorID.getValue() == "tileoutline")
    SoLDMGlobalResourceParameters::setVisualFeedbackParam( SoLDMGlobalResourceParameters::DRAW_TILE_OUTLINE, state );

  else if (cpt->auditorID.getValue() == "dataoutline")
    SoLDMGlobalResourceParameters::setVisualFeedbackParam( SoLDMGlobalResourceParameters::DRAW_TOPOLOGY, state );

  else if (cpt->auditorID.getValue() == "loadunloadtiles")
    SoLDMGlobalResourceParameters::setVisualFeedbackParam( SoLDMGlobalResourceParameters::SHOW_LOAD_UNLOAD_TILES, state );

  else if (cpt->auditorID.getValue() == "fakedata")
    SoLDMGlobalResourceParameters::setVisualFeedbackParam( SoLDMGlobalResourceParameters::USE_FAKE_DATA, state );


  else if (cpt->auditorID.getValue() == "viewculling")
    SoLDMGlobalResourceParameters::setViewCulling( state );

  else if (cpt->auditorID.getValue() == "screenres")
    SoLDMGlobalResourceParameters::setScreenResolutionCulling( state );

  else if (cpt->auditorID.getValue() == "viewpointrefine")
    SoLDMGlobalResourceParameters::setViewpointRefinement( state );

  else if (cpt->auditorID.getValue() == "sliceequalresolution")
    SoLDMGlobalResourceParameters::setSliceEqualResolution( state );

  else if (cpt->auditorID.getValue() == "ignoreFullyTransparency")
    SoLDMGlobalResourceParameters::setIgnoreFullyTransparentTiles( state );

  else if (cpt->auditorID.getValue() == "proximityVisitor")
  {
    if(state)
    {
      m_volData->getLdmManagerAccess().setTileVisitor(m_proxiVisitor);
      m_proxiVisitor->valuationChangeNotify();
    }
    else{
      m_volData->getLdmManagerAccess().setTileVisitor(m_tileVisitor);
      m_tileVisitor->valuationChangeNotify();
    }
  }

  // Main CPU memory GUI management
  else if (cpt->auditorID.getValue() == "mainmemparamauto" )
  {
    // Checkbox activated means LDM in automatic mode for CPU memory sharing
    // Disable sliders
    SoDialogIntegerSlider* slider = (SoDialogIntegerSlider*)m_top->searchForAuditorId("mainmemorymb");
    if  ( state )
    {  
      slider->enable = false;
      // -1 means the resources is managed automatically by LDM
      slider->value = 0;
      m_volData->ldmResourceParameters.getValue()->maxMainMemory = -1;
    }
    else
    {
      slider->enable = true;
      slider->value = SoLDMGlobalResourceParameters::getMaxMainMemory();
    }

    slider = (SoDialogIntegerSlider*)m_top->searchForAuditorId("notifyrate");
    if  ( state )
    {  
      slider->enable = false;
      // -1 means the resources is managed automatically by LDM
      slider->value = 0;
      m_volData->ldmResourceParameters.getValue()->loadNotificationRate = -1;
    }
    else
    {
      slider->enable = true;
      slider->value = m_volData->getResourceManager().getLoadNotificationRate();
    }
  }

  // GPU 3D GUI management
  else if (cpt->auditorID.getValue() == "texmemparamauto" )
  {
    // Checkbox activated means LDM in automatic mode for CPU memory sharing
    // Disable sliders
    SoDialogIntegerSlider* slider = (SoDialogIntegerSlider*)m_top->searchForAuditorId("numtexturesmb");
    if  ( state )
    {  
      slider->enable = false;
      // -1 means the resources is managed automatically by LDM
      slider->value = 1;
      m_volData->ldmResourceParameters.getValue()->maxTexMemory = -1;
    }
    else
    {
      slider->enable = true;
      slider->value = SoLDMGlobalResourceParameters::getMaxTexMemory();
    }

    slider = (SoDialogIntegerSlider*)m_top->searchForAuditorId("textureloadrate");
    if  ( state )
    {  
      slider->enable = false;
      // -1 means the resources is managed automatically by LDM
      slider->value = 0;
      m_volData->ldmResourceParameters.getValue()->tex3LoadRate = -1;
    }
    else
    {
      slider->enable = true;
      slider->value = m_volData->ldmResourceParameters.getValue()->tex3LoadRate;
    }
  }

  // GPU 2D GUI management
  else if (cpt->auditorID.getValue() == "2dtexmemparamauto" )
  {
    // Checkbox activated means LDM in automatic mode for CPU memory sharing
    // Disable sliders
    SoDialogIntegerSlider* slider = (SoDialogIntegerSlider*)m_top->searchForAuditorId("slicetextureloadrate");
    if  ( state )
    {  
      slider->enable = false;
      // -1 means the resources is managed automatically by LDM
      slider->value = 0;
      m_volData->ldmResourceParameters.getValue()->tex2LoadRate = -1;
    }
    else
    {
      slider->enable = true;
      slider->value = m_volData->ldmResourceParameters.getValue()->tex2LoadRate;
    }
  }

  //make it redraw
  m_volumeData->touch();
}

/*
void
ResourceManagerAuditor::updateDialog()
{
#define DISUPDATE(auditorid, rsfield) {\
  int value = m_volumeData->rsfield.getValue();\
  SoDialogIntegerSlider* dis = (SoDialogIntegerSlider*)m_top->searchForAuditorId(SbString(auditorid));\
  if (dis)\
    dis->value = value;\
  }

  DISUPDATE( "mainmemorymb", ldmResourceParameters.getValue()->maxMainMemory     );

  DISUPDATE( "numtexturesmb"  , ldmResourceParameters.getValue()->maxTexMemory     );
  DISUPDATE( "textureloadrate", ldmResourceParameters.getValue()->tex3LoadRate     );

  DISUPDATE( "slicenumtextures"    , ldmResourceParameters.getValue()->max2DTextures );
  DISUPDATE( "slicetextureloadrate", ldmResourceParameters.getValue()->tex2LoadRate  );
}
*/
#endif //CAVELIB


