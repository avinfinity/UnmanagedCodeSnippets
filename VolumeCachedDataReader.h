#ifndef __VOLUMECACCHEDATAREADER_H__
#define __VOLUMECACCHEDATAREADER_H__


#include "stdio.h"
#include "string"
#include "OnlineTiledReader.h"

namespace Zeiss
{
	namespace IMT
	{
		namespace NG
		{
			namespace NeoInsights
			{
				namespace Volume
				{
					namespace DataProcessing
					{


						class VolumeCachedDataReader
						{

						public:


							VolumeCachedDataReader();

							~VolumeCachedDataReader();

							static void CachedDataImageProviderCallBack(const int& zz, void *sliceDataProvider, void *sliceData);

							void initialize( std::wstring filePath, size_t volumeWidth, size_t volumeHeight,
								             size_t volumeDepth, double* voxelSize, size_t headerSkipSize, 
											 double renderTargetRatio, OnlineTilesGenerator::ProgressFunc func, 
											 void* progressDataStruct);

							void updateCache(unsigned int isoThreshold);

							void ReadSlice(size_t z, unsigned short *sliceData, bool buildAdditionalInfo = true);

							void ReadSliceOnline(size_t z, unsigned short *sliceData);

							unsigned short *getRawVolumeData();

							OnlineTiledReader *getTileReader();

							void setRawVolumeData(unsigned short*& rawDataPtr);

							void CalculateIntertiaTensor(const INT64& threshold, double* centerOfGravityVoxel, double* inertialTensor);

							void CalculateCenterOfGravity(const INT64& threshold, double *cogVoxel, double *cogWorld);

							size_t width();
							size_t height();
							size_t depth();

							void releaseRawData();

							std::vector< float > _ColorMap;
							size_t _IsoThreshold;

							void updateColorMap();

						protected:

							void processSlice(size_t z, unsigned short *sliceData);

						protected:

							unsigned short *_RawVolumeData ,  *_ResizedVolumeData;



							double _VoxelSize[3];

							FILE *_RAWFile;

							size_t _VolumeWidth, _VolumeHeight, _VolumeDepth;

							size_t _ResizedWidth, _ResizedHeight, _ResizedDepth;

							size_t _HeaderSkipSize;

							OnlineTiledReader *_Reader;



						};
					}
				}
			}
		}
	}
}




#endif