
#include "stdafx.h"
#include "VolumeCachedDataReader.h"
#include "ISO50Thresholdcalculator.h"
#include "stdio.h"
#include "stdlib.h"
#include "ipp.h"


static void HandleIppError(IppStatus err,
	const char *file,
	int line) {
	if (err != ippStsNoErr) {
		printf("IPP Error code %d in %s at line %d\n", err,
			file, line);
		exit(EXIT_FAILURE);
	}
}

#define ippSafeCall( err ) (HandleIppError( err, __FILE__, __LINE__ ))


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
						// Type of interpolation, the following values are possible :

						// IPPI_INTER_NN - nearest neighbor interpolation,

						//	IPPI_INTER_LINEAR - trilinear interpolation,

						//	IPPI_INTER_CUBIC - tricubic interpolation,

						//	IPPI_INTER_CUBIC2P_BSPLINE - B - spline,

						//	IPPI_INTER_CUBIC2P_CATMULLROM - Catmull - Rom spline,

						//	IPPI_INTER_CUBIC2P_B05C03 - special two - parameters filter(1 / 2, 3 / 10).


						void resizeVolume_Uint16C1(unsigned short *inputVolume, int iWidth, int iHeight, int iDepth, unsigned short *outputVolume, int oWidth,
							int oHeight, int oDepth , double resizeRatio)
						{
							IpprVolume inputVolumeSize, outputVolumeSize;

							int srcStep = iWidth * sizeof(unsigned short);
							int srcPlaneStep = iWidth * iHeight * sizeof(unsigned short);
							IpprCuboid srcVoi;

							int dstStep = oWidth * sizeof(unsigned short);
							int dstPlaneStep = oWidth * oHeight * sizeof(unsigned short);
							IpprCuboid dstVoi;

							double xFactor = resizeRatio, yFactor = resizeRatio, zFactor = resizeRatio;
							double xShift = 0, yShift = 0, zShift = 0;

							int interpolation = IPPI_INTER_NN;//IPPI_INTER_CUBIC2P_B05C03;//

							// Type of interpolation, the following values are possible :

							// IPPI_INTER_NN - nearest neighbor interpolation,

							//	IPPI_INTER_LINEAR - trilinear interpolation,

							//	IPPI_INTER_CUBIC - tricubic interpolation,

							//	IPPI_INTER_CUBIC2P_BSPLINE - B - spline,

							//	IPPI_INTER_CUBIC2P_CATMULLROM - Catmull - Rom spline,

							//	IPPI_INTER_CUBIC2P_B05C03 - special two - parameters filter(1 / 2, 3 / 10).

							inputVolumeSize.width = iWidth;
							inputVolumeSize.height = iHeight;
							inputVolumeSize.depth = iDepth;

							srcVoi.x = 0;
							srcVoi.y = 0;
							srcVoi.z = 0;

							srcVoi.width = iWidth;
							srcVoi.height = iHeight;
							srcVoi.depth = iDepth;

							dstVoi.x = 0;
							dstVoi.y = 0;
							dstVoi.z = 0;

							dstVoi.width = oWidth;
							dstVoi.height = oHeight;
							dstVoi.depth = oDepth;

							Ipp8u *computationBuffer;

							int bufferSize = 0;

							

							ippSafeCall(ipprResizeGetBufSize(srcVoi, dstVoi, 1, interpolation, &bufferSize));

							computationBuffer = new Ipp8u[bufferSize];

							ippSafeCall(ipprResize_16u_C1V(inputVolume, inputVolumeSize, srcStep, srcPlaneStep, srcVoi, outputVolume, dstStep,
								dstPlaneStep, dstVoi, xFactor, yFactor, zFactor, xShift, yShift,
								zShift, interpolation, computationBuffer));

							delete[] computationBuffer;

						}


						void resizeVolume_Uint16C1_MT(unsigned short *inputVolume, int iWidth, int iHeight, int iDepth, unsigned short *outputVolume, int oWidth,
							int oHeight, int oDepth, double resizeRatio)
						{
							int bandWidth = 32;

							int numBands = oDepth % 32 == 0 ? oDepth / bandWidth : oDepth / bandWidth + 1;

							int extension = (1.0 / resizeRatio) + 1;

							if (numBands > 1)
							{

#pragma omp parallel for
								for (int bb = 0; bb < numBands; bb++)
								{
									IpprVolume inputVolumeSize, outputVolumeSize;

									int srcStep = iWidth * sizeof(unsigned short);
									int srcPlaneStep = iWidth * iHeight * sizeof(unsigned short);
									IpprCuboid srcVoi;

									int dstStep = oWidth * sizeof(unsigned short);
									int dstPlaneStep = oWidth * oHeight * sizeof(unsigned short);
									IpprCuboid dstVoi;

									double xFactor = resizeRatio, yFactor = resizeRatio, zFactor = resizeRatio;
									double xShift = 0, yShift = 0, zShift = 0;

									int interpolation = IPPI_INTER_LINEAR;

									int sourceBand = (int)(bandWidth / resizeRatio + 1);

									if (bb == 0 || bb == numBands - 1)
									{
										sourceBand += extension;
									}
									else
									{
										sourceBand += 2 * extension;
									}

									double destStartLine = bandWidth * bb;

									int destBandWidth = bandWidth;

									double sourceStartLine = destStartLine / resizeRatio;

									if (bb == numBands - 1)
									{
										destBandWidth = oDepth - bandWidth * bb;
									}

									dstVoi.x = 0;
									dstVoi.y = 0;
									dstVoi.z = 0;

									dstVoi.width = oWidth;
									dstVoi.height = oHeight;
									dstVoi.depth = destBandWidth;

									size_t sourceDataShift = 0;
									size_t destDataShift = 0;

									double sourceLineZ = bandWidth * bb / resizeRatio;

									int sourceStartZ = (int)(sourceLineZ - extension);

									if (bb == 0)
										sourceStartZ = 0;

									if (bb == numBands - 1)
									{
										sourceBand = iDepth - sourceStartZ;
									}

									srcVoi.x = 0;
									srcVoi.y = 0;
									srcVoi.z = 0;

									srcVoi.width = iWidth;
									srcVoi.height = iHeight;
									srcVoi.depth = sourceBand;

									inputVolumeSize.width = iWidth;
									inputVolumeSize.height = iHeight;
									inputVolumeSize.depth = sourceBand;

									sourceDataShift = (size_t)sourceStartZ * (size_t)iWidth * (size_t)iHeight;
									destDataShift = (size_t)bandWidth * (size_t)bb * (size_t)oWidth * (size_t)oHeight;

									Ipp8u *computationBuffer;

									zShift = -destStartLine + sourceStartZ * resizeRatio;

									int bufferSize = 0;

									ippSafeCall(ipprResizeGetBufSize(srcVoi, dstVoi, 1, interpolation, &bufferSize));

									computationBuffer = new Ipp8u[bufferSize];

									ippSafeCall(ipprResize_16u_C1V(inputVolume + sourceDataShift, inputVolumeSize, srcStep, srcPlaneStep,
										srcVoi, outputVolume + destDataShift, dstStep, dstPlaneStep, dstVoi,
										xFactor, yFactor, zFactor, xShift, yShift, zShift, interpolation, computationBuffer));

									delete[] computationBuffer;
								}
							}
							else
							{
								resizeVolume_Uint16C1(inputVolume, iWidth, iHeight, iDepth, outputVolume, oWidth, oHeight, oDepth, resizeRatio);
							}

						}






						VolumeCachedDataReader::VolumeCachedDataReader()
						{
							_RawVolumeData = 0;

							_Reader = new OnlineTiledReader();
						}

						VolumeCachedDataReader::~VolumeCachedDataReader()
						{
						}


						void VolumeCachedDataReader::CachedDataImageProviderCallBack(const int& zz, void *sliceDataProvider, void *sliceData)
						{
							VolumeCachedDataReader *sliceProvider = static_cast<VolumeCachedDataReader *>(sliceDataProvider);

							if (sliceProvider)
							{
								sliceProvider->ReadSliceOnline(zz, (unsigned short*)sliceData);
							}
						}

						void VolumeCachedDataReader::initialize(std::wstring filePath, size_t volumeWidth, size_t volumeHeight,
							size_t volumeDepth, double* voxelSize, size_t headerSkipSize , double renderTargetRatio ,  OnlineTilesGenerator::ProgressFunc func , void* progressDataStruct)
						{
							_VolumeWidth = volumeWidth;
							_VolumeHeight = volumeHeight;
							_VolumeDepth = volumeDepth;

							size_t volumeSize = volumeWidth * volumeHeight * volumeDepth;

							_RawVolumeData = new unsigned short[volumeSize];

							_RAWFile = 0;
							
							_wfopen_s(&_RAWFile , filePath.c_str(), L"rb");

							_HeaderSkipSize = headerSkipSize;

							memcpy(_VoxelSize, voxelSize, sizeof(double) * 3);

							size_t filePos = _HeaderSkipSize;

							_fseeki64(_RAWFile, filePos, SEEK_SET);

							fread(_RawVolumeData, sizeof(unsigned short), volumeSize, _RAWFile);

							if (renderTargetRatio > 0.99)
							{
								_ResizedWidth = _VolumeWidth;
								_ResizedDepth = _VolumeDepth;
								_ResizedHeight = _VolumeHeight;

								_ResizedVolumeData = _RawVolumeData;
							}
							else
							{
								_ResizedWidth = _VolumeWidth * renderTargetRatio;
								_ResizedHeight = _VolumeHeight * renderTargetRatio;
								_ResizedDepth = _VolumeDepth * renderTargetRatio;
								
								size_t resizedVolumeSize = _ResizedWidth * _ResizedHeight * _ResizedDepth;

								_ResizedVolumeData = new unsigned short[resizedVolumeSize];

								memset(_ResizedVolumeData, 0, resizedVolumeSize * sizeof(unsigned short));

								resizeVolume_Uint16C1_MT(  _RawVolumeData, _VolumeWidth, _VolumeHeight, _VolumeDepth, _ResizedVolumeData, 
									                       _ResizedWidth, _ResizedHeight, _ResizedDepth , renderTargetRatio);
							}

							_Reader->setVolumeInfo( _ResizedWidth, _ResizedHeight, _ResizedDepth, voxelSize[0] / renderTargetRatio, 
								                    voxelSize[1] / renderTargetRatio, voxelSize[2] / renderTargetRatio, 16, headerSkipSize, 1);

							_Reader->setProgressCallback(func, progressDataStruct);

							OnlineTilesGenerator::ImageProviderFunc imgFunc(CachedDataImageProviderCallBack);

							_Reader->initializeTiles( imgFunc , this );

							ISO50ThresholdCalculator thresholdCalculator;

							std::vector< int64_t > histogram;

							_Reader->getHistogram(histogram);

							std::cout << " computing frounhoffer threshold " << std::endl;

							_IsoThreshold = thresholdCalculator.fraunhoufferThreshold(_ResizedWidth, _ResizedHeight, _ResizedDepth,
								voxelSize[0] / renderTargetRatio, voxelSize[1] / renderTargetRatio, voxelSize[2] / renderTargetRatio, _ResizedVolumeData);//31680;// 


							std::cout << " Frounhoffer Threshold : " << _IsoThreshold << std::endl;

							thresholdCalculator.createColorMap(_IsoThreshold, _ColorMap);

						}


						void VolumeCachedDataReader::ReadSlice(size_t z, unsigned short *sliceData, bool buildAdditionalInfo)
						{
							size_t sliceSize = _VolumeWidth * _VolumeHeight;

							size_t filePos = (size_t)z * sliceSize * sizeof(unsigned short) + _HeaderSkipSize;

							_fseeki64(_RAWFile, filePos, SEEK_SET);

							fread(sliceData, sizeof(unsigned short), sliceSize, _RAWFile);

							if (buildAdditionalInfo)
							{
								processSlice(z, sliceData);
							}
						}


						void VolumeCachedDataReader::ReadSliceOnline( size_t z , unsigned short *sliceData )
						{
							size_t step = _ResizedWidth * _ResizedHeight;

							memcpy(sliceData, _ResizedVolumeData + step * z, step * sizeof(unsigned short));
						   
						}

						void VolumeCachedDataReader::processSlice(size_t z, unsigned short *sliceData)
						{
							size_t sliceSize = _VolumeWidth * _VolumeHeight;

							memcpy((_RawVolumeData + z * sliceSize), sliceData, sliceSize * sizeof(unsigned short));
						}


						unsigned short *VolumeCachedDataReader::getRawVolumeData()
						{
							return _RawVolumeData;
						}

						OnlineTiledReader* VolumeCachedDataReader::getTileReader()
						{
							return _Reader;
						}


						void VolumeCachedDataReader::setRawVolumeData(unsigned short*& rawDataPtr)
						{
						
							_RawVolumeData = rawDataPtr;
						
						}


						void VolumeCachedDataReader::updateCache(unsigned int isoThreshold)
						{


						}

						void VolumeCachedDataReader::CalculateCenterOfGravity(const INT64& threshold, double *cogVoxel, double *cogWorld)
						{
							const size_t sliceSZ = _VolumeWidth * _VolumeHeight;

							INT64 sum[3] = { 0, 0, 0 };
							INT64  count = 0;
							for (long z = 0; z < _VolumeDepth; ++z)
							{
								for (long y = 0; y < _VolumeHeight; ++y)
								{
									for (long x = 0; x < _VolumeWidth; ++x)
									{
										if (_RawVolumeData[x + y * _VolumeWidth + z * sliceSZ] >= threshold)
										{
											sum[0] += x;
											sum[1] += y;
											sum[2] += z;
											++count;
										}
									}
								}
							}


							for (long i = 0; i < 3; ++i)
							{
								INT64 denominator = count > 0 ? count : 1;
								cogVoxel[i] = static_cast<long>(sum[i] / denominator);
								cogWorld[i] = static_cast<double>(sum[i]) / static_cast<double>(denominator)* _VoxelSize[i];
							}

						}

						void VolumeCachedDataReader::CalculateIntertiaTensor(const INT64& threshold, double* centerOfGravityVoxel, double* inertialTensor)
						{
							const size_t sliceSZ = _VolumeWidth * _VolumeHeight;

							double sumxx = 0; double sumyy = 0; double sumzz = 0; double sumxy = 0; double sumxz = 0; double sumyz = 0;
							INT64  count = 0;

							for (long z = -(long)centerOfGravityVoxel[2], zi = 0; zi < _VolumeDepth; ++z, ++zi)
							{
								double zz = z*z;

								for (long y = -(long)centerOfGravityVoxel[1], yi = 0; yi < _VolumeHeight; ++y, ++yi)
								{
									double yy = y*y;
									double yz = y*z;
									for (long x = -(long)centerOfGravityVoxel[0], xi = 0; xi < _VolumeWidth; ++x, ++xi)
									{
										if (_RawVolumeData[xi + yi*_VolumeWidth + zi*sliceSZ] >= threshold)
										{
											sumxx += x*x;
											sumyy += yy;
											sumzz += zz;
											sumxy -= x*y;
											sumxz -= x*z;
											sumyz -= yz;
											++count;
										}
									}
								}
							}

							double denominator = count > 0 ? static_cast<double>(count) : 1;

							inertialTensor[0] = double(sumyy) / denominator*_VoxelSize[1] * _VoxelSize[1] + double(sumzz) / denominator*_VoxelSize[2] * _VoxelSize[2];
							inertialTensor[1 * 3 + 1] = double(sumzz) / denominator*_VoxelSize[2] * _VoxelSize[2] + double(sumxx) / denominator*_VoxelSize[0] * _VoxelSize[0];
							inertialTensor[2 * 3 + 2] = double(sumxx) / denominator*_VoxelSize[0] * _VoxelSize[0] + double(sumyy) / denominator*_VoxelSize[1] * _VoxelSize[1];
							inertialTensor[1] = double(sumxy) / denominator*_VoxelSize[0] * _VoxelSize[1];
							inertialTensor[1 * 3 + 0] = inertialTensor[1];
							inertialTensor[2] = double(sumxz) / denominator*_VoxelSize[0] * _VoxelSize[2];
							inertialTensor[2 * 3] = inertialTensor[2];
							inertialTensor[1 * 3 + 2] = double(sumyz) / denominator * _VoxelSize[1] * _VoxelSize[2];
							inertialTensor[2 * 3 + 1] = inertialTensor[1 * 3 + 2];

						}


						size_t VolumeCachedDataReader::width()
						{

							return _VolumeWidth;
						}


						size_t VolumeCachedDataReader::height()
						{
						
							return _VolumeHeight;
						}
						
						
						size_t VolumeCachedDataReader::depth()
						{
						
							return _VolumeDepth;
						}


						void VolumeCachedDataReader::releaseRawData()
						{

							if(_RawVolumeData)
							delete _RawVolumeData;

							_RawVolumeData = 0;

							delete _Reader;
						}

					}
				}
			}
		}
	}
}