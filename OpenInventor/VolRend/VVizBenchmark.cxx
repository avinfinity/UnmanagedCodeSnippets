/*==============================================================================
 *   File        : VVizBenchmark.cpp
 *   Classes     : VVizBenchmark
 *   Author(s)   : J.HUMMEL
 *   Date        : February-2003
 *   updated     : July 2003- add timeStamps to be able to see refinement happening
 *                 instead of seeing constant user interaction
 *==============================================================================
 *
 * Description : Perform benchmark test of VolumeViz extension
 * Record user interaction and play it back to output a timing result.
 *
 * Usage :
 * Slices must be created BEFORE to start recording.
 *
 *============================================================================*/

//NOTICE : the only dependency this class has with volRend is the use of dialog state variable
#include "volRend.h"

#ifndef CAVELIB
#include <Inventor/Xt/viewers/SoXtViewer.h>
#endif //CAVELIB

#include "VVizBenchmark.h"
#include <Inventor/STL/iostream>
#include <time.h>

void replaySensorCB(void *data, SoSensor *)
{
  //The ideal texture front hasn't been reached yet, let's render again
  VVizBenchmark* bench = (VVizBenchmark*)data;
  bench->replay(FALSE, bench->m_numLoop,FALSE);
}


/*------------------------------------------------------------------------------
                              Constructor
------------------------------------------------------------------------------*/
VVizBenchmark::VVizBenchmark():m_enableFeedback(FALSE)
{
  m_viewer = NULL;
  m_camera = NULL;
  m_recordMode = 0;
  m_outputFile = NULL;
  m_stop = FALSE;
  m_numOrthoSlice = 0;
  m_numLoop = 1;
  outputResult = new char[255];
  strcpy(outputResult,"result.out");//default name
  benchFileName = new char[255];
  benchFileName='\0';
  doNotPlay = FALSE;
  m_replaySensor = new SoAlarmSensor(replaySensorCB, this);
  m_outputResult = NULL;
  m_startIndex = 0;
}

VVizBenchmark::~VVizBenchmark()
{
  reset();
  //delete outputResult;
  //delete benchFileName;
}

/*------------------------------------------------------------------------------

------------------------------------------------------------------------------*/
SoPerspectiveCamera *VVizBenchmark::GetCamera()
{
  return m_camera;
}

void VVizBenchmark::setEnableFeedback(SbBool enable)
{
  m_enableFeedback = enable;
}


#ifndef CAVELIB
void VVizBenchmark::setViewer(SoXtViewer * viewer)
{
 m_viewer = viewer;
 m_camera = (SoPerspectiveCamera *)(viewer->getCamera());
}
#endif //CAVELIB

/*------------------------------------------------------------------------------
                              Reset stacks of state
------------------------------------------------------------------------------*/
void VVizBenchmark::reset(void)
{
    m_indices.clear();
    m_orthoSliceState.clear();
    m_obliqueSliceState.clear();
    m_cameraState.clear();
    m_dialogState.clear();
    m_timeStamp.clear();
}

/*------------------------------------------------------------------------------
Callbakcs functions : keyboard, camera move, oblique and ortho slice move
------------------------------------------------------------------------------*/

void VVizBenchmark::keyPressCB(void *userData, SoEventCallback * eventCB)
{
  VVizBenchmark * p = (VVizBenchmark *)userData;
  p->procKeyPressEvent(eventCB);
}

static SoNodeSensor * cameraSensor = 0;

static void cameraCB( void * data, SoSensor * )
{
  VVizBenchmark * bench = reinterpret_cast<VVizBenchmark *>( data );
  bench->recordCamera( bench->GetCamera() );
}

void VVizBenchmark::sliceStartCallback(void *, SoJackDragger *)
{
  if(cameraSensor)
    cameraSensor->detach();
}

void VVizBenchmark::sliceFinishCallback(void *user_data, SoJackDragger *)
{
  VVizBenchmark * bench = reinterpret_cast<VVizBenchmark *>( user_data );
  if(cameraSensor)
    cameraSensor->attach(bench->GetCamera());
}

