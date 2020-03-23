#pragma once

#include "stdio.h"
#include "string"
#include <Inventor/SbVec.h>
//#include "ZxoVec.h"
//#include <VolumeViz/readers/SoVolumeReader.h>
#include "volumetopooctree.h"
#include "cstdint"
#include  <functional>
#include "fstream"


class SoCpuBufferObject;

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

						void defaultProgressReporter(const float& progress, void *dataStructure, void* zxoWriter);

						void defaultImageProvider(const int&, void*);

						class OnlineTilesGenerator
						{
						public:

							typedef std::function<void(const float&, void *dataStructure, void* writer)> ProgressFunc;

							typedef std::function<void(const int&, void *, void*)> ImageProviderFunc;

							struct Vec3i32 {

								int _X, _Y, _Z;

							};

							OnlineTilesGenerator();

							bool generate(std::vector<SoCpuBufferObject*>& tileBuffers , ImageProviderFunc& func, void *imageProviderData);

							void setVolumeInfo(int w, int h, int d, float voxelSizeX, float voxelSizeY, float voxelSizeZ, int bitDepth, int rawHeaderOffset, int dataType);

							size_t tileSize();

							imt::volume::VolumeTopoOctree *topoTree();

							void setProgressCallback(ProgressFunc& func, void *progressDataStructure);

							void getHistogram(std::vector< int64_t >& histogram);

							~OnlineTilesGenerator();

						protected:


							void getImage(int zz, unsigned short* data);

							void buildLevelDimensions();

							void processLevel(int level);

							void processHighestResolutionLevelWithSliceRead();

							void computeMinMax(unsigned short *tile, unsigned int& minVal, unsigned int& maxVal);

							void sampleTile(const SbVec3i32& tileDim, int dataType, int border, const void* const octantTile[8], const int octantExists[8], void *parentTile);

							template <typename T>
							void sampleDecimation(const SbVec3i32& tileDim, const void* const childTile, void *parentTile, int octant, int border, const int octantExists[8])
							{
								SbVec2i32 shiftParentOctant;
								SbVec3i32 halfTileDim;
								SbVec3f ratio;
								SbVec3i32 shiftOctant;

								getShiftAndHalfTileDim(shiftParentOctant, halfTileDim, tileDim, octant, border);
								getRatio(ratio, shiftOctant, tileDim, halfTileDim, octant, border, octantExists);

								int tileDim1 = tileDim[0];
								int tileDim2 = tileDim[0] * tileDim[1];

								int ijkshiftParent = shiftParentOctant[0];

								T* parentPtr = (T*)parentTile;
								T* octantPtr = (T*)childTile;

								int pLine, pSlice = 0;
								int iShift, jShift, kShift, ioctant, iparent = 0;

								for (int k = 0; k < halfTileDim[2] * tileDim2; k += tileDim2)
								{
									pLine = 0;
									for (int j = 0; j < halfTileDim[1] * tileDim1; j += tileDim1)
									{
										for (int i = 0; i < halfTileDim[0]; i++)
										{
											iparent = (k + j + i);
											iShift = (int)floor((i * ratio[0]) + 0.5);
											jShift = (int)floor((pLine * ratio[1]) + 0.5) * tileDim1;
											kShift = (int)floor((pSlice * ratio[2]) + 0.5) * tileDim2;
											ioctant = (iShift + shiftOctant[0]) + (jShift + tileDim1*shiftOctant[1]) + (kShift + tileDim2*shiftOctant[2]);

											iparent += ijkshiftParent;
											parentPtr[iparent] = octantPtr[ioctant];
										}

										// If the octant next to current does not exist,
										// fill parent tile with proper value to avoid bad interpolation

										// 1 - Fill X axis with the last value of the line
										// only for even octant
#if 1
										if (!(octant % 2) && !octantExists[octant + 1])
										{
											int baseOffset = ijkshiftParent + (pSlice * tileDim2) + (tileDim1 * pLine) + halfTileDim[0];
											std::fill(parentPtr + baseOffset, parentPtr + baseOffset + halfTileDim[0], parentPtr[iparent]);
										}
#endif
										pLine++;
									}

									// 2 - Fill Y axis with the last line of the octant
									if ((octant == 0 || octant == 1 || octant == 4 || octant == 5) && !octantExists[octant + 2])
									{

										int dstOffset = (halfTileDim[0] * (octant % 2)) + (tileDim2 * pSlice) + tileDim[0] * halfTileDim[1];
										int srcOffset = (halfTileDim[0] * (octant % 2)) + (tileDim2 * pSlice) + (tileDim[0] * (halfTileDim[1] - 1));

										if (octant == 4 || octant == 5)
											dstOffset += halfTileDim[2] * tileDim2;

										for (int y = 0; y < halfTileDim[0]; y++)
										{
											memcpy(parentPtr + dstOffset, parentPtr + srcOffset, halfTileDim[0] * sizeof(unsigned short));

											dstOffset += tileDim[0];
										}


									}

									// 3 - Fill other part of the parent tile
									if ((octant == 0 || octant == 4) && !octantExists[octant + 3])
									{
										int dstOffset = halfTileDim[0] + (halfTileDim[1] * tileDim[0]) + (pSlice * tileDim2);

										if (octant == 4)
											dstOffset += halfTileDim[2] * tileDim2;

										int neededByte = ((halfTileDim[1] - 1) * tileDim[0]) + halfTileDim[0];
										for (int i = 0; i < halfTileDim[0]; i++)
										{
											std::fill(parentPtr + dstOffset, parentPtr + dstOffset + halfTileDim[0], parentPtr[neededByte]);
											dstOffset += tileDim[0];
										}
									}

									pSlice++;
								}
							};

							void getShiftAndHalfTileDim(SbVec2i32& shiftParentOctant, SbVec3i32& halfTileDim, const SbVec3i32& tileDim, int octant, int border) const;

							/// <summary>
							/// used by our custom sample tile algorithm
							/// </summary>
							void getRatio(SbVec3f &ratio, SbVec3i32 &shiftOctant, SbVec3i32 tileDim, SbVec3i32 halfTileDim, int octant, int border, const int octantExists[8]) const;



						protected:

							ProgressFunc _UpdateProgress;
							ImageProviderFunc _ImageProviderFunc;
							void *_ProgressDataStructure, *_ImageProviderDataStruct;
							void *_ImageProviderData;
							float _Progress;
							imt::volume::VolumeTopoOctree *_TopoTree;
							std::vector< Vec3i32 > mLevelDimensions;
							int64_t _Histogram[USHRT_MAX + 1];

							size_t _TileDim, _VolumeSizeX, _VolumeSizeY, _VolumeSizeZ;
							double _VoxelSizeX, _VoxelSizeY, _VoxelSizeZ;

							size_t _MinVal, _MaxVal;
							bool _CancelConversion;

							std::vector< SoCpuBufferObject* > _TileBuffers;

							size_t _FileIdCounter;
						};
					}
				}
			}
		}
	}
}

