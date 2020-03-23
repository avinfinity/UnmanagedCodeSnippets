#ifndef RenderArea_H
#define RenderArea_H

#include <Inventor/misc/SoRef.h>
#include <Inventor/sys/SoGL.h>
#include <QtOpenGL/QGLWidget>
#include <Inventor/actions/SoAction.h>

class SoRenderAreaCore;
class SoNode;

/**
 * Class to render an OpenInventor scene in a Qt OpenGL window
 */
class RenderArea : public QGLWidget
{
 public:
  
  /**
   * This constructor initializes the QGLWidget using a default QGLFormat.
   */ 
  RenderArea( QWidget* parent );

  /** Destructor */
  virtual ~RenderArea();

  /*
   * Sets the scene graph to render.
   */
  virtual void setSceneGraph(SoNode* sceneGraph);


 protected:

  /** 
   * This method is called by Qt in order to allow custom parameters 
   * for OpenGL. 
   * This function is used in this demo to enable ZBuffer test.
   */
  virtual void initializeGL();

  /**
   * This method is called by Qt when the size of the widget is modified. 
   * We need to know when the size is modified in order to update the viewport for
   * the internal computation of the clipping planes, aspect ratio, mouse coordinates...
   *
   * It is called when the widget is displayed for the first time and when the size changes.
   */
  virtual void resizeGL( int width, int height );

  /**
   * This method is called when Qt wants to render the OpenGL surface.
   * It is called also when OpenInventor needs an update through the oivRenderCB function.
   * OpenInventor must be able to inform us of the required update when the scenegraph is 
   * modified.
   */
  virtual void paintGL();

 protected:

  /** 
   * Instance of the class which provides the algorithms for viewing and managing 
   * OpenInventor scene graphs.
   */
  SoRef<SoRenderAreaCore> m_renderAreaCore;

  SoNode* m_sceneGraph;

private:

  /**
   * Check if there are event pending on system event queue and ask for
   * aborting event to keep interactivity.
   */
  static SbBool s_abortRenderCallback(SoAction* action, void* );

  /** Qt's OpenGL context */
  SoGLContext* m_context;
};

#endif // RenderArea_H


