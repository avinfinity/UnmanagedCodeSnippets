/*=======================================================================
 *** THE CONTENT OF THIS WORK IS PROPRIETARY TO FEI S.A.S, (FEI S.A.S.),            ***
 ***              AND IS DISTRIBUTED UNDER A LICENSE AGREEMENT.                     ***
 ***                                                                                ***
 ***  REPRODUCTION, DISCLOSURE,  OR USE,  IN WHOLE OR IN PART,  OTHER THAN AS       ***
 ***  SPECIFIED  IN THE LICENSE ARE  NOT TO BE  UNDERTAKEN  EXCEPT WITH PRIOR       ***
 ***  WRITTEN AUTHORIZATION OF FEI S.A.S.                                           ***
 ***                                                                                ***
 ***                        RESTRICTED RIGHTS LEGEND                                ***
 ***  USE, DUPLICATION, OR DISCLOSURE BY THE GOVERNMENT OF THE CONTENT OF THIS      ***
 ***  WORK OR RELATED DOCUMENTATION IS SUBJECT TO RESTRICTIONS AS SET FORTH IN      ***
 ***  SUBPARAGRAPH (C)(1) OF THE COMMERCIAL COMPUTER SOFTWARE RESTRICTED RIGHT      ***
 ***  CLAUSE  AT FAR 52.227-19  OR SUBPARAGRAPH  (C)(1)(II)  OF  THE RIGHTS IN      ***
 ***  TECHNICAL DATA AND COMPUTER SOFTWARE CLAUSE AT DFARS 52.227-7013.             ***
 ***                                                                                ***
 ***                   COPYRIGHT (C) 1996-2014 BY FEI S.A.S,                        ***
 ***                        MERIGNAC, FRANCE                                        ***
 ***                      ALL RIGHTS RESERVED                                       ***
**=======================================================================*/
/*=======================================================================
** Author      : VSG (MMM YYYY)
**=======================================================================*/

// volCavelibFuncs
//
// CAVELib specific functions for VolRend demo
//
// "main" is in this file when the CAVELIB macro is defined
//
// volumeVizInit in mainVolVizdemo.cxx just creates the scene graph


// CaveHandleEvents
//
// A basic CAVELib + Open Inventor example
//
//----------------------------------------------------------------------
//
//  In the spirit of Inventor Mentor chapter 15, examples 1 - 3.
//
//  Process Open Inventor events so draggers (etc) will work.
//
//  By default loads a file (derived from Mentor example 15.2) that uses
//  3 translate1Draggers to change the x, y, and z of a translation.
//
//  Please see the example program CaveReadFile for discussion of reading
//  and displaying Open Inventor files. This example extends CaveReadFile.
//
//  This example shows how to:
//
//    1) Implement a "pointer" for the wand input device.
//       See new function: InventorAddWand()
//
//    2) Update the wand pointer geometry when the wand moves.
//       See new function: InventorUpdateWand()
//
//    3) Convert changes in wand state into Inventor events.
//       When a button is pressed we generate an SoControllerButtonEvent
//       and when the wand moves we generate an SoTrackerEvent.
//       See new function: InventorHandleEvents()
//
//  This example does not implement any "navigation".  Navigation is a
//  CAVELib specific topic.  Consult the CAVELib docs and examples.
//
//----------------------------------------------------------------------

#ifdef CAVELIB

// Define this macro to use caveTime instead of OIV's realTime
#define USE_CAVETIME 1

// Header files for Open Inventor
#include <Inventor/SoDB.h>
#include <Inventor/SoInput.h>
#include <Inventor/SoInteraction.h>

#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoTransformSeparator.h>
#include <Inventor/nodes/SoRotation.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTransform.h>

#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/elements/SoGLLazyElement.h>
#include <Inventor/elements/SoProjectionMatrixElement.h>
#include <Inventor/elements/SoViewingMatrixElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/fields/SoSFTime.h>
#include <Inventor/sensors/SoNodeSensor.h>
#include <Inventor/nodes/SoWWWInline.h>

    /////////////////////////////////////
    // BEGIN NEW CODE FOR THIS EXAMPLE //
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/nodes/SoLightModel.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/nodes/SoPickStyle.h>
#include <Inventor/nodes/SoVertexProperty.h>
#include <Inventor/nodes/SoSwitch.h>

#include <Inventor/actions/SoHandleEventAction.h>
#include <Inventor/events/SoTrackerEvent.h>
#include <Inventor/events/SoControllerButtonEvent.h>

