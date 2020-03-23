#include <RenderAreaExaminer.h>

#include <Inventor/nodes/SoGroup.h>

#include "ViewerComponents/SceneExaminer.h"


//------------------------------------------------------------------------------
RenderAreaExaminer::RenderAreaExaminer( QWidget* parent )
  : RenderAreaInteractive( parent )
{
  m_rootSceneGraph = new SceneExaminer;
  m_appSceneGraph = new SoGroup;
  m_rootSceneGraph->addChild(m_appSceneGraph);
  RenderAreaInteractive::setSceneGraph(m_rootSceneGraph);
}

//------------------------------------------------------------------------------
void
RenderAreaExaminer::setSceneGraph(SoNode *sceneGraph)
{
  m_appSceneGraph->removeAllChildren();
  m_appSceneGraph->addChild(sceneGraph);
}

//------------------------------------------------------------------------------
void
RenderAreaExaminer::viewAll(const SbViewportRegion &viewport)
{
  m_rootSceneGraph->viewAll(viewport);
}

//------------------------------------------------------------------------------
SceneExaminer*
RenderAreaExaminer::getSceneExaminer()
{
  return m_rootSceneGraph;
}
