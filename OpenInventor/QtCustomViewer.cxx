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
** Author      : Tristan Mehamli (Jan 2009)
**=======================================================================*/

#include <QBitmap>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtCore/QEvent>

#include "QtCustomViewer.h"

#include <Inventor/SoInput.h>
#include <Inventor/helpers/SbFileHelper.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/projectors/SbSphereSheetProjector.h>
#include <Inventor/Qt/SoQt.h>
#include <Inventor/Qt/viewers/SoQtCursors.h>
#include <Inventor/Qt/viewers/SoQtIcons.h>
#include <Inventor/sensors/SoFieldSensor.h>


#define HEIGHT 450
#define PBUTTON_SIZE 28
#define SLIDER_LONG 50
#define WIDTH  450
#define WHEEL_DELTA 120
#define WHEEL_DRAG 5

static SbBool s_firstDrag = TRUE;
static int s_lwdValue = 0;
static int s_rwdValue = 0;
static int s_bwdValue = 0;

//---------------------------------------------------------------
class MyInput : public SoInput
{
public:
  MyInput( QProgressBar* pgb ) : SoInput() 
  {
    m_pgbPtr = pgb;
  }

  ~MyInput() {};

  virtual void updateReadPercent( double readPercentage )
  {
    m_pgbPtr->setValue( int(readPercentage) );
  }

private:
  QProgressBar* m_pgbPtr;
};

//---------------------------------------------------------------
QtCustomViewer::QtCustomViewer( QWidget* parent )
: QWidget( parent )
{
  setAttribute( Qt::WA_DeleteOnClose );
  resize( WIDTH, HEIGHT );
  QVBoxLayout* vblcv = new QVBoxLayout();

  SoQt::init( this );

  m_guiAlgo = new SoGuiAlgoViewers();
  m_guiAlgo->setViewerType( SoGuiAlgoViewers::EXAMINER );
  m_renderArea = new SoQtRenderArea( this, "CVRenderArea", true, true, true, m_guiAlgo );
  m_renderArea->setEventCallback( QtCustomViewer::renderAreaEventCB, this ); // Register this callback to recieve events from the render area
  m_zoomSensor = NULL;
  m_currentViewerMode = IDLE_MODE;
  m_lastMotionTime = SbTime::zero();
  m_toggledCameraType = false;
  m_altSwitchBack = false;
  m_boxZoom = false;
  m_boxSelection = false;

  vblcv->addWidget( m_renderArea->getWidget() );
  setLayout( vblcv );

  setupRemoteControl();
  setupCursors();
  setupPopupMenu();
}

//---------------------------------------------------------------
QtCustomViewer::~QtCustomViewer()
{
}

//---------------------------------------------------------------
void 
QtCustomViewer::closeEvent( QCloseEvent* /*unused*/ )
{
  mb_quit();
}

//---------------------------------------------------------------
void 
QtCustomViewer::keyPressEvent( QKeyEvent* keyPress )
{
  // Add processPickEvent in case key must be processed by the scene graph

  switch (keyPress->key())
  {
  case Qt::Key_Escape:
    {
      if ( m_guiAlgo->isViewing() )
      {
        m_pick->setChecked( true );
        pb_pickMode();
      }
      else
      {
        m_view->setChecked( true );
        pb_viewingMode();
      }
    }
    break;
  case Qt::Key_Alt:
    {
      if ( !m_guiAlgo->isViewing() )
      {
        m_altSwitchBack = TRUE;
        pb_viewingMode();
      }
    }
    break;
  case Qt::Key_Home:
    pb_resetToHomePosition();
    break;
  case Qt::Key_S:
    pb_seekMode();
    break;
  default:
    break;
  }
}

//---------------------------------------------------------------
void 
QtCustomViewer::keyReleaseEvent( QKeyEvent* keyRelease )
{
  // Add processPickEvent in case key must be processed by the scene graph

  if ( m_altSwitchBack && (keyRelease->key() == Qt::Key_Alt) ) 
  {
    // If Alt-key, then return to PICK (if we had switched)
    pb_pickMode();
    m_altSwitchBack = FALSE;  // Clear the flag
  }
}

//---------------------------------------------------------------
void 
QtCustomViewer::mousePressEvent( QMouseEvent* mousePress )
{
  if ( (mousePress->button() == Qt::RightButton) )
  {
    displayPopupMenu( QCursor::pos() );
    return;
  }
  
  if ( processPickEvents(mousePress) ) // Consumes the event if the viewer is in picking mode.
    return;

  else if ( (mousePress->button() == Qt::LeftButton) || (mousePress->button() == Qt::MidButton) )
  {
    SbVec2s raSize = m_guiAlgo->getGlxSize(); 
    SbVec2s locator;

    locator[0] = mousePress->x();
    locator[1] = raSize[1] - mousePress->y();
    m_guiAlgo->setCurrentMousePositionLocator( locator );

    if ( m_currentViewerMode == SEEK_MODE ) 
    {
      if ( !m_guiAlgo->isSeekMode() )
        switchViewerMode( mousePress->buttons() | mousePress->modifiers() ); // Prevents from being stuck in seek mode
      else
      {
        m_guiAlgo->seekToPoint( locator );
        return;
      }
    }
    else
      switchViewerMode( mousePress->buttons() | mousePress->modifiers() );

    m_guiAlgo->stopSpinAnimation();
    m_guiAlgo->interactiveCountInc();
  }
}

//---------------------------------------------------------------
void 
QtCustomViewer::mouseReleaseEvent( QMouseEvent* mouseRelease )
{
  if ( processPickEvents(mouseRelease) )
    return;

  SbVec2s raSize = m_guiAlgo->getGlxSize();
  SbVec2s locator;
    
  if ( (mouseRelease->button() == Qt::LeftButton) || (mouseRelease->button() == Qt::MidButton) )  
  {
    locator[0] = mouseRelease->x();
    locator[1] = raSize[1] - mouseRelease->y();
    m_guiAlgo->setCurrentMousePositionLocator( locator );

    if ( m_currentViewerMode == VIEW_MODE )
    { 
      if ( (SoDB::getCurrentTime() - m_lastMotionTime).getMsecValue() < 100 )
      {
        m_guiAlgo->startSpinAnimation();
        m_guiAlgo->interactiveCountInc();
      }
    }
    if ( m_currentViewerMode != SEEK_MODE )
      m_guiAlgo->interactiveCountDec();

    switchViewerMode( mouseRelease->buttons() | mouseRelease->modifiers() );
  }
}

//---------------------------------------------------------------
void 
QtCustomViewer::mouseMoveEvent( QMouseEvent* mouseMove )
{
  if ( processPickEvents(mouseMove) )
    return;

  SbVec2s raSize = m_guiAlgo->getGlxSize();
  SbVec2s locator = m_guiAlgo->getCurrentMousePositionLocator();

  SbVec2f newLocator;
  float mx = mouseMove->x() / float(raSize[0]); 
  float my = (raSize[1] - mouseMove->y()) / float(raSize[1]);
  newLocator.setValue( mx, my );

  switch (m_currentViewerMode) 
  {
  case VIEW_MODE:
    {  
      m_lastMotionTime = SoDB::getCurrentTime();
      m_guiAlgo->spinCamera( newLocator );
    }
    break;
  case PAN_MODE:
    m_guiAlgo->panCamera( newLocator );
    break;
  case DOLLY_MODE:
    m_guiAlgo->dollyCamera( SbVec2s(mouseMove->x(), raSize[1] - mouseMove->y()) );
    break;
  default:
    // not managed
    break;
  }
}

