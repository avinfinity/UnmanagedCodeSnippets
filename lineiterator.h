#ifndef __VC_LINEIETERATOR_H__
#define __VC_LINEIETERATOR_H__

#include "voxel.h"
#include "eigenincludes.h"


namespace imt{
	
	namespace volume{
		
		
		class LineIterator
		{
			
			
		public:
			
			LineIterator(Eigen::Vector3f& vec, Voxel& initVox, Eigen::Vector3f& volumeOrigin, Eigen::Vector3f& voxelStep);
			
			Voxel next();
			

		protected:

			Voxel mPrevVox , mInitVox;

			Eigen::Vector3f mDir;

			Eigen::Vector3f mVolumeOrigin, mVoxelStep;

			float mTMaxX, mTMaxY, mTMaxZ;

			int mStepX, mStepY, mStepZ;
			
			
		};
		
		
		
		
	}
	
	
	
}




#endif