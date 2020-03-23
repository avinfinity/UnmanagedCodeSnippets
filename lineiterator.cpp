#include "lineiterator.h"
#include "iostream"

namespace imt{
	
	namespace volume{



		void computeIntersectionCoeff( Eigen::Vector3f& origin , Eigen::Vector3f& dir , Voxel& vox , Eigen::Vector3f& volumeOrigin , Eigen::Vector3f& voxelStep , Eigen::Vector3f& delT )
		{
			float x1 = vox.x * voxelStep(0) + volumeOrigin(0), 
				  x2 = ( vox.x + 1 ) * voxelStep(0) + volumeOrigin(0), 
				  y1 =  vox.y * voxelStep(1) + volumeOrigin(1), 
				  y2 = ( vox.y + 1 ) * voxelStep(1) + volumeOrigin(1), 
				  z1 = vox.z * voxelStep(2) + volumeOrigin(2), 
				  z2 = ( vox.z + 1 ) * voxelStep(2) + volumeOrigin(2);


			float t = 0;

			float length = voxelStep.norm() * 0.5;

			length *= 1.0001f;

			Eigen::Vector3f n1(1, 0, 0), n2(0, 1, 0), n3(0, 0, 1);

			float t1 = (x1 - origin.dot(n1)) / (dir.dot(n1));
			float t2 = (x2 - origin.dot(n1)) / (dir.dot(n1));

			float t3 = (y1 - origin.dot(n2)) / (dir.dot(n2));
			float t4 = (y2 - origin.dot(n2)) / (dir.dot(n2));

			float t5 = (z1 - origin.dot(n3)) / (dir.dot(n3));
			float t6 = (z2 - origin.dot(n3)) / (dir.dot(n3));

			Eigen::Vector3f p;

			if ( t1 > 0 && t1 < length )
			{
				p = origin + dir * t1;
			}
			else if ( t2 > 0 && t2 < length )
			{
				p = origin + dir * t2;
			} 
			else if ( t3 > 0 && t3 < length )
			{
				p = origin + dir * t3;
			}
			else if ( t4 > 0 && t4 < length )
			{
				p = origin + dir * t4;
			}
			else if ( t5 > 0 && t5 < length )
			{
				p = origin + dir * t5;
			}
			else if ( t6 > 0 && t6 < length )
			{
				p = origin + dir * t6;
			}

			delT = p - origin;


		}
		
		
			LineIterator::LineIterator( Eigen::Vector3f& vec , Voxel& initVox , Eigen::Vector3f& volumeOrigin , Eigen::Vector3f& voxelStep ) 
			{
				mInitVox = initVox;

				mPrevVox = mInitVox;

				mDir = vec;

				mVolumeOrigin = volumeOrigin;
				mVoxelStep = voxelStep;

				Eigen::Vector3f origin;

				origin(0) = ( mInitVox.x + 0.5 ) * voxelStep(0) + volumeOrigin( 0 );
				origin(1) = (mInitVox.y + 0.5) * voxelStep(1) + volumeOrigin( 1 );
				origin(2) = (mInitVox.z + 0.5) * voxelStep(2) + volumeOrigin( 2 );

				Eigen::Vector3f delT;

				computeIntersectionCoeff(origin, mDir, mInitVox, mVolumeOrigin, mVoxelStep, delT);

				

				mTMaxX = std::abs( delT(0) );
				mTMaxY = std::abs( delT(1) );
				mTMaxZ = std::abs( delT(2) );

				mStepX = (mDir(0) > 0 ? 1 : -1);
				mStepY = mDir(1) > 0 ? 1 : -1;
				mStepZ = mDir(2) > 0 ? 1 : -1;
				
			}
			
			Voxel LineIterator::next()
			{
				float tDeltaX = 1.0 / std::abs(mDir(0)), tDeltaY = 1.0 / std::abs(mDir(1)), tDeltaZ = 1.0 / std::abs( mDir(2) );

				float X = mPrevVox.x, Y = mPrevVox.y, Z = mPrevVox.z;

				if ( mTMaxX < mTMaxY && mTMaxX < mTMaxZ )
				{
					mTMaxX += tDeltaX;
					mPrevVox.x += mStepX;
				}
				else if ( mTMaxY < mTMaxZ )
				{
					mTMaxY += tDeltaY;
					mPrevVox.y += mStepY;
				}
				else
				{
					mTMaxZ += tDeltaZ;
					mPrevVox.z += mStepZ;
				}

				return mPrevVox;
			}
		
		
	}
	
}