void VVizBenchmark::sliceMotionCallback(void *user_data, SoJackDragger *)
{
  VVizBenchmark * bench = reinterpret_cast<VVizBenchmark *>( user_data );
  if (bench->m_recordMode)
  {
    bench->recordOrthoSlice( );
  }
}
void VVizBenchmark::obliSliceMotionCallback(void *user_data, SoJackDragger *)
{
  VVizBenchmark * bench = reinterpret_cast<VVizBenchmark *>( user_data );
  if (bench->m_recordMode)
  {
    bench->recordObliqueSlice( );
  }
}

void VVizBenchmark::record()
{
  doNotPlay = FALSE;
  m_numOrthoSlice = m_lastNumSlices;
  //record current dialog state
  for(int i=0;i<m_numOrthoSlice;i++)
  m_bench.recordDialogState(  m_switchOrthoTab[m_currentSlice]->whichChild.getValue(),
                             m_volRenderObliSliceSwitch->whichChild.getValue(),
                             m_obliClipPlane->on.getValue(),
                             m_orthoTab[i]->clipping.getValue(),i);

  m_recordMode = (m_recordMode == 1) ? 0:1;
  if(m_recordMode)
  {
    reset();
    m_startRecording = 1;
    // attach a sensor to the camera
    m_timer.reset();
    if (cameraSensor == 0)
    {
      cameraSensor = new SoNodeSensor( cameraCB, this );
    }
    cameraSensor->attach( GetCamera() );
  }
  else
  {
    stop();
  }
}

void VVizBenchmark::stop()
{
  if(cameraSensor)cameraSensor->detach();
  m_recordMode = 0;
  m_stop = 1;
}

void VVizBenchmark::play(SbBool rewind, int numLoop)
{
  m_stop = 0;
  m_startIndex = 0;
  if(doNotPlay) return; //bad trajectory file

  // turn off record mode
  if(m_recordMode)
  {
    m_recordMode = 0;
    cameraSensor->detach();
  }
  replay(rewind,numLoop);
}

void VVizBenchmark::rewind()
{
  play(TRUE);
}