//---------------------------------------------------------------
void 
QtCustomViewer::wheelEvent( QWheelEvent* mouseWheel )
{
  mouseWheel->accept();

  float newValue = (mouseWheel->delta()) / WHEEL_DELTA;
  m_guiAlgo->mouseWheelMotion( newValue );
}

//---------------------------------------------------------------
bool 
QtCustomViewer::eventFilter( QObject* obj, QEvent* event )
{
  QWidget* widget = (QWidget*)obj;
  switch (event->type())
  {
  case QEvent::Close:
    if ( widget->isVisible() )
    {
      widget->hide();
      IDM_MPOPUP_HIDERC->setChecked( false );
    }
    else
    {
      widget->show();
      IDM_MPOPUP_HIDERC->setChecked( true );
    }
    return true;
  default:
    return false;
  }
}

//---------------------------------------------------------------
SbBool 
QtCustomViewer::renderAreaEventCB( void* userData, QEvent* anyevent )
{
  QtCustomViewer* customViewer = (QtCustomViewer*)userData;
 
  switch (anyevent->type())
  {
  case QEvent::KeyPress: // 6
    customViewer->keyPressEvent( (QKeyEvent*)anyevent );
    return TRUE;
  case QEvent::KeyRelease: // 7
    customViewer->keyReleaseEvent( (QKeyEvent*)anyevent );
    return TRUE;
  case QEvent::MouseButtonPress: // 2
    customViewer->mousePressEvent( (QMouseEvent*)anyevent );
    return TRUE;
  case QEvent::MouseButtonRelease: // 3
    customViewer->mouseReleaseEvent( (QMouseEvent*)anyevent );
    return TRUE;
  case QEvent::MouseMove: // 5
    customViewer->mouseMoveEvent( (QMouseEvent*)anyevent );
    return TRUE;
  case QEvent::Wheel: // 31
    customViewer->wheelEvent( (QWheelEvent*)anyevent );
    return TRUE;
  default:
    return FALSE; // In case other events than those above should be processed by the render area
  }
}

//---------------------------------------------------------------
void
QtCustomViewer::zoomSensorCB( void* p, SoSensor* /*unused*/ )
{
  QtCustomViewer* v = (QtCustomViewer*)p;
  if ( !v->isVisible() )
    return;

  // Finally update the zoom slider and text field if the value has changed
  float zoom = v->getCameraZoom();
  v->setZoomSliderPosition( zoom );
}

//---------------------------------------------------------------
void 
QtCustomViewer::displayPopupMenu( QPoint point )
{
  m_mainMenu->exec( point );
  m_guiAlgo->stopSpinAnimation();
}

//---------------------------------------------------------------
SbBool 
QtCustomViewer::processPickEvents( QEvent* anEvent )
{
  if ( !m_guiAlgo->isViewing() )
  {
    // Send the event to the scene graph if the viewing is off
    m_renderArea->setEventCallback( NULL, NULL ); // This will prevent the event to be sent again to QtCustomViewer
    m_renderArea->sendEvent( anEvent ); // Make the render area process the event
    m_renderArea->setEventCallback( QtCustomViewer::renderAreaEventCB, this );

    static int xPos1, yPos1, xPos2, yPos2;
    QMouseEvent* me = (QMouseEvent*)anEvent;
    static bool mouseDown = false;

    switch (anEvent->type()) // Process the event for the picking
    {
    case QEvent::MouseButtonPress:
      if ( me->button() == Qt::LeftButton )
      {
        mouseDown = true;
        xPos1 = me->x();
        yPos1 = me->y();
        xPos2 = xPos1 + 1;
        yPos2 = yPos1 + 1;
      }
      break;
    case QEvent::MouseButtonRelease:
      if ( me->button() == Qt::LeftButton )
      {
        if ( m_boxZoom )
          m_guiAlgo->doBoxZoom( xPos1, yPos1, xPos2, yPos2 ); // Otherwise box selection will be made
        mouseDown = false;
      }
      break;
    case QEvent::MouseMove:
      if ( mouseDown )
      {
        xPos2 = me->x();
        yPos2 = me->y();
      }
      break;
    default:
      break;
    }

    return TRUE;
  }

  return FALSE;
}