#include <Inventor/nodes/SoWWWInline.h>
    // END NEW CODE FOR THIS EXAMPLE //
    ///////////////////////////////////

// Header file for CAVELib
#include <cave_ogl.h>

// Sleep n milliseconds
#ifdef _WIN32
#define SLEEP(n) (Sleep(n))
#else
#include <unistd.h>
#define SLEEP(n) (usleep(n*1000))
#endif

#ifndef M_PI
#  define M_PI           3.14159265358979323846
#endif


//---------------------------------------------------------------------
// VolRend global variables

extern SoSwitch  *m_VolRenderSwitch;
extern SoSwitch  *m_volRenderOrthSliceSwitch;
extern SoSwitch  *m_volROISwitch;
extern SoSwitch **m_switchOrthoTab;


//---------------------------------------------------------------------
// Global info for Open Inventor (exactly one copy)

SoRotation *m_headlightRotation = NULL;

    /////////////////////////////////////
    // BEGIN NEW CODE FOR THIS EXAMPLE //
SoVertexProperty        *m_wandGeometry = NULL;       // Wand pointer geometry
SoTrackerEvent          *m_trackerEvent = NULL;       // Wand tracker event
SoControllerButtonEvent *m_buttonEvent  = NULL;       // Wand button event
SoHandleEventAction     *m_handleEventAction = NULL;  // To process wand events
    // END NEW CODE FOR THIS EXAMPLE //
    ///////////////////////////////////

//mmh
// Static vars and constants for nav and UI
//TODO: cleanup
static SoTransform *m_sceneTransform = NULL;
static float m_rotateIncr = 0.01f;
static float m_panIncr = 0.01f;
static float m_rotateAngle = 0;
static float m_panX = 0;
static float m_panY = 0;
static float m_panZ = 0;
static float m_autoRotate = 0;

//---------------------------------------------------------------------
// Per-pipe info for Open Inventor (one copy per pipe)

typedef struct {
    SoGLRenderAction *renderAction;
} oivPipeInfo_t;

oivPipeInfo_t *oivPipeInfo = NULL;

//---------------------------------------------------------------------
// Forward decls

void InventorInit(int, const char **);
void InventorThreadInit( SoSeparator *sceneRoot);
void InventorFrameUpdate( SoSeparator *sceneRoot );
void InventorDraw( SoSeparator *sceneRoot );
SoSeparator *InventorReadFile(const char *filename);
void InventorAddHeadlight( SoSeparator *sceneRoot );
void InventorFitScene( SoSeparator *sceneRoot, SoSeparator *geometry );

    /////////////////////////////////////
    // BEGIN NEW CODE FOR THIS EXAMPLE //
void InventorAddWand( SoSeparator *sceneRoot );
void InventorUpdateWand();
void InventorHandleEvents( SoSeparator *sceneRoot );
    // END NEW CODE FOR THIS EXAMPLE //
    ///////////////////////////////////

// Function to create VolViz scene graph (mainVolVizDemo.cxx)

extern SoSeparator *volumeVizInit( int, char** );


//---------------------------------------------------------------------
// Configuration constants

const float WAND_RAY_LENGTH = 5.0f;

///////////////////////////////////////////////////////////////////////

void sceneNodeSensorCallback(void *data, SoSensor *sensor)
{
  SoGLRenderAction *action = (SoGLRenderAction *)data; 
  action->updateTranspList(sensor);
}
///////////////////////////////////////////////////////////////////////

