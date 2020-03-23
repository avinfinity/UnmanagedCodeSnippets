
#ifndef _ALL_HEADERS_
#define _ALL_HEADERS_

//header files
#include <math.h>   //for floor()
#define PI 3.1415926535
//#define SPACEBALL
#ifndef WIN32
#include <unistd.h>
#endif

#ifdef SPACEBALL
//SpaceBall
#include <Inventor/Xt/devices/SoXtSpaceball.h>
#include <Inventor/events/SoSpaceballButtonEvent.h>
#include <Inventor/events/SoMotion3Event.h>
#ifdef _WIN32
#  define SoXtSpaceball SoWinSpaceball
#endif
#endif //spaceball

//INVENTOR
#ifndef CAVELIB
#include <Inventor/Xt/SoXtDirectionalLightEditor.h>
#include <Inventor/Xt/SoXtMaterialEditor.h>
#include <Inventor/Xt/SoXtFileSelectionDialog.h>
#include <Inventor/Xt/viewers/SoXtExaminerViewer.h>
#endif

#include <Inventor/nodes/SoTextureUnit.h>
#include <Inventor/sensors/SoAlarmSensor.h>
#include <Inventor/manips/SoTransformerManip.h>
#include <Inventor/nodes/SoResetTransform.h>
#include <Inventor/sensors/SoTimerSensor.h>
#include <Inventor/nodes/SoGradientBackground.h>
#include <Inventor/nodes/SoImageBackground.h>
#include <Inventor/draggers/SoTabBoxDragger.h>
#include <Inventor/draggers/SoJackDragger.h>
#include <Inventor/draggers/SoHandleBoxDragger.h>
#include <Inventor/draggers/SoTransformerDragger.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoCallback.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/draggers/SoJackDragger.h>
#include <Inventor/manips/SoJackManip.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/nodes/SoClipPlane.h>
#include <Inventor/nodes/SoAnnotation.h>
#include <Inventor/nodes/SoFont.h>
#include <Inventor/nodes/SoLightModel.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoText2.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoComplexity.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/engines/SoTimeCounter.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/nodes/SoPickStyle.h>
#include <Inventor/nodes/SoVertexProperty.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoIndexedLineSet.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/nodes/SoPolygonOffset.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/nodes/SoShadowGroup.h>
#include <Inventor/nodes/SoInteractiveComplexity.h>
#include <Inventor/sensors/SoNodeSensor.h>
#include <Inventor/sensors/SoOneShotSensor.h>
#include <Inventor/nodes/SoSphere.h>
#include <Inventor/nodes/SoCylinder.h>
#include <Inventor/nodes/SoCone.h>
#include <Inventor/nodes/SoRotation.h>
#include <Inventor/nodes/SoScale.h>
#include <Inventor/nodes/SoAntiSquish.h>
#include <Inventor/threads/SbThreadMutex.h>
#include <Inventor/lock/SoLockMgr.h>
#include <Inventor/elements/SoTextureQualityElement.h>
#include <Inventor/nodes/SoEventCallback.h>
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/events/SoLocation2Event.h>
#include <Inventor/events/SoKeyboardEvent.h>
#include <Inventor/SoPickedPoint.h>
#include <VolumeViz/details/SoVolumeRenderDetail.h>
#include <VolumeViz/details/SoOrthoSliceDetail.h>
#include <VolumeViz/details/SoObliqueSliceDetail.h>
#include <Inventor/nodes/SoPackedColor.h>
#ifdef WIN32
#include <Inventor/Win/SoConsole.h>
#endif
#include <Inventor/STL/vector>

//VOLUMEVIZ
#include <VolumeViz/nodes/SoVolumeGroup.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/nodes/SoVolumeRender.h>
#include <LDM/nodes/SoTransferFunction.h>
#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/nodes/SoVolumeRender.h>
#include <VolumeViz/nodes/SoVolumeSkin.h>
#include <LDM/nodes/SoROI.h>
#include <LDM/nodes/SoTransferFunction.h>
#include <VolumeViz/nodes/SoOrthoSlice.h>
#include <VolumeViz/nodes/SoObliqueSlice.h>
#include <VolumeViz/nodes/SoVolumeRenderingQuality.h>
#include <VolumeViz/nodes/SoVolumeIsosurface.h>
#include <VolumeViz/nodes/SoVolumeClippingGroup.h>
#include <VolumeViz/nodes/SoUniformGridClipping.h>
#include <LDM/nodes/SoDataRange.h>
#include <VolumeViz/readers/SoVRGenericFileReader.h>
#include <VolumeViz/readers/SoVRSegyFileReader.h>
#include <VolumeViz/draggers/SoOrthoSliceDragger.h>
#include <LDM/nodes/SoGeometryPriority.h> 
#include <LDM/SoLDMProximityVisitor.h>
#include <LDM/manips/SoROIManip.h>

#ifndef CAVELIB
//DIALOGVIZ
#include <DialogViz/SoDialogVizAll.h>
#endif

//...
#include "VVizBenchmark.h"

struct OrthoSlicesData {
  bool animEnabled;
  SoTimerSensor *timerSensor;
  float dir;
  int sliceNumber;
  int fps;
};

#endif