//---------------------------------------------------------------
void 
QtCustomViewer::setupRemoteControl()
{
  // Create the remote control
  m_remoteControl = new QWidget( this, Qt::Window );
  m_remoteControl->setAttribute( Qt::WA_DeleteOnClose);
  m_remoteControl->installEventFilter( this );

  // Setup the menu bar
  m_menuBar = new QMenuBar( m_remoteControl );
  QMenu* fileMenu = m_menuBar->addMenu( tr("&File") );
  fileMenu->addAction( tr("&Open"), this, SLOT(mb_openFile()) );
  fileMenu->addSeparator();
  fileMenu->addAction( tr("&Quit"), this, SLOT(mb_quit()) );
  QMenu* helpMenu = m_menuBar->addMenu( tr("&Help") );
  helpMenu->addAction( tr("&About"), this, SLOT(mb_about()) );

  // Setup the buttons
  QGroupBox* buttonGroupBox = new QGroupBox( tr("Controls"), m_remoteControl );
  QHBoxLayout* hblbgb = new QHBoxLayout();

  IDB_PUSH_PICK = QPixmap( (const char**) PICK_data );
  IDB_PUSH_VIEW = QPixmap( (const char**) VIEW_data );
  IDB_PUSH_HOME = QPixmap( (const char**) HOME_data );
  IDB_PUSH_SETHOME = QPixmap( (const char**) SETHOME_data );
  IDB_PUSH_VIEWALL = QPixmap( (const char**) VIEWALL_data );
  IDB_PUSH_SEEK = QPixmap( (const char**) SEEK_data );
  IDB_PUSH_PERSP = QPixmap( ( const char**)persp_data );
  IDB_PUSH_ORTHO = QPixmap( ( const char**)ortho_data );
  IDB_PUSH_UP = QPixmap( (SbFileHelper::expandString("$OIVHOME/src/Inventor/examples/Qt4/QtCustomViewer/Png/fh_blor28.png")).getString() );
  QBitmap maskUp = IDB_PUSH_UP.createMaskFromColor( Qt::white );
  IDB_PUSH_UP.setMask( maskUp );
  IDB_PUSH_DOWN = QPixmap( (SbFileHelper::expandString("$OIVHOME/src/Inventor/examples/Qt4/QtCustomViewer/Png/fb_blor28.png")).getString() );
  QBitmap maskDown = IDB_PUSH_DOWN.createMaskFromColor( Qt::white );
  IDB_PUSH_DOWN.setMask( maskDown );
  IDB_PUSH_LEFT = QPixmap( (SbFileHelper::expandString("$OIVHOME/src/Inventor/examples/Qt4/QtCustomViewer/Png/fg_blor28.png")).getString() );
  QBitmap maskLeft = IDB_PUSH_LEFT.createMaskFromColor( Qt::white );
  IDB_PUSH_LEFT.setMask( maskLeft );
  IDB_PUSH_RIGHT = QPixmap( (SbFileHelper::expandString("$OIVHOME/src/Inventor/examples/Qt4/QtCustomViewer/Png/fd_blor28.png")).getString() );
  QBitmap maskRight = IDB_PUSH_RIGHT.createMaskFromColor( Qt::white );
  IDB_PUSH_RIGHT.setMask( maskRight );
  IDB_PUSH_BOX_SELECTION = QPixmap( (SbFileHelper::expandString("$OIVHOME/src/Inventor/examples/Qt4/QtCustomViewer/Png/boxSelection.png")).getString() );
  QBitmap maskSelection = IDB_PUSH_BOX_SELECTION.createMaskFromColor( Qt::white );
  IDB_PUSH_BOX_SELECTION.setMask( maskSelection );
  IDB_PUSH_BOX_ZOOM = QPixmap( (SbFileHelper::expandString("$OIVHOME/src/Inventor/examples/Qt4/QtCustomViewer/Png/boxZoom.png")).getString() );
  QBitmap maskZoom = IDB_PUSH_BOX_ZOOM.createMaskFromColor( Qt::white );
  IDB_PUSH_BOX_ZOOM.setMask( maskZoom );

  m_pick = new QPushButton( buttonGroupBox );
  m_pick->setIconSize( QSize(PBUTTON_SIZE-2, PBUTTON_SIZE-2) );
  m_pick->setFixedSize( PBUTTON_SIZE, PBUTTON_SIZE );
  m_pick->setFocusPolicy( Qt::NoFocus );
  m_pick->setIcon( IDB_PUSH_PICK );
  m_pick->setCheckable( true );
  m_remoteControl->connect( m_pick, SIGNAL(clicked()), this, SLOT(pb_pickMode()) );

  m_view = new QPushButton( buttonGroupBox );
  m_view->setIconSize( QSize(PBUTTON_SIZE-2, PBUTTON_SIZE-2) );
  m_view->setFixedSize( PBUTTON_SIZE, PBUTTON_SIZE );
  m_view->setFocusPolicy( Qt::NoFocus );
  m_view->setIcon( IDB_PUSH_VIEW );
  m_view->setCheckable( true );
  m_remoteControl->connect( m_view, SIGNAL(clicked()), this, SLOT(pb_viewingMode()) );

  QPushButton* home = new QPushButton( buttonGroupBox );
  home->setIconSize( QSize(PBUTTON_SIZE-2, PBUTTON_SIZE-2) );
  home->setFixedSize( PBUTTON_SIZE, PBUTTON_SIZE );
  home->setFocusPolicy( Qt::NoFocus );
  home->setIcon( IDB_PUSH_HOME );
  m_remoteControl->connect( home, SIGNAL(clicked()), this, SLOT(pb_resetToHomePosition()) );

  QPushButton* setHome = new QPushButton( buttonGroupBox );
  setHome->setIconSize( QSize(PBUTTON_SIZE-2, PBUTTON_SIZE-2) );
  setHome->setFixedSize( PBUTTON_SIZE, PBUTTON_SIZE );
  setHome->setFocusPolicy( Qt::NoFocus );
  setHome->setIcon( IDB_PUSH_SETHOME );
  m_remoteControl->connect( setHome, SIGNAL(clicked()), this, SLOT(pb_saveHomePosition()) );

  QPushButton* viewAll = new QPushButton( buttonGroupBox );
  viewAll->setIconSize( QSize(PBUTTON_SIZE-2, PBUTTON_SIZE-2) );
  viewAll->setFixedSize( PBUTTON_SIZE, PBUTTON_SIZE );
  viewAll->setFocusPolicy( Qt::NoFocus );
  viewAll->setIcon( IDB_PUSH_VIEWALL );
  m_remoteControl->connect( viewAll, SIGNAL(clicked()), this, SLOT(pb_viewAll()) );

  m_seek = new QPushButton( buttonGroupBox );
  m_seek->setIconSize( QSize(PBUTTON_SIZE-2, PBUTTON_SIZE-2) );
  m_seek->setFixedSize( PBUTTON_SIZE, PBUTTON_SIZE );
  m_seek->setFocusPolicy( Qt::NoFocus );
  m_seek->setIcon( IDB_PUSH_SEEK );
  m_remoteControl->connect( m_seek, SIGNAL(clicked()), this, SLOT(pb_seekMode()) );

  m_cam = new QPushButton( buttonGroupBox );
  m_cam->setIconSize( QSize(PBUTTON_SIZE-2, PBUTTON_SIZE-2) );
  m_cam->setFixedSize( PBUTTON_SIZE, PBUTTON_SIZE );
  m_cam->setFocusPolicy( Qt::NoFocus );
  m_cam->setIcon( IDB_PUSH_PERSP );
  m_remoteControl->connect( m_cam, SIGNAL(clicked()), this, SLOT(pb_toggleCameraType()) );

  m_pbBoxZoom = new QPushButton( buttonGroupBox );
  m_pbBoxZoom->setFixedSize( PBUTTON_SIZE, PBUTTON_SIZE );
  m_pbBoxZoom->setFocusPolicy( Qt::NoFocus );
  m_pbBoxZoom->setIcon( IDB_PUSH_BOX_ZOOM );
  m_pbBoxZoom->setCheckable( true );
  m_pbBoxZoom->setEnabled( false );
  m_remoteControl->connect( m_pbBoxZoom, SIGNAL(clicked()), this, SLOT(pb_boxZoom()) );

  m_pbBoxSelection = new QPushButton( buttonGroupBox );
  m_pbBoxSelection->setFixedSize( PBUTTON_SIZE, PBUTTON_SIZE );
  m_pbBoxSelection->setFocusPolicy( Qt::NoFocus );
  m_pbBoxSelection->setIcon( IDB_PUSH_BOX_SELECTION );
  m_pbBoxSelection->setCheckable( true );
  m_pbBoxSelection->setEnabled( false );
  m_remoteControl->connect( m_pbBoxSelection, SIGNAL(clicked()), this, SLOT(pb_boxSelection()) );

  hblbgb->addWidget( m_pick );
  hblbgb->addWidget( m_view );
  hblbgb->addWidget( home );
  hblbgb->addWidget( setHome );
  hblbgb->addWidget( viewAll );
  hblbgb->addWidget( m_seek );
  hblbgb->addWidget( m_cam );
  hblbgb->addWidget( m_pbBoxZoom );
  hblbgb->addWidget( m_pbBoxSelection );
  hblbgb->addStretch( 1 );
  buttonGroupBox->setLayout( hblbgb );

  // Setup the camera interactions
  QGroupBox* cameraGroupBox = new QGroupBox( tr("Camera interactions"), m_remoteControl );
  QHBoxLayout* hblcgb = new QHBoxLayout();
 
  QGroupBox* leftTWGroupBox = new QGroupBox( tr("RotX"), cameraGroupBox );
  QVBoxLayout* vblltwgb = new QVBoxLayout();
  m_leftWheelUp = new QPushButton( leftTWGroupBox );
  m_leftWheelUp->setAutoRepeat( true );
  m_leftWheelDown = new QPushButton( buttonGroupBox );
  m_leftWheelDown->setAutoRepeat( true );
  m_leftWheelUp->setFixedSize( PBUTTON_SIZE, PBUTTON_SIZE );
  m_leftWheelUp->setIconSize( QSize(PBUTTON_SIZE-4, PBUTTON_SIZE-4) );
  m_leftWheelUp->setFocusPolicy( Qt::NoFocus );
  m_leftWheelUp->setIcon( IDB_PUSH_UP );
  m_leftWheelDown->setFixedSize( PBUTTON_SIZE, PBUTTON_SIZE );
  m_leftWheelDown->setIconSize( QSize(PBUTTON_SIZE-4, PBUTTON_SIZE-4) );
  m_leftWheelDown->setFocusPolicy( Qt::NoFocus );
  m_leftWheelDown->setIcon( IDB_PUSH_DOWN );
  m_remoteControl->connect( m_leftWheelUp, SIGNAL(clicked()), this, SLOT(leftWheelOther()) );
  m_remoteControl->connect( m_leftWheelDown, SIGNAL(clicked()), this, SLOT(leftWheelOther()) );
  m_remoteControl->connect( m_leftWheelUp, SIGNAL(released()), this, SLOT(leftWheelOther()) );
  m_remoteControl->connect( m_leftWheelDown, SIGNAL(released()), this, SLOT(leftWheelOther()) );
  m_remoteControl->connect( m_leftWheelUp, SIGNAL(pressed()), this, SLOT(leftWheelDrag()) );
  m_remoteControl->connect( m_leftWheelDown, SIGNAL(pressed()), this, SLOT(leftWheelDrag()) );
  vblltwgb->addWidget( m_leftWheelUp );
  vblltwgb->addWidget( m_leftWheelDown );
  leftTWGroupBox->setLayout( vblltwgb );

  QGroupBox* bottomTWGroupBox = new QGroupBox( tr("RotY"), cameraGroupBox );
  QHBoxLayout* hblbtwgb = new QHBoxLayout();
  m_bottomWheelLeft = new QPushButton( bottomTWGroupBox );
  m_bottomWheelLeft->setAutoRepeat( true );
  m_bottomWheelRight = new QPushButton( bottomTWGroupBox );
  m_bottomWheelRight->setAutoRepeat( true );
  m_bottomWheelLeft->setFixedSize( PBUTTON_SIZE, PBUTTON_SIZE );
  m_bottomWheelLeft->setIconSize( QSize(PBUTTON_SIZE-4, PBUTTON_SIZE-4) );
  m_bottomWheelLeft->setFocusPolicy( Qt::NoFocus );
  m_bottomWheelLeft->setIcon( IDB_PUSH_LEFT );
  m_bottomWheelRight->setFixedSize( PBUTTON_SIZE, PBUTTON_SIZE );
  m_bottomWheelRight->setIconSize( QSize(PBUTTON_SIZE-4, PBUTTON_SIZE-4) );
  m_bottomWheelRight->setFocusPolicy( Qt::NoFocus );
  m_bottomWheelRight->setIcon( IDB_PUSH_RIGHT );
  m_remoteControl->connect( m_bottomWheelLeft, SIGNAL(clicked()), this, SLOT(bottomWheelOther()) );
  m_remoteControl->connect( m_bottomWheelRight, SIGNAL(clicked()), this, SLOT(bottomWheelOther()) );
  m_remoteControl->connect( m_bottomWheelLeft, SIGNAL(released()), this, SLOT(bottomWheelOther()) );
  m_remoteControl->connect( m_bottomWheelRight, SIGNAL(released()), this, SLOT(bottomWheelOther()) );
  m_remoteControl->connect( m_bottomWheelLeft, SIGNAL(pressed()), this, SLOT(bottomWheelDrag()) );
  m_remoteControl->connect( m_bottomWheelRight, SIGNAL(pressed()), this, SLOT(bottomWheelDrag()) );
  hblbtwgb->addWidget( m_bottomWheelLeft );
  hblbtwgb->addWidget( m_bottomWheelRight );
  bottomTWGroupBox->setLayout( hblbtwgb );

  QGroupBox* rightTWGroupBox = new QGroupBox( tr("Dolly"), cameraGroupBox );
  QVBoxLayout* vblrtwgb = new QVBoxLayout();
  m_rightWheelUp = new QPushButton( rightTWGroupBox );
  m_rightWheelUp->setAutoRepeat( true );
  m_rightWheelDown = new QPushButton( rightTWGroupBox );
  m_rightWheelDown->setAutoRepeat( true );
  m_rightWheelUp->setFixedSize( PBUTTON_SIZE, PBUTTON_SIZE );
  m_rightWheelUp->setIconSize( QSize(PBUTTON_SIZE-4, PBUTTON_SIZE-4) );
  m_rightWheelUp->setFocusPolicy( Qt::NoFocus );
  m_rightWheelUp->setIcon( IDB_PUSH_UP );
  m_rightWheelDown->setFixedSize( PBUTTON_SIZE, PBUTTON_SIZE );
  m_rightWheelDown->setIconSize( QSize(PBUTTON_SIZE-4, PBUTTON_SIZE-4) );
  m_rightWheelDown->setFocusPolicy( Qt::NoFocus );
  m_rightWheelDown->setIcon( IDB_PUSH_DOWN );
  m_remoteControl->connect( m_rightWheelUp, SIGNAL(clicked()), this, SLOT(rightWheelOther()) );
  m_remoteControl->connect( m_rightWheelDown, SIGNAL(clicked()), this, SLOT(rightWheelOther()) );
  m_remoteControl->connect( m_rightWheelUp, SIGNAL(released()), this, SLOT(rightWheelOther()) );
  m_remoteControl->connect( m_rightWheelDown, SIGNAL(released()), this, SLOT(rightWheelOther()) );
  m_remoteControl->connect( m_rightWheelUp, SIGNAL(pressed()), this, SLOT(rightWheelDrag()) );
  m_remoteControl->connect( m_rightWheelDown, SIGNAL(pressed()), this, SLOT(rightWheelDrag()) );
  vblrtwgb->addWidget( m_rightWheelUp );
  vblrtwgb->addWidget( m_rightWheelDown );
  rightTWGroupBox->setLayout( vblrtwgb );

  m_zoomGroupBox = new QGroupBox( tr("Zoom"), cameraGroupBox );
  QHBoxLayout* hblzgb = new QHBoxLayout();
  m_zoomSldRange.setValue( 1., 140. );
  m_zoomSlider = new QScrollBar( Qt::Horizontal, m_zoomGroupBox );
  m_zoomSlider->setMinimum( 0 );
  m_zoomSlider->setMaximum( 1000 );
  m_zoomSlider->setSingleStep( 1 );
  m_zoomSlider->setFixedWidth( SLIDER_LONG );
  m_zoomSlider->setFocus();
  m_zoomField = new QLineEdit( m_zoomGroupBox );
  m_zoomField->setFixedSize( ((m_remoteControl->fontMetrics()).width(QString("180.00"))+2), ((m_remoteControl->fontMetrics()).height()+2) );
  m_zoomField->setAlignment( Qt::AlignHCenter );
  m_remoteControl->connect( m_zoomField, SIGNAL(returnPressed (void)), this, SLOT(zs_textChanged(void)) );
  m_remoteControl->connect( m_zoomField, SIGNAL(editingFinished (void)), this, SLOT(zs_textChanged(void)) );
  m_remoteControl->connect( m_zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(zs_update(int)) );
  hblzgb->addWidget( m_zoomSlider );
  hblzgb->addSpacing( 4 );
  hblzgb->addWidget( m_zoomField );
  m_zoomGroupBox->setLayout( hblzgb );

  hblcgb->addWidget( leftTWGroupBox );
  hblcgb->addWidget( bottomTWGroupBox );
  hblcgb->addWidget( rightTWGroupBox );
  hblcgb->addWidget( m_zoomGroupBox );
  hblcgb->addStretch( 1 );
  cameraGroupBox->setLayout( hblcgb );

  // Setup the progress bar
  QGroupBox* progressGroupBox = new QGroupBox( tr("Loading"), m_remoteControl );
  QHBoxLayout* hblpgb = new QHBoxLayout();

  m_pgb = new QProgressBar( progressGroupBox );
  m_pgb->setMaximum( 100 );
  m_pgb->setMinimum( 0 );

  hblpgb->addWidget( m_pgb );
  progressGroupBox->setLayout( hblpgb );

  // Setup the layout
  QVBoxLayout* vblrc = new QVBoxLayout();
  vblrc->insertSpacing( 0, 10 );
  vblrc->addWidget( buttonGroupBox );
  vblrc->addWidget( cameraGroupBox );
  vblrc->addWidget( progressGroupBox );
  m_remoteControl->setLayout( vblrc );
  m_remoteControl->setWindowTitle( "Custom Viewer Remote Control" );
  m_remoteControl->show();
}

