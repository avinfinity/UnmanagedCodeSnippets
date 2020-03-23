#include "overlappedvoxelsegmentation.h"
#include "iostream"
#include "opencvincludes.h"
#include "omp.h"

namespace imt{

	namespace volume {





		OverlappedVoxelsSegmentation::OverlappedVoxelsSegmentation(imt::volume::VolumeInfo& volumeInfo) : _VolumeInfo(volumeInfo)
		{



		}


		OverlappedVoxelsSegmentation::SobelGradientOperator3x3x3::SobelGradientOperator3x3x3()
		{
			int hx[3] = { 1, 2, 1 }, hy[3] = { 1, 2, 1 }, hz[3] = { 1, 2, 1 };
			int hpx[3] = { 1, 0, -1 }, hpy[3] = { 1, 0, -1 }, hpz[3] = { 1, 0, -1 };

			//build the kernel
			for (int m = 0; m <= 2; m++)
				for (int n = 0; n <= 2; n++)
					for (int k = 0; k <= 2; k++)
					{
						_KernelGx[m][n][k] = hpx[m] * hy[n] * hz[k];
						_KernelGy[m][n][k] = hx[m] * hpy[n] * hz[k];
						_KernelGz[m][n][k] = hx[m] * hy[n] * hpz[k];
					}
		}


		void OverlappedVoxelsSegmentation::SobelGradientOperator3x3x3::init(size_t volumeW, size_t volumeH, size_t volumeD, double voxelSizeX,
			double voxelSizeY, double voxelSizeZ, unsigned short *volumeData)
		{
			_VolumeW = volumeW;
			_VolumeH = volumeH;
			_VolumeD = volumeD;

			_VoxelSizeX = voxelSizeX;
			_VoxelSizeY = voxelSizeY;
			_VoxelSizeZ = voxelSizeZ;

			_StepZ = _VolumeW * _VolumeH;
			_StepY = _VolumeW;

			_VolumeData = volumeData;
		}

#define grayValue(x , y , z)  _VolumeData[ _StepZ * z + _StepY * y + x ] 

		void OverlappedVoxelsSegmentation::SobelGradientOperator3x3x3::apply(Eigen::Vector3f& point, Eigen::Vector3f& gradient)
		{
			double sumx = 0, sumy = 0, sumz = 0;

			for (int mm = -1; mm <= 1; mm++)
				for (int nn = -1; nn <= 1; nn++)
					for (int kk = -1; kk <= 1; kk++)
					{
						int xx = point(0) + mm;
						int yy = point(1) + nn;
						int zz = point(2) + kk;

						unsigned short gVal = grayValue(xx, yy, zz);

						sumx += _KernelGx[mm + 1][nn + 1][kk + 1] * gVal;
						sumy += _KernelGy[mm + 1][nn + 1][kk + 1] * gVal;
						sumz += _KernelGz[mm + 1][nn + 1][kk + 1] * gVal;
					}


			gradient(0) = sumx;
			gradient(1) = sumy;
			gradient(2) = sumz;

			gradient.normalize();
		}


#define toggle(number , n) number |= 1UL << n
#define reset(number , n ) number &= ~(1UL << n)
//#define 


