#ifndef __IMT_VOLUME_VOLUMERENDERERCPU_H__
#define __IMT_VOLUME_VOLUMERENDERERCPU_H__

#include <vector>
#include "trackballcamera.h"
#include "TransferFunction.h"

namespace imt 
{

	namespace volume 
	{

		class VolumeRendererCPU 
		{


		public:

			VolumeRendererCPU( unsigned short* volumeData , int w , int h , int d );

			void render( TrackBallCamera* camera );

			void setTransferFunction(TransferFunction* transferFunction);



		protected:

			int mWidth, mHeight, mDepth;

			unsigned short* mVolumeData;

			std::vector<int64_t> mHistogram;

			TrackBallCamera* mCamera;

			TransferFunction* mTransferFunction;
		};



	}


}



#endif