//---------------------------------------------------------------
void 
QtCustomViewer::setupCursors()
{
  dollyCursor = QCursor( QPixmap(( const char** ) Classic_Dolly) );
  panCursor   = QCursor( QPixmap(( const char** ) Classic_Pan) );
  rollCursor  = QCursor( QPixmap(( const char** ) Classic_Roll) );
  seekCursor  = QCursor( QPixmap(( const char** ) Classic_Seek) );
  spinCursor  = QCursor( QPixmap(( const char** ) Classic_Spin) );

  m_renderArea->setCursor( spinCursor );
}

//---------------------------------------------------------------
void 
QtCustomViewer::setupPopupMenu()
{
  m_mainMenu = NULL; 

  // Draw style menu
  m_drawMenu = new QMenu( this );
  // Idle style
  IDM_DPOPUP_ASIS = m_drawMenu->addAction( "&as Is", this, SLOT(pm_asIs()) );
  IDM_DPOPUP_ASIS->setCheckable( true );
  IDM_DPOPUP_HLINE = m_drawMenu->addAction( "&hidden line", this, SLOT(pm_hiddenLine()) );
  IDM_DPOPUP_HLINE->setCheckable( true );
  IDM_DPOPUP_NOTEX = m_drawMenu->addAction( "&no texture", this, SLOT(pm_noTexture()) );
  IDM_DPOPUP_NOTEX->setCheckable( true );
  IDM_DPOPUP_LOWRES = m_drawMenu->addAction( "&low resolution", this, SLOT(pm_lowResolution()) );
  IDM_DPOPUP_LOWRES->setCheckable( true );
  IDM_DPOPUP_WIRE = m_drawMenu->addAction( "&wire frame", this, SLOT(pm_wireFrame()) );
  IDM_DPOPUP_WIRE->setCheckable( true );
  IDM_DPOPUP_POINTS = m_drawMenu->addAction( "&points", this, SLOT(pm_points()) );
  IDM_DPOPUP_POINTS->setCheckable( true );
  IDM_DPOPUP_BBOX = m_drawMenu->addAction( "&bounding box", this, SLOT(pm_boundingBox()) );
  IDM_DPOPUP_BBOX->setCheckable( true );
  m_drawMenu->addSeparator();
  // Move style
  IDM_DPOPUP_MSAMEAS = m_drawMenu->addAction( "&move same as still", this, SLOT(pm_moveSameAsStill()) );
  IDM_DPOPUP_MSAMEAS->setCheckable( true );
  IDM_DPOPUP_MNOTEX= m_drawMenu->addAction( "&move no texture", this, SLOT(pm_moveNoTexture()) );
  IDM_DPOPUP_MNOTEX->setCheckable( true );
  IDM_DPOPUP_MLOWRES= m_drawMenu->addAction( "&move low res", this, SLOT(pm_moveLowRes()) );
  IDM_DPOPUP_MLOWRES->setCheckable( true );
  IDM_DPOPUP_MWIRE = m_drawMenu->addAction( "&move wire frame", this, SLOT(pm_moveWireFrame()) );
  IDM_DPOPUP_MWIRE->setCheckable( true );
  IDM_DPOPUP_MPOINTS = m_drawMenu->addAction( "&move points", this, SLOT(pm_movePoints()) );
  IDM_DPOPUP_MPOINTS->setCheckable( true );
  IDM_DPOPUP_MBBOX = m_drawMenu->addAction( "&move bounding box", this, SLOT(pm_moveBoundingBox()) );
  IDM_DPOPUP_MBBOX->setCheckable( true );
  m_drawMenu->addSeparator();
  // Buffer style
  IDM_DPOPUP_SBUFFER = m_drawMenu->addAction( "&single buffer", this, SLOT(pm_singleBuffer()) );
  IDM_DPOPUP_SBUFFER->setCheckable( true );
  IDM_DPOPUP_DBUFFER = m_drawMenu->addAction( "&double buffer", this, SLOT(pm_doubleBuffer()) );
  IDM_DPOPUP_DBUFFER->setCheckable( true );
  IDM_DPOPUP_IBUFFER = m_drawMenu->addAction( "&interactive buffer", this, SLOT(pm_interactiveBuffer()) );
  IDM_DPOPUP_IBUFFER->setCheckable( true );

  // Main menu
  m_mainMenu = new QMenu( this );
  m_mainMenu->addMenu( m_drawMenu );
  m_drawMenu->setTitle( "&Draw Style" );
  IDM_MPOPUP_FULLSCREEN = m_mainMenu->addAction( "&FullScreen", this, SLOT(pm_fullScreen()) );
  IDM_MPOPUP_FULLSCREEN->setCheckable( true );
  IDM_MPOPUP_HIDERC = m_mainMenu->addAction( "&Remote control", this, SLOT(pm_hideRemoteControl()) );
  IDM_MPOPUP_HIDERC->setCheckable( true );

  m_curPopupDrawItem   = IDM_DPOPUP_ASIS;
  m_curPopupMoveItem   = IDM_DPOPUP_MSAMEAS;
  m_curPopupBufferItem = IDM_DPOPUP_DBUFFER;
  IDM_DPOPUP_ASIS->setChecked( true );
  IDM_DPOPUP_MSAMEAS->setChecked( true );
  IDM_DPOPUP_DBUFFER->setChecked( true );
  IDM_MPOPUP_HIDERC->setChecked( true );
}

