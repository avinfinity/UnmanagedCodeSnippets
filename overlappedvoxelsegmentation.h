#ifndef __IMT_OVERLAPPEDVOXELSEGMENTATION_H__
#define __IMT_OVERLAPPEDVOXELSEGMENTATION_H__

#include "volumeinfo.h"

namespace imt{
	
	namespace volume{

		
	class OverlappedVoxelsSegmentation
	{



	public:

		class SobelGradientOperator3x3x3
		{
			int _KernelGx[3][3][3], _KernelGy[3][3][3], _KernelGz[3][3][3];

		public:


			SobelGradientOperator3x3x3();

			void init(size_t volumeW, size_t volumeH, size_t volumeD, double voxelSizeX,
				double voxelSizeY, double voxelSizeZ, unsigned short *volumeData);

			void apply(Eigen::Vector3f& point, Eigen::Vector3f& gradient);

		protected:


			size_t _VolumeW, _VolumeH, _VolumeD;

			int64_t _StepZ, _StepY;

			double _VoxelSizeX, _VoxelSizeY, _VoxelSizeZ;

			unsigned short *_VolumeData;
		};


		
		OverlappedVoxelsSegmentation( imt::volume::VolumeInfo& volumeInfo );

		void compute( std::vector< std::pair<int , int> >& initialMaterialRegions );



	protected:

		void fillMask( Eigen::Vector3i inputPoint, int materialId , int ignoreMaterialId , unsigned char* mask);
		

	protected:


		std::vector<int> _GrayValueToMaterialId;


		imt::volume::VolumeInfo& _VolumeInfo;
		
		SobelGradientOperator3x3x3 *_GradientOperator;

		int64_t _StepZ, _StepY;

		double _VoxelSizeX, _VoxelSizeY, _VoxelSizeZ;

		unsigned short *_VolumeData;
		
	};		
	
	
	
	
	
	}
	
	
	
}



#endif