#ifndef _Animator_
#define _Animator_

#if defined(_WIN32)
#if _DEBUG
#pragma comment(lib,"ViewerComponentsD")
#else
#pragma comment(lib,"ViewerComponents")
#endif
#endif

#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/fields/SoSFTime.h>
#include "AnimatorListener.h"

class SoSensor;

/**
* Base class for real-time animation.
*
* The real-time animation is based on a field sensor connected to the
* @B realTime @b global field.
* The #animate function is called whenever the @B realTime @b global field changes.
*/
class Animator
{

public:

  /**
  * Start the animation and notify the animation listener if any.
  */
  virtual void start();

  /**
  * Stop the animation and notify the animation listener if any.
  */
  virtual void stop();

  /**
  * Set the listener to receive notifications of animation changes.
  */
  void setListener( AnimatorListener* listener );

protected:
  SoSFTime* m_realTimeField; // pointer to "realTime" global field

  Animator();
  virtual ~Animator();

  /**
  * Function called whenever the @B realTime @b global field changes. Override
  * this function to create your animation.
  */
  virtual void animate() = 0;

private:
  SoFieldSensor* m_animationSensor;
  AnimatorListener* m_listener;

  static void animationCB( void *data, SoSensor *sensor );
};

#endif // _Animator_