//---------------------------------------------------------------
void 
QtCustomViewer::setSceneGraph( SoNode* newScene )
{
  m_guiAlgo->setSceneGraph( newScene );
  m_renderArea->show();
}

//---------------------------------------------------------------
void
QtCustomViewer::switchViewerMode( unsigned int state )
{
  if ( (state & Qt::LeftButton) && (state & Qt::MidButton) )
  {
    m_currentViewerMode = DOLLY_MODE;
    m_renderArea->setCursor( dollyCursor );
  }
  else if ( state & Qt::LeftButton )
  {
	  if ( state & Qt::ControlModifier ) 
    {
      if ( state & Qt::ShiftModifier )
      {
        m_currentViewerMode = DOLLY_MODE;
        m_renderArea->setCursor( dollyCursor );
      }
      else
      {
        m_currentViewerMode = PAN_MODE;
        m_renderArea->setCursor( panCursor );
        m_guiAlgo->activatePanning();
      }
    }
    else 
    {
      if ( state & Qt::ShiftModifier ) 
      {
        m_currentViewerMode = PAN_MODE;
        m_renderArea->setCursor( panCursor );
        m_guiAlgo->activatePanning();
      }
      else 
      {
        m_renderArea->setCursor( spinCursor );
        m_currentViewerMode = VIEW_MODE;
        m_guiAlgo->activateSpinning();
      }
    }
  }    
  else if ( state & Qt::MidButton ) 
  {
    if ( state & Qt::ControlModifier )
    {
      m_renderArea->setCursor( dollyCursor );
      m_currentViewerMode = DOLLY_MODE;
    }
    else
    {
      m_renderArea->setCursor( panCursor );
      m_currentViewerMode = PAN_MODE;
      m_guiAlgo->activatePanning();
    }
  }
  else
  {
    if ( (m_currentViewerMode == SEEK_MODE) && m_guiAlgo->isSeekMode() )
    {
      m_renderArea->setCursor( seekCursor );
      m_currentViewerMode = SEEK_MODE;
    }
    else
    {
      m_renderArea->setCursor( spinCursor );
      m_currentViewerMode = IDLE_MODE;
    }
  }
}

