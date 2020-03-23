#include "viewerwidget.h"
#include "QApplication"
#include "QString"
#include "QFile"
#include "QDataStream"
#include <Inventor/lock/SoLockMgr.h>
#include <Inventor/errors/SoDebugError.h>

void UnlockLicense();

int main( int argc , char **argv )
{


	UnlockLicense();

	QApplication app(argc, argv);

	//imt::volume::VolumeInfo volInfo;
	
	ViewerWidget vw;

	//vw.setVolInfo(&volInfo);

	vw.show();

	return app.exec();
}


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

