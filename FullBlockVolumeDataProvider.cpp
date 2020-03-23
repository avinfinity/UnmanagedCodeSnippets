#include "stdafx.h"
#include "FullBlockVolumeDataProvider.h"
#include <Inventor/devices/SoCpuBufferObject.h>
#include <algorithm>
#include "CPUBuffer.h"
#include "VolumeUtility.h"
#include "VolumeCachedDataReader.h"

namespace Zeiss {
	namespace IMT {

		namespace NG {

			namespace NeoInsights {

				namespace Volume {

					namespace DataProcessing {

						FullBlockVolumeDataProvider::FullBlockVolumeDataProvider( CPUBuffer *fullVolumeBlock , size_t volumeW , size_t volumeH , size_t volumeD , 
							double *voxelSize  /*, OnlineTilesGenerator::ProgressFunc& progressUpdateF, void* progressDataStruct , void *dataManager ,
							double initProgress , double progressBand*/ )
						{
							memcpy(_VoxelSize, voxelSize, sizeof(double) * 3);

							_VolumeWidth = volumeW;
							_VolumeHeight = volumeH;
							_VolumeDepth = volumeD;

							_RawVolumeData = fullVolumeBlock;

							size_t maxLevels = 20;

							_LevelDimensions.reserve(maxLevels);
							_RAWVolumeDataAtLevels.reserve(maxLevels);

							_LevelDimensions.resize(maxLevels);

							_RAWVolumeDataAtLevels.resize(maxLevels);

							for (int ii = 0; ii < maxLevels; ii++)
							{
								_RAWVolumeDataAtLevels[ii] = 0;
							}

							_RAWVolumeDataAtLevels[0] = fullVolumeBlock;

							if (_VolumeWidth < 32 || _VolumeHeight < 32 || _VolumeDepth < 32)
							{
								_RAWVolumeDataAtLevels.resize(1);
								_LevelDimensions.resize(1);

								return;
							}
								 

							//double iProgress = initProgress;
							//double progBand = progressBand;

							for ( int ii = 0; ii < maxLevels; ii++ )
							{
								_LevelDimensions[ii].Width = _VolumeWidth >> ii;
								_LevelDimensions[ii].Height = _VolumeHeight >> ii;
								_LevelDimensions[ii].Depth = _VolumeDepth >> ii;

								if (_LevelDimensions[ii].Width < 32 || _LevelDimensions[ii].Height < 32 || _LevelDimensions[ii].Depth < 32)
								{
									_LevelDimensions.resize(ii);
									_RAWVolumeDataAtLevels.resize(ii);
									
									break;
								}

								if (ii > 0)
								{
									_RAWVolumeDataAtLevels[ii] = new CPUBuffer;

									_RAWVolumeDataAtLevels[ii]->resize(_LevelDimensions[ii].Width * _LevelDimensions[ii].Height * _LevelDimensions[ii].Depth * sizeof(unsigned short));

									imt::volume::VolumeUtility::resizeVolume_Uint16C1_MT(static_cast<unsigned short*>(_RAWVolumeDataAtLevels[ii - 1]->data()), _LevelDimensions[ii - 1].Width,
										_LevelDimensions[ii - 1].Height, _LevelDimensions[ii - 1].Depth, static_cast<unsigned short*>(_RAWVolumeDataAtLevels[ii]->data()),
										_LevelDimensions[ii].Width, _LevelDimensions[ii].Height, _LevelDimensions[ii].Depth , 0.5);

									//iProgress += progBand / 2;

									//progBand = progBand / 2;

								}

							}

							//progressUpdateF(initProgress + progressBand, progressDataStruct, dataManager);
							
						}



