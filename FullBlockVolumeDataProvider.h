#pragma once
#include "IVolumeDataProvider.h"
#include "OnlineTilesGenerator.h"

namespace Zeiss {
	namespace IMT {

		namespace NG {

			namespace NeoInsights {

				namespace Volume {

					namespace DataProcessing {


						class CPUBuffer;


						class FullBlockVolumeDataProvider
						{

						public:



							struct RoiData
							{
								size_t _Resolution;
								size_t Width, Height, Depth;
								size_t _StartX, _StartY, _StartZ;

								CPUBuffer *_Buffer;
							};

							struct SliceData
							{
								//do note that this coordinate system can be built using plane normal and plane position
								double _CoordSys[16];

								size_t _SliceWidth, _SliceHeight;

								CPUBuffer *_Buffer;

							};


							FullBlockVolumeDataProvider( CPUBuffer *fullVolumeBlock, size_t volumeW, size_t volumeH, size_t volumeD,
								                         double *voxelSize/*, OnlineTilesGenerator::ProgressFunc& progressUpdateF, void* progressDataStruct, void *dataManager,
								                         double initProgress, double progressBand*/);

							virtual void GetRoiBuffer(RoiData& roiData);
							virtual void GetSliceBuffer(SliceData& sliceData);
							virtual void GetSliceBuffer(SliceData& sliceData, std::vector< double >& colorMap);
							virtual const CPUBuffer* GetFullVolumeData() const;

							virtual ~FullBlockVolumeDataProvider();


						protected:


							void clearData();


							struct VolumeDim
							{
								size_t Width, Height, Depth;

							};

							unsigned short valueAt(double x, double y, double z, unsigned int width, unsigned int height,const unsigned short *volumeData);

							void interpolateImage(const double* xAxis, const double* yAxis, const double* origin, double step, double *voxelSize, int w, int h,
								int volumeW, int volumeH, int volumeD, const unsigned short *volumeData, unsigned short* interpolatedImage);

							void interpolateImage(const double* xAxis, const double* yAxis, const double* origin, double step, double *voxelSize, int w, int h,
								int volumeW, int volumeH, int volumeD, std::vector< double >& colorMap, const unsigned short *volumeData, unsigned short* interpolatedImage);

							void computeSliceDimensions(double wS, double hS, double dS, double voxelSize, double* transform, int& w, int& h);

							void matrixVectorMultiplication(int m, int n, const double *matrix, const double *vec, double *outputVec);


						protected:

							double _VoxelSize[3];

							size_t _VolumeWidth, _VolumeHeight, _VolumeDepth;
						
							CPUBuffer *_RawVolumeData;

							std::vector< CPUBuffer* > _RAWVolumeDataAtLevels;

							std::vector< VolumeDim > _LevelDimensions;
						
						};



					}

				}


			}


		}

	}
}
