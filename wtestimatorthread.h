#ifndef __IMT_WTESTIMATORTHREAD_H__
#define __IMT_WTESTIMATORTHREAD_H__

#include "volumeinfo.h"
#include "QThread"
#include "wallthicknessestimator.h"


namespace imt{

	namespace volume{


		class WTEstimatorThread : public QThread
		{

		public:

			enum METHOD{ RAY = 0 , SPHERE  };

			WTEstimatorThread(imt::volume::VolumeInfo *volume, imt::volume::WallthicknessEstimator *wte);

			void setMethod(METHOD method);

			void run();


		protected:

			imt::volume::VolumeInfo *mVolume;

			METHOD mMethod;

			imt::volume::WallthicknessEstimator *mWte;


		};





	}




}


#endif