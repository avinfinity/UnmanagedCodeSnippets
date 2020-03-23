#include <Inventor/SoDB.h>
#include "Animator.h"

void
Animator::animationCB( void* data, SoSensor* )
{
  Animator* animator = (Animator*) data;
  animator->animate();
}

Animator::Animator()
  : m_listener( NULL )
{
  m_realTimeField = (SoSFTime *) SoDB::getGlobalField( "realTime" );
  m_animationSensor = new SoFieldSensor( Animator::animationCB, this );
}

Animator::~Animator()
{
  delete m_animationSensor;
}

void
Animator::start()
{
  if ( m_animationSensor->getAttachedField() == NULL )
  {
    m_animationSensor->attach( m_realTimeField );
    m_animationSensor->schedule();

    if ( m_listener != NULL )
      m_listener->animationStarted();
  }
}

void
Animator::stop()
{
  if ( m_animationSensor->getAttachedField() != NULL )
  {
    m_animationSensor->detach();
    m_animationSensor->unschedule();

    if ( m_listener != NULL )
      m_listener->animationStopped();
  }
}

void
Animator::setListener( AnimatorListener* listener )
{
  m_listener = listener;
}