						//copies the from volume to intersction roi of roiData and volume
						void FullBlockVolumeDataProvider::GetRoiBuffer( RoiData& roiData )
						{


							size_t level = roiData._Resolution;


							size_t startX = std::max((size_t)0, roiData._StartX);
							size_t startY = std::max((size_t)0, roiData._StartY);
							size_t startZ = std::max((size_t)0, roiData._StartZ);

							size_t requiredBufferSize = roiData.Width * roiData.Height * roiData.Depth * sizeof(unsigned short);

							roiData._Buffer->resize(requiredBufferSize);

							memset(roiData._Buffer->data(), 0, requiredBufferSize);

							size_t volumeWidth = _LevelDimensions[level].Width;
							size_t volumeHeight = _LevelDimensions[level].Height;
							size_t volumeDepth = _LevelDimensions[level].Depth;

							size_t srcLineStep = volumeWidth;
							size_t srcPlaneStep = volumeWidth * volumeHeight;

							unsigned short *buffer = (unsigned short*)roiData._Buffer->data();
							unsigned short *volumeBuffer = (( unsigned short* )_RAWVolumeDataAtLevels[level]->data()) + startZ * srcPlaneStep
								                           + startY * srcLineStep + startX;

							size_t dstLineStep = roiData.Width;
							size_t dstPlaneStep = roiData.Width * roiData.Height;

							size_t xEnd = std::min(roiData.Width + roiData._StartX - 1, volumeWidth - 1);
							size_t yEnd = std::min(roiData.Height + roiData._StartY - 1, volumeHeight - 1);
							size_t zEnd = std::min(roiData.Depth + roiData._StartZ - 1, volumeDepth - 1);
							

							size_t lineBufferSize = (xEnd - startX + 1) * sizeof(unsigned short);

							size_t depth = zEnd - startZ + 1;
							size_t height = yEnd - startY + 1;

							size_t dstShift = ( startZ - roiData._StartZ ) * dstPlaneStep + ( startY - roiData._StartY) * dstLineStep + (startX - roiData._StartX);

							unsigned short *dbf = buffer + dstShift;

							for (int zz = 0; zz < depth; zz++)
							{
								unsigned short *vbf = volumeBuffer + zz * srcPlaneStep;
								dbf = buffer + dstShift + zz * dstPlaneStep;

								for (int yy = 0; yy < height; yy++)
								{
									memcpy(dbf, vbf, lineBufferSize);
									dbf += dstLineStep;
									vbf += srcLineStep;
								}
							}


						}

						void FullBlockVolumeDataProvider::GetSliceBuffer(SliceData& sliceData)
						{

							double step = (_VoxelSize[0] + _VoxelSize[1] + _VoxelSize[2]) / 3;

							int sliceWidth, sliceHeight;

							computeSliceDimensions(_VoxelSize[0], _VoxelSize[1], _VoxelSize[2], step, sliceData._CoordSys, sliceWidth, sliceHeight);
							
							sliceData._Buffer->resize( sliceWidth * sliceHeight * sizeof(unsigned) );

							double xAxis[3], yAxis[3] , sliceOrigin[3];
							
							unsigned short *vData = ( unsigned short* )sliceData._Buffer->data();

							interpolateImage( xAxis, yAxis, sliceOrigin, step, _VoxelSize, sliceWidth, sliceHeight, 
							 	              _VolumeWidth, _VolumeHeight, _VolumeDepth, 
								             (unsigned short*)_RawVolumeData->data(), vData);

						}


						void FullBlockVolumeDataProvider::GetSliceBuffer( SliceData& sliceData , std::vector< double >& colorMap )
						{
						    
							double step = (_VoxelSize[0] + _VoxelSize[1] + _VoxelSize[2]) / 3;

							int sliceWidth, sliceHeight;

							computeSliceDimensions(_VoxelSize[0] * _VolumeWidth , _VoxelSize[1] * _VolumeHeight, _VoxelSize[2] * _VolumeDepth , step, sliceData._CoordSys, sliceWidth, sliceHeight);

							sliceData._Buffer->resize( 3 * sliceWidth * sliceHeight * sizeof(unsigned) );

							double xAxis[3], yAxis[3], sliceOrigin[3];

							unsigned short *vData = (unsigned short*)sliceData._Buffer->data();

							interpolateImage(xAxis, yAxis, sliceOrigin, step, _VoxelSize, sliceWidth, sliceHeight,
								_VolumeWidth, _VolumeHeight, _VolumeDepth, colorMap ,
								(unsigned short*)_RawVolumeData->data(), vData);
						}