void VVizBenchmark::procKeyPressEvent(SoEventCallback *eventCB)
{
  const SoEvent * event = eventCB->getEvent();

  if(SO_KEY_PRESS_EVENT(event, R))
  {
    record();
  }
  else if(SO_KEY_PRESS_EVENT(event, P))
  {
    play();
  }
  else if(SO_KEY_PRESS_EVENT(event, W))//write
  {
    writeTrajectory((char*)"C:/temp/VVizBenchmark.dat");
  }
  else if(SO_KEY_PRESS_EVENT(event, L))//load
  {
    readTrajectory((char*)"C:/temp/VVizBenchmark.dat");
    replay();
  }
}
/*------------------------------------------------------------------------------
                      Read/Write to/into specified filename
------------------------------------------------------------------------------*/
void VVizBenchmark::readTrajectory(char * filename)
{
  doNotPlay = FALSE;
  benchFileName = filename;
  m_outputResult = NULL;
  FILE * fp = fopen(filename, "r");
  if(!fp)
  {
    SoDebugError::post("VVizBenchmark::readTrajectory","cannot open %s for read",filename);
    doNotPlay = TRUE;
    return;
  }

  reset();

  int n;
  if(m_enableFeedback)SoDebugError::postInfo(" VVizBenchmark: set trajectory from file %s\n", filename);

  //How many moves (either camera or slice)have been recorded
  fscanf(fp, "%d", &n);

  float x,y,z,rx,ry,rz,angle,asratio,nnear, ffar, focal, hangle;

  int i;

  //for each of those moves
  //read indices info stack : objectType (camera or slice?),starting and arriving point to replay
  for(i = 0; i < n; i++)
  {
    int *index = new int[3];
    fscanf(fp,"%d %d %d",
      &index[0],&index[1],&index[2]);

    m_indices.push_back(index);
  }

  //Read how many camera position has been recorded and camera info for each of those position
  fscanf(fp, "%d", &n);
  CameraState camera;
  for(i = 0; i < n; i++)
  {
    fscanf(fp,"%f %f %f %f %f %f %f %f %f %f %f %f",
      &x,&y,&z,&rx,&ry,&rz,&angle,&asratio,
      &nnear, &ffar, &focal, &hangle);

    camera.position = SbVec3f(x,y,z);
    camera.rotAxis = SbVec3f(rx,ry,rz);
    camera.rotAngle = angle;
    camera.aspectRatio = asratio;
    camera.nearDistance = nnear;
    camera.farDistance = ffar;
    camera.focalDistance = focal;
    camera.heightAngle = hangle;
    m_cameraState.push_back(camera);

  }

  //Read how many ortho slice position has been recorded and slice state info for each of those position
  fscanf(fp, "%d", &n);
  OrthoSliceState orthoSliceState;
  fscanf(fp,"%d", &m_numOrthoSlice);
  for(i = 0; i < n; i++)
  {
    fscanf(fp,"%d %d %d %d %d %d %d",&orthoSliceState.alphaUse,&orthoSliceState.axis,
                                  &orthoSliceState.clipping,&orthoSliceState.clippingSide,
                                  &orthoSliceState.interpolation,&orthoSliceState.sliceNumber,
                                  &orthoSliceState.currentSlice);
    m_orthoSliceState.push_back(orthoSliceState);
  }

  //Read how many oblique slice position has been recorded and slice state info for each of those position
  fscanf(fp, "%d", &n);
  ObliqueSliceState obliqueSliceState;
  float distanceFromOrigin=0;
  float xnorm,ynorm,znorm;xnorm=ynorm=znorm=0;
  for(i = 0; i < n; i++)
  {
    fscanf(fp,"%d %d %f %f %f %f",&obliqueSliceState.alphaUse,
                         &obliqueSliceState.interpolation,
                         &xnorm,&ynorm,&znorm,&distanceFromOrigin);

    SbVec3f normal(xnorm,ynorm,znorm);
    SbPlane plane(normal,distanceFromOrigin);
    obliqueSliceState.plane = plane;
    m_obliqueSliceState.push_back(obliqueSliceState);
  }

  //read dialog state
  fscanf(fp, "%d", &n);
  VolRendDialogState volRendDialogState;
  volRendDialogState.obliqueClipping=volRendDialogState.obliqueSliceVisibility=volRendDialogState.orthoClipping
    =volRendDialogState.orthoSliceVisibility=0;
  for(i = 0; i < n; i++)
  {
    fscanf(fp, "%d %d %d %d", &volRendDialogState.obliqueClipping,
                                  &volRendDialogState.obliqueSliceVisibility,
                                  &volRendDialogState.orthoClipping,
                                  &volRendDialogState.orthoSliceVisibility);
    m_dialogState.push_back(volRendDialogState);
  }

  //read time stamps
  fscanf(fp, "%d", &n);
  TimeStampState timeStamp;
  for(i = 0; i < n; i++)
  {
    float temp = 0;
    fscanf(fp, "%f", &temp);
    timeStamp.elapsedTime = temp;
    m_timeStamp.push_back(timeStamp);
  }

  fclose(fp);
}