int
main(int argc, char **argv) 
{
    // Special initialization for UNIX machines
#ifndef _WIN32
    XInitThreads();
#if defined(sun)
    glXInitThreadsSUN();
#endif
#endif

    //_putenv( "CAVEDEBUGCONFIG=VERBOSE" );
  
    // Configure CAVE based on CAVE config file and/or command
    // line arguments. CAVELib arguments are removed after they
    // are parsed.
    CAVEConfigure( &argc, argv, NULL );

    // Global app initialization (before starting render threads)
    InventorInit( argc, (const char**)argv );

    //-----------------------------------------------------------------
    // Make the scene graph root node
    SoSeparator *sceneRoot = new SoSeparator;
    sceneRoot->ref();
    sceneRoot->setName( "SceneRoot" ); // for debugging

    // Set render thread init callback
    // (will be called once for each render thread)
    CAVEInitApplication( (CAVECALLBACK)InventorThreadInit, 1, sceneRoot );

    // If we are creating a headlight there's no point in trying to
    // render cache the top level Separator.  The cache would always
    // be blown when the headlight is updated.
    sceneRoot->renderCaching = SoSeparator::OFF;

#if 0
    // Read the file
    char *filename = (char*)"data/sliderBox.iv";
    if (argc > 1)
      filename = argv[1];

    SoSeparator *geometry = InventorReadFile( filename );
#else
    SoSeparator *geometry = volumeVizInit( argc, argv );
#endif

    if (geometry == NULL) {
      printf( "*** Failed to create scene graph...\n" );
      return 0;
    }

    geometry->ref();
    geometry->setName( "FileRoot" ); // for debugging

    /////////////////////////////////////
    // BEGIN NEW CODE FOR THIS EXAMPLE //

    // Add a pointer for the wand
    InventorAddWand( sceneRoot );

    // END NEW CODE FOR THIS EXAMPLE //
    ///////////////////////////////////

    // Add a headlight (this should go before the "fit" transforms)
    InventorAddHeadlight( sceneRoot );

    // Add some transforms so geometry will fit in CAVE
    InventorFitScene( sceneRoot, geometry );

    // Add geometry to scene graph
    sceneRoot->addChild( geometry );
    //-----------------------------------------------------------------

    // Set render thread frame update callback
    // (each render thread will call exactly once per frame)
    CAVEFrameFunction((CAVECALLBACK)InventorFrameUpdate, 1, sceneRoot );

    // Set render thread draw callback
    // (each render thread will call one or times per frame for drawing)
    // Pass the root of the scene graph to be drawn
    CAVEDisplay( (CAVECALLBACK)InventorDraw, 1, sceneRoot );

    // Init CAVELib (starts the rendering threads etc)
    CAVEInit();

    // For distributed CAVELib apps
    //  - Only the master node should check for the ESC key. 
    //  - Check for CAVESync->Quit, which is set by a slave nodes
    //    display thread when it loses connectio to the master node
    // For an app that will not be run distributed, can simply
    // check for the ESC key    
    while(!(CAVEgetbutton(CAVE_ESCKEY) && CAVEDistribMaster()) && 
		  !(CAVESync->Quit) ) {
      CAVEUSleep(10);
    }

    CAVEExit();
    return 0;
}

    /////////////////////////////////////
    // BEGIN NEW CODE FOR THIS EXAMPLE //

///////////////////////////////////////////////////////////////////////
//
// Create simple geometry to represent wand pointer in scene
// (application will call this when creating the scene graph)

void
InventorAddWand( SoSeparator *sceneRoot )
{
    SoSeparator  *pWandSep   = new SoSeparator( 4 );
    SoPickStyle  *pPStyle    = new SoPickStyle;
    SoLightModel *pLModel    = new SoLightModel;
    SoDrawStyle  *pDStyle    = new SoDrawStyle;
    SoLineSet    *pLineSet   = new SoLineSet;

    // NOTE: This is the global variable updated in InventorUpdateWand
    m_wandGeometry = new SoVertexProperty;

    // Make wand unpickable and unaffected by lighting
    pPStyle->style = SoPickStyle::UNPICKABLE;
    pLModel->model = SoLightModel::BASE_COLOR;
    pDStyle->lineWidth = 2;

    m_wandGeometry->orderedRGBA.setValue( 0xFFCC33FF );
    m_wandGeometry->materialBinding = SoVertexProperty::PER_PART;
    m_wandGeometry->vertex.set1Value( 0, SbVec3f(0,0,0) );
    m_wandGeometry->vertex.set1Value( 1, WAND_RAY_LENGTH * SbVec3f(0,0,-1) );

    pLineSet->vertexProperty = m_wandGeometry;
    pLineSet->numVertices = 2;

    pWandSep->addChild( pPStyle );
    pWandSep->addChild( pLModel );
    pWandSep->addChild( pDStyle );
    pWandSep->addChild( pLineSet );

    sceneRoot->addChild( pWandSep );
}


///////////////////////////////////////////////////////////////////////
//
// Update wand pointer geometry to match current position/orientation
// (InventorHandleEvents will call this function when wand info changes)

void InventorUpdateWand()
{
    float wandPos[3], wandFront[3];
    CAVEGetPosition(CAVE_WAND, wandPos);
    CAVEGetVector  (CAVE_WAND_FRONT,wandFront);

    SbVec3f pt1(wandPos[0],wandPos[1],wandPos[2]);
    SbVec3f pt2(pt1);
    pt2 += WAND_RAY_LENGTH * SbVec3f(wandFront[0],wandFront[1],wandFront[2]);

    // NOTE: This is the global variable initialized in InventorAddWand
    if (m_wandGeometry) {
        m_wandGeometry->vertex.set1Value( 0, pt1 );
        m_wandGeometry->vertex.set1Value( 1, pt2 );
    }
}

