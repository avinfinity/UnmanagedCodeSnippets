#include <Inventor/nodes/SoCamera.h>
#include "SeekAnimator.h"

SeekAnimator::SeekAnimator( SoCameraInteractor* cameraInteractor, SoNode* sceneGraph )
  : Animator()
  , m_cameraInteractor( cameraInteractor )
  , m_sceneGraph( sceneGraph )
  , m_duration( 2 )
  , m_relativeDistance( 50 )
{
}

SeekAnimator::~SeekAnimator()
{
}
void
SeekAnimator::setCameraInteractor( SoCameraInteractor* cameraInteractor )
{
  m_cameraInteractor = cameraInteractor;
}

void
SeekAnimator::setSceneGraph( SoNode* sceneGraph )
{
  m_sceneGraph = sceneGraph;
}

float
SeekAnimator::getDuration() const
{
  return m_duration;
}

void
SeekAnimator::setDuration( float duration )
{
  m_duration = duration;
}

float
SeekAnimator::getRelativeDistance() const
{
  return m_relativeDistance;
}

void
SeekAnimator::setRelativeDistance( float distance )
{
  m_relativeDistance = distance;
}

void
SeekAnimator::setUp( const SbVec3f& seekPoint, const SbViewportRegion& vpRegion )
{
  m_vpRegion = vpRegion;

  // compute new camera position/orientation
  SbMatrix mx;
  SbVec3f viewVector;

  // save camera starting point
  SoCamera* camera = m_cameraInteractor->getCamera();
  m_saveCameraPosition = camera->position.getValue();
  m_saveCameraOrientation = camera->orientation.getValue();

  // compute the distance the camera will be from the seek point
  // and update the camera focal distance
  SbVec3f seekVec( seekPoint - camera->position.getValue() );
  float distance = seekVec.length() * ( m_relativeDistance / 100.0f );
  camera->focalDistance = distance;

  // get the camera new orientation
  mx = camera->orientation.getValue();
  viewVector.setValue( -mx[2][0], -mx[2][1], -mx[2][2] );
  SbRotation changeOrient;
  changeOrient.setValue( viewVector, seekPoint - camera->position.getValue() );
  m_newCameraOrientation = camera->orientation.getValue() * changeOrient;

  // find the camera final position based on orientation and distance
  mx = m_newCameraOrientation;
  viewVector.setValue( -mx[2][0], -mx[2][1], -mx[2][2] );
  m_newCameraPosition = seekPoint - distance * viewVector;
}

void
SeekAnimator::start()
{
  // now check if animation sensor needs to be scheduled
  if ( m_duration == 0 )
  {
    // jump to new location, no animation needed
    interpolateSeekAnimation( 1.0 );
  }
  else
  {
    // get animation starting time
    Animator::start();
    m_startTime = m_realTimeField->getValue();
  }
}

void
SeekAnimator::animate()
{
  // get the time difference
  SbTime time = m_realTimeField->getValue();
  float sec = (float)( (time - m_startTime).getValue() );
  if ( sec == 0.0 )
    sec = 1.0f / 72.0f;  // at least one frame (needed for first call)
  float t = sec / m_duration;

  // check to make sure the values are correctly clipped
  if ( t > 1.0 )
    t = 1.0;
  else if ( (1.0 - t) < 0.0001 )
    t = 1.0;    // this will be the last one

  // interpolate the animation
  interpolateSeekAnimation( t );

  // stops seek if this was the last interval
  if ( t == 1.0 )
    stop();
}

void
SeekAnimator::interpolateSeekAnimation( float t )
{
  // Now position the camera according to the animation time
  // use and ease-in ease-out approach
  float cos_t = (float)( 0.5f - ( 0.5f * cos(t * M_PI) ) );

  // get camera new rotation
  m_cameraInteractor->setOrientation( SbRotation::slerp( m_saveCameraOrientation, m_newCameraOrientation, cos_t ) );

  // get camera new position
  m_cameraInteractor->setPosition( m_saveCameraPosition + ( m_newCameraPosition - m_saveCameraPosition ) * cos_t );

  m_cameraInteractor->adjustClippingPlanes( m_sceneGraph, m_vpRegion );
}
