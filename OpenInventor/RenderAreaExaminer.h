#ifndef RenderAreaExaminer_H
#define RenderAreaExaminer_H

#include "RenderAreaInteractive.h"

class SceneExaminer;
class SoGroup;

/**
 * Class to render an OpenInventor scene graph in a Qt OpenGL window.
 * This class extends RenderAreaInteractive to add examiner viewer behaviors.
 */
class RenderAreaExaminer : public RenderAreaInteractive
{
 public:
  
  /** Constructor */
  RenderAreaExaminer( QWidget* parent );

  /*
   * Sets the scene graph to render.
   */
  virtual void setSceneGraph(SoNode* sceneGraph);

  /**
   * Move the camera to view the scene defined by the given path. 
   * Equivalent to calling the SoCamera method viewAll(). Camera position is changed, but not orientation. 
   */
  void viewAll(const SbViewportRegion &viewport);

  /**
   * Returns the scene examiner.
   * @return The scene examiner
   */
  SceneExaminer* getSceneExaminer();

private:
  SceneExaminer* m_rootSceneGraph;
  SoGroup* m_appSceneGraph;
};

#endif // RenderAreaExaminer_H
