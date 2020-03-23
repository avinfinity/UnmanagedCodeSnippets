#ifndef __IMT_VOLUMESEGMENTER_H__
#define __IMT_VOLUMESEGMENTER_H__

#include "volumeinfo.h"
#include "voxel.h"
#include "maxflow/graph.h"

namespace imt{
	
	
	namespace volume{
		
		typedef Graph< unsigned short, unsigned short, long > VolumeGraph;


		
		
		class VolumeSegmenter
		{
			
			
			public:

				struct BoundingBox{

					int mXMin, mXMax, mYMin, mYMax, mZMin, mZMax;

				};

			
			VolumeSegmenter( imt::volume::VolumeInfo& volInfo );

			void compute( int isoValue , int range , int band );

		protected:

			void buildGraph();

			void detectEdgePoints();

			void buildPartitions(std::vector< Voxel >& voxels, std::vector< std::vector<Voxel> >& voxelPartitions, std::vector< BoundingBox >& boundingBoxes);

			void computeBoundingBox( std::vector< Voxel >& voxels, BoundingBox& bb );


		protected:


			imt::volume::VolumeInfo& mVolumeInfo;
			std::vector< Voxel > mEdgeVoxels;
			std::vector< Eigen::Vector3f > mEdgePoints, mEdgePointNormals;

			VolumeGraph *mVolGraph;

			


		};
		
		
		
		
		
		
	}
	
	
	
	
	
}





#endif