//---------------------------------------------------------------
void
QtCustomViewer::setDrawStyle( SoGuiAlgoViewers::DrawType type, SoGuiAlgoViewers::DrawStyle style )
{
  m_guiAlgo->setDrawStyle( type, style );

  // Update the popup menu entries
  switch (m_guiAlgo->getDrawStyle(SoGuiAlgoViewers::STILL)) 
  {
  case SoGuiAlgoViewers::VIEW_AS_IS:
    updateMenu( m_curPopupDrawItem, IDM_DPOPUP_ASIS ); 
    break;
  case SoGuiAlgoViewers::VIEW_HIDDEN_LINE:
    updateMenu( m_curPopupDrawItem, IDM_DPOPUP_HLINE ); 
    break;
  case SoGuiAlgoViewers::VIEW_NO_TEXTURE:
    updateMenu( m_curPopupDrawItem, IDM_DPOPUP_NOTEX ); 
    break;
  case SoGuiAlgoViewers::VIEW_LOW_COMPLEXITY:
    updateMenu( m_curPopupDrawItem, IDM_DPOPUP_LOWRES ); 
    break;
  case SoGuiAlgoViewers::VIEW_LINE:
    updateMenu( m_curPopupDrawItem, IDM_DPOPUP_WIRE ); 
    break;
  case SoGuiAlgoViewers::VIEW_POINT:
    updateMenu( m_curPopupDrawItem, IDM_DPOPUP_POINTS ); 
    break;
  case SoGuiAlgoViewers::VIEW_BBOX:
    updateMenu( m_curPopupDrawItem, IDM_DPOPUP_BBOX ); 
    break;
  }

  switch (m_guiAlgo->getDrawStyle(SoGuiAlgoViewers::INTERACTIVE)) 
  {
  case SoGuiAlgoViewers::VIEW_SAME_AS_STILL:
    updateMenu( m_curPopupMoveItem, IDM_DPOPUP_MSAMEAS );
    break;
  case SoGuiAlgoViewers::VIEW_NO_TEXTURE:
    updateMenu( m_curPopupMoveItem, IDM_DPOPUP_MNOTEX );
    break;
  case SoGuiAlgoViewers::VIEW_LOW_COMPLEXITY:
    updateMenu( m_curPopupMoveItem, IDM_DPOPUP_MLOWRES );
    break;
  case SoGuiAlgoViewers::VIEW_LINE:
    updateMenu( m_curPopupMoveItem, IDM_DPOPUP_MWIRE );
    break;
  case SoGuiAlgoViewers::VIEW_LOW_RES_LINE:
    updateMenu( m_curPopupMoveItem, IDM_DPOPUP_MWIRE );
    break;
  case SoGuiAlgoViewers::VIEW_POINT:
    updateMenu( m_curPopupMoveItem, IDM_DPOPUP_MPOINTS );
    break;
  case SoGuiAlgoViewers::VIEW_LOW_RES_POINT:
    updateMenu( m_curPopupMoveItem, IDM_DPOPUP_MPOINTS );
    break;
  case SoGuiAlgoViewers::VIEW_BBOX:
    updateMenu( m_curPopupMoveItem, IDM_DPOPUP_MBBOX );
    break;
  }
}

//---------------------------------------------------------------
void 
QtCustomViewer::updateMenu( QAction* curItem, QAction* item )
{
  curItem->setChecked( false );
  if ( curItem == m_curPopupDrawItem )
    m_curPopupDrawItem = item;
  else if ( curItem == m_curPopupMoveItem )
    m_curPopupMoveItem = item;
  else if ( curItem == m_curPopupBufferItem )
    m_curPopupBufferItem = item;
  item->setChecked( true );
}

//---------------------------------------------------------------
void
QtCustomViewer::setBufferingType( SoGuiAlgoViewers::BufferType type )
{
  m_guiAlgo->setBufferingType( type );

  // Update the popup menu entries
  switch (type) 
  {
  case SoGuiAlgoViewers::BUFFER_SINGLE:
    m_renderArea->setDoubleBuffer( FALSE );
    updateMenu( m_curPopupBufferItem, IDM_DPOPUP_SBUFFER );
    break;
  case SoGuiAlgoViewers::BUFFER_DOUBLE:
    m_renderArea->setDoubleBuffer( TRUE );
    updateMenu( m_curPopupBufferItem, IDM_DPOPUP_DBUFFER );
    break;
  case SoGuiAlgoViewers::BUFFER_INTERACTIVE:
    m_renderArea->setDoubleBuffer( FALSE );
    updateMenu( m_curPopupBufferItem, IDM_DPOPUP_IBUFFER );
    break;
  }
}

//---------------------------------------------------------------
void
QtCustomViewer::setCameraZoom( float zoom )
{
  SoCamera* camera = m_guiAlgo->getCamera();
  if ( camera == NULL )
    return;

  // Detach sensor while we update the field
  m_zoomSensor->detach();

  if ( camera->isOfType(SoPerspectiveCamera::getClassTypeId()) )
    ((SoPerspectiveCamera *)camera)->heightAngle = zoom * M_PI / 180.0;
  else if ( camera->isOfType(SoOrthographicCamera::getClassTypeId()) )
    ((SoOrthographicCamera *)camera)->height = zoom;

  // Reattach the sensor
  if ( camera->isOfType(SoPerspectiveCamera::getClassTypeId()) )
    m_zoomSensor->attach( &((SoPerspectiveCamera *)camera)->heightAngle );
}

//---------------------------------------------------------------
float
QtCustomViewer::getCameraZoom()
{
  SoCamera* camera = m_guiAlgo->getCamera();
  if ( camera == NULL )
    return 0;

  if ( camera->isOfType(SoPerspectiveCamera::getClassTypeId()) )
    return (float)(((SoPerspectiveCamera *)camera)->heightAngle.getValue() * 180.0f / M_PI);
  else if ( camera->isOfType(SoOrthographicCamera::getClassTypeId()) )
    return ((SoOrthographicCamera *)camera)->height.getValue();
  else 
    return 0;
}

//---------------------------------------------------------------
void
QtCustomViewer::setZoomSliderPosition( float zoom )
{
  // Find the slider position, using a square root distance to make the
  // slider smoother and less sensitive when close to zero.
  float f = (zoom - m_zoomSldRange[0]) / (m_zoomSldRange[1] - m_zoomSldRange[0]);
  f = (f < 0.0f) ? 0.0f : ((f > 1.0f) ? 1.0f : f);
  f = (float)(sqrt(f));

  // Finally position the slider
  int val = int(f * 1000);
  m_zoomSlider->setValue( val );
}

//---------------------------------------------------------------
void
QtCustomViewer::setZoomFieldString( float zoom )
{
  QString string;
  string = string.setNum( zoom, 'f', 1 ); 
  m_zoomField->setText( string );
}

//---------------------------------------------------------------
void 
QtCustomViewer::pb_toggleCameraType()
{
  m_guiAlgo->stopSpinAnimation();

  m_toggledCameraType = !m_toggledCameraType;
  if ( m_toggledCameraType )
  {
    m_cam->setIcon( IDB_PUSH_ORTHO );
    m_zoomGroupBox->setEnabled( false );
  }
  else
  {
    m_cam->setIcon( IDB_PUSH_PERSP );
    m_zoomGroupBox->setEnabled( true );
  }

  m_guiAlgo->toggleCameraType();
}

