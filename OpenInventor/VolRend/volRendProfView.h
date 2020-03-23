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
** Author      : Pascal Estrade (Apr 2000)
** Modified by : Mike Heck (Aug 2000)    
** Modified by : Thierry Dufour (Sep 2000) 
** Modified by : Pascal Estrade (Dec 2000)
** Modified by : Jerome Hummel (Dec 2002)  
**=======================================================================*/

#ifndef  _SO_VOLUME_REND_PROF_VIEW_
#define  _SO_VOLUME_REND_PROF_VIEW_

#ifndef CAVELIB

#include <Inventor/Xt/viewers/SoXtExaminerViewer.h>
#include <Inventor/Xt/viewers/SoXtPlaneViewer.h>
#include <Inventor/nodes/SoSeparator.h>

class volRendProfView {
public:
	// Constructor
    volRendProfView( Widget myProfileWindow );
	// Destructor
	~volRendProfView();

	SoSeparator *getProfileRoot();
	SoSeparator *getTextProfileRoot();
	void showProfile();
	void hideProfile();
	SbBool isVisibleProfile();
	void profileViewAll();
	
private:
	SoXtPlaneViewer *m_profileViewer;
	//SoXtExaminerViewer *m_profileViewer;
	SoSeparator *m_profileRoot;
	SoSeparator *m_textRoot;
#ifdef _WIN32
	HWND hwnd;
#endif
};

#endif //CAVELIB

#endif	// _SO_VOLUME_REND_PROF_VIEW_


