#ifndef QEventToSoEvent_h
#define QEventToSoEvent_h

#include <QWidget>
#include <QMouseEvent> 
#include <QWheelEvent>
#include <QKeyEvent>
#include <QTouchEvent>

#include <Inventor/ViewerComponents/SoEventBuilder.h>

class QEventToSoEvent {
public:

  static SoMouseButtonEvent* getMousePressEvent(QMouseEvent* qevent, QWidget* wsource)
  {
    return m_ivEvent.getMousePressEvent(qevent->x(), (wsource->height()-1)-qevent->y(),
      getButtonId(qevent),
      qevent->modifiers() & Qt::AltModifier,
      qevent->modifiers() & Qt::ControlModifier,
      qevent->modifiers() & Qt::ShiftModifier);
  }

  static SoMouseButtonEvent* getMouseReleaseEvent(QMouseEvent* qevent, QWidget* wsource)
  {
    return m_ivEvent.getMouseReleaseEvent(qevent->x(), (wsource->height()-1)-qevent->y(),
      getButtonId(qevent),
      qevent->modifiers() & Qt::AltModifier,
      qevent->modifiers() & Qt::ControlModifier,
      qevent->modifiers() & Qt::ShiftModifier);
  }

  static SoMouseButtonEvent* getMouseDoubleClickEvent(QMouseEvent* qevent, QWidget* wsource)
  {
    return m_ivEvent.getMouseDoubleClickEvent(qevent->x(), (wsource->height()-1)-qevent->y(),
      getButtonId(qevent),
      qevent->modifiers() & Qt::AltModifier,
      qevent->modifiers() & Qt::ControlModifier,
      qevent->modifiers() & Qt::ShiftModifier);
  }

  static SoMouseWheelEvent* getMouseWheelEvent(QWheelEvent * qevent, QWidget* )
  {
    return m_ivEvent.getMouseWheelEvent(qevent->delta(),
                                        qevent->modifiers() & Qt::AltModifier,
                                        qevent->modifiers() & Qt::ControlModifier,
                                        qevent->modifiers() & Qt::ShiftModifier);
  }

  static SoLocation2Event* getMouseMoveEvent( QMouseEvent* qevent, QWidget* wsource )
  {
    return m_ivEvent.getMouseMoveEvent(qevent->x(), (wsource->height()-1)-qevent->y(),
                                       qevent->modifiers() & Qt::AltModifier,
                                       qevent->modifiers() & Qt::ControlModifier,
                                       qevent->modifiers() & Qt::ShiftModifier);
  }

  static SoLocation2Event* getMouseEnterEvent( QEvent* , QWidget* )
  {
    return m_ivEvent.getMouseEnterEvent(0, 0,
                                        false,
                                        false,
                                        false);
  }

  static SoLocation2Event* getMouseLeaveEvent( QEvent* , QWidget* )
  {
    return m_ivEvent.getMouseLeaveEvent(0, 0,
                                        false,
                                        false,
                                        false);
  }

  static SoKeyboardEvent* getKeyPressEvent( QKeyEvent *qevent, QWidget* )
  {
    SoKeyboardEvent::Key ivKey = getIvKey(qevent);
    return m_ivEvent.getKeyPressEvent(ivKey,
                                     qevent->modifiers() & Qt::AltModifier,
                                     qevent->modifiers() & Qt::ControlModifier,
                                     qevent->modifiers() & Qt::ShiftModifier);
  }

  static SoKeyboardEvent* getKeyReleaseEvent( QKeyEvent *qevent, QWidget* )
  {
    SoKeyboardEvent::Key ivKey = getIvKey(qevent);
    return m_ivEvent.getKeyReleaseEvent(ivKey,
                                   qevent->modifiers() & Qt::AltModifier,
                                   qevent->modifiers() & Qt::ControlModifier,
                                   qevent->modifiers() & Qt::ShiftModifier);
  }

  const std::vector<const SoEvent*>& getTouchEvents(QTouchEvent *qevent, QWidget* wsource );

private:
  static bool initClass();
  static bool s_init;
  static SoMouseButtonEvent::Button getButtonId(QMouseEvent* qevent);
  static SoKeyboardEvent::Key getIvKey(QKeyEvent *qevent);
  static SoEventBuilder m_ivEvent; 
  std::vector<const SoEvent*> m_soeventlist;

  // Mapping from QT virtual keys to SoKeyboardEvent::Key enum
  static SoKeyboardEvent::Key keyMap[256];
  static SoKeyboardEvent::Key keyMap2[97];
  static int keyMapInitFlag;

};

#endif // QEventToSoEvent