//---------------------------------------------------------------
void 
QtCustomViewer::pb_viewAll()
{
  m_guiAlgo->stopSpinAnimation();
  m_guiAlgo->viewAll();
}

//---------------------------------------------------------------
void 
QtCustomViewer::pb_saveHomePosition()
{
  m_guiAlgo->stopSpinAnimation();
  m_guiAlgo->saveHomePosition();
}

//---------------------------------------------------------------
void 
QtCustomViewer::pb_resetToHomePosition()
{
  m_guiAlgo->stopSpinAnimation();
  m_guiAlgo->resetToHomePosition();
}

//---------------------------------------------------------------
void 
QtCustomViewer::pb_pickMode()
{
  m_guiAlgo->stopSpinAnimation();
  if ( !m_guiAlgo->isViewing() ) // If the button is already down
    m_pick->setChecked( true );
  else
  {
    m_guiAlgo->setViewing( false );
    m_view->setChecked( false );
    m_seek->setEnabled( false );
    m_pbBoxZoom->setEnabled( true );
    m_boxZoom = false;
    m_pbBoxSelection->setEnabled( true );
    m_boxSelection = false;
    m_guiAlgo->setBoxDrawingMode( FALSE );
    m_guiAlgo->setBoxSelectionMode( FALSE );
  }

  m_renderArea->setCursor( normalCursor );
  m_currentViewerMode = PICK_MODE;
}

//---------------------------------------------------------------
void 
QtCustomViewer::pb_viewingMode()
{
  if ( m_guiAlgo->isViewing() ) // If the button is already down
    m_view->setChecked( true );
  else
  {
    m_guiAlgo->setViewing( true );
    m_pick->setChecked( false );
    if ( !m_seek->isEnabled() )
      m_seek->setEnabled( true );
    if ( m_pbBoxZoom->isEnabled() )
      m_pbBoxZoom->setEnabled( false );
    if ( m_pbBoxZoom->isChecked() )
      m_pbBoxZoom->setChecked( false );
    m_boxZoom = false;
    if ( m_pbBoxSelection->isEnabled() )
      m_pbBoxSelection->setEnabled( false );
    if ( m_pbBoxSelection->isChecked() )
      m_pbBoxSelection->setChecked( false );
    m_boxSelection = false;
  }

  m_renderArea->setCursor( spinCursor );
  m_currentViewerMode = IDLE_MODE;
}

//---------------------------------------------------------------
void 
QtCustomViewer::pb_seekMode()
{
  m_guiAlgo->stopSpinAnimation();
  m_guiAlgo->setSeekMode( !m_guiAlgo->isSeekMode() );
  if ( m_guiAlgo->isSeekMode() )
  {
    m_renderArea->setCursor( seekCursor );
    m_currentViewerMode = SEEK_MODE;
  }
}

//---------------------------------------------------------------
void 
QtCustomViewer::pb_boxZoom()
{
  m_boxZoom = !m_boxZoom;

  m_guiAlgo->setBoxDrawingMode( m_boxZoom );

  if ( m_boxSelection )
  {
    m_boxSelection = false;
    m_guiAlgo->setBoxSelectionMode( FALSE );
  }
  if ( m_pbBoxSelection->isChecked() )
    m_pbBoxSelection->setChecked( false );
}

//---------------------------------------------------------------
void 
QtCustomViewer::pb_boxSelection()
{
  m_boxSelection = !m_boxSelection;

  m_guiAlgo->setBoxDrawingMode( m_boxSelection );
  m_guiAlgo->setBoxSelectionMode( m_boxSelection );

  if ( m_boxZoom )
    m_boxZoom = false;
  if ( m_pbBoxZoom->isChecked() )
    m_pbBoxZoom->setChecked( false );
}

//---------------------------------------------------------------
void 
QtCustomViewer::pm_asIs()
{
  setDrawStyle( SoGuiAlgoViewers::STILL, SoGuiAlgoViewers::VIEW_AS_IS );
}

//---------------------------------------------------------------
void 
QtCustomViewer::pm_hiddenLine()
{
  setDrawStyle( SoGuiAlgoViewers::STILL, SoGuiAlgoViewers::VIEW_HIDDEN_LINE );
}

//---------------------------------------------------------------
void 
QtCustomViewer::pm_noTexture()
{
  setDrawStyle( SoGuiAlgoViewers::STILL, SoGuiAlgoViewers::VIEW_NO_TEXTURE );
}

//---------------------------------------------------------------
void 
QtCustomViewer::pm_lowResolution()
{
  setDrawStyle( SoGuiAlgoViewers::STILL, SoGuiAlgoViewers::VIEW_LOW_COMPLEXITY );
}

//---------------------------------------------------------------
void 
QtCustomViewer::pm_wireFrame()
{
  setDrawStyle( SoGuiAlgoViewers::STILL, SoGuiAlgoViewers::VIEW_LINE );
}

//---------------------------------------------------------------
void 
QtCustomViewer::pm_points()
{
  setDrawStyle( SoGuiAlgoViewers::STILL, SoGuiAlgoViewers::VIEW_POINT );
}

//---------------------------------------------------------------
void 
QtCustomViewer::pm_boundingBox()
{
  setDrawStyle( SoGuiAlgoViewers::STILL, SoGuiAlgoViewers::VIEW_BBOX );
}

//---------------------------------------------------------------
void 
QtCustomViewer::pm_moveSameAsStill()
{
  setDrawStyle( SoGuiAlgoViewers::INTERACTIVE, SoGuiAlgoViewers::VIEW_SAME_AS_STILL );
}

//---------------------------------------------------------------
void 
QtCustomViewer::pm_moveNoTexture()
{
  setDrawStyle( SoGuiAlgoViewers::INTERACTIVE, SoGuiAlgoViewers::VIEW_NO_TEXTURE );
}

//---------------------------------------------------------------
void 
QtCustomViewer::pm_moveLowRes()
{
  setDrawStyle( SoGuiAlgoViewers::INTERACTIVE, SoGuiAlgoViewers::VIEW_LOW_COMPLEXITY );
}

//---------------------------------------------------------------
void 
QtCustomViewer::pm_moveWireFrame()
{
  setDrawStyle( SoGuiAlgoViewers::INTERACTIVE, SoGuiAlgoViewers::VIEW_LINE );
}

//---------------------------------------------------------------
void 
QtCustomViewer::pm_movePoints()
{
  setDrawStyle( SoGuiAlgoViewers::INTERACTIVE, SoGuiAlgoViewers::VIEW_POINT );
}

//---------------------------------------------------------------
void 
QtCustomViewer::pm_moveBoundingBox()
{
  setDrawStyle( SoGuiAlgoViewers::INTERACTIVE, SoGuiAlgoViewers::VIEW_BBOX );
}

//---------------------------------------------------------------
void 
QtCustomViewer::pm_singleBuffer()
{
  setBufferingType( SoGuiAlgoViewers::BUFFER_SINGLE );
}

//---------------------------------------------------------------
void 
QtCustomViewer::pm_doubleBuffer()
{
  setBufferingType( SoGuiAlgoViewers::BUFFER_DOUBLE );
}

//---------------------------------------------------------------
void 
QtCustomViewer::pm_interactiveBuffer()
{
  setBufferingType( SoGuiAlgoViewers::BUFFER_INTERACTIVE );
}

