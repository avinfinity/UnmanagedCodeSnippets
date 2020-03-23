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

#include "QtCustomViewer.h"

#include <Inventor/SoDB.h>
#include <Inventor/Qt/SoQtExtEventApplication.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/lock/SoLockMgr.h>

void UnlockLicense()
{
	const char* pssw_OpenIV_beg = "License OpenInventor 9.6 1-Jan-0 0 ";
	const char* pssw_OpenIV_end = " \"APP-CALIGO\"";
	const int pssw_OpenIV_main[] =
	{
		'6' + 0, '5' + 1, '6' + 2, '8' + 3, '3' + 4,
		'c' + 5, 'm' + 6, 'u' + 7, 'k' + 8, 'i' + 9,
		'3' + 10, 'a' + 11, 0
	};

	const char* pssw_VolumeVizLDM_beg = "License VolumeVizLDM 9.6 1-Jan-0 0 ";
	const char* pssw_VolumeVizLDM_end = " \"APP-CALIGO\"";
	const int pssw_VolumeVizLDM_main[] =
	{
		'm' + 0, 't' + 1, '8' + 2, 'p' + 3, '5' + 4,
		't' + 5, 'x' + 6, 'c' + 7, 'w' + 8, '5' + 9,
		'h' + 10, 'v' + 11, 0
	};

	char letter[2];
	letter[1] = '\0';
	int i;

	static SbString string_OpenIV;
	string_OpenIV.makeEmpty();
	string_OpenIV += pssw_OpenIV_beg;
	for (i = 0; pssw_OpenIV_main[i] != 0; i++)
	{
		letter[0] = (char)(pssw_OpenIV_main[i] - i);
		string_OpenIV += letter;
	}
	string_OpenIV += pssw_OpenIV_end;

	string_OpenIV += ":";

	string_OpenIV += pssw_VolumeVizLDM_beg;
	for (i = 0; pssw_VolumeVizLDM_main[i] != 0; i++)
	{
		letter[0] = (char)(pssw_VolumeVizLDM_main[i] - i);
		string_OpenIV += letter;
	}
	string_OpenIV += pssw_VolumeVizLDM_end;

	SoLockManager::SetUnlockString((char*)string_OpenIV.getString());
}



//------------------------------------------------------------
int
main( int argc, char* *argv )
{
	UnlockLicense();

  SoQtExtEventApplication app( argc, argv );
  
  // Create a viewer
  QtCustomViewer* myViewer = new QtCustomViewer( NULL );
  app.setActiveWindow( myViewer );
 
  myViewer->setWindowTitle( "Custom Viewer (Qt)" );
  myViewer->show();

  // Loop forever
  return app.exec();
}



