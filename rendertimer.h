/*=======================================================================
** VSG_COPYRIGHT_TAG
**=======================================================================*/
/*=======================================================================
** Author      : Benjamin Grange (MMM YYYY)
**=======================================================================*/

#ifndef SORenderTimer_H
#define SORenderTimer_H

#include <Inventor/sensors/SoSensor.h>
#include <Inventor/sensors/SoSystemTimer.h>
#include <Inventor/misc/SoRef.h>

#include <QTimer>

class RenderTimer : public QObject, public SoSystemTimer
{
	Q_OBJECT
public:

	RenderTimer();

	virtual void start();

	virtual void stop();

	virtual void setDelay(int time);

	virtual bool isPending() const;

	virtual void setRepeat(bool flag);

	virtual void setTask(SoSystemTimerTask* task);

	private Q_SLOTS:
	void execCallback();

private:
	QTimer timer;
	SoRef<SoSystemTimerTask> m_task;
	int m_delay;
};

#endif
