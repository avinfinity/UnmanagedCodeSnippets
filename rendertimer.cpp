#include "rendertimer.h"

//------------------------------------------------------------------------------
RenderTimer::RenderTimer()
	: m_task(NULL), m_delay(0)
{
}

//------------------------------------------------------------------------------
void
RenderTimer::stop()
{
	timer.stop();
}

//------------------------------------------------------------------------------
void
RenderTimer::start()
{
	timer.start(m_delay);
}

//------------------------------------------------------------------------------
void
RenderTimer::setDelay(int time)
{
	m_delay = time;
}

//------------------------------------------------------------------------------
bool
RenderTimer::isPending() const
{
	return timer.isActive();
}

//------------------------------------------------------------------------------
void
RenderTimer::setRepeat(bool flag)
{
	timer.setSingleShot(!flag);
}

//------------------------------------------------------------------------------
void
RenderTimer::setTask(SoSystemTimerTask* task)
{
	m_task = task;
	connect(&timer, SIGNAL(timeout()), this, SLOT(execCallback()));
}

//------------------------------------------------------------------------------
void
RenderTimer::execCallback()
{
	if (m_task.ptr() != NULL)
		m_task->run();
}



#include "rendertimer.moc"