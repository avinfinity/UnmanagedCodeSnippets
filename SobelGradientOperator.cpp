#include "stdafx.h"
#include "SobelGradientOperator.h"
#include <algorithm>


namespace Zeiss {

	namespace IMT {

		namespace NG {

			namespace NeoInsights {


				namespace Volume {

					namespace MeshGenerator {


						SobelGradientOperator::SobelGradientOperator( unsigned short* cornerBasedVolumeData, 
							int width, int height, int sliceBandWidth ) : _VolumeData( cornerBasedVolumeData ) , 
							_VolumeW(width), _VolumeH(height) , _VolumeD(sliceBandWidth)
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



							_ZStep = _VolumeW * _VolumeH;
							_YStep = _VolumeW;

						}




#define grayValue(x , y , z)  _VolumeData[ _ZStep * z + _YStep * y + x ] 
						unsigned short SobelGradientOperator::valueAt(double x, double y, double z)
						{
							unsigned short interpolatedValue = 0;

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




						void SobelGradientOperator::apply(double *point, double *gradient, bool normalize)
						{
							double sumx = 0, sumy = 0, sumz = 0;

							double x = point[0];// / _VoxelSizeX;
							double y = point[1];// / _VoxelSizeY;
							double z = point[2];// / _VoxelSizeZ;

							for (int mm = -1; mm <= 1; mm++)
								for (int nn = -1; nn <= 1; nn++)
									for (int kk = -1; kk <= 1; kk++)
									{
										double xx = x + mm;
										double yy = y + nn;
										double zz = z + kk;

										unsigned short gVal = valueAt(xx, yy, zz);

										sumx += _KernelGx[mm + 1][nn + 1][kk + 1] * gVal;
										sumy += _KernelGy[mm + 1][nn + 1][kk + 1] * gVal;
										sumz += _KernelGz[mm + 1][nn + 1][kk + 1] * gVal;
									}


							gradient[0] = sumx;
							gradient[1] = sumy;
							gradient[2] = sumz;

							//normalize the gradient
							if (normalize)
							{
								double gradientNorm = sqrt(sumx * sumx + sumy * sumy + sumz * sumz);

								gradient[0] /= gradientNorm;
								gradient[1] /= gradientNorm;
								gradient[2] /= gradientNorm;
							}
							
						}


						SobelGradientOperator::~SobelGradientOperator()
						{

						}


					}
				}
			}
		}
	}
}