		void OverlappedVoxelsSegmentation::compute( std::vector< std::pair<int, int> >& initialMaterialRegions )
		{

			SobelGradientOperator3x3x3 *gradientOperator = new SobelGradientOperator3x3x3();

			gradientOperator->init(_VolumeInfo.mWidth, _VolumeInfo.mHeight, _VolumeInfo.mDepth,
				_VolumeInfo.mVoxelStep(0), _VolumeInfo.mVoxelStep(1), _VolumeInfo.mVoxelStep(2),
				(unsigned short*)_VolumeInfo.mVolumeData);

			std::vector< std::pair<int, int> > sortedMaterialRegions = initialMaterialRegions;

			std::sort(sortedMaterialRegions.begin(), sortedMaterialRegions.end(), [](const std::pair<int, int>& region1, const std::pair<int, int>& region2)->bool {
			
				return region1.first > region2.first;
			});

			int64_t w = _VolumeInfo.mWidth;
			int64_t h = _VolumeInfo.mHeight;
			int64_t d = _VolumeInfo.mDepth;

			int64_t maskVolumeSize = w * h * d;

			unsigned char* maskData = new unsigned char[maskVolumeSize];
			unsigned short* volumeData = (unsigned short*)_VolumeInfo.mVolumeData;

			memset(maskData, 0, maskVolumeSize);

			int nMaterials = sortedMaterialRegions.size();

			int64_t zStep = w * h;
			int64_t yStep = w;
			
			for (int mm = 0; mm < nMaterials; mm++)
			{
				if (mm == 0)
				{
					std::vector<Eigen::Vector3f> materialBoundaryPoints;

					int materialIsoValue = sortedMaterialRegions[mm].first;

					int currentMaterialId = nMaterials - mm;

					for ( int zz = 0; zz < d; zz++)
						for ( int yy = 0; yy < h; yy++)
							for ( int xx = 0; xx < w; xx++)
							{
								int voxelGrayValue = volumeData[zStep * zz + yStep * yy + xx];

								if ( volumeData[zStep * zz + yStep * yy + xx] >= sortedMaterialRegions[mm].first )
								{
									maskData[zStep * zz + yStep * yy + xx] = nMaterials - mm;

									bool isBoundaryVoxel = false;
								
									//check if the voxel lies on the boundary of the material
									for ( int z = -1; z >= 1; z++ )
										for( int y = -1; y >= 1; y++ )
											for ( int x = -1; x >= 1; x++ )
											{
												if (x == 0 && y == 0 && z == 0)
													continue;

												int xv = xx + x;
												int yv = yy + y;
												int zv = zz + z;

												int neighborVoxelGrayValue = volumeData[ zStep * zz + yStep * yy + xx ];

												if ( (voxelGrayValue - materialIsoValue) * (neighborVoxelGrayValue - materialIsoValue) < 0 )
												{
													//it's a boundary voxel
													isBoundaryVoxel = true;

													break;
												}
											}


									if ( isBoundaryVoxel )
									{
										Eigen::Vector3f point(xx, yy, zz);
										Eigen::Vector3f gradient;

										//compute the sobel gradient and find the corresponding normal
										gradientOperator->apply(point, gradient);

										Eigen::Vector3f currentPoint = point + gradient * 5;

										Eigen::Vector3i currentPointI = currentPoint.cast<int>();

										int extremeGrayValue = grayValue(currentPointI(0), currentPointI(1), currentPointI(2));

										int materialId = _GrayValueToMaterialId[extremeGrayValue];

										if ( materialId == currentMaterialId )
										{
											std::cout << " bad gradient " << std::endl;
										}

										// lets back propagate to certain distance to check if indeed this voxel currently lies on surface of 
										// assumed material
										Eigen::Vector3i  backPropagationPoint  = (point - gradient * 5).cast<int>();

										int backMaterialId = _GrayValueToMaterialId[grayValue(backPropagationPoint(0), 
											                                        backPropagationPoint(1), backPropagationPoint(2))];

										if ( backMaterialId != currentMaterialId )
										{
											continue;
										}


										//now along the normal we need to propagate to a certain distance and clear noisy material values
										for (int vv = 0; vv < 10; vv++)
										{
											Eigen::Vector3i currentPoint = ( point + gradient * vv * 0.5 ).cast<int>();

											fillMask( currentPoint, materialId, currentMaterialId , maskData );
										}

									}
								}

								


							}
				}

				
			}

		}



		void OverlappedVoxelsSegmentation::fillMask( Eigen::Vector3i inputPoint , int materialId, int ignoreMaterialId, unsigned char* mask )
		{
			for ( int64_t zz = inputPoint(2) - 1; zz <= inputPoint(2) + 1; zz++ )
				for (int64_t yy = inputPoint(1) - 1; yy <= inputPoint(1) + 1; yy++)
					for(int64_t xx = inputPoint(0) - 1; xx <= inputPoint(0) + 1; xx++)
			         {
						if ( _GrayValueToMaterialId[grayValue(xx, yy, zz)] != ignoreMaterialId )
						{
							mask[_StepZ * zz + _StepY * yy + xx] = materialId;
						}

			         }

		}





	}

}