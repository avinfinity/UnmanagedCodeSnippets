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

#ifndef QT_CUSTOM_VIEWER_H
#define QT_CUSTOM_VIEWER_H

//////////////////////////////////////////////////////////////////////////////
//
//  Class: QtCustomViewer
//
//////////////////////////////////////////////////////////////////////////////

// Qt
#include <QtCore/QPoint>
#include <QAction>
#include <QCloseEvent>
#include <QGroupBox>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QPixmap>
#include <QProgressBar>
#include <QScrollBar>
#include <QWheelEvent>
#include <QWidget>

#include <Inventor/Gui/viewers/SoGuiAlgoViewers.h>
#include <Inventor/nodes/SoNode.h>
#include <Inventor/Qt/SoQtRenderArea.h>

class QtCustomViewer : public QWidget
{
  Q_OBJECT

public:

  enum ViewerModes
  {
    PICK_MODE,
    VIEW_MODE,
    PAN_MODE,
    DOLLY_MODE,
    SEEK_MODE,
    IDLE_MODE = -1
  };

  QtCustomViewer( QWidget* parent );
  ~QtCustomViewer();

  // Events
  virtual void closeEvent( QCloseEvent* unused );
  virtual void keyPressEvent( QKeyEvent* keyPress );
  virtual void keyReleaseEvent( QKeyEvent* keyRelease );
  virtual void mousePressEvent( QMouseEvent* mousePress );
  virtual void mouseReleaseEvent( QMouseEvent* mouseRelease );
  virtual void mouseMoveEvent( QMouseEvent* mouseMove );
  virtual void wheelEvent( QWheelEvent* mouseWheel );

protected:
  bool eventFilter( QObject* obj, QEvent* event ); // Used to catch the close event of the RC in order to hide it and update the popup menu
                                                   // Could have been used too to catch events from the render area...

private:
  static SbBool renderAreaEventCB( void* userData, QEvent* anyevent ); // Redirects keyboard and mouse events from the render area to QtCustomViewer
  static void zoomSensorCB( void* p, SoSensor* unused );

  void displayPopupMenu( QPoint point );
  SbBool processPickEvents( QEvent* anEvent );
  void setupRemoteControl();
  void setupCursors();
  void setupPopupMenu();
  void setSceneGraph( SoNode* newScene );
  void switchViewerMode( unsigned int state );
  void setDrawStyle( SoGuiAlgoViewers::DrawType type, SoGuiAlgoViewers::DrawStyle style );
  void updateMenu( QAction* curItem, QAction* item );
  void setBufferingType( SoGuiAlgoViewers::BufferType type );
  void setCameraZoom( float zoom );
  float getCameraZoom();
  void setZoomSliderPosition( float zoom );
  void setZoomFieldString( float zoom );

  // Gui
  QGroupBox* m_zoomGroupBox;
  QMenu* m_mainMenu, *m_drawMenu;
  QMenuBar* m_menuBar;
  QProgressBar* m_pgb;
  QWidget* m_remoteControl;
  // Buttons
  QPushButton* m_pick;
  QPushButton* m_view;
  QPushButton* m_seek;
  QPushButton* m_cam;
  QPushButton* m_leftWheelUp;
  QPushButton* m_leftWheelDown;
  QPushButton* m_rightWheelUp;
  QPushButton* m_rightWheelDown;
  QPushButton* m_bottomWheelLeft;
  QPushButton* m_bottomWheelRight;
  QPushButton* m_pbBoxZoom;
  QPushButton* m_pbBoxSelection;
  // Buttons icons
  QPixmap IDB_PUSH_PICK;
  QPixmap IDB_PUSH_VIEW;
  QPixmap IDB_PUSH_HOME;
  QPixmap IDB_PUSH_SETHOME;
  QPixmap IDB_PUSH_VIEWALL;
  QPixmap IDB_PUSH_SEEK;
  QPixmap IDB_PUSH_PERSP;
  QPixmap IDB_PUSH_ORTHO;
  QPixmap IDB_PUSH_UP;
  QPixmap IDB_PUSH_DOWN;
  QPixmap IDB_PUSH_LEFT;
  QPixmap IDB_PUSH_RIGHT;
  QPixmap IDB_PUSH_BOX_ZOOM;
  QPixmap IDB_PUSH_BOX_SELECTION;
  // Cursors
  QCursor normalCursor; 
  QCursor dollyCursor;
  QCursor panCursor;
  QCursor rollCursor;
  QCursor seekCursor;
  QCursor spinCursor;
  // Popup menu items
  QAction* IDM_DPOPUP_ASIS;
  QAction* IDM_DPOPUP_HLINE;
  QAction* IDM_DPOPUP_NOTEX;
  QAction* IDM_DPOPUP_LOWRES;
  QAction* IDM_DPOPUP_WIRE;
  QAction* IDM_DPOPUP_POINTS;
  QAction* IDM_DPOPUP_BBOX;
  QAction* IDM_DPOPUP_MSAMEAS ;
  QAction* IDM_DPOPUP_MNOTEX;
  QAction* IDM_DPOPUP_MLOWRES;
  QAction* IDM_DPOPUP_MWIRE;
  QAction* IDM_DPOPUP_MPOINTS;
  QAction* IDM_DPOPUP_MBBOX;
  QAction* IDM_DPOPUP_SBUFFER;
  QAction* IDM_DPOPUP_DBUFFER;
  QAction* IDM_DPOPUP_IBUFFER;
  QAction* IDM_MPOPUP_FULLSCREEN;
  QAction* IDM_MPOPUP_HIDERC;
  QAction* m_curPopupDrawItem;
  QAction* m_curPopupMoveItem;
  QAction* m_curPopupBufferItem;
  // Zoom slider
  QLineEdit* m_zoomField;
  QScrollBar* m_zoomSlider;
  SoFieldSensor* m_zoomSensor; // Attached to camera zoom field

  // Class members
  SoGuiAlgoViewers* m_guiAlgo;
  SoQtRenderArea* m_renderArea;
  ViewerModes m_currentViewerMode;
  SbTime m_lastMotionTime;
  bool m_toggledCameraType, m_altSwitchBack, m_boxZoom, m_boxSelection;
  SbVec2f m_zoomSldRange;

private Q_SLOTS:
  // Slots
  // Buttons
  void pb_toggleCameraType();
  void pb_viewAll();
  void pb_saveHomePosition();
  void pb_resetToHomePosition();
  void pb_pickMode();
  void pb_viewingMode();
  void pb_seekMode();
  void pb_boxZoom();
  void pb_boxSelection();

  // Popup menu
  void pm_asIs();
  void pm_hiddenLine();
  void pm_noTexture();
  void pm_lowResolution();
  void pm_wireFrame();
  void pm_points();
  void pm_boundingBox();
  void pm_moveSameAsStill();
  void pm_moveNoTexture();
  void pm_moveLowRes();
  void pm_moveWireFrame();
  void pm_movePoints();
  void pm_moveBoundingBox();
  void pm_singleBuffer();
  void pm_doubleBuffer();
  void pm_interactiveBuffer();
  void pm_fullScreen();
  void pm_hideRemoteControl();

  // Thumb wheels
  void leftWheelDrag();
  void leftWheelOther();
  void rightWheelDrag();
  void rightWheelOther();
  void bottomWheelDrag();
  void bottomWheelOther();

  // Menu bar
  void mb_openFile();
  void mb_quit();
  void mb_about();

  // Zoom
  void zs_update( int value );
  void zs_textChanged();
};

#endif //QT_CUSTOM_VIEWER_H


