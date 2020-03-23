#include "Operators.h"
#include "iostream"
#include "opencvincludes.h"
#include "eigenincludes.h"
#include "omp.h"
#include "math.h"
#include "algorithm"

namespace imt{
	namespace volume{

		void SobelGradientOperator3x3::init()
		{
			int hx[3] = { 1, 2, 1 }, hy[3] = { 1, 2, 1 }, hz[3] = { 1, 2, 1 };
			int hpx[3] = { -1, 0, 1 }, hpy[3] = { -1, 0, 1 }, hpz[3] = { -1, 0, 1 };
			std::stringstream osx, osy, osz;
			//build the kernel
			for (int k = 0; k <= 2; k++)
				for (int n = 0; n <= 2; n++)
					for (int m = 0; m <= 2; m++)
					{
						_KernelGx[m][n][k] = hpx[m] * hy[n] * hz[k];
						_KernelGy[m][n][k] = hx[m] * hpy[n] * hz[k];
						_KernelGz[m][n][k] = hx[m] * hy[n] * hpz[k];
					}
		}

		void SobelGradientOperator3x3::apply(imt::volume::VolumeInfo& volInfo, Eigen::Vector3f *gradientData)
		{
			int d = volInfo.mDepth;
			int h = volInfo.mHeight;
			int w = volInfo.mWidth;

			size_t zStep = h * w;
			size_t yStep = w;

			unsigned short *vData = (unsigned short*)volInfo.mVolumeData;

			//currently gradient is not defined at the boundaries

			#pragma omp parallel for
			for (int zz = 1; zz < d - 1; zz++)
				for (int yy = 1; yy < h - 1; yy++)
					for (int xx = 1; xx < w - 1; xx++)
					{
						Eigen::Vector3f *gradient = gradientData + zz * zStep + yy * yStep + xx;

						//apply the convolution
						convolve(xx, yy, zz, zStep, yStep, vData, *gradient);
					}
		}

		void SobelGradientOperator3x3::convolve(int x, int y, int z, size_t zStep, size_t yStep,
			unsigned short *volData, Eigen::Vector3f& gradient)
		{

			float sumx = 0, sumy = 0, sumz = 0;

			for (int zz = z - 1, mm = -1; zz <= z + 1; zz++, mm++)
				for (int yy = y - 1, nn = -1; yy <= y + 1; yy++, nn++)
					for (int xx = x - 1, kk = -1; xx <= x + 1; xx++, kk++)
					{
						int gVal = *(volData + zStep * zz + yStep * yy + xx);

						sumx += _KernelGx[mm + 1][nn + 1][kk + 1] * gVal;
						sumy += _KernelGy[mm + 1][nn + 1][kk + 1] * gVal;
						sumz += _KernelGz[mm + 1][nn + 1][kk + 1] * gVal;
					}

			gradient(0) = sumx;
			gradient(1) = sumy;
			gradient(2) = sumz;

		}

		void SobelGradientOperatorMag3x3::apply(imt::volume::VolumeInfo& volInfo, unsigned short *gradientMag)
		{
			int d = volInfo.mDepth;
			int h = volInfo.mHeight;
			int w = volInfo.mWidth;

			size_t zStep = h * w;
			size_t yStep = w;

			unsigned short *vData = (unsigned short*)volInfo.mVolumeData;

			//currently gradient is not defined at the boundaries

			#pragma omp parallel for
			for (int zz = 1; zz < d - 1; zz++)
				for (int yy = 1; yy < h - 1; yy++)
					for (int xx = 1; xx < w - 1; xx++)
					{
						Eigen::Vector3f temp;
						//apply the convolution
						convolve(xx, yy, zz, zStep, yStep, vData, temp);
						*(gradientMag + zz * zStep + yy * yStep + xx) = temp.norm();
					}
		}

		void GaussianOperator3x3::init(float variance)
		{
			double scale = 0;

			for (int m = 0; m <= 2; m++)
				for (int n = 0; n <= 2; n++)
					for (int k = 0; k <= 2; k++)
					{
						_Kernel[m][n][k] = exp(-1 * (pow((m - 1), 2) + pow((n - 1), 2) + pow((k - 1), 2)) / (2 * variance));
						scale += _Kernel[m][n][k];
					}

			double sum = 0;
			for (int m = 0; m <= 2; m++)
				for (int n = 0; n <= 2; n++)
					for (int k = 0; k <= 2; k++)
						_Kernel[m][n][k] = _Kernel[m][n][k] / scale;
		}
		void GaussianOperator3x3::convolve(int x, int y, int z, size_t zStep, size_t yStep, unsigned short *vData, unsigned short& value)
		{
			double sum = 0;
			
			for (int zz = z - 1, mm = -1; zz <= z + 1; zz++, mm++)
				for (int yy = y - 1, nn = -1; yy <= y + 1; yy++, nn++)
					for (int xx = x - 1, kk = -1; xx <= x + 1; xx++, kk++)
						sum += _Kernel[mm+1][nn+1][kk+1] *  *(vData + zz * zStep + yy * yStep + xx );
			value = floor(sum);
		}

		void GaussianOperator3x3::apply(imt::volume::VolumeInfo &volInfo, unsigned short* gaussian)
		{
			unsigned int d = volInfo.mDepth, h = volInfo.mHeight, w = volInfo.mWidth;
			size_t zStep = h*w, yStep = w;

			unsigned short *vData = (unsigned short*)volInfo.mVolumeData;

			//Edge Voxel not supported

			#pragma omp parallel for
			for (int zz = 1; zz < d - 1; zz++)
				for (int yy = 1; yy < h - 1; yy++)
					for (int xx = 1; xx < w - 1; xx++)
					{
						unsigned short temp;
						convolve(xx, yy, zz, zStep, yStep, vData, temp);
						*(gaussian + zz * zStep + yy * yStep + xx) = temp;
					}
		}