///////////////////////////////////////////////////////////////////////
//
// Open Inventor event handler
// (InventorFrameUpdate will call this function to handle events)
//
// In this example we only check for button1 changes (because we know
// that's the only thing SoSelection, draggers, SoVRMLTouchSensor, etc
// actually care about).  The code is easily extended to more devices.

void
InventorHandleEvents( SoSeparator *sceneRoot )
{
    static SbVec3f lastWandPos(-1,-1,-1);
    static SbVec3f lastWandOri(-1,-1,-1); // Really 3 Euler angles!

    // Define tolerance for detecting change in wand position/orientation
    const float tolerance = 0.00001f;

    // Get change in wand button state (if any)
    int btn1 = CAVEButtonChange( 1 );

    // Get current wand tracker info
    SbVec3f wandPos, wandOri;
    CAVEGetPosition   ( CAVE_WAND, (float*)&wandPos );
    CAVEGetOrientation( CAVE_WAND, (float*)&wandOri ); // Euler angles
    wandOri *= (float)(M_PI/180);                      // Degrees to radians

    // Check if button 1 state changed since last time
    // 0 means no change, 1 means pressed, -1 means released
    if (btn1 != 0) {

        // Create a controller button change event
        // Store which button was pressed and the new state
        // Also store tracker info in case Inventor needs to do picking
        SoControllerButtonEvent *pEvent = m_buttonEvent;
        pEvent->setTime( SbTime::getTimeOfDay() );
        pEvent->setButton( SoControllerButtonEvent::BUTTON1 );
        pEvent->setState( (btn1 > 0) ? SoButtonEvent::DOWN : SoButtonEvent::UP );
        pEvent->setPosition3  ( wandPos );
        pEvent->setOrientation( wandOri[0], wandOri[1], wandOri[2] );

        // Send the event to the scene graph
        m_handleEventAction->setEvent( pEvent );
        m_handleEventAction->apply( sceneRoot );
    }

    // Check for significant change in wand position/orientation
    else if (! wandPos.equals(lastWandPos, tolerance) ||
             ! wandOri.equals(lastWandOri, tolerance)   ) {

        // Create a tracker change event
        // Store the tracker info
        SoTrackerEvent *pEvent = m_trackerEvent;
        pEvent->setTime( SbTime::getTimeOfDay() );
        pEvent->setPosition3  ( wandPos );
        pEvent->setOrientation( wandOri[0], wandOri[1], wandOri[2] );

        // Send the event to the scene graph
        m_handleEventAction->setEvent( pEvent );
        m_handleEventAction->apply( sceneRoot );

        // Also update the wand pointer geometry
        InventorUpdateWand();
    }

    // Remember the current tracker values
    lastWandPos = wandPos;
    lastWandOri = wandOri;

    // Reset data set transform
    if (CAVEgetbutton( CAVE_SPACEKEY )) {
      m_panX = m_panY = m_panZ = m_rotateAngle = 0;
      m_sceneTransform->translation.setValue( 0, 0, 0 );
      m_sceneTransform->rotation.setValue( 0, 0, 0, 0 );
    }

    // Autorotate
    else if (CAVEgetbutton( CAVE_AKEY )) {
      if (CAVEgetbutton(CAVE_LEFTSHIFTKEY))
        m_autoRotate = 0;
      else
        m_autoRotate++;
    }

    // Translate data set
    else if (CAVEgetbutton( CAVE_HKEY )) {
      m_panX -= CAVEgetbutton(CAVE_LEFTSHIFTKEY) ? 3 * m_panIncr : m_panIncr;
      m_sceneTransform->translation.setValue( m_panX, m_panY, m_panZ );
    }
    else if (CAVEgetbutton( CAVE_LKEY )) {
      m_panX += CAVEgetbutton(CAVE_LEFTSHIFTKEY) ? 3 * m_panIncr : m_panIncr;
      m_sceneTransform->translation.setValue( m_panX, m_panY, m_panZ );
    }
    else if (CAVEgetbutton( CAVE_JKEY )) {
      m_panY -= CAVEgetbutton(CAVE_LEFTSHIFTKEY) ? 3 * m_panIncr : m_panIncr;
      m_sceneTransform->translation.setValue( m_panX, m_panY, m_panZ );
    }
    else if (CAVEgetbutton( CAVE_KKEY )) {
      m_panY += CAVEgetbutton(CAVE_LEFTSHIFTKEY) ? 3 * m_panIncr : m_panIncr;
      m_sceneTransform->translation.setValue( m_panX, m_panY, m_panZ );
    }
    else if (CAVEgetbutton( CAVE_NKEY )) {
      m_panZ -= CAVEgetbutton(CAVE_LEFTSHIFTKEY) ? 3 * m_panIncr : m_panIncr;
      m_sceneTransform->translation.setValue( m_panX, m_panY, m_panZ );
    }
    else if (CAVEgetbutton( CAVE_MKEY )) {
      m_panZ += CAVEgetbutton(CAVE_LEFTSHIFTKEY) ? 3 * m_panIncr : m_panIncr;
      m_sceneTransform->translation.setValue( m_panX, m_panY, m_panZ );
    }

    // Rotate data set
    else if (CAVEgetbutton( CAVE_UKEY )) {
      m_rotateAngle -= CAVEgetbutton(CAVE_LEFTSHIFTKEY) ? 3 * m_rotateIncr : m_rotateIncr;
      m_sceneTransform->rotation.setValue( SbVec3f(0,1,0), m_rotateAngle );
    }
    else if (CAVEgetbutton( CAVE_IKEY )) {
      m_rotateAngle += CAVEgetbutton(CAVE_LEFTSHIFTKEY) ? 3 * m_rotateIncr : m_rotateIncr;
      m_sceneTransform->rotation.setValue( SbVec3f(0,1,0), m_rotateAngle );
    }

    // Slice visibility
    else if (CAVEgetbutton( CAVE_SKEY )) {
      int which =(CAVEgetbutton(CAVE_LEFTSHIFTKEY)) ? SO_SWITCH_NONE : SO_SWITCH_ALL;
      if (which != m_volRenderOrthSliceSwitch->whichChild.getValue())
        m_volRenderOrthSliceSwitch->whichChild = which;
    }

    // Volume visibility
    else if (CAVEgetbutton( CAVE_VKEY )) {
      int which =(CAVEgetbutton(CAVE_LEFTSHIFTKEY)) ? SO_SWITCH_NONE : SO_SWITCH_ALL;
      if (which != m_VolRenderSwitch->whichChild.getValue())
        m_VolRenderSwitch->whichChild = which;
    }

    // ROI/Manip visibility
    else if (CAVEgetbutton( CAVE_RKEY )) {
      int which =(CAVEgetbutton(CAVE_LEFTSHIFTKEY)) ? SO_SWITCH_NONE : SO_SWITCH_ALL;
      if (which != m_volROISwitch->whichChild.getValue())
        m_volROISwitch->whichChild = which;
    }

    if (m_autoRotate) {
      m_rotateAngle += m_autoRotate * m_rotateIncr;
      m_sceneTransform->rotation.setValue( SbVec3f(0,1,0), m_rotateAngle );
    }
}
    // END NEW CODE FOR THIS EXAMPLE   //
    /////////////////////////////////////

