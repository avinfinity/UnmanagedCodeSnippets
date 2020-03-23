#ifndef _SceneExaminer_
#define _SceneExaminer_

#if defined(_WIN32)
#if _DEBUG
#pragma comment(lib,"ViewerComponentsD")
#else
#pragma comment(lib,"ViewerComponents")
#endif
#endif

#include "SceneInteractor.h"
#include "SeekAnimator.h"

#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/events/SoLocation2Event.h>

class SoMouseWheelEvent;
class SoKeyboardEvent;
class SoLocation2Event;
class SoScaleGestureEvent;
class SoEvent;
class SoTouchEvent;
class SoRotateGestureEvent;

/**
* Tool class for easily building a basic interactive OpenInventor application 
* without using existing viewer classes.
*
* The SceneExaminer is a simple extension of the SoSeparator node that allows
* handling of Open Inventor events and interaction with a camera similar to
* the behavior of the classic Open Inventor viewer class SoXtExaminerViewer.
*
* The SceneExaminer is not comparable with a classic OpenInventor viewer as 
* it does not provide any GUI (no buttons, no popup menu) and fewer interactive
* features (no animation, no seek-to-point). However it does provide a touch 
* event handler that allows manipulating a scene on a touch device.
*
* The SceneExaminer uses an instance of SoCameraInteractor 
* to manipulate the camera in response to OpenInventor events.
*
* Notes:
*
* - The SceneExaminer needs a component that builds OpenInventor events (SoEvent) 
*   from native system events. Such behavior is provided by the SoEventBuilder class.
*
* - A basic version of SceneExaminer is a supported part of the Open Inventor API,
*   but the prebuilt \if_java jar \else library \endif is located in the example folders,
*   not in the usual folder.
*
* - The basic version of SceneExaminer is also provided as source code in the example folders
*   to allow applications to customize and build their own interactive tool class. @BR
*  \if_cpp See $OIVHOME/src/Inventor/gui/ViewerComponents \endif
*  \if_dotnet See $OIVHOME/src/demos/Inventor/Gui/ViewerComponents \endif
*  \if_java See $OIVHOME/examples/sources/inventor/gui/awt/viewercomponents \endif
*
* @USAGE
*
* - With a mouse:
*    - @B Left Mouse: @b Rotate the scene or seek to point if seek mode is activated.
*    - @B Left Mouse + Ctrl or Middle Mouse: @b Pan the scene.
*    - @B Left Mouse + Ctrl + Shift: @b Zoom in/out.
*    - @B Mouse Wheel: @b Zoom in/out (zoom center is the mouse cursor location).
*    - @B Escape key: @b Enable/Disable picking mode.
*    - @B S key: @b Activate/Deactivate seek mode.
*    .
* @BR
*
* - With a touchscreen:
*    - @B 1 finger: @b Rotate the scene.
*    - @B 2 fingers: @b Rotate the scene on the screen plane, zoom in/out and pan (rotation and zoom center are located between the two fingers).
*    - @B Double tap: @b View all the scene.
*
* \htmlonly </UL> \endhtmlonly
*
* @SEE_ALSO
*   SceneInteractor, SoCameraInteractor
*/
class SceneExaminer : public SceneInteractor
{

public:
  
  /**
  * Interaction Mode (viewing or picking)
  */
  enum InteractionMode
  {
    VIEWING,
    PICKING
  };

  /** Constructor */
  SceneExaminer();

  /** Destructor */
  virtual ~SceneExaminer();

  /**
  * Enable or disable picking mode. Default is true.
  */
  void enablePicking(bool enabled);

  /**
  * Returns if picking is enabled.
  */
  bool isPickingEnabled();

  /**
  * Enable or disable zoom. Default is true.
  */
  void enableZoom(bool enabled);

  /**
  * Returns if zoom is enabled.
  */
  bool isZoomEnabled();

  /**
  * Enable or disable camera panning. Default is true.
  */
  void enablePan(bool enabled);

  /**
  * Returns if camera panning is enabled.
  */
  bool isPanEnabled();

  /**
  * Enable or disable camera orbiting. Default is true.
  */
  void enableOrbit(bool enabled);

  /**
  * Returns if camera orbiting is enabled.
  */
  bool isOrbitEnabled();

  /**
  * Enable or disable camera rotation. Default is true.
  */
  void enableRotate(bool enabled);

  /**
  * Returns if camera rotation is enabled.
  */
  bool isRotateEnabled();

  /**
  * Set interaction mode to viewing or picking. Default is VIEWING.
  */
  void setInteractionMode(SceneExaminer::InteractionMode mode);

  /**
  * Returns the current interaction mode.
  */
  SceneExaminer::InteractionMode getInteractionMode();

  virtual void setCameraMode( SceneInteractor::CameraMode mode );

  /**
  * Set the interaction into or out off seek mode (default is off).
  */
  void setSeekMode( bool onOrOff );

protected:
  virtual void mouseWheelMoved( SoMouseWheelEvent* wheelEvent, SoHandleEventAction* action );
  virtual void mouseMoved( SoLocation2Event* mouseEvent, SoHandleEventAction* action );
  virtual void mousePressed( SoMouseButtonEvent* mouseEvent, SoHandleEventAction* action );
  virtual void mouseReleased( SoMouseButtonEvent* mouseEvent, SoHandleEventAction* action );
  virtual void keyPressed( SoKeyboardEvent* keyEvent, SoHandleEventAction* action );
  virtual void keyReleased( SoKeyboardEvent* keyEvent, SoHandleEventAction* action );
  virtual void touch( SoTouchEvent* touchEvent, SoHandleEventAction* action );
  virtual void zoom( SoScaleGestureEvent* scaleEvent, SoHandleEventAction* action );
  virtual void rotate( SoRotateGestureEvent* rotateEvent, SoHandleEventAction* action );

private:

  bool m_isPickingEnabled;
  bool m_isZoomEnabled;
  bool m_isPanEnabled;
  bool m_isOrbitEnabled;
  bool m_isRotateEnabled;
  bool m_isButton1Down;
  bool m_isButton2Down;
  bool m_isTouchOrbitActivated;
  InteractionMode m_activeMode;
  SbVec2f m_mousePositionNorm;
  int m_mouseWheelDelta;

  bool m_isSeekMode;
  SeekAnimator* m_seekAnimator;

  SoMouseButtonEvent m_touchMouseButtonEvent;
  SoLocation2Event m_touchLocation2Event;

  // given an SoTouchEvent convert it into MouseButton or Location2Event
  SoEvent* convertTouchEvent( SoTouchEvent* touchEvent );

  void seekToPoint( const SbVec2f& mouseNormPosition, const SbViewportRegion& vpRegion);

  class SeekAnimatorListener : public AnimatorListener
  {
  public:
    SeekAnimatorListener( SceneExaminer* sceneExaminer )
      : m_sceneExaminer( sceneExaminer)
    {}
    ~SeekAnimatorListener(){}

    virtual void animationStarted(){}

    virtual void animationStopped(){ m_sceneExaminer->setSeekMode( false ); }

  private:
    SceneExaminer* m_sceneExaminer;
  };
};

#endif // _SceneExaminer_
