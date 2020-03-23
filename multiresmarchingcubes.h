#ifndef __IMT_MULTIRESMARCHINGCUBES_H__
#define __IMT_MULTIRESMARCHINGCUBES_H__


#include "volumeinfo.h"

namespace imt {

	namespace volume {


		class MultiResMarchingCubes {


		public:


			MultiResMarchingCubes( imt::volume::VolumeInfo& volume , int isoThreshold , int maxLevel );

			void compute();


		protected:


			struct EdgeInfo
			{
				int edgeType;
				int edgeId;
				int64_t trianglePointId;
				Eigen::Vector3f coords;
			};

			class GradientCalculator 
			{

			public:

				GradientCalculator(int64_t width, int64_t height, int64_t depth, unsigned short* volumeData);

				void computeGradient(float x, float y, float z, Eigen::Vector3f& gradient);

				float valueAt(float x, float y, float z);

			protected:

				int64_t _Width, _Height, _Depth;

				int64_t _ZStep, _YStep;

				unsigned short *_VolumeData;

				int _KernelGx[3][3][3], _KernelGy[3][3][3], _KernelGz[3][3][3];
			};




			//sample corners at resolution level 2
			void sampleCorners2( int x , int y , int z, std::vector<float>& grayValues);

			//sample corners at resolution level 1
			void sampleCorners1( int x , int y , int z, std::vector<float>& grayValues);

			//sample corners at resolution level 0
			void sampleCorners( int x , int y , int z, std::vector<float>& grayValues , int step = 1);

			void sampleCornersFromReinterpolatedSlices(int x, int y, int z, float* reinterpolatedSlices, std::vector<float>& grayValues, int step = 1);

			double grayValueAtCorner(int x, int y, int z);

			void createSurface( std::vector< float > &leaf_node,
				                Eigen::Vector3i &index_3d ,	std::vector<Eigen::Vector3f> &cloud , std::vector<EdgeInfo>& edgeInfos , int step = 1 );


			void interpolateEdge( Eigen::Vector3f &p1 ,	Eigen::Vector3f &p2,
				                  float val_p1 , float val_p2 ,	Eigen::Vector3f &output);



			void interpolateEdgeX(Eigen::Vector3i& voxelPosition,
				 float val_p1, float val_p2, Eigen::Vector3f &output, int step = 1);
			void interpolateEdgeY(Eigen::Vector3i& voxelPosition,
				 float val_p1, float val_p2, Eigen::Vector3f &output, int step = 1);

			void interpolateEdgeZ(Eigen::Vector3i& voxelPosition,
				 float val_p1, float val_p2, Eigen::Vector3f &output, int step = 1);

			
			void reinterpolateSlices( int z , int strip , float *reinterpolatedSlices );


		protected:

			void mergeDuplicateVertices(std::vector< MultiResMarchingCubes::EdgeInfo >& edgeInfos, std::vector<Eigen::Vector3f>& surfaceVertices , std::vector<unsigned int>& surfaceIndices , std::vector<Eigen::Vector3f>& mergedVertices );

		protected:

			imt::volume::VolumeInfo& _Volume;

			int _IsoThreshold , _MaxLevel;

			int64_t _ZStep, _YStep;

			double _VolumeReductionCoefficient;

			unsigned short *_VolumeData;

			

		};





	}


}





#endif