///////////////////////////////////////////////////////////////////////
//
// Read an Inventor or VRML file
//
// In this example we just read the file.  In a real application you
// might want to use an SoSearchAction or SoCallbackAction to check the
// resulting scene graph for cameras (which won't work well in a CAVE)
// or other interesting nodes.  You might also want to do things like
// optimize the model for display using an SoReorganizeAction.

SoSeparator *
InventorReadFile(const char *filename)
{
  // Open the input file
  SoInput mySceneInput;
  if (!mySceneInput.openFile(filename)) {
    fprintf(stderr, "Cannot open file %s\n", filename);
    return NULL;
  }
  
  // Read the whole file into the database
  SoSeparator *myGraph = SoDB::readAll(&mySceneInput);
  if (myGraph == NULL) {
    fprintf(stderr, "Problem reading file %s\n", filename);
    return NULL;
  } 
  
  mySceneInput.closeFile();
  return myGraph;
}

///////////////////////////////////////////////////////////////////////
//
// Create a headlight similar to an Open Inventor viewer
//
// Rotation will be updated on each frame so the light shines in the
// direction we are looking.  This is a useful lighting for looking at
// objects, but you might prefer to insert point lights.

void
InventorAddHeadlight( SoSeparator *sceneRoot )
{
    // Headlight must be in a Group, not a Separator, or the
    // light will not affect the rest of the scene graph.
    SoGroup *pHeadlightGroup = new SoTransformSeparator(2);

    SoDirectionalLight *pHeadlightNode = new SoDirectionalLight;
    pHeadlightNode->direction.setValue(SbVec3f(.2f, -.2f, -.9797958971f));

    // NOTE: This is the global variable updated in InventorFrameUpdate
    m_headlightRotation = new SoRotation;
    
    // ResetTransform node prevents rotation from affecting scene graph
    pHeadlightGroup->addChild( m_headlightRotation );
    pHeadlightGroup->addChild( pHeadlightNode );
    
    sceneRoot->addChild( pHeadlightGroup );
}

