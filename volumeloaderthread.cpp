#include "volumeloaderthread.h"
#include "QFile"
#include "QFileInfo"
#include "iostream"

namespace imt{
	
	namespace volume{
		
		
		
		VolumeLoaderThread::VolumeLoaderThread(VolumeInfo *volumeInfo) : mVolumeInfo( volumeInfo )
		{

		}

		void VolumeLoaderThread::setFilePath( QString volumeFilePath )
		{
			mVolumeFilePath = volumeFilePath;
		}

		void VolumeLoaderThread::run()
	    {

			if ( !mVolumeInfo )
			{
				return;
			}

			QFileInfo fileInfo(mVolumeFilePath);

			if ( fileInfo.suffix() == "wtadat" )
			{

				std::cout << "  loading volume " << std::endl;

				mVolumeInfo->loadVolume(mVolumeFilePath);



				if (mVolumeInfo->mVolumeElementSize == 2)
				{
					//mVolumeInfo->convertToByteArray();
				}

				
			}


			//QFile file("C:/projects/Wallthickness/data/Engine.raw");

			//file.open(QIODevice::ReadOnly);

			//QByteArray data = file.readAll();

			//mVolumeInfo->mWidth = 256;
			//mVolumeInfo->mHeight = 256;
			//mVolumeInfo->mDepth = 110;

			//if ( mVolumeInfo->mVolumeData )
			//{
			//	delete [] mVolumeInfo->mVolumeData;
			//}

			//mVolumeInfo->mVolumeData = new unsigned char[ mVolumeInfo->mWidth * mVolumeInfo->mHeight * mVolumeInfo->mDepth ];
			//
			//memcpy( mVolumeInfo->mVolumeData, data.data(), mVolumeInfo->mWidth * mVolumeInfo->mHeight * mVolumeInfo->mDepth );

		}
		
		
	}
	
}


#include "volumeloaderthread.moc"