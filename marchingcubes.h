#ifndef __IMT_MARCHINGCUBES_H__
#define __IMT_MARCHINGCUBES_H__

#include "volumeinfo.h"
#include "eigenincludes.h"


namespace imt
{
	
	namespace volume
	{
		
        class MarchingCubes
		{
			
			public:
			
			MarchingCubes( imt::volume::VolumeInfo& volume , int isoThreshold );
		    
            std::vector<Eigen::Vector3f> compute();

		protected:

			void interpolateEdge3(float* p1, float* p2, float val_p1, float val_p2, float* output);

			void createSurface(const std::vector<float> &leaf_node,
				const Eigen::Vector3i &index_3d,
				std::vector<Eigen::Vector3f> &cloud);

			void interpolateEdge(Eigen::Vector3f& p1, Eigen::Vector3f& p2, float val_p1, float val_p2, Eigen::Vector3f& output);

			void getNeighborList1D(std::vector<float> &leaf,
					Eigen::Vector3i &index3d);
			
 
		protected:

			unsigned int _IsoLevel;
			Eigen::Vector3f _StartPoint;
			Eigen::Vector3f _VoxelLength;

			imt::volume::VolumeInfo &_VolumeInfo;
		};
		

		struct DMCVertex {

			Eigen::Vector3f position;
			float sign;
			float offset;

		};
		
		void runMarchingCubes(const std::vector<DMCVertex>& vertices,
			std::vector<Eigen::Vector3f>& surfaceTriangles);
		
		
	}
	
}




#endif
