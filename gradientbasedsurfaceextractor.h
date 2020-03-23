#ifndef __IMT_GRADIENTBASEDSURFACEEXTRACTOR_H__
#define __IMT_GRADIENTBASEDSURFACEEXTRACTOR_H__

#include "MultiResolutionMarchingCubes.h"
#include "eigenincludes.h"
#include <vector>


using namespace Zeiss::IMT::NG::NeoInsights::Volume::MeshGenerator;

namespace imt {

	namespace volume {


		class GradientBasedSurfaceExtractor : public  MultiResolutionMarchingCubes
		{

		public:

			GradientBasedSurfaceExtractor(unsigned short* volumeData, int32_t volumeWidth, int32_t volumeHeight, int32_t volumeDepth,
				double voxelStepX, double voxelStepY, double voxelStepZ);


			void compute( int grayValue1, int grayValue2, std::vector<double>& vertices, std::vector<double>& vetexNormals,
				          std::vector<unsigned int>& surfaceIndices );



		protected:

			void sampleCorners(int x, int y, int z, std::vector<float>& grayValues, int step);

			bool sampleCornersBasedOnGradient(int x, int y, int z, std::vector<bool>& grayValues , std::vector<Eigen::Vector3f>& edgePoints , int step );

			bool updateCornerFlags( std::vector<int>& edgeFlags , std::vector<int>& cornerFlags );
				 
			bool checkGradientMaximaOnEdgeX( int x , int y , int z , int step );

			float maxGradientX(int x, int y, int z, int step);
			float maxGradientY(int x, int y, int z, int step);
			float maxGradientZ(int x, int y, int z, int step);

			void gradientBasedInterpolateX(int x, int y, int z, int step, Eigen::Vector3f& interpolatedPoint);
			void gradientBasedInterpolateY(int x, int y, int z, int step, Eigen::Vector3f& interpolatedPoint);
			void gradientBasedInterpolateZ(int x, int y, int z, int step, Eigen::Vector3f& interpolatedPoint);

			void processEdgeX( int x, int y, int z, int step, int cornerId1,
				               int cornerId2, int edgeId, std::vector<int>& cornerFlag,
				               std::vector<int>& edgeFlag , Eigen::Vector3f& edgePoint );
			void processEdgeY(int x, int y, int z, int step, int cornerId1,
				int cornerId2, int edgeId, std::vector<int>& cornerFlag,
				std::vector<int>& edgeFlag, Eigen::Vector3f& edgePoint);

			void processEdgeZ(int x, int y, int z, int step, int cornerId1,
				int cornerId2, int edgeId, std::vector<int>& cornerFlag,
				std::vector<int>& edgeFlag, Eigen::Vector3f& edgePoint);


		protected:

			int _StartThreshold, _EndThreshold;


		};



	}



}





#endif