						const CPUBuffer* FullBlockVolumeDataProvider::GetFullVolumeData() const
						{
						  return _RawVolumeData;
						}


#define grayValue(x , y , z)  volumeData[ zStep * z + yStep * y + x ] 
						unsigned short FullBlockVolumeDataProvider::valueAt(double x, double y, double z, unsigned int width, unsigned int height, const unsigned short *volumeData)
						{
							unsigned short interpolatedValue = 0;

							size_t zStep = width * height;
							size_t yStep = width;

							int lx = (int)x;
							int ly = (int)y;
							int lz = (int)z;

							int ux = (int)std::ceil(x);
							int uy = (int)std::ceil(y);
							int uz = (int)std::ceil(z);

							double xV = x - lx;
							double yV = y - ly;
							double zV = z - lz;

							double c000 = grayValue(lx, ly, lz);
							double c100 = grayValue(ux, ly, lz);
							double c010 = grayValue(lx, uy, lz);
							double c110 = grayValue(ux, uy, lz);
							double c001 = grayValue(lx, ly, uz);
							double c101 = grayValue(ux, ly, uz);
							double c011 = grayValue(lx, uy, uz);
							double c111 = grayValue(ux, uy, uz);

							double interpolatedValF = c000 * (1.0 - xV) * (1.0 - yV) * (1.0 - zV) +
								c100 * xV * (1.0 - yV) * (1.0 - zV) +
								c010 * (1.0 - xV) * yV * (1.0 - zV) +
								c110 * xV * yV * (1.0 - zV) +
								c001 * (1.0 - xV) * (1.0 - yV) * zV +
								c101 * xV * (1.0 - yV) * zV +
								c011 * (1.0 - xV) * yV * zV +
								c111 * xV * yV * zV;

							interpolatedValue = (unsigned short)interpolatedValF;

							return interpolatedValue;
						}


						void FullBlockVolumeDataProvider::interpolateImage( const double* xAxis, const double* yAxis, const double* origin, double step, double *voxelSize, int w, int h,
							int volumeW, int volumeH, int volumeD,  const unsigned short *volumeData, unsigned short* interpolatedImage )
						{
							unsigned short *iiData = interpolatedImage;

							double shiftY = h * 0.5;
							double shiftX = w * 0.5;

							for (int yy = 0; yy < h; yy++)
								for (int xx = 0; xx < w; xx++)
								{
									double coords[3];

									coords[0] = origin[0] + yAxis[0] * (yy - shiftY) * step + xAxis[0] * (xx - shiftX) * step;
									coords[1] = origin[1] + yAxis[1] * (yy - shiftY) * step + xAxis[1] * (xx - shiftX) * step;
									coords[2] = origin[2] + yAxis[2] * (yy - shiftY) * step + xAxis[2] * (xx - shiftX) * step;

									double x3d = (coords[0] / step);
									double y3d = (coords[1] / step);
									double z3d = (coords[2] / step);

									if (x3d < 1 || x3d >= volumeW - 1 ||
										y3d < 1 || y3d >= volumeH - 1 ||
										z3d < 1 || z3d >= volumeD - 1)
									{
										iiData[yy * w + xx] = 0;
										continue;
									}

									//triliear interpolation is required here
									iiData[yy * w + xx] = valueAt( x3d, y3d, z3d, volumeW, volumeH , volumeData);

								}
						}


