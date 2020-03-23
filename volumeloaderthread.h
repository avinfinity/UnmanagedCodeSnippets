#ifndef __IMT_VOLUMELOADER_THREAD__
#define __IMT_VOLUMELOADER_THREAD__

#include "QThread"
#include "QString"
#include "volumeinfo.h"
#include "QObject"

namespace imt{
	
	namespace volume{
		
		
	class VolumeLoaderThread : public QThread
	{


		Q_OBJECT
		
		
		public:
		
		
		VolumeLoaderThread( VolumeInfo *volumeInfo );
		
		void setFilePath( QString volumeFilePath );
		
		void run();


	signals:

		void volumeLoadedS();


	protected:

		QString mVolumeFilePath;

		VolumeInfo *mVolumeInfo;
		
	};	
		
		
		
		
		
		
	}
	
}





#endif