///////////////////////////////////////////////////////////////////////
//
// Scale and translate the scene to fit inside the CAVE
//
// Note: This will be a little more complicated if you want to allow
//       for rotating or scaling the scene about its geometric center.
//
// In this example we just compute a sphere that contains the bounding
// box of the geometry, scale the sphere to fit inside a CAVE, then
// move the sphere to be centered in Y (height) and half-way through
// the far (front) wall in Z (depth).  This is one of many possible
// strategies appropriate for looking at objects.

void
InventorFitScene( SoSeparator *sceneRoot, SoSeparator *geometry )
{
    // First get the bounding box for all the geometry
    SoGetBoundingBoxAction gba( SbViewportRegion(500,500) );
    gba.apply( geometry );
    SbBox3f bbox = gba.getBoundingBox();

    // Find a sphere that completely contains the geometry
    SbVec3f center = 0.5 * (bbox.getMin() + bbox.getMax());
    float radius = (bbox.getMax() - center).length();

    // Scale to "standard size" in CAVE coordinates
    const float STD_SIZE  = 10; // Max dimension of scene in CAVE units
//    float scale  = STD_SIZE / (2 * radius);

    float sceneSize = SoPreferences::getFloat( "VOLREND_caveSceneSize", STD_SIZE );
    float scale  = sceneSize / (2 * radius);

    // Move center of geometry to origin for nice rotations
    float xtrans1 = -(scale * center[0]);
    float ytrans1 = -(scale * center[1]);
    float ztrans1 = -(scale * center[2]);

    // Move center of geometry to center of far (front) wall
    SbVec3f scenePos = SoPreferences::getVec3f( "VOLREND_caveScenePos", SbVec3f(0,5,-5) );
    float xtrans2 = scenePos[0]; // 0;
    float ytrans2 = scenePos[1]; // (scale * radius);
    float ztrans2 = scenePos[2]; //-(scale * radius);

    // Add scene position (in CAVE space) transform
    SoTransform *pFitTransform2 = new SoTransform;
    pFitTransform2->translation = SbVec3f( xtrans2, ytrans2, ztrans2 );
    sceneRoot->addChild( pFitTransform2 );

    // Add transform for user interface
    m_sceneTransform = new SoTransform;
    sceneRoot->addChild( m_sceneTransform );

    // Add scale and center transform
    SoTransform *pFitTransform = new SoTransform;
    pFitTransform->scaleFactor = SbVec3f(scale,scale,scale);
    pFitTransform->translation = SbVec3f( xtrans1, ytrans1, ztrans1 );
    sceneRoot->addChild( pFitTransform );
}

////////////////////////////////////////////////////////////////////////////
////////////////////////   CAVE Functions  /////////////////////////////////
//
// Open Inventor global initialization
// (called before render threads created)