void VVizBenchmark::writeTrajectory(char* filename)
{
  if(!m_outputFile)
  {
    if(m_enableFeedback)SoDebugError::postWarning("VVizBenchmark::writeTrajectory",
      "No output file specified.  Write into C:/VVizBenchmark.dat");
    m_outputFile = new char[20];
    sprintf(m_outputFile,"%s",filename);
  }
  FILE * fp = fopen(m_outputFile, "w");
  if(!fp)
  {
    SoDebugError::post("VVizBenchmark::writeTrajectory",
                    "Cannot open %s for write",m_outputFile);
    return;
  }
  if(m_enableFeedback)SoDebugError::postInfo("VVizBenchmark: write trajectory into %s\n", m_outputFile);

  //Write how many moves have been recorded
  int n = int(m_indices.size());
  fprintf(fp, "%d\n", n);

  int i;

  //for each of those moves:
  //indices info : object type, starting and arriving point
  for(i = 0; i < n; i++)
  {
    fprintf(fp, "%d %d %d \n",
      m_indices[i][0],m_indices[i][1],m_indices[i][2]);
  }

  n = int(m_cameraState.size());
  fprintf(fp, "%d\n", n);
   for(i = 0; i < n; i++)
  {
    fprintf(fp, "%f %f %f %f %f %f %f %f %f %f %f %f\n",
      m_cameraState[i].position[0],m_cameraState[i].position[1],m_cameraState[i].position[2],
      m_cameraState[i].rotAxis[0],m_cameraState[i].rotAxis[1],m_cameraState[i].rotAxis[2],
      m_cameraState[i].rotAngle,m_cameraState[i].aspectRatio,m_cameraState[i].nearDistance,m_cameraState[i].farDistance,
      m_cameraState[i].focalDistance,m_cameraState[i].heightAngle);
  }

  //Slice info
  //write number of ortho slice position recorded
  n = int(m_orthoSliceState.size());
  fprintf(fp, "%d\n", n);
  //write ortho slice state value for each of those position.
  fprintf(fp,"%d\n",m_numOrthoSlice);
  for(i = 0; i < n; i++)
  {
    fprintf(fp, "%d %d %d %d %d %d %d\n", m_orthoSliceState[i].alphaUse,m_orthoSliceState[i].axis,
                                        m_orthoSliceState[i].clipping,m_orthoSliceState[i].clippingSide,
                                        m_orthoSliceState[i].interpolation,m_orthoSliceState[i].sliceNumber,
                                        m_orthoSliceState[i].currentSlice);
  }

  //write number of oblique slice position recorded
  n = int(m_obliqueSliceState.size());
  fprintf(fp, "%d\n", n);
  //write oblique slice state value for each of those position.
  SbVec3f normal;
  for(i = 0; i < n; i++)
  {
    normal = m_obliqueSliceState[i].plane.getNormal();
    fprintf(fp, "%d %d %f %f %f %f \n", m_obliqueSliceState[i].alphaUse,
                               m_obliqueSliceState[i].interpolation,
                               normal[0],normal[1],normal[2],m_obliqueSliceState[i].plane.getDistanceFromOrigin());
  }

  //write dialog state
  n = int(m_dialogState.size());
  fprintf(fp, "%d\n", n);
  for(i = 0; i < n; i++)
  {
    fprintf(fp, "%d %d %d %d \n", m_dialogState[i].obliqueClipping,
                                  m_dialogState[i].obliqueSliceVisibility,
                                  m_dialogState[i].orthoClipping,
                                  m_dialogState[i].orthoSliceVisibility);
  }

  //write time stamp state
  n = int(m_timeStamp.size());
  fprintf(fp, "%d\n", n);
  for(i = 0; i < n; i++)
  {
    double temp = m_timeStamp[i].elapsedTime;
    fprintf(fp, "%g \n", temp);
  }

  fclose(fp);
  return;
}

