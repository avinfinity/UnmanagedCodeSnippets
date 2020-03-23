#include <RenderAreaInteractive.h>

#include <QMouseEvent> 
#include <QWheelEvent>
#include <QKeyEvent>

#include <Inventor/ViewerComponents/SoRenderAreaCore.h>
#include <Inventor/events/SoLocation2Event.h>
#include <Inventor/events/SoMouseWheelEvent.h>
#include <Inventor/SoDB.h>
#include <QtTimer.h>

#include "iostream"

void 
redrawRequest(void *userData)
{
  RenderAreaInteractive* _this = (RenderAreaInteractive*)userData;
  _this->update();

 
}

//------------------------------------------------------------------------------
RenderAreaInteractive::RenderAreaInteractive( QWidget* parent )
  : RenderArea( parent )
{
  setAttribute(Qt::WA_AcceptTouchEvents, true);
  SoDB::setSystemTimer(new QtTimer);

}

//------------------------------------------------------------------------------
RenderAreaInteractive::~RenderAreaInteractive()
{
}

//------------------------------------------------------------------------------
void
RenderAreaInteractive::initializeGL()
{
  RenderArea::initializeGL();
  m_renderAreaCore->setRedrawRequestCallback(&redrawRequest,this);

  m_renderAreaCore->setInteractive(SoRenderAreaCore::FORCE_INTERACTION);
}

//------------------------------------------------------------------------------
void
RenderAreaInteractive::mousePressEvent( QMouseEvent* qevent )
{
  SoEvent* event = QEventToSoEvent::getMousePressEvent(qevent,this);
  m_renderAreaCore->processEvent(event);
}

//------------------------------------------------------------------------------
void 
RenderAreaInteractive::mouseReleaseEvent( QMouseEvent* qevent )
{
  SoEvent* event = QEventToSoEvent::getMouseReleaseEvent(qevent,this);
  m_renderAreaCore->processEvent(event);
}

//------------------------------------------------------------------------------
void 
RenderAreaInteractive::mouseMoveEvent( QMouseEvent* qevent )
{

	//std::cout << " in mouse move " << std::endl;

  SoEvent* event = QEventToSoEvent::getMouseMoveEvent(qevent,this);
  m_renderAreaCore->processEvent(event);

  
}

//------------------------------------------------------------------------------
void 
RenderAreaInteractive::wheelEvent( QWheelEvent* qevent )
{
  SoEvent* event = QEventToSoEvent::getMouseWheelEvent(qevent,this);
  m_renderAreaCore->processEvent(event);
}

//------------------------------------------------------------------------------
void 
RenderAreaInteractive::keyPressEvent( QKeyEvent* qevent )
{
  SoEvent* event = QEventToSoEvent::getKeyPressEvent(qevent,this);
  m_renderAreaCore->processEvent(event);
}

//------------------------------------------------------------------------------
void 
RenderAreaInteractive::keyReleaseEvent( QKeyEvent* qevent )
{
  SoEvent* event = QEventToSoEvent::getKeyReleaseEvent(qevent,this);
  m_renderAreaCore->processEvent(event);
}

//------------------------------------------------------------------------------
void 
RenderAreaInteractive::enterEvent(QEvent * qevent)
{
  SoEvent* event = QEventToSoEvent::getMouseEnterEvent(qevent,this);
  m_renderAreaCore->processEvent(event);
}

//------------------------------------------------------------------------------
void 
RenderAreaInteractive::leaveEvent(QEvent * qevent)
{
  SoEvent* event = QEventToSoEvent::getMouseLeaveEvent(qevent,this);
  m_renderAreaCore->processEvent(event);
}

//------------------------------------------------------------------------------
void 
RenderAreaInteractive::mouseDoubleClickEvent(QMouseEvent * qevent)
{
  SoEvent* event = QEventToSoEvent::getMouseDoubleClickEvent(qevent,this);
  m_renderAreaCore->processEvent(event);
}

//------------------------------------------------------------------------------
bool 
RenderAreaInteractive::event(QEvent * qevent)
{
  QTouchEvent *qtouchEvent = dynamic_cast<QTouchEvent*>(qevent);
  if (qtouchEvent != NULL) 
  {
    const std::vector<const SoEvent*>& soeventlist = m_eventbuilder.getTouchEvents(qtouchEvent,this);
    m_renderAreaCore->processEvents(soeventlist);
    return true;
  } else
    return QWidget::event(qevent);
}