//---------------------------------------------------------------
void 
QtCustomViewer::pm_fullScreen()
{
  m_renderArea->setFullScreen( !m_renderArea->isFullScreen() );
}

//---------------------------------------------------------------
void 
QtCustomViewer::pm_hideRemoteControl()
{
  m_remoteControl->setVisible( !m_remoteControl->isVisible() );
}

//---------------------------------------------------------------
void 
QtCustomViewer::leftWheelDrag()
{
  // For the first move, invoke the start callbacks
  if ( s_firstDrag ) 
  {
    m_guiAlgo->leftWheelStart();
    s_firstDrag = FALSE;
  }
  if ( m_leftWheelUp->isDown() )
    s_lwdValue += WHEEL_DRAG;
  else if ( m_leftWheelDown->isDown() )
    s_lwdValue -= WHEEL_DRAG;

  m_guiAlgo->leftWheelMotion( (float)(s_lwdValue * M_PI / 180.0f) );
}

//---------------------------------------------------------------
void 
QtCustomViewer::leftWheelOther()
{
  m_guiAlgo->leftWheelFinish();
  s_firstDrag = TRUE;
}

//---------------------------------------------------------------
void 
QtCustomViewer::rightWheelDrag()
{
  // For the first move, invoke the start callbacks
  if ( s_firstDrag ) 
  {
    m_guiAlgo->rightWheelStart();
    s_firstDrag = FALSE;
  }
  if ( m_rightWheelUp->isDown() )
    s_rwdValue -= WHEEL_DRAG;
  else if ( m_rightWheelDown->isDown() )
    s_rwdValue += WHEEL_DRAG;

  m_guiAlgo->rightWheelMotion( (float)(s_rwdValue * M_PI / 180.0f) );
}

//---------------------------------------------------------------
void 
QtCustomViewer::rightWheelOther()
{
  m_guiAlgo->rightWheelFinish();
  s_firstDrag = TRUE;
}

//---------------------------------------------------------------
void 
QtCustomViewer::bottomWheelDrag()
{
  // For the first move, invoke the start callbacks
  if ( s_firstDrag ) 
  {
    m_guiAlgo->bottomWheelStart();
    s_firstDrag = FALSE;
  }
  if ( m_bottomWheelLeft->isDown() )
    s_bwdValue -= WHEEL_DRAG;
  else if ( m_bottomWheelRight->isDown() )
    s_bwdValue += WHEEL_DRAG;

  m_guiAlgo->bottomWheelMotion( (float)(s_bwdValue * M_PI / 180.0f) );
}

//---------------------------------------------------------------
void 
QtCustomViewer::bottomWheelOther()
{
  m_guiAlgo->bottomWheelFinish();
  s_firstDrag = TRUE;
}

//---------------------------------------------------------------
void 
QtCustomViewer::mb_openFile()
{
  SbString dataPath = SbFileHelper::expandString("$OIVHOME/data/models/");

  QString filter = tr("Inventor files (*.iv)");
  QString filename = 
    QFileDialog::getOpenFileName( m_remoteControl, "Open Inventor File...", 
    QString().fromUtf16( dataPath.toUtf16() ), filter );

  if ( filename != QString::null )
  {
    MyInput input( m_pgb );
    if ( !input.openFile(filename.toLocal8Bit().data()) ) 
    {
      fprintf( stderr, "Cannot open file %s.\n", filename.toLocal8Bit().data() );
      return;
    }

    SoSeparator* myGraph = SoDB::readAll( &input );
    if ( myGraph == NULL ) 
    {
      fprintf( stderr, "A problem occured when loading the file %s.\n", filename.toLocal8Bit().data() );
      return;
    }
    setSceneGraph( myGraph );
    m_pgb->reset();

    // Reset the "wheels" values as we loaded a new scene
    s_firstDrag = TRUE;
    s_lwdValue = 0;
    s_rwdValue = 0;
    s_bwdValue = 0;
    m_guiAlgo->setLeftWheelVal( 0 );
    m_guiAlgo->setBottomWheelVal( 0 );
    m_guiAlgo->setRightWheelVal( 0 );

    // Set the zoom sensor
    m_zoomSensor = new SoFieldSensor( QtCustomViewer::zoomSensorCB, this );
    SoCamera* camera = m_guiAlgo->getCamera();
    if ( camera->isOfType(SoPerspectiveCamera::getClassTypeId()) )
    {
      m_zoomSensor->attach( &((SoPerspectiveCamera*)m_guiAlgo->getCamera())->heightAngle );
      m_zoomGroupBox->setEnabled( true );
    }
    else if ( camera->isOfType(SoOrthographicCamera::getClassTypeId()) )
    {
      m_zoomGroupBox->setEnabled( false );
      m_toggledCameraType = true;
      m_cam->setIcon( IDB_PUSH_ORTHO );
    }
    float zoom = getCameraZoom();
    setZoomSliderPosition( zoom );
    setZoomFieldString( zoom );

    // Reset to view mode
    pb_viewingMode();
    m_view->setChecked( true );

    // Uncheck and disable box buttons
    m_pbBoxZoom->setChecked( false );
    m_pbBoxZoom->setEnabled( false );
    m_pbBoxSelection->setChecked( false );
    m_pbBoxSelection->setEnabled( false );
    m_guiAlgo->setBoxDrawingMode( FALSE );
    m_guiAlgo->setBoxSelectionMode( FALSE );
  }
  else
    fprintf( stderr, "Error, couldn't open the file selected." );
}

//---------------------------------------------------------------
void 
QtCustomViewer::mb_quit()
{
  // No need to delete m_guiAlgo as this will be done when
  // m_renderArea will be deleted.
  delete m_renderArea;
  if ( m_zoomSensor )
    delete m_zoomSensor;

  SoQt::finish();

  qApp->exit( 0 );
}

//---------------------------------------------------------------
void 
QtCustomViewer::mb_about()
{
  QMessageBox::about( m_remoteControl, tr("Custom Viewer"), tr("This application is an example on how to simply create a viewer \
                                                                from scratch using the SoGuiAlgoViewers API.  \
                                                                OpenInventor (c) Copyright FEI S.A.S. All rights reserved") );
}

//---------------------------------------------------------------
void
QtCustomViewer::zs_update( int value )
{
  float f = (value) / 1000.;
  f *= f;
  float zoom = m_zoomSldRange[0] + f * (m_zoomSldRange[1] - m_zoomSldRange[0]); 
  setZoomFieldString( zoom );
  // Now update the camera and text field
  setCameraZoom( zoom );
}

//---------------------------------------------------------------
void
QtCustomViewer::zs_textChanged()
{            
  SoCamera* camera = m_guiAlgo->getCamera();
  if ( !camera )
    return;

  float zoom = m_zoomField->text().toFloat();
  if ( zoom >= 0 )
  {
    // Check for valid perspective camera range
    if ( camera != NULL && camera->isOfType(SoPerspectiveCamera::getClassTypeId()) )
      zoom = (zoom < 0.01f) ? 0.01f : ((zoom > 179.99f) ? 179.99f : zoom);

    // Check if the newly typed value changed the slider range
    if ( zoom < m_zoomSldRange[0] )
      m_zoomSldRange[0] = zoom;
    else if ( zoom > m_zoomSldRange[1] )
      m_zoomSldRange[1] = zoom;

    // Update the slider and camera zoom values.
    setCameraZoom( zoom );
    setZoomSliderPosition( zoom );   
  }
  else
    zoom = getCameraZoom();
}


