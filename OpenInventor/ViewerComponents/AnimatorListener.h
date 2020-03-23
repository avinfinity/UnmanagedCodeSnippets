#ifndef _AnimatorListener_
#define _AnimatorListener_

#if defined(_WIN32)
#if _DEBUG
#pragma comment(lib,"ViewerComponentsD")
#else
#pragma comment(lib,"ViewerComponents")
#endif
#endif

class AnimatorListener
{

public:
  AnimatorListener() {};

  virtual ~AnimatorListener() {};

  virtual void animationStarted() = 0;

  virtual void animationStopped() = 0;

};
#endif // _AnimatorListener_