void
InventorInit( int argc, const char **argv )
{
    // Get number of pipes (rendering threads)
    int numPipes = CAVENumPipes();

    // Allocate per-pipe (per render thread) data
    oivPipeInfo = new oivPipeInfo_t[numPipes];
    memset( oivPipeInfo, 0, numPipes*sizeof(oivPipeInfo_t) );

    // Initialize Open Inventor
    // Be sure to call threadInit so MT support is enabled
    //
    // Note: SoDB initializes the "core" set of nodes.
    // Since we don't know what kind of files we might be reading,
    // also initialize nodekits and interaction classes.
    SoDB::threadInit();
    SoInteraction::threadInit();

    // Read inlines immediately
    SoWWWInline::setReadAsSoFile( TRUE );

#ifdef USE_CAVETIME
    // If using caveTime instead of OIV's realTime...
    //  - Initialize global realTime field to caveTime
    //  - Suppress updating of global realTime in processTimerQueue
    //    (it would update with time-since-dawn-of-unix)
    SoSFTime *realTime = (SoSFTime *) SoDB::getGlobalField("realTime");
    SbTime time2( *CAVETime );
    realTime->setValue( time2 );             // initialize global realTime
    SoDB::setRealTimeInterval( SbTime(0.) ); // suppress realTime update
#endif

    // Render caching
    // Set to 0 to disable render caching
    // Else set to (at least) number of pipes
    //     (because pipes usually cannot share display lists)
    if (numPipes > SoDB::getNumRenderCaches())
        SoDB::setNumRenderCaches( numPipes );

    /////////////////////////////////////
    // BEGIN NEW CODE FOR THIS EXAMPLE //

    // Create wand events to use later
    m_trackerEvent = new SoTrackerEvent;
    m_buttonEvent  = new SoControllerButtonEvent;

    // Create handle event action to use later (fake viewport for now)
    m_handleEventAction = new SoHandleEventAction(SbViewportRegion(500,500));

    // END NEW CODE FOR THIS EXAMPLE   //
    /////////////////////////////////////

    // Parameters for keyboard interface
    m_panIncr    = SoPreferences::getFloat( "VOLREND_cavePanIncr", 0.01f );
    m_rotateIncr = SoPreferences::getFloat( "VOLREND_caveRotIncr", 0.01f );
}

///////////////////////////////////////////////////////////////////////
//
// Open Inventor per-thread initialization
// (CAVELib will call this exactly once from each render thread)
//
void
InventorThreadInit()
{
    // Setup thread local storage for this thread
    SoDB::threadInit();
    SoInteraction::threadInit();

    // Create a render action for this thread
    SbViewportRegion vp;
    SoGLRenderAction *action = new SoGLRenderAction(vp);

    // Assume CAVELib does not try to setup display list sharing,
    // So each thread's render action should have a different cache context.
    int id = CAVEPipeNumber();
    action->setCacheContext( id );

    // Encourage render caching for all render threads
    // (this method should have a better name :-)
    action->setRenderingIsRemote( TRUE );

    // ?? Needed to ensure volume rendering is blended ??
    action->setTransparencyType( SoGLRenderAction::DELAYED_BLEND );

    // Store the render action to use later
    oivPipeInfo[id].renderAction = action;
}

///////////////////////////////////////////////////////////////////////
//
// Open Inventor frame update
// (CAVELib will call this exactly once per frame from each render thread)
//
void
InventorFrameUpdate( SoSeparator *sceneRoot )
{
    if (CAVEMasterDisplay()) {
      // Update Open Inventor's "realtime" (so sensors/engines will work)
      SoSFTime *realTime = (SoSFTime *) SoDB::getGlobalField("realTime");
#ifdef USE_CAVETIME
      SbTime time2( *CAVETime );
      realTime->setValue( time2 );
#else
      realTime->setValue(SbTime::getTimeOfDay());
#endif

      // Update the headlight rotation
      // (compute the rotation from default light direction to view direction)
      float eyevec[3];
      CAVEGetVector( CAVE_HEAD_FRONT, eyevec );

      // NOTE: This is the global variable initialized in InventorAddHeadlight
      SbRotation camrot( SbVec3f(0,0,-1),SbVec3f(eyevec[0], eyevec[1], eyevec[2]) );
      m_headlightRotation->rotation.setValue( camrot );

      /////////////////////////////////////
      // BEGIN NEW CODE FOR THIS EXAMPLE //

      // Check for wand changes that should trigger Inventor events.
      // (sceneRoot is needed to apply the HandleEventAction)
      InventorHandleEvents( sceneRoot );

      // If application has any time sensors, make this call to process them.
      SoDB::getSensorManager()->processTimerQueue();

      // In case there are tasks scheduled in the delay queue or idle queue,
      // make this call.  Note SoVRMLTimeSensor nodes that are activated and
      // deactivated will not reset until the idle queue is processed.
      SoDB::getSensorManager()->processDelayQueue(TRUE);

      // END NEW CODE FOR THIS EXAMPLE   //
      /////////////////////////////////////

      // Sync with other display threads
      CAVEDisplayBarrier();
    }
    else {
      // Non-master threads must wait for master to do its thing
      CAVEDisplayBarrier();
    }
}

