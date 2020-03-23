/*=======================================================================
** VSG_COPYRIGHT_TAG
**=======================================================================*/

#include <QtTimer.h>

//------------------------------------------------------------------------------
QtTimer::QtTimer()
  : m_task(NULL), m_delay(0)
{
}

//------------------------------------------------------------------------------
void
QtTimer::stop()
{
  timer.stop();
}

//------------------------------------------------------------------------------
void
QtTimer::start()
{
  timer.start(m_delay);
}

//------------------------------------------------------------------------------
void
QtTimer::setDelay(int time)
{
  m_delay = time;
}

//------------------------------------------------------------------------------
bool
QtTimer::isPending() const
{
  return timer.isActive();
}

//------------------------------------------------------------------------------
void
QtTimer::setRepeat(bool flag)
{
  timer.setSingleShot(!flag);
}

//------------------------------------------------------------------------------
void
QtTimer::setTask(SoSystemTimerTask* task)
{
  m_task = task;
  connect(&timer, SIGNAL(timeout()), this, SLOT(execCallback()));
}

//------------------------------------------------------------------------------
void
QtTimer::execCallback()
{
  if ( m_task.ptr() != NULL )
    m_task->run();
}