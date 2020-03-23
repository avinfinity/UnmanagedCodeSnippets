#ifndef _SeekAnimator_
#define _SeekAnimator_

#if defined(_WIN32)
#if _DEBUG
#pragma comment(lib,"ViewerComponentsD")
#else
#pragma comment(lib,"ViewerComponents")
#endif
#endif

#include <Inventor/ViewerComponents/SoCameraInteractor.h>
#include <Inventor/SbViewportRegion.h>
#include "Animator.h"

/**
* Seek animation.
*
* This class allows animating a camera by moving it to a desired seek point.
* This point is specified by the #setUp function.
* Duration of the animation and relative distance from the viewpoint to the
* specified point can be adjusted.
*/
class SeekAnimator : public Animator
{

public:

  SeekAnimator( SoCameraInteractor* cameraInteractor, SoNode* sceneGraph );

  ~SeekAnimator();

  virtual void start();

  /**
  * Set the camera interactor.
  */
  void setCameraInteractor( SoCameraInteractor* cameraInteractor );

  /**
  * Set the scene graph.
  */
  void setSceneGraph( SoNode* sceneGraph );

  /**
  * Get the duration of the seek animation in seconds (default is 2).
  */
  float getDuration() const;

  /**
  * Set the duration of the seek animation in seconds (default is 2).
  */
  void setDuration( float duration );

  /**
  * Get the relative distance from the viewpoint to the seek point (default is 50%).
  */
  float getRelativeDistance() const;

  /**
  * Set the relative distance from the viewpoint to the seek point.
  * The given value must be in range [0, 100%] (default is 50%).
  */
  void setRelativeDistance( float distance );

  /**
  * Set up the seek animation.
  *
  * Given the seek point, this routine will compute the new camera's position
  * and orientation. Then call #start() to start the animation.
  */
  void setUp( const SbVec3f& seekPoint, const SbViewportRegion& vpRegion );

protected:

  virtual void animate();

private:
  SoCameraInteractor* m_cameraInteractor;
  SoNode* m_sceneGraph;
  float m_duration;
  float m_relativeDistance;

  SbViewportRegion m_vpRegion;
  SbVec3f m_saveCameraPosition;
  SbRotation m_saveCameraOrientation;
  SbRotation m_newCameraOrientation;
  SbVec3f m_newCameraPosition;
  SbTime m_startTime;

  void interpolateSeekAnimation( float t );

};
#endif // _SeekAnimator_