/*------------------------------------------------------------------------------
                              Replay recorded test
------------------------------------------------------------------------------*/
void VVizBenchmark::replay(SbBool rewind, int numLoop, bool init)
{
  if(m_stop)return;

  if(!m_outputResult)
  {
    //create header
    time_t ltime;
    time( &ltime );

    const char *bfn = "no file";
    if (benchFileName) {
      int j;
      for (j = (int)strlen(benchFileName); j > 0; j--)
        if (benchFileName[j] == '/' || benchFileName[j] == '\\')
          break;
      bfn = benchFileName+j;
    }
    m_outputResult = fopen(outputResult, "w");
    fprintf( m_outputResult, "# ------- Benchmark for VolumeViz -------\n");
    fprintf( m_outputResult, ctime( &ltime ) );
    fprintf( m_outputResult, "Data played: %s\n", m_filename.toLatin1() );
    fprintf( m_outputResult, "Benchmark file used: %s\n", bfn );
    fprintf( m_outputResult, "\nResults:\n" );
  }//end header for output result file

  if(init)
  {
    m_volRenderOrthSliceSwitch->whichChild = SO_SWITCH_ALL;
    if(m_numOrthoSlice!=m_lastNumSlices)
    {
      //recreate set of slices
      createOrthoSlices(m_numOrthoSlice);
    }

    //initial ortho slices position:
    int *current = new int[m_lastNumSlices];
    SoRef<SoOrthoSlice> orthoSlice;
    if(m_orthoSliceState.size()){
      current[0] = m_orthoSliceState[0].currentSlice;
      orthoSlice = m_orthoTab[current[0]];
      orthoSlice->sliceNumber.setValue(m_orthoSliceState[0].sliceNumber);
      orthoSlice->axis.setValue(m_orthoSliceState[0].axis);
      orthoSlice->clipping.setValue(m_orthoSliceState[0].clipping);
      orthoSlice->clippingSide.setValue(m_orthoSliceState[0].clippingSide);
    }
    int sliceFound=1;
    for(unsigned int i=0;i<m_orthoSliceState.size();i++)
    {
      int count = 0;
      for(int j=0;j<sliceFound;j++)
      {
        if(m_orthoSliceState[i].currentSlice==current[j])
          count++;
      }
      if(!count)
      {
        current[sliceFound] = m_orthoSliceState[i].currentSlice;
        orthoSlice = m_orthoTab[current[sliceFound++]];
        orthoSlice->sliceNumber.setValue(m_orthoSliceState[i].sliceNumber);
        int axis = m_orthoSliceState[i].axis;
        orthoSlice->axis.setValue(axis);
        orthoSlice->clipping.setValue(m_orthoSliceState[i].clipping);
        orthoSlice->clippingSide.setValue(m_orthoSliceState[i].clippingSide);
        if(sliceFound == m_lastNumSlices)break;
      }
    }
    render();

    // visibility FALSE for dragger
    m_draggerVolRenderSwitch->whichChild = SO_SWITCH_NONE;
    m_draggerObliSwitch->whichChild = SO_SWITCH_NONE;
  }//end if (init)

  //Set dialog state :
  int orthoDragger, obliqueDragger;

  //save
  orthoDragger = m_draggerVolRenderSwitch->whichChild.getValue();
  obliqueDragger = m_draggerObliSwitch->whichChild.getValue();

  int n = int(m_indices.size());
  float totalTime;
  float avgTimePerFrame;
  float avgFrameRate = 0.0;
  int dialogStackIndex = 0;

  SbElapsedTime usage;
  for(int i = m_startIndex; i < n; i++)//numMove
  {
    if(m_indices[i][0] == TIME)//TIME STAMPS
    {
      //remember where we were in the algorithm before to return and come back later
      m_startIndex = i+1;
      m_numLoop = numLoop;

      //schedule the next time to call this function
      m_replaySensor->setTimeFromNow(SbTime(m_timeStamp[m_indices[i][1]].elapsedTime));
      m_replaySensor->schedule();

      return;

    }//end TIMESTAMP

    if(m_indices[i][0] == CAMERA)//camera
    {
      usage.reset();
      if(!rewind){
        for(int j=m_indices[i][1]; j<m_indices[i][1]+m_indices[i][2];j++)
        {
          m_camera->orientation.setValue(m_cameraState[j].rotAxis,m_cameraState[j].rotAngle);
          m_camera->aspectRatio.setValue(m_cameraState[j].aspectRatio);
          m_camera->nearDistance.setValue(m_cameraState[j].nearDistance);
          m_camera->farDistance.setValue(m_cameraState[j].farDistance);
          m_camera->focalDistance.setValue(m_cameraState[j].focalDistance);
          m_camera->heightAngle.setValue(m_cameraState[j].heightAngle);
          m_camera->position.setValue(m_cameraState[j].position);
          render();
        }
      }
      else{
        for(int j=m_indices[i][1]+m_indices[i][2]; j>m_indices[i][1];j--)
        {
          m_camera->orientation.setValue(m_cameraState[j].rotAxis,m_cameraState[j].rotAngle);
          m_camera->aspectRatio.setValue(m_cameraState[j].aspectRatio);
          m_camera->nearDistance.setValue(m_cameraState[j].nearDistance);
          m_camera->farDistance.setValue(m_cameraState[j].farDistance);
          m_camera->focalDistance.setValue(m_cameraState[j].focalDistance);
          m_camera->heightAngle.setValue(m_cameraState[j].heightAngle);
          m_camera->position.setValue(m_cameraState[j].position);
          render();
        }
      }
      double diff = usage.getElapsed();
      if(m_indices[i][2] > 1)
      {
        totalTime = (float)diff;
        avgTimePerFrame = totalTime/((float)m_indices[i][2]);
        avgFrameRate = 0.0;
        if(avgTimePerFrame > 0.0)
          avgFrameRate = 1/avgTimePerFrame;
        if(m_enableFeedback)
          SoDebugError::postInfo("VVizBenchmark","Camera move: %f frames/second\n",avgFrameRate);
        if(m_outputResult)fprintf(m_outputResult,"Camera move: %f frames/second\n",avgFrameRate);

      }
    }//end camera
    else if (m_indices[i][0] == ORTHO_SLICE)
    {
      usage.reset();
      if(!rewind){
        for (int j=m_indices[i][1]; j<m_indices[i][1]+m_indices[i][2];j++)
        {
          int currentSlice = m_orthoSliceState[j].currentSlice;
          SoRef<SoOrthoSlice> orthoSlice = m_orthoTab[currentSlice];
          orthoSlice->sliceNumber.setValue(m_orthoSliceState[j].sliceNumber);
          orthoSlice->alphaUse.setValue(m_orthoSliceState[j].alphaUse);
          orthoSlice->axis.setValue(m_orthoSliceState[j].axis);
          orthoSlice->clipping.setValue(m_orthoSliceState[j].clipping);
          orthoSlice->clippingSide.setValue(m_orthoSliceState[j].clippingSide);
          orthoSlice->interpolation.setValue(m_orthoSliceState[j].interpolation);
          render();
        }
      }
      else{
        for(int j=m_indices[i][1]+m_indices[i][2]; j>m_indices[i][1];j--)
        {
          //int currentSlice = m_orthoSliceState[j].currentSlice;
          m_orthoSlice->sliceNumber.setValue(m_orthoSliceState[j].sliceNumber);
          m_orthoSlice->alphaUse.setValue(m_orthoSliceState[j].alphaUse);
          m_orthoSlice->axis.setValue(m_orthoSliceState[j].axis);
          m_orthoSlice->clipping.setValue(m_orthoSliceState[j].clipping);
          m_orthoSlice->clippingSide.setValue(m_orthoSliceState[j].clippingSide);
          m_orthoSlice->interpolation.setValue(m_orthoSliceState[j].interpolation);
          render();
        }

      }
      double diff = usage.getElapsed(); // in sec
      if(m_indices[i][2] > 1)
      {
        totalTime = (float)diff;//@tochange-usage.Diff();
        avgTimePerFrame = totalTime/((float) m_indices[i][2]);
        avgFrameRate = 0.0;
        if(avgTimePerFrame > 0.0)
          avgFrameRate = 1/avgTimePerFrame;
        if(m_enableFeedback)SoDebugError::postInfo("VVizBenchmark","Ortho Slice move: %f frames/second\n",
          avgFrameRate);
        if(m_outputResult)fprintf(m_outputResult,"Ortho Slice move: %f frames/second\n",avgFrameRate);
      }
    }//end ortho
    else if (m_indices[i][0] == OBLIQUE_SLICE)
    {
      m_volRenderObliSliceSwitch->whichChild = SO_SWITCH_ALL;
      usage.reset();
      if(!rewind){
        for (int j=m_indices[i][1]; j<m_indices[i][1]+m_indices[i][2];j++)
        {
          SbPlane plane = m_obliqueSliceState[j].plane;
          m_obliqueSlice->plane.setValue(plane);
          m_obliqueSlice->alphaUse.setValue(m_obliqueSliceState[j].alphaUse);
          m_obliqueSlice->interpolation.setValue(m_obliqueSliceState[j].interpolation);
          render();
        }
      }
      else
      {
        for(int j=m_indices[i][1]+m_indices[i][2]; j>m_indices[i][1];j--)
        {
          SbPlane plane = m_obliqueSliceState[j].plane;
          m_obliqueSlice->plane.setValue(plane);
          m_obliqueSlice->alphaUse.setValue(m_obliqueSliceState[j].alphaUse);
          m_obliqueSlice->interpolation.setValue(m_obliqueSliceState[j].interpolation);
          render();
        }
      }
      double diff = usage.getElapsed(); // in sec
      if(m_indices[i][2] > 1)
      {
        totalTime = (float)diff;//@tochange-usage.Diff();
        avgTimePerFrame = totalTime/((float) m_indices[i][2]);
        avgFrameRate = 0.0;
        if(avgTimePerFrame > 0.0)
          avgFrameRate = 1/avgTimePerFrame;
        if(m_enableFeedback)SoDebugError::postInfo("VVizBenchmark","Oblique Slice move: %f frames/second\n",
          avgFrameRate);
        if(m_outputResult)fprintf(m_outputResult,"Oblique Slice move: %f frames/second\n",avgFrameRate);

      }
    }//end obli
    else if (m_indices[i][0] == DIALOG)
    {
      if(!rewind)
      {
        dialogStackIndex = m_indices[i][1];
        for (int j=m_indices[i][1]; j<m_indices[i][1]+m_indices[i][2];j++)
        {
          //clipping plane
          m_obliClipPlane->on = m_dialogState[j].obliqueClipping;
          //slice visibility
          m_volRenderObliSliceSwitch->whichChild = m_dialogState[j].obliqueSliceVisibility;
          //ortho clipping
          if(m_orthoSliceState.size()){
            int currentSlice = m_orthoSliceState[j].currentSlice;
            m_orthoTab[currentSlice]->clipping = m_dialogState[j].orthoClipping;
            //ortho visibility
            m_switchOrthoTab[currentSlice]->whichChild = m_dialogState[j].orthoSliceVisibility;
          }
        }
      }
      else
      {
        dialogStackIndex = m_indices[i][1];
        for(int j=m_indices[i][1]+m_indices[i][2]; j>m_indices[i][1];j--)
        {
          //clipping plane
          m_obliClipPlane->on = m_dialogState[j].obliqueClipping;
          //slice visibility
          m_volRenderObliSliceSwitch->whichChild = m_dialogState[j].obliqueSliceVisibility;
          //ortho clipping
          if(m_orthoSliceState.size()){
            int currentSlice = m_orthoSliceState[j].currentSlice;
            m_orthoTab[currentSlice]->clipping = m_dialogState[j].orthoClipping;
            //ortho visibility
            m_volRenderOrthSliceSwitch->whichChild = m_dialogState[j].orthoSliceVisibility;
          }
        }
      }//end rewind case
    }//end if case dialog
  }//end for num Moves
  if(m_enableFeedback)SoDebugError::postInfo("VVizBenchmark","--------\n\n\n\n");

  //restore initial dialog state :
  if(m_dialogState.size()){
    m_obliClipPlane->on = m_dialogState[dialogStackIndex].obliqueClipping;
    //slice visibility
    m_volRenderObliSliceSwitch->whichChild = m_dialogState[dialogStackIndex].obliqueSliceVisibility;
    //ortho clipping
    if(m_orthoSliceState.size()){
      int currentSlice = m_orthoSliceState[0].currentSlice;
      SoRef<SoOrthoSlice> orthoSlice = m_orthoTab[currentSlice];
      orthoSlice->clipping = m_dialogState[dialogStackIndex].orthoClipping;
      //ortho visibility
      m_volRenderOrthSliceSwitch->whichChild = m_dialogState[dialogStackIndex].orthoSliceVisibility;
    }
  }
  m_draggerVolRenderSwitch->whichChild = orthoDragger;
  m_draggerObliSwitch->whichChild = obliqueDragger;

  /*
  TODO : On linux system, the fclose crashes the second time it is done...
  */
  if(m_outputResult)
    fclose(m_outputResult);

  //if looping, refire
  numLoop--;
  if(numLoop>0)
    play(FALSE,numLoop);

}
/*------------------------------------------------------------------------------
                              Record functions
------------------------------------------------------------------------------*/
//record timeStamp.
#define TIME_STAMP 0.1
void VVizBenchmark::checkTime(void)
{
  double elapsed = m_timer.getElapsed();
  if(elapsed>=TIME_STAMP)
  {
    //update indices
    int last = (int)m_indices.size()-1;
    if(last < 0 || m_indices[last][0] != TIME)
    {
      int *index = new int[3];
      index[0] = TIME;
      index[1] = (int)m_timeStamp.size();
      index[2] = 1;
      m_indices.push_back(index);
    }
    else
    {
      m_indices[last][2]++;
    }
    //record a timestamp
    TimeStampState timeStamp;
    timeStamp.elapsedTime = elapsed;
    m_timeStamp.push_back(timeStamp);
  }
  m_timer.reset();
}

