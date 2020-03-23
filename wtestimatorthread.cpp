#include "wtestimatorthread.h"


namespace imt{

	namespace volume
	{

		WTEstimatorThread::WTEstimatorThread(imt::volume::VolumeInfo *volume, imt::volume::WallthicknessEstimator *wte) : mVolume(volume), mWte( wte )
		{


		}



		void WTEstimatorThread::setMethod(METHOD method)
		{

			mMethod = method;

		}



		void WTEstimatorThread::run()
		{
			if ( mMethod == RAY )
			{
				mWte->computeBrepRayTraceThickness(*mVolume);
			}
			else
			{
				mWte->computeBrepSphereThickness(*mVolume);
			}


		}




	}




}