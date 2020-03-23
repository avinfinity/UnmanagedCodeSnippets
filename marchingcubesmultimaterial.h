#ifndef __VC_MARCHINGCUBESMULTIMATERIAL_H__
#define __VC_MARCHINGCUBESMULTIMATERIAL_H__

#include "volumeinfo.h"

namespace imt {

	namespace volume {


		class MarchingCubesMultiMaterial {


		public:

			MarchingCubesMultiMaterial( imt::volume::VolumeInfo& volume , std::vector<std::pair<int , int>>& isoThresholds);

			void compute();

			void compute(int isoThreshold);



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

				double _VoxelSizeX, _VoxelSizeY, _VoxelSizeZ;

				unsigned short *_VolumeData;
			};


		protected:

			void createSurface( const std::vector<float> &leaf_node , const Eigen::Vector3i &index_3d , 
				                std::vector<Eigen::Vector3f> &cloud );


			void createSurface( const std::vector<float> &leaf_node, const Eigen::Vector3i &index_3d,
				                std::vector<Eigen::Vector3f> &cloud , int isoThreshold );


			void getNeighborList1D( std::vector<float> &leaf , Eigen::Vector3i &index3d );


			void interpolateEdge(Eigen::Vector3f& p1, Eigen::Vector3f& p2, float val_p1, float val_p2, Eigen::Vector3f& output);
			void interpolateEdge(Eigen::Vector3f& p1, Eigen::Vector3f& p2, float val_p1, float val_p2, Eigen::Vector3f& output , int isoThreshold);

			Eigen::Vector3f indexWiseMul( const Eigen::Vector3f& vec1, const Eigen::Vector3i index );

			bool validateTriangle(Eigen::Vector3f* triangle, int materialId  );


		protected:

			imt::volume::VolumeInfo& _Volume;

			std::vector<std::pair<int, int>> _IsoThresholds;

			int _IsoLevel;

			std::vector<int> _GrayValueToMaterialId;

			Eigen::Vector3f _StartPoint;

			SobelGradientOperator3x3x3 *_DerivativeOperator;


		};



	}





}





#endif