void VVizBenchmark::recordCamera(SoPerspectiveCamera * camera)
{
  checkTime();
  int last = (int)m_indices.size()-1;
  if(last < 0 || m_indices[last][0] != CAMERA)
  {
    int *index = new int[3];
    index[0] = CAMERA;
    index[1] = (int)m_cameraState.size();
    index[2] = 1;
    m_indices.push_back(index);
  }
  else
  {
    m_indices[last][2]++;
  }

  // record camera info at time to
  SbVec3f axis;
  float angle;
  camera->orientation.getValue(axis,angle);
  CameraState cameraState;
  cameraState.aspectRatio = camera->aspectRatio.getValue();
  cameraState.farDistance = camera->farDistance.getValue();
  cameraState.focalDistance = camera->focalDistance.getValue();
  cameraState.heightAngle = camera->heightAngle.getValue();
  cameraState.nearDistance = camera->nearDistance.getValue();
  cameraState.position = camera->position.getValue();
  cameraState.rotAngle = angle;
  cameraState.rotAxis = axis;

  m_cameraState.push_back(cameraState);
}

void VVizBenchmark::recordOrthoSlice()
{
  checkTime();

  if(m_startRecording){
    recordCamera(GetCamera());
    m_startRecording=0;
  }
  int last = (int)m_indices.size()-1;
  if(last < 0 ||m_indices[last][0] != ORTHO_SLICE)
  {
    int *index = new int[3];
    index[0] = ORTHO_SLICE;
    index[1] = (int)m_orthoSliceState.size();
    index[2] = 1;

   m_indices.push_back(index);
  }
  else
  {
   m_indices[last][2]++;
  }

  OrthoSliceState orthoSliceState;

  orthoSliceState.alphaUse = m_orthoTab[m_currentSlice]->alphaUse.getValue();
  orthoSliceState.axis = m_orthoTab[m_currentSlice]->axis.getValue();
  orthoSliceState.clipping = m_orthoTab[m_currentSlice]->clipping.getValue();
  orthoSliceState.clippingSide = m_orthoTab[m_currentSlice]->clippingSide.getValue();
  orthoSliceState.interpolation = m_orthoTab[m_currentSlice]->interpolation.getValue();
  orthoSliceState.sliceNumber = m_orthoTab[m_currentSlice]->sliceNumber.getValue();
  orthoSliceState.currentSlice = m_currentSlice;

  m_orthoSliceState.push_back(orthoSliceState);
}