///////////////////////////////////////////////////////////////////////
//
// Open Inventor frame draw
// (CAVELib will this one or more times per from from each render thread)
//
void
InventorDraw( SoSeparator *sceneRoot )
{
    int id = CAVEPipeNumber();

    // Push all OpenGL attributes
    // (so Open Inventor will not affect objects drawn by CAVELib)
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    // Clear window
    glClearColor( 0.2f ,0.4f ,0.5f, 1 );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
    // Get the Open Inventor render action for this thread.
    SoGLRenderAction *pAction = oivPipeInfo[id].renderAction;

    // Viewport
    // SoGLRenderAction always sets the SoViewportRegionElement.
    // Always set up the viewport correctly (both window size
    // and actual viewport size) in case the size has changed.
    int vpx,vpy,vpwidth,vpheight;
    int wx,wy,wwidth,wheight;
    CAVEGetViewport( &vpx, &vpy, &vpwidth, &vpheight );
    CAVEGetWindowGeometry( &wx, &wy, &wwidth, &wheight );
    SbViewportRegion vp( wwidth, wheight ); // Initialize with window size
    vp.setViewportPixels( vpx, vpy, vpwidth, vpheight );
    pAction->setViewportRegion( vp );

    // Open Inventor traversal state

    // First make sure the traversal state has been created, so we can
    // pre-load some state elements with values already set by CAVELib.
    SoState *pState = pAction->getState();
    if (pState == NULL) {
        pAction->setUpState();
        pState = pAction->getState();
    }

    // Traversal state Part 1: Material properties
    //
    // During traversal Open Inventor tracks the OpenGL state to avoid
    // unnecessary attribute setting.  However it also remembers OpenGL
    // state between traversals, which is not valid here because we're
    // pushing and popping the OpenGL state.  Reset the state tracker.
    SoGLLazyElement *pLazyElem = SoGLLazyElement::getInstance( pState );
    pLazyElem->reset( pState, SoLazyElement::ALL_MASK );

    // Traversal state Part 2: View volume
    //
    // Get the head position, orientation and projection info from CAVELib.
    // Then set the Inventor view volume element using the info from CAVELib.
    //
    // Note: Open Inventor's render caching algorithm needs to track which
    //       node set each element.  Since we're setting at the top of the
    //       scene graph, we'll pretend it was "sceneRoot" that did it.
    float xeye,yeye,zeye;
    float eyevec[3];
    float frustum[6];
    float viewmat[4][4];
    CAVEGetEyePosition( CAVEEye, &xeye, &yeye, &zeye );
    CAVEGetVector( CAVE_HEAD_FRONT, eyevec );
    CAVEGetProjection( CAVEWall, CAVEEye, frustum, viewmat );

    SbRotation camrot( SbVec3f(0,0,-1),SbVec3f(eyevec[0], eyevec[1], eyevec[2]) );
    SbViewVolume viewVol;
    viewVol.frustum( frustum[0], frustum[1], frustum[2], frustum[3], frustum[4], frustum[5] );
    viewVol.rotateCamera( camrot );
    viewVol.translateCamera( SbVec3f(xeye, yeye, zeye) );
    SoViewVolumeElement::set( pState, sceneRoot, viewVol );

    // Traversal state Part 3: ModelView and Projection matrices
    //
    // CAVELib has already sent ModelView and Projection matrices to OpenGL.
    // Initialize Inventor's current matrix elements with the current matrices.
    SbMatrix viewMatrix, projMatrix;
    glGetFloatv( GL_MODELVIEW_MATRIX , (float*)viewMatrix );
    glGetFloatv( GL_PROJECTION_MATRIX, (float*)projMatrix );
    SoViewingMatrixElement::set(    pState, sceneRoot, viewMatrix, FALSE );
    SoProjectionMatrixElement::set( pState, sceneRoot, projMatrix, FALSE );

    // This additional setting is required if any nodes do view culling
    // (we know that VolumeViz5 does this at several levels)
    SoModelMatrixElement::setCullMatrix( pState, sceneRoot, viewVol.getMatrix());//mmh

    // Render caching
    // Since we're pretending that sceneRoot set the traversal state elements
    // above, the render caching algorithm must think this node has changed.
    sceneRoot->touch();

    // Normal vectors
    // If input file doesn't have unit vectors we'll get screwy lighting.
    // Open Inventor automatically enables GL_NORMALIZE, but only once.
    // Since we're inside a push/pop, we'll have to do it every time.
    glEnable( GL_NORMALIZE );

    // Render
    pAction->apply( sceneRoot );

    // Restore the OpenGL attributes
    glPopAttrib();
}

#endif //CAVELIB


