#ifndef __ZEISS_IMT_NG_NEOINSIGHTS_VOLUME_DATAPROCESSING_MESHGENERATOR_H__
#define __ZEISS_IMT_NG_NEOINSIGHTS_VOLUME_DATAPROCESSING_MESHGENERATOR_H__

#include <stdint.h>
#include <vector>


namespace Zeiss {
	namespace IMT {

		namespace NG {

			namespace NeoInsights {

				namespace Volume {

					namespace MeshGenerator {


						class SobelGradientOperator;


						class MultiResolutionMarchingCubes
						{



						protected:

							struct Vector3d
							{

								double x, y, z;

								Vector3d();
								Vector3d(double xVal , double yVal , double zVal);

							};


							struct EdgeInfo
							{

								int _EdgeType;
								int64_t _EdgeId;
								int64_t _TrianglePointId;
								Vector3d _Coords;
							};



							struct Index3d
							{

								int x, y, z;

								Index3d()
								{
									x = 0;
									y = 0;
									z = 0;
								}

								Index3d(int x, int y, int z)
								{
									this->x = x;
									this->y = y;
									this->z = z;
								}


								inline Vector3d operator * (const double& step)
								{

									Vector3d coords;

									coords.x = x * step;
									coords.y = y * step;
									coords.z = z * step;

									return coords;

								}

							};


						public:

							MultiResolutionMarchingCubes(unsigned short* volumeData, int32_t volumeWidth, int32_t volumeHeight, int32_t volumeDepth,
								double voxelStepX, double voxelStepY, double voxelStepZ);


							void compute(int32_t isoThreshold, std::vector<double>& vertices, std::vector<double>& vetexNormals, std::vector<unsigned int>& surfaceIndices);


							~MultiResolutionMarchingCubes();



						protected:



							void interpolateEdge(Vector3d &p1, Vector3d &p2,
								float val_p1, float val_p2, Vector3d &output);

							int interpolateEdgeX(Index3d& voxelPosition,
								float val_p1, float val_p2, Vector3d &output, int step);

							int interpolateEdgeY(Index3d& voxelPosition,
								float val_p1, float val_p2, Vector3d &output, int step);

							int interpolateEdgeZ(Index3d& voxelPosition,
								float val_p1, float val_p2, Vector3d &output, int step);


							void mergeDuplicateVertices(std::vector< MultiResolutionMarchingCubes::EdgeInfo >& edgeInfos, std::vector<double>& surfaceVertices,
								std::vector<unsigned int>& surfaceIndices);


							bool createSurface(std::vector< float > &leaf_node,
								MultiResolutionMarchingCubes::Index3d &index_3d,
								std::vector<MultiResolutionMarchingCubes::EdgeInfo>& edgeInfos, SobelGradientOperator* gradientOperator,
								int step );

							bool isCorner(Index3d index , int res);


							void reinterpolateSlices(int z, int strip, float *reinterpolatedSlices);

							//sample corners at resolution level 2
							void sampleCorners2(int x, int y, int z, std::vector<float>& grayValues);

							//sample corners at resolution level 1
							void sampleCorners1(int x, int y, int z, std::vector<float>& grayValues);

							float grayValueAtCorner(int x, int y, int z);

							//sample corners at resolution level 0
							void sampleCorners(int x, int y, int z, std::vector<float>& grayValues, int step = 1);

						protected:

							unsigned short *_VolumeData;
							int32_t _VolumeWidth, _VolumeHeight, _VolumeDepth;
							double _VoxelStepX, _VoxelStepY, _VoxelStepZ;

							int  _MaxLevel;

							double _IsoThreshold;

							int64_t _ZStep, _YStep;

							double _DeviationThreshold;


						};

					}
				}
			}
		}
	}
}


#endif