void VVizBenchmark::recordObliqueSlice()
{
  checkTime();

  if(m_startRecording){
    recordCamera(GetCamera());
    m_startRecording=0;
  }
  int last = (int)m_indices.size()-1;
  if(last < 0 ||m_indices[last][0] != OBLIQUE_SLICE)
  {
    int *index = new int[3];
    index[0] = OBLIQUE_SLICE;
    index[1] = (int)m_obliqueSliceState.size();
    index[2] = 1;

    m_indices.push_back(index);
  }
  else
  {
   m_indices[last][2]++;
  }

  ObliqueSliceState obliqueSliceState;
  obliqueSliceState.alphaUse = m_obliqueSlice->alphaUse.getValue();
  obliqueSliceState.plane = m_obliqueSlice->plane.getValue();
  obliqueSliceState.interpolation = m_obliqueSlice->interpolation.getValue();

  m_obliqueSliceState.push_back(obliqueSliceState);
}

void VVizBenchmark::recordDialogState(int orthoSliceVisibility,
                                      int obliqueSliceVisibility,
                                      SbBool obliqueClipping,
                                      SbBool orthoClipping, int currentSlice)
{

  int last = (int)m_indices.size()-1;
  if(last < 0 ||m_indices[last][0] != DIALOG)
  {
    int *index = new int[3];
    index[0] = DIALOG;
    index[1] = (int)m_dialogState.size();
    index[2] = 1;

   m_indices.push_back(index);
  }
  else
  {
   m_indices[last][2]++;
  }

  VolRendDialogState volRendDialogState;
  volRendDialogState.orthoSliceVisibility = orthoSliceVisibility;
  volRendDialogState.obliqueSliceVisibility = obliqueSliceVisibility;
  volRendDialogState.obliqueClipping = obliqueClipping;
  volRendDialogState.orthoClipping = orthoClipping;
  volRendDialogState.currentSlice  = currentSlice;

  m_dialogState.push_back(volRendDialogState);
}

// Consolidate all render requests in one place
void VVizBenchmark::render()
{
#ifndef CAVELIB
  m_viewer->render();
#endif
}


