#ifndef CAVELIB

#include "volRendProfView.h"
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/nodes/SoPackedColor.h>

#if !defined(_WIN32) && !defined(__APPLE__)
#include <Xm/MwmUtil.h>
#endif

volRendProfView::volRendProfView( Widget myProfileWindow ) {

	// Scene Graph.
	m_profileRoot = new SoSeparator();
	m_profileRoot->ref();

	m_textRoot = new SoSeparator();
	m_profileRoot->addChild(m_textRoot);
	SoPackedColor *txtColor = new SoPackedColor();
	txtColor->orderedRGBA.setValue(0xFFFF00FF);
	m_textRoot->addChild(txtColor);
	SoOrthographicCamera *profileCamera = new SoOrthographicCamera();
	m_profileRoot->addChild(profileCamera);

#if defined(_WIN32)
#if !defined(SOQT)
	// Under _WIN32, creating an Inventor component (a viewer in this
	// case) with no parent is not supported.  Under UNIX/X/Motif a
	// top level shell would be automatically created to be the parent,
	// but here we need to create a simple window ourselves.
	
	RECT rect;
	GetWindowRect( myProfileWindow, &rect );
	hwnd = CreateWindow( "button",
                        "Pick Profile",
  	        	        WS_CAPTION| WS_SIZEBOX  ,//WS_OVERLAPPEDWINDOW | BS_OWNERDRAW,
  					    rect.left,			    // X position
  					    rect.bottom - 20,       // Y position
  					    rect.right - rect.left, // Width
  					    rect.bottom - rect.top, // Height
  					    NULL,                   // Parent
  					    NULL,                   // Menu
  					    SoXt::getInstance(),    // App instance handle
  					    NULL );                 // Window creation data

	m_profileViewer = new SoXtPlaneViewer( hwnd );
	ShowWindow( hwnd, SW_SHOW );
  SbVec2s profileRect((short)(rect.right - rect.left), (short)(( rect.bottom - rect.top )/3.));
	m_profileViewer->setSize(profileRect);
#endif //SOQT
#elif defined(__APPLE__)
  m_profileViewer = new SoXtPlaneViewer();
#else
	
	m_profileViewer = new SoXtPlaneViewer();
	short sizeX, sizeY;
	XtVaGetValues(myProfileWindow, XmNwidth, &sizeX, NULL);
	XtVaGetValues(myProfileWindow, XmNheight, &sizeY, NULL);
	m_profileViewer->setSize( SbVec2s(sizeX, sizeY/3) );
	short posX, posY;
	XtVaGetValues(myProfileWindow, XmNx, &posX, NULL);
	XtVaGetValues(myProfileWindow, XmNy, &posY, NULL);
	Widget window = m_profileViewer->getShellWidget();
	XtVaSetValues(window, XmNx, posX-5, NULL);
	XtVaSetValues(window, XmNy, posY+sizeY-25, NULL);
	XtVaSetValues(window, XmNmwmFunctions, MWM_FUNC_MOVE | MWM_FUNC_MINIMIZE | MWM_FUNC_RESIZE, NULL ); 
#endif
	
    m_profileViewer->setTitle( "Pick Profile" );
	m_profileViewer->setDecoration( FALSE );
	m_profileViewer->setSceneGraph(m_profileRoot);

	m_profileViewer->viewAll();
	m_profileViewer->show();
	m_profileViewer->getCamera()->viewportMapping.setValue(SoCamera::LEAVE_ALONE);
	m_profileViewer->hide();

}

volRendProfView::~volRendProfView() {
}

SoSeparator *
volRendProfView::getProfileRoot() {
	return m_profileRoot;
}
SoSeparator *
volRendProfView::getTextProfileRoot() {
	return m_textRoot;
}
void
volRendProfView::showProfile() {
	m_profileViewer->show();
#ifdef _WIN32
	ShowWindow( hwnd, SW_SHOW );
#endif
}
void
volRendProfView::hideProfile() {
	m_profileViewer->hide();
}
SbBool 
volRendProfView::isVisibleProfile() {
	return m_profileViewer->isVisible();
}
void
volRendProfView::profileViewAll() {
	//m_profileViewer->getCamera()->viewportMapping.setValue(SoCamera::LEAVE_ALONE);
	m_profileViewer->viewAll();
}

#endif //CAVELIB


