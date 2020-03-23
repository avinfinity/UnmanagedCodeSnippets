#include "stdafx.h"
#include "OnlineTilesGenerator.h"
#include <algorithm>
#include "omp.h"
#include <mutex>
#include <atomic>
#include <algorithm>

#include <Inventor/devices/SoCpuBufferObject.h>

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

						OnlineTilesGenerator::OnlineTilesGenerator()
						{
							_TopoTree = new imt::volume::VolumeTopoOctree();

							_CancelConversion = false;

						}


						void OnlineTilesGenerator::setVolumeInfo(int w, int h, int d, float voxelSizeX, float voxelSizeY,
							float voxelSizeZ, int bitDepth, int rawHeaderOffset, int dataType)
						{
						
							_VolumeSizeX = w;
							_VolumeSizeY = h;
							_VolumeSizeZ = d;
						
							_VoxelSizeX = voxelSizeX;
							_VoxelSizeY = voxelSizeY;
							_VoxelSizeZ = voxelSizeZ;

							_TileDim = 256;
							
							imt::volume::VolumeTopoOctree::Vec3i dim;

							dim._X = _VolumeSizeX;
							dim._Y = _VolumeSizeY;
							dim._Z = _VolumeSizeZ;

							int minDim = INT_MAX;

							int maxDim = 0;

							minDim = std::min(minDim, (int)_VolumeSizeX);
							minDim = std::min(minDim, (int)_VolumeSizeY);
							minDim = std::min(minDim, (int)_VolumeSizeZ);

							maxDim = std::max(maxDim, (int)_VolumeSizeX);
							maxDim = std::max(maxDim, (int)_VolumeSizeY);
							maxDim = std::max(maxDim, (int)_VolumeSizeZ);


							if (minDim > 256)
							{
								_TileDim = 256;
							}
							else if (minDim > 128)
							{
								_TileDim = 128;
							}
							else
							{
								_TileDim = 64;
							}

							if (maxDim <= 512)
							{
								_TileDim = std::min((int)_TileDim, 128);
							}


							_TopoTree->init(dim, _TileDim, 0);
						
						}


						size_t OnlineTilesGenerator::tileSize()
						{
							return _TileDim;
						}
						
						imt::volume::VolumeTopoOctree* OnlineTilesGenerator::topoTree()
						{
							return _TopoTree;
						}


						void OnlineTilesGenerator::setProgressCallback(ProgressFunc& func, void *progressDataStructure)
						{

							_ProgressDataStructure = progressDataStructure;
							_UpdateProgress = func;

						}


						void OnlineTilesGenerator::getHistogram(std::vector< int64_t >& histogram)
						{
							
							histogram.resize(USHRT_MAX + 1);

							memcpy(histogram.data(), _Histogram, (USHRT_MAX + 1) * sizeof(int64_t));
						}

						void OnlineTilesGenerator::buildLevelDimensions()
						{

							int numLevels = _TopoTree->getNumLevels();

							mLevelDimensions.resize(numLevels);

							for (int ll = 0; ll < numLevels; ll++)
							{
								if (ll == 0)
								{
									mLevelDimensions[ll]._X = _VolumeSizeX % _TileDim == 0 ? _VolumeSizeX : (_VolumeSizeX / _TileDim + 1) * _TileDim;
									mLevelDimensions[ll]._Y = _VolumeSizeY % _TileDim == 0 ? _VolumeSizeY : (_VolumeSizeY / _TileDim + 1) * _TileDim;
									mLevelDimensions[ll]._Z = _VolumeSizeZ % _TileDim == 0 ? _VolumeSizeZ : (_VolumeSizeZ / _TileDim + 1) * _TileDim;
								}
								else
								{
									int w = mLevelDimensions[ll - 1]._X / 2;
									int h = mLevelDimensions[ll - 1]._Y / 2;
									int d = mLevelDimensions[ll - 1]._Z / 2;

									mLevelDimensions[ll]._X = w  % _TileDim == 0 ? w : (w / _TileDim + 1) * _TileDim;
									mLevelDimensions[ll]._Y = h % _TileDim == 0 ? h : (h / _TileDim + 1) * _TileDim;
									mLevelDimensions[ll]._Z = d % _TileDim == 0 ? d : (d / _TileDim + 1) * _TileDim;
								}
							}
						}



						bool OnlineTilesGenerator::generate(std::vector<SoCpuBufferObject*>& tileBuffers , ImageProviderFunc& func, void *imageProviderData)
						{
							_ImageProviderFunc = func;

							_ImageProviderDataStruct = imageProviderData;

							_TileBuffers = tileBuffers;

							_Progress = 0;

							imt::volume::VolumeTopoOctree::Vec3i dim;

							dim._X = _VolumeSizeX;
							dim._Y = _VolumeSizeY;
							dim._Z = _VolumeSizeZ;

							int numLevels = _TopoTree->getNumLevels();
							int numThreads = omp_get_num_procs();

							int numFileIds = _TopoTree->getNumFiles();

							buildLevelDimensions();

							for (int ff = 0; ff < numFileIds; ff++)
							{
								auto fileNode = _TopoTree->getFileNode(ff);

								fileNode->_Data = (unsigned short*)_TileBuffers[ff]->map(SoBufferObject::READ_WRITE);
							}

							processHighestResolutionLevelWithSliceRead();

							//process each level
							for (int ll = numLevels - 2; ll >= 0; ll--)
							{
								if (_CancelConversion)
								{
									return false;
								}

								processLevel(ll);
							}

							//write here two information ( 1. min and max values for each tiles.  2. logical to disk map  )
							int nLevels = _TopoTree->getNumLevels();

							//_UpdateProgress(0.999f, _ProgressDataStructure, this);

							return true;
						}



						void OnlineTilesGenerator::processLevel(int level)
						{
							// Giving it to utilize maximum number of threads avaliable on system
							//leads to crashes on somesystem where cores are more than > 16.
							//Hence on a safer note it has been sustituted as 4. To be fixed later on.
							int numThreads = omp_get_num_procs();

							int levelW = mLevelDimensions[level]._X;
							int levelH = mLevelDimensions[level]._Y;
							int levelD = mLevelDimensions[level]._Z;

							int nextLevelW = mLevelDimensions[level + 1]._X;
							int nextLevelH = mLevelDimensions[level + 1]._Y;
							int nextLevelD = mLevelDimensions[level + 1]._Z;

							int numBands = levelD / (2 * _TileDim);

							int threadBandRange = numBands / numThreads;
							int incrementOffset = numBands % numThreads;

							auto currentLevelNodes = _TopoTree->getFileNodes(level);

							//check number of available processors on the system
							int numMaxThreads = omp_get_num_procs();

							//first sort the ndoes
							//we need to sort highest resolution tiles such that they are sorted in order of slices , this will make sure that we can read volume data from disk
							//in chunk of slices. Reading such a way would be both faster and better candidate for out of core implementation.
							std::sort(currentLevelNodes.begin(), currentLevelNodes.end(), [](const imt::volume::VolumeTopoOctree::TopoOctreeNode* item1, const imt::volume::VolumeTopoOctree::TopoOctreeNode* item2)->bool {

								return ((item1->_Position._Z < item2->_Position._Z) ||
									(item1->_Position._Z == item2->_Position._Z && item1->_Position._Y < item2->_Position._Y) ||
									(item1->_Position._Z == item2->_Position._Z && item1->_Position._Y == item2->_Position._Y && item1->_Position._X < item2->_Position._X));
							});


							std::atomic< int > nodeCounter = 0;

							unsigned short *neededBuffer = new unsigned short[_TileDim * _TileDim * _TileDim * numThreads * 9];

							std::mutex writeMutex;

							int tempFileId = 0;

							float totalFileIds = (float)_TopoTree->getNumFiles();

							int numCurrentLevelNodes = (int)currentLevelNodes.size();

#pragma omp parallel num_threads(numThreads)
							{
								int threadId = omp_get_thread_num();

								int bandWidth = threadBandRange;

								if (threadId <= incrementOffset)
								{
									bandWidth++;
								}

								unsigned short* currentTile = 0;

								while (1)
								{
									int nodeLoc = nodeCounter++;

									if (nodeLoc >= numCurrentLevelNodes)
									{
										break;
									}

									auto node = currentLevelNodes[nodeLoc];

									int fId = node->_NodeId;

									currentTile = node->_Data;

									memset(currentTile, 0, _TileDim * _TileDim * _TileDim * sizeof(unsigned short));

									void *octantTiles[8];
									int octantExists[8];

									for (int ii = 0; ii < 8; ii++)
									{
										if (node->_Chidren[ii])
										{
											octantExists[ii] = 1;

											size_t cFId = node->_Chidren[ii]->_NodeId;
											octantTiles[ii] = node->_Chidren[ii]->_Data;//_TileBuffers[cFId];
										}
										else
										{
											octantExists[ii] = 0;
											octantTiles[ii] = 0;
										}
									}

									SbVec3i32 tileDim;

									tileDim[0] = _TileDim;
									tileDim[1] = _TileDim;
									tileDim[2] = _TileDim;

									//this is the step which actually samples the low resolution tile from it's 8 high resolution children tiles
									sampleTile(tileDim, 1, 0, octantTiles, octantExists, currentTile);

									if (_CancelConversion)
									{
										delete[] neededBuffer;

										break;
									}

									computeMinMax(currentTile, node->_MinVal, node->_MaxVal);

									if ((_FileIdCounter / totalFileIds - _Progress) > 0.01)
									{
										//_Progress = _FileIdCounter / totalFileIds;

										//_UpdateProgress(_Progress, _ProgressDataStructure, this);
									}

#pragma omp critical
									{
										_FileIdCounter++;
									}


								}
							}

							delete[] neededBuffer;
						}




						void OnlineTilesGenerator::processHighestResolutionLevelWithSliceRead()
						{
							//first get the extended dimensions
							int extendedW = mLevelDimensions[0]._X;
							int extendedH = mLevelDimensions[0]._Y;
							int extendedD = mLevelDimensions[0]._Z;

							int numLevels = _TopoTree->getNumLevels();

							int level = numLevels - 1;
							auto highestResolutionNodes = _TopoTree->getFileNodes(numLevels - 1);


							//check number of available processors on the system
							int numMaxThreads = omp_get_num_procs();

							float totalFileIds = (float)_TopoTree->getNumFiles();


							//first sort the nodes
							//we need to sort highest resolution tiles such that they are sorted in order of slices , this will make sure that we can read volume data from disk
							//in chunk of slices. Reading such a way would be both faster and better candidate for out of core implementation.
							std::sort(highestResolutionNodes.begin(), highestResolutionNodes.end(), [](const imt::volume::VolumeTopoOctree::TopoOctreeNode* item1,
								const imt::volume::VolumeTopoOctree::TopoOctreeNode* item2)->bool
							{

								return ((item1->_Position._Z < item2->_Position._Z) ||
									(item1->_Position._Z == item2->_Position._Z && item1->_Position._Y < item2->_Position._Y) ||
									(item1->_Position._Z == item2->_Position._Z && item1->_Position._Y == item2->_Position._Y && item1->_Position._X < item2->_Position._X));
							});

							int numWrittenNodes = 0;

							for (int ii = 0; ii < (USHRT_MAX + 1); ii++)
							{
								_Histogram[ii] = 0;
							}

							size_t intermediateSize = _TileDim * _VolumeSizeX * _VolumeSizeY;

							unsigned short *intermediateBuffer = new unsigned short[intermediateSize];


							int zT = _VolumeSizeZ % _TileDim == 0 ? _VolumeSizeZ / _TileDim : _VolumeSizeZ / _TileDim + 1;
							int yT = _VolumeSizeY % _TileDim == 0 ? _VolumeSizeY / _TileDim : _VolumeSizeY / _TileDim + 1;
							int xT = _VolumeSizeX % _TileDim == 0 ? _VolumeSizeX / _TileDim : _VolumeSizeX / _TileDim + 1;

							size_t bufferSize = _TileDim * _TileDim * _VolumeSizeX;

							size_t sliceSize = _VolumeSizeX * _VolumeSizeY;

							_MinVal = UINT16_MAX;
							_MaxVal = 0;

							for (int tz = 0; tz < zT; tz++)
							{
								int zLen = _TileDim;

								if (tz == zT - 1)
								{
									zLen = _VolumeSizeZ - (zT - 1) * _TileDim;

									memset(intermediateBuffer, 0, intermediateSize * sizeof(unsigned short));
								}

								unsigned short *tbf2 = intermediateBuffer;

								for (int zz = tz * _TileDim; zz < tz * _TileDim + zLen; zz++)
								{
									getImage(zz, tbf2);

									tbf2 += _VolumeSizeX * _VolumeSizeY;
								}

								for (int ty = 0; ty < yT; ty++)
								{

									int yInit = ty * _TileDim;

									int yLen = _TileDim;
									int xLen = _TileDim;

									if (ty == yT - 1)
									{
										yLen = _VolumeSizeY - (yT - 1) * _TileDim;
									}

									//copy the read buffer to tiles and write them to disk
									for (int tx = 0; tx < xT; tx++)
									{
										auto node = highestResolutionNodes[numWrittenNodes];

										unsigned short *tbf = node->_Data;

										if (_CancelConversion)
										{
											delete[] intermediateBuffer;

											return;
										}

										for (int zz = 0; zz < zLen; zz++)
										{

											//reinitialize the tbf pointer as Y dim may be less than tile dim around boundary tiles
											tbf = node->_Data + zz * _TileDim * _TileDim;

											for (int yy = yInit; yy < yInit + yLen; yy++)
											{
												int actualY = yInit + yy;

												unsigned short *buf = intermediateBuffer + zz * _VolumeSizeX * _VolumeSizeY + yy * _VolumeSizeX + tx * _TileDim;

												if (tx == xT - 1)
												{
													xLen = _VolumeSizeX - (xT - 1) * _TileDim;
												}

												memcpy(tbf, buf, xLen * sizeof(unsigned short));

												for (int ii = 0; ii < xLen; ii++)
												{
													_Histogram[tbf[ii]]++;
												}

												tbf += _TileDim;
											}
										}

										computeMinMax(node->_Data, node->_MinVal, node->_MaxVal);

										numWrittenNodes++;

										if ((numWrittenNodes / totalFileIds - _Progress) > 0.01)
										{
											//_Progress = numWrittenNodes / totalFileIds;

											//_UpdateProgress(_Progress, _ProgressDataStructure, this);
										}
									}

								}

							}

							delete[] intermediateBuffer;
						}


						void OnlineTilesGenerator::computeMinMax(unsigned short *tile, unsigned int& minVal, unsigned int& maxVal)
						{
							size_t tileSize = _TileDim * _TileDim * _TileDim;

							minVal = USHRT_MAX;
							maxVal = 0;

							for (size_t tt = 0; tt < tileSize; tt++)
							{
								minVal = std::min(minVal, (unsigned int)tile[tt]);
								maxVal = std::max(maxVal, (unsigned int)tile[tt]);
							}

						}

						void OnlineTilesGenerator::sampleTile(const SbVec3i32& tileDim, int dataType, int border, const void* const octantTile[8], const int octantExists[8], void *parentTile)
						{
							for (int octant = 0; octant < 8; octant++)
							{
								if (!octantExists[octant])
									continue;

								sampleDecimation< unsigned short>(tileDim, octantTile[octant], parentTile, octant, border, octantExists);
							}
						}


						void OnlineTilesGenerator::getShiftAndHalfTileDim(SbVec2i32& shiftParentOctant, SbVec3i32& halfTileDim, const SbVec3i32& tileDim, int octant, int border) const
						{
							SbVec3i32 halfTile2 = tileDim / 2;
							SbVec3i32 halfTile1 = tileDim - halfTile2;
							int tileDim1 = tileDim[0];
							int tileDim2 = tileDim[0] * tileDim[1];

							halfTileDim = halfTile1;

							shiftParentOctant = SbVec2i32(0, 0);
							if (octant & 1)
							{
								shiftParentOctant[0] = halfTile1[0];
								shiftParentOctant[1] = 1;
								halfTileDim[0] = halfTile2[0];
							}

							if (octant & 2)
							{
								shiftParentOctant[0] += tileDim1 * halfTile1[1];
								shiftParentOctant[1] += tileDim1;
								halfTileDim[1] = halfTile2[1];
							}

							if (octant & 4)
							{
								shiftParentOctant[0] += tileDim2 * halfTile1[2];
								shiftParentOctant[1] += tileDim2;
								halfTileDim[2] = halfTile2[2];
							}

							if (border == 0)
								shiftParentOctant[1] = 0;
						}

						void OnlineTilesGenerator::getRatio(SbVec3f &ratio, SbVec3i32 &shiftOctant, SbVec3i32 tileDim, SbVec3i32 halfTileDim, int octant, int border, const int octantExists[8]) const
						{
							shiftOctant = SbVec3i32(0, 0, 0);

							// Border == 1 is a special case
							if (border == 1)
							{
								ratio = SbVec3f(2.0, 2.0, 2.0);
								if (octant % 2)
									shiftOctant[0] = border;

								if (octant != 0 && octant != 1 && octant != 4 && octant != 5)
									shiftOctant[1] = border;

								if (octant >= 4)
									shiftOctant[2] = border;

								return;
							}

							int leftBorder = int(border / 2);
							int rightBorder = border - leftBorder;

							// Compute the ratio for X axis
							if (octant % 2)
							{
								ratio[0] = (float)(tileDim[0] - rightBorder) / (float)halfTileDim[0];
								shiftOctant[0] = leftBorder;
							}
							else
							{
								if (octantExists[octant + 1])
									ratio[0] = (float)(tileDim[0] - leftBorder) / (float)halfTileDim[0];
								else
									ratio[0] = (float)tileDim[0] / (float)halfTileDim[0];

								shiftOctant[0] = 0;
							}

							// Compute ratio for Y axis
							if (octant == 0 || octant == 1 || octant == 4 || octant == 5)
							{
								if (octantExists[octant + 2])
									ratio[1] = (float)(tileDim[1] - leftBorder) / (float)halfTileDim[1];
								else
									ratio[1] = (float)tileDim[1] / (float)halfTileDim[1];

								shiftOctant[1] = 0;
							}
							else
							{
								ratio[1] = (float)(tileDim[1] - rightBorder) / (float)halfTileDim[1];
								shiftOctant[1] = leftBorder;
							}

							// Compute ratio for Z axis
							if (octant < 4)
							{
								if (octantExists[octant + 4])
									ratio[2] = (float)(tileDim[2] - leftBorder) / (float)halfTileDim[2];
								else
									ratio[2] = (float)tileDim[2] / (float)halfTileDim[2];

								shiftOctant[2] = 0;
							}
							else
							{
								ratio[2] = (float)(tileDim[2] - rightBorder) / (float)halfTileDim[2];
								shiftOctant[2] = leftBorder;
							}
						}

						void OnlineTilesGenerator::getImage(int zz, unsigned short* data)
						{
							_ImageProviderFunc(zz, _ImageProviderDataStruct, data);							
						}




						OnlineTilesGenerator::~OnlineTilesGenerator()
						{


						}
					}
				}
			}
		}
	}
}
