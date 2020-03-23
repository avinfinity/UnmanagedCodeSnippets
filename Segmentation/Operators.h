#ifndef _IMT_OPERATOR_H_
#define _IMT_OPERATOR_H_

#include "volumeinfo.h"

namespace imt{
	namespace volume{
		
		class SobelGradientOperator3x3
		{
			int _KernelGx[3][3][3], _KernelGy[3][3][3], _KernelGz[3][3][3];


		public:

			void init();

			void apply(imt::volume::VolumeInfo& volInfo, Eigen::Vector3f *gradientData);

		protected:
			void convolve(int x, int y, int z, size_t zStep, size_t yStep, unsigned short *volData, Eigen::Vector3f& gradient);

		};

		class SobelGradientOperatorMag3x3 : public SobelGradientOperator3x3
		{
			public:
				void apply(imt::volume::VolumeInfo& volInfo, unsigned short *gradientMag);
		};

		class NormalizedSobelGradientOperator3x3x3 : public SobelGradientOperator3x3
		{
			public:
				void apply(imt::volume::VolumeInfo& volInfo, float *Gradient);
		};

		class GaussianOperator3x3
		{
			double _Kernel[3][3][3];

		public:

			void init(float variance);

			void apply(imt::volume::VolumeInfo& volInfo, unsigned short *gaussian);

		protected:
			void convolve(int x, int y, int z, size_t zStep, size_t yStep, unsigned short *volData, unsigned short& value);
		};

		class MedianOperator3x3
		{
		public:
			void init(unsigned int KernelSize);
			void apply(imt::volume::VolumeInfo& volInfo, unsigned short* median);
			
		protected:
			unsigned int windowSize;
			void convolve(int x, int y, int z, size_t zStep, size_t yStep, unsigned short*volData, unsigned short& value);
		};

		class CentralDerivativeOperator3x3x3
		{
			float _Kernelx[3][3][3], _Kernely[3][3][3], _Kernelz[3][3][3];

			void convolve(int x, int y, int z, size_t zStep, size_t yStep, unsigned short* volData, Eigen::Vector3f& value);

		public:
			void init();

			void apply(imt::volume::VolumeInfo& volInfo, Eigen::Vector3f *Gradient);
		};
		
	}
}
#endif