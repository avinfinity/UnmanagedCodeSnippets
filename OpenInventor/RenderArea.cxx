#include <RenderArea.h>

#include <Inventor/SbViewportRegion.h>
#include <Inventor/nodes/SoNode.h>
#include <Inventor/ViewerComponents/SoRenderAreaCore.h>
#include <Inventor/devices/SoGLContext.h>
#include <Inventor/SoSceneManager.h>
#include <Inventor/elements/SoInteractionElement.h>

//------------------------------------------------------------------------------
RenderArea::RenderArea( QWidget* parent )
  : QGLWidget( parent ),
  m_renderAreaCore(NULL),
  m_sceneGraph(NULL),
  m_context(NULL)
{
  //Allow the widget to receive keyboard events
  setFocusPolicy(Qt::StrongFocus);

  // swap buffers manually so when we abort a rendering we keep the previous one
  setAutoBufferSwap( false );
}

//------------------------------------------------------------------------------
RenderArea::~RenderArea()
{
  // remove the ref added in setSceneGraph
  if (m_sceneGraph != NULL)
    m_sceneGraph->unref();

  if ( m_context != NULL )
  {
    m_context->unref();
    m_context = NULL;
  }
}

//------------------------------------------------------------------------------
void
RenderArea::setSceneGraph(SoNode *sceneGraph)
{
  // unref previous scene graph
  if (m_sceneGraph != NULL)
    m_sceneGraph->unref();

  m_sceneGraph = sceneGraph;
  // keep an additional ref to the scene graph, because the scene renderer 
  // may be created later.
  if (m_sceneGraph != NULL)
    m_sceneGraph->ref();

  if (m_renderAreaCore.ptr() != NULL)
    m_renderAreaCore->setSceneGraph(m_sceneGraph);
}

//------------------------------------------------------------------------------
void
RenderArea::initializeGL()
{
  // Create a SoGLContext with Qt's GL context
  // SoGLContext has a default count of 1 so we must not ref it here
  m_context = SoGLContext::getCurrent(true);
  m_renderAreaCore = new SoRenderAreaCore(m_context);
  m_renderAreaCore->setSceneGraph(m_sceneGraph);

  // check if there are incoming events when rendering, and if so abort the rendering
  m_renderAreaCore->getSceneManager()->setAbortRenderCallback( RenderArea::s_abortRenderCallback, this );
}

//------------------------------------------------------------------------------
void
RenderArea::paintGL()
{
  // update the camera far and near planes and render the scene graph.
  SoRenderAreaCore::RenderStatus renderStatus = m_renderAreaCore->render();

  // Check if the rendering is made well
  // If not : don't swap the buffer because we can have actifacts in the buffer
  if ( renderStatus != SoRenderAreaCore::ABORTED )
    swapBuffers();
  else
  {
    //Ask for a new render if the previous frame was aborted (for example, a frame
    //may have been aborted because some GUI events were waiting to be processed)
    m_renderAreaCore->getSceneManager()->scheduleRedraw();
  }
}

//------------------------------------------------------------------------------
void
RenderArea::resizeGL( int width, int height )
{
  SbViewportRegion viewportRegion(width,height);
  m_renderAreaCore->setViewportRegion(viewportRegion);
}

//------------------------------------------------------------------------------
SbBool
RenderArea::s_abortRenderCallback(SoAction* action, void*)
{
  SoState* state = action->getState();

  // check if we are rendering a frame in INTERATIVE mode. In that case, don't abort rendering,
  // else we might abort all frame and we won't have any rendering.
  // if frame is in STILL mode (non interacting), then we will check for event in event queue
  // to get back interactive as soon as possible.
  if ( !SoInteractionElement::isInteracting(state) )
  {
    SoSceneManager* sceneMgr = action->getSceneManager();
#if defined (_WIN32)
    if ( GetQueueStatus( QS_MOUSEMOVE | QS_MOUSEBUTTON) )
      return TRUE;
#elif defined(__APPLE__)
    //TODO: NOT IMPLEMENTED
#else // UNIX
    //TODO: NOT IMPLEMENTED
#endif
  }

  return FALSE;
}


#include "RenderArea.moc"
