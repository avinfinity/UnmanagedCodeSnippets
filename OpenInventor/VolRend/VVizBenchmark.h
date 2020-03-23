/*==============================================================================
 *   File        : VVizBenchmark.h
 *   Classes     : VVizBenchmark
 *   Author(s)   : J.HUMMEL
 *   Date        : February-2003
 *   updated     : July 2003- add timeStamps to be able to see refinement happening
 *                 instead of seeing constant user interaction
 *==============================================================================
 *
 * Description : Perform benchmark test of VolumeViz extension.
 * Record user interaction and play it back to output a timing result.
 * 
 * Usage :
 * Slices must be created BEFORE to start recording.
 *
 *============================================================================*/

#ifndef _VVizBenchmark_H_
#define _VVizBenchmark_H_

#include <Inventor/SbLinear.h>

class SoPerspectiveCamera;
class SoEventCallback;
class SoSensor;
class SoOrthoSlice;
class SoObliqueSlice;
class SoJackDragger;
class SbThread;
class SbThreadBarrier;
class SoAlarmSensor;
class SoXtViewer;

#include <Inventor/SbElapsedTime.h>
#include <Inventor/STL/vector>

class VVizBenchmark
{
public:
  VVizBenchmark();
  ~VVizBenchmark(void);

  //set/get actors of benchmark test
  void setViewer(SoXtViewer * viewer);
  void setObliqueSlice(SoObliqueSlice *slice){m_obliqueSlice = slice;}
  void setOrthoSlice(SoOrthoSlice *slice){m_orthoSlice = slice;}
  SoOrthoSlice *getOrthoSlice(void){return m_orthoSlice;}
  SoPerspectiveCamera * GetCamera();

  //I/O
  void readTrajectory (char * filename);
  void writeTrajectory(char * filename);


  // basic benchmark functions
  void record();
  void play(SbBool rewind=FALSE, int numLoop=1);
  void stop();
  void recordCamera(SoPerspectiveCamera * camera);
  void rewind();
  //loop


  //list of callback used when moves must be recorded
  static void keyPressCB(void *userData, SoEventCallback * eventCB);
  static void sliceMotionCallback(void *user_data, SoJackDragger *dragger);
  static void obliSliceMotionCallback(void *user_data, SoJackDragger *dragger);
  static void sliceStartCallback(void *user_data, SoJackDragger *dragger);
  static void sliceFinishCallback(void *user_data, SoJackDragger *dragger);

  //enable/disable console output messages
  void setEnableFeedback(SbBool enable);

  //add a dialog state if the user changed something
  void recordDialogState(    int orthoSliceVisibility,
    int obliqueSliceVisibility,
    SbBool obliqueClipping,
    SbBool orthoClipping, int);
  
  void reset(void);
  SbBool isRecording(){return m_recordMode;}

  void render();

  //result filename returned by the benchmark object when playing a benchmark data file
  char* outputResult;
  void replay(SbBool rewind = FALSE, int numLoop = 1, bool init=1);
  int m_numLoop;//record numLoop for timeStamp
private:

  //check if user has waited and record the invent if lower than the TIME_STAMP threshold
  void checkTime(void);

  //to check bogus usage
  SbBool doNotPlay;
  
  //benchmark file name
  char *benchFileName;
  
  //start index for the m_indices stack in replay function in case we have to return and
  //call back the function for time stamp usage
  int m_startIndex;

  void recordOrthoSlice();
  void recordObliqueSlice();

  void procKeyPressEvent(SoEventCallback *eventCB);

  //Objects which must be recorded and associated state struct
  //---ORTHOSLICE----
  SoOrthoSlice *m_orthoSlice;
  struct OrthoSliceState{
    uint32_t  sliceNumber; 
    int       axis;
    int       interpolation ;
    int       alphaUse ;
    int       clippingSide ;
    SbBool    clipping ;
    int       currentSlice;
  };
  //---OBLIQUESLICE----
  SoObliqueSlice *m_obliqueSlice;
  struct ObliqueSliceState{
    SbPlane   plane;
    int       interpolation ;
    int       alphaUse ;
  };

  //------CAMERA-------
  SoPerspectiveCamera * m_camera;
  struct CameraState{
    SbVec3f position;
    SbVec3f rotAxis;
    float rotAngle;
    float aspectRatio;
    float nearDistance;
    float farDistance;
    float focalDistance;
    float heightAngle;
  };

  //------DIALOG-------
  struct VolRendDialogState{
    int orthoSliceVisibility;
    int obliqueSliceVisibility;
    SbBool obliqueClipping;
    SbBool orthoClipping;
    int currentSlice;
  };
  
  //TIME
  struct TimeStampState{
    double elapsedTime;
  };
  //Stack of recorded state
  std::vector<int *>	m_indices;
  std::vector<OrthoSliceState> m_orthoSliceState;
  std::vector<ObliqueSliceState> m_obliqueSliceState;
  std::vector<CameraState> m_cameraState;
  std::vector<VolRendDialogState> m_dialogState;
  std::vector<TimeStampState> m_timeStamp;

  SoXtViewer * m_viewer;
  int m_recordMode;
  char *m_outputFile;
  SbBool m_enableFeedback;

  enum ObjectType{
    ORTHO_SLICE,
    CAMERA,
    OBLIQUE_SLICE,
    DIALOG,
    TIME
  };

  SbBool m_startRecording;

  //SbThread* m_thread;
  //static void *threadRoutine(void* userData);
  SbBool m_stop;
  SbThreadBarrier* m_barrier;

  int m_numOrthoSlice;

  SbElapsedTime m_timer;
  SoAlarmSensor* m_replaySensor;

  FILE *m_outputResult;
};

#endif /*_VVizBenchmark_H_*/


