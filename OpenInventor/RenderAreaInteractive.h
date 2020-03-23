#ifndef RenderAreaInteractive_H
#define RenderAreaInteractive_H

#include "RenderArea.h"
#include "QEventToSoEvent.h"

class QRenderAreaCore;
class QMouseEvent;
class QWheelEvent;
class QKeyEvent;

/**
 * Class to render an OpenInventor scene in a Qt OpenGL window.
 * This class extends RenderArea to add mouse and keyboard interactions.
 */
class RenderAreaInteractive : public RenderArea
{

 public:

  /**
   * This constructor initializes the QGLWidget using a default QGLFormat.
   */ 
  RenderAreaInteractive( QWidget* parent );

  /**
   * Destructor.
   */
  virtual ~RenderAreaInteractive();

 protected:

  /** 
   * This method is called by Qt in order to allow custom parameters for OpenGL. 
   * This function is used in this demo to enable ZBuffer test.
   */
  virtual void initializeGL();

  /**
   * This method is called by Qt when a mouse button is pressed.
   * It starts the trackball rotation.
   */
  virtual void mousePressEvent( QMouseEvent* event );

  /**
   * This method is called by Qt when a mouse button is released.
   * It ends the trackball rotation.
   */
  virtual void mouseReleaseEvent( QMouseEvent* event );

  /**
   * This method is called by Qt when the mouse moves.
   * It updates the camera position if a mouse button is pressed.
   */
  virtual void mouseMoveEvent( QMouseEvent* event );

  /**
   * This method is called by Qt when the mouse wheel is used.
   * It is used to change the zoom level.
   */
  virtual void wheelEvent( QWheelEvent* event );

  /**
   * This method is called by Qt when a key is pressed.
   */
  virtual void keyPressEvent ( QKeyEvent * event );

  /**
   * This method is called by Qt when a key is released.
   */
  virtual void keyReleaseEvent ( QKeyEvent * event );

  /**
   * This method is called by Qt when the cursor enters the window.
   */
  virtual void enterEvent(QEvent * event);

  /**
   * This method is called by Qt when the cursor leaves the window.
   */
  virtual void leaveEvent(QEvent * event);

  /**
   * This method is called by Qt when a double-click occurs.
   */
  virtual void mouseDoubleClickEvent(QMouseEvent * event);

  /**
   * This method is called by Qt for all events.
   * It is overridden to handle touch events.
   */
  virtual bool event(QEvent * qevent);

private:
  QEventToSoEvent m_eventbuilder;
};

#endif // RenderAreaInteractive_H