						void FullBlockVolumeDataProvider::interpolateImage(const double* xAxis, const double* yAxis, const double* origin, double step, double *voxelSize, int w, int h,
							int volumeW, int volumeH, int volumeD, std::vector< double >& colorMap, const unsigned short *volumeData, unsigned short* interpolatedImage)
						{
							unsigned short *iiData = interpolatedImage;

							double shiftY = h * 0.5;
							double shiftX = w * 0.5;

							for (int yy = 0; yy < h; yy++)
								for (int xx = 0; xx < w; xx++)
								{
									double coords[3];

									coords[0] = origin[0] + yAxis[0] * (yy - shiftY) * step + xAxis[0] * (xx - shiftX) * step;
									coords[1] = origin[1] + yAxis[1] * (yy - shiftY) * step + xAxis[1] * (xx - shiftX) * step;
									coords[2] = origin[2] + yAxis[2] * (yy - shiftY) * step + xAxis[2] * (xx - shiftX) * step;

									double x3d = (coords[0] / step);
									double y3d = (coords[1] / step);
									double z3d = (coords[2] / step);

									if (x3d < 1 || x3d >= volumeW - 1 ||
										y3d < 1 || y3d >= volumeH - 1 ||
										z3d < 1 || z3d >= volumeD - 1)
									{
										iiData[3 * yy * w + 3 * xx] = 0;
										iiData[3 * yy * w + 3 * xx + 1] = 0;
										iiData[3 * yy * w + 3 * xx + 2] = 0;
										continue;
									}

									//triliear interpolation is required here
									unsigned short grayValue = valueAt(x3d, y3d, z3d, volumeW, volumeH, volumeData);

									//triliear interpolation is required here
									iiData[3 * yy * w + 3 * xx] = (unsigned char)(colorMap[grayValue] * 255.0);
									iiData[3 * yy * w + 3 * xx + 1] = (unsigned char)(colorMap[grayValue] * 255.0);
									iiData[3 * yy * w + 3 * xx + 2] = (unsigned char)(colorMap[grayValue] * 255.0);

								}
						}


						void FullBlockVolumeDataProvider::computeSliceDimensions(double wS, double hS, double dS, double step, double* transform, int& w, int& h)
						{

							double xMin = DBL_MAX, xMax = -FLT_MAX, yMin = DBL_MAX,
								yMax = -FLT_MAX, zMin = DBL_MAX, zMax = -FLT_MAX;;

							for (int ii = 0; ii < 2; ii++)
								for (int jj = 0; jj < 2; jj++)
									for (int kk = 0; kk < 2; kk++)
									{
										double corner4[4] = { ii * wS, jj * hS, kk * dS, 1 };

										double proj4[4];

										matrixVectorMultiplication(4, 4, transform, corner4, proj4);

										proj4[0] /= proj4[3];
										proj4[1] /= proj4[3];
										proj4[2] /= proj4[3];

										double xProj = proj4[0];
										double yProj = proj4[1];
										double zProj = proj4[2];

										xMin = std::min(xMin, xProj);
										xMax = std::max(xMax, xProj);

										yMin = std::min(yMin, yProj);
										yMax = std::max(yMax, yProj);

										zMin = std::min(zMin, zProj);
										zMax = std::max(zMax, zProj);
									}

							w = (int)((xMax - xMin) / step);
							h = (int)((yMax - yMin) / step);

						}


						//Matrix is m x n and vector is n x 1
						void FullBlockVolumeDataProvider::matrixVectorMultiplication(int m, int n, const double *matrix, const double *vec, double *outputVec)
						{
							for ( int rr = 0; rr < m; rr++ )
							{
								double sumVal = 0;

								const double *currentRow = matrix + rr * n;

								for (int cc = 0; cc < n; cc++)
								{
									sumVal += currentRow[cc] * vec[cc];
								}

								outputVec[rr] = sumVal;
							}
						}



						void FullBlockVolumeDataProvider::clearData()
						{
						  size_t numLevels = _RAWVolumeDataAtLevels.size();

						  for (int ll = 1; ll < numLevels; ll++)
						  {
							  if (_RAWVolumeDataAtLevels[ll])
							  {
								  delete _RAWVolumeDataAtLevels[ll];
							  }
						  }

						  _RAWVolumeDataAtLevels.clear();

						}


						FullBlockVolumeDataProvider::~FullBlockVolumeDataProvider()
						{
							clearData();
						}
					}
				}
			}
		}
	}
}