		void MedianOperator3x3::init(unsigned int KernelSize)
		{
			windowSize = KernelSize;
		}

		void MedianOperator3x3::convolve(int x, int y, int z, size_t zStep, size_t yStep, unsigned short* volData, unsigned short& value)
		{
			std::vector<unsigned short> window(windowSize*windowSize*windowSize);
			std::vector<unsigned short>::iterator it = window.begin();
			for (int zz = z - (windowSize - 1) / 2; zz <= z + (windowSize - 1) / 2; zz++)
				for (int yy = y - (windowSize - 1) / 2; yy <= y + (windowSize - 1) / 2; yy++)
					for (int xx = x - (windowSize - 1) / 2; xx <= x + (windowSize - 1) / 2; xx++)
					{
						*it = *(volData + zz*zStep + yy*yStep + xx);
						++it;
					}
			int midIndex = (window.size()-1) / 2;
			std::nth_element(window.begin(), window.begin() + midIndex, window.end());
			value = window[midIndex];
		}

		void MedianOperator3x3::apply(imt::volume::VolumeInfo &volInfo, unsigned short* median)
		{
			unsigned int d = volInfo.mDepth, h = volInfo.mHeight, w = volInfo.mWidth;
			size_t zStep = h*w, yStep = w;
			unsigned short* vData = (unsigned short*)volInfo.mVolumeData;
			//Edge Voxel not supported

			#pragma omp parallel for
			for (int zz = (windowSize - 1)/2; zz < d - (windowSize - 1)/2; zz++)
				for (int yy = (windowSize - 1) / 2; yy < h - (windowSize - 1)/2; yy++)
					for (int xx = (windowSize - 1)/2; xx < w - (windowSize - 1) / 2; xx++)
					{
						convolve(xx, yy, zz, zStep, yStep, vData, *(median + zz*zStep + yy*yStep + xx));
					}
		}
		
		void CentralDerivativeOperator3x3x3::init()
		{
			float hd[3] = { -0.5, 0, 0.5 }, hs[3] = { 0, 1, 0 };
			
			for (int k = 0; k <= 2; k++)
				for (int n = 0; n <= 2; n++)
					for (int m = 0; m <= 2; m++)
					{
						_Kernelx[m][n][k] = hd[m] * hs[n] * hs[k];
						_Kernely[m][n][k] = hs[m] * hd[n] * hs[k];
						_Kernelz[m][n][k] = hs[m] * hs[n] * hd[k];
 					}
		}

		void CentralDerivativeOperator3x3x3::apply(imt::volume::VolumeInfo& volInfo, Eigen::Vector3f *gradient)
		{
			unsigned int d = volInfo.mDepth, h = volInfo.mHeight, w = volInfo.mWidth;
			size_t zStep = h*w, yStep = w;
			unsigned short* vData = (unsigned short*)volInfo.mVolumeData;

			#pragma omp parallel for
			for (int zz = 1; zz < d - 1; zz++)
				for (int yy = 1; yy < h - 1; yy++)
					for (int xx = 1; xx < w - 1; xx++)
					{
						convolve(xx, yy, zz, zStep, yStep, vData, *(gradient + zz*zStep + yy*yStep + xx));
					}
		}

		void CentralDerivativeOperator3x3x3::convolve(int xx, int yy, int zz, size_t zStep, size_t yStep, unsigned short* vData, Eigen::Vector3f& value)
		{
			float sumx = 0, sumy = 0, sumz = 0;
			for (int nz = zz - 1, kk = 0; nz <= zz + 1; nz++, kk++)
				for (int ny = yy - 1, nn = 0; ny <= yy + 1; ny++, nn++)
					for (int nx = xx - 1, mm = 0; nx <= xx + 1; nx++, mm++)
					{
						sumx += _Kernelx[mm][nn][kk] * *(vData + nx + ny*yStep + nz*zStep);
						sumy += _Kernely[mm][nn][kk] * *(vData + nx + ny*yStep + nz*zStep);
						sumz += _Kernelz[mm][nn][kk] * *(vData + nx + ny*yStep + nz*zStep);
					}
			value[0] = sumx; value[1] = sumy; value[2] = sumz;
		}
		
		void NormalizedSobelGradientOperator3x3x3::apply(imt::volume::VolumeInfo& volInfo, float* Gradient)
		{
			int d = volInfo.mDepth;
			int h = volInfo.mHeight;
			int w = volInfo.mWidth;

			size_t yStep = w, zStep = w*h;

			unsigned short *vData = (unsigned short*)volInfo.mVolumeData;

			//currently gradient is not define at the boundaries

			#pragma omp parallel for
			for (int zz = 1; zz < d - 1; zz++)
				for (int yy = 1; yy < h - 1; yy++)
					for (int xx = 1; xx < w - 1; xx++)
					{
						Eigen::Vector3f temp;
						//apply the convolution
						convolve(xx, yy, zz, zStep, yStep, vData, temp);
						*(Gradient + xx + yy*yStep + zz*zStep) = (temp.norm()) / (*(vData + xx + yy*yStep + zz*zStep));
					}
		}
	}
}