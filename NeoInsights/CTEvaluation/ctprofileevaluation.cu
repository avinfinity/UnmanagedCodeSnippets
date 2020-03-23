#define  CUDA_LAUNCH_BLOCKING 1
#include "ctprofileevaluation.cuh"
#include <algorithm>
#include "stdio.h"


namespace imt 
{

	namespace volume 
	{
		namespace cuda
		{


			static void HandleError(cudaError_t err,
				const char *file,
				int line) {
				if (err != cudaSuccess) {
					printf("%s in %s at line %d\n", cudaGetErrorString(err),
						file, line);
					exit(EXIT_FAILURE);
				}
			}


			struct ProfileEvaluatorData {

				char data[256];
			};

			struct KernelData 
			{
				float data[48];
			};

#define HANDLE_ERROR( err ) (HandleError( err, __FILE__, __LINE__ ))

			//We put the copy of single profile evaluator into a char array and deserialize it into device profile evaluator , the advantage is 
		    // that all the parameters since remain constant for a profile evaluator , we dont need to reinitialize it for all the threads , but rather we can simply copy it
			__device__ __constant__  ProfileEvaluatorData profileEvaluatorData; //char profileEvaluatorData[256]; 
			__device__ __constant__  KernelData constGaussKernelData , constCannyKernelData , constSecDerKernelData;
			
			
			//these kernels stay constant during gradient based extrema estimation
			__device__ __constant__ float gaussPreEvaluatedKernel[43], cannyPreEvaluatedKernel[43], secDerPreEvaluatedKernel[43];


			__device__ void ippsConvert_16u32f(unsigned short* pSrc, float* pDst, int len)
				{
					for (int ll = 0; ll < len; ll++)
					{
						pDst[ll] = pSrc[ll];
					}

				}


			__device__ void ippsSet_16s(short value, short* arr, int len)
				{
					for (int ll = 0; ll < len; ll++)
					{
						arr[ll] = value;
					}
				}


			__device__ void ippsNorm_L2_32f(float* arr, int len, float* norm)
				{
					*norm = 0;

					for (int ll = 0; ll < len; ll++)
					{
						*norm += arr[ll] * arr[ll];
					}

					*norm = sqrtf(*norm);

				}

			__device__ void ippsSqr_32f_I(float* coeffs, int length)
				{
					for (int ii = 0; ii < length; ii++)
					{
						coeffs[ii] = coeffs[ii] * coeffs[ii];
					}
				}


			__device__ void ippsDivC_32f_I(float denom, float* arr, int  length)
				{
					float invDenom = 1.0f / denom;

					for (int ii = 0; ii < length; ii++)
					{
						arr[ii] *= invDenom; ///= denom; //can use fast inbuilt division function
					}
				}


			__device__ void ippsExp_32f_I(float* arr, int length)
				{
					for (int ii = 0; ii < length; ii++)
					{
						arr[ii] = expf(arr[ii]);
					}

				}


			__device__ void ippsCopy_32f(float *src, float* dst, int len)
				{
					memcpy(dst, src, len * sizeof(float));

				   //for (int ll = 0; ll < len; ll++)
				   //{
					  // dst[ll] = src[ll];
				   //}
				}


			__device__ void ippsCopy_32f(unsigned short *src, float* dst, int len)
				{
					for (int ii = 0; ii < len; ii++)
					{
						dst[ii] = src[ii];
					}
					//memcpy(dst, src, len * sizeof(float));
				}


			__device__ void ippsMul_32f_I(const float* pSrc, float* pSrcDst, int len)
				{
					for (int ii = 0; ii < len; ii++)
					{
						pSrcDst[ii] = pSrcDst[ii] * pSrc[ii];
					}

				}


			__device__ void ippsAddC_32f_I(float val, float *srcDst, int length)
				{
					for (int ll = 0; ll < length; ll++)
					{
						srcDst[ll] += val;
					}
				}



			__device__ int fillGaussCoeffsCUDA(float* gaussCoeffs, float shoch2, int length, float* tempVector)
				{
					ippsSqr_32f_I(gaussCoeffs, length);
					ippsDivC_32f_I(-2.0f * shoch2, gaussCoeffs, length);
					ippsExp_32f_I(gaussCoeffs, length);


					return 0;
				}

			__device__ int fillCoeffsCannyCUDA(float* gaussCoeffs, float shoch2, int length, float* tempVector)
			{
				ippsSqr_32f_I(gaussCoeffs, length);
				ippsDivC_32f_I(-2.0f * shoch2, gaussCoeffs, length);
				ippsExp_32f_I(gaussCoeffs, length);


				return 0;
			}

			__device__ int fillCannyCoeffsCUDA(float* cannyCoeffs, float shoch2, int length, float* t)
				{
					ippsCopy_32f(cannyCoeffs, t, length);
					ippsSqr_32f_I(cannyCoeffs, length);
					ippsDivC_32f_I(-2.0f*shoch2, cannyCoeffs, length);
					ippsExp_32f_I(cannyCoeffs, length);
					ippsDivC_32f_I(-shoch2, cannyCoeffs, length);
					ippsMul_32f_I(t, cannyCoeffs, length);

					return 0;
				}


			__device__ int fillSecDerCoeffsCUDA(float* secDerCoeffs, float shoch2, int length, float* t)
				{
					/*if (!t)
					{
						throw "Memory allocation failed";
					}*/


					ippsSqr_32f_I(secDerCoeffs, length);
					ippsDivC_32f_I(-2.0f*shoch2, secDerCoeffs, length);
					ippsCopy_32f(secDerCoeffs, t, length);
					ippsExp_32f_I(secDerCoeffs, length);

					ippsAddC_32f_I(0.5f, t, length);

					ippsMul_32f_I(t, secDerCoeffs, length);
					ippsDivC_32f_I(-0.5f*shoch2, secDerCoeffs, length);

					return 0;
				}



			__device__ void ippsDotProd_32f(float* src1, float* src2, int len, float* result)
				{
					for (int ll = 0; ll < len; ll++)
					{
						*result += src1[ll] * src2[ll];
					}
				}


			__device__ void ippsDotProd_32f(unsigned short* src1, float* src2, int len, float* result)
				{
					for (int ll = 0; ll < len; ll++)
					{
						*result += src1[ll] * src2[ll];
					}
				}

			__device__ void ippsDotProd_32f(unsigned short* src1, unsigned short* src2, int len, float* result)
				{
					for (int ll = 0; ll < len; ll++)
					{
						*result += src1[ll] * src2[ll];
					}
				}


			__device__ void ippsSub_32f_I(float* pSrc, float* pSrcDst, int length)
				{
					for (int ll = 0; ll < length; ll++)
					{
						pSrcDst[ll] -= pSrc[ll];
					}
				}


			__device__ void ippsSub_32f_I(unsigned short* pSrc, float* pSrcDst, int length)
				{
					for (int ll = 0; ll < length; ll++)
					{
						pSrcDst[ll] -= pSrc[ll];
					}
				}



			__device__ void ippsConv_32f(const float* pSrc1, int src1Len, const float* pSrc2, int src2Len, float* pDst)
				{
					int dstLen = src1Len + src2Len - 1;

					for (int ll = 0; ll < dstLen; ll++)
					{
						float conv = 0;

						int start = __max(0, ll - src2Len + 1);
						int end = __min(ll, src1Len - 1);

						for (int kk = start; kk <= end; kk++)
						{
							//int p = ll - kk;

							conv += pSrc1[kk] * pSrc2[ll - kk];
						}

						pDst[ll] = conv;
					}

				}

			__device__ void ippsConv_32f(const unsigned short* pSrc1, int src1Len, const float* pSrc2, int src2Len, float* pDst)
				{
					int dstLen = src1Len + src2Len - 1;

					for (int ll = 0; ll < dstLen; ll++)
					{
						float conv = 0;

						int start = __max(0, ll - src2Len + 1);
						int end = __min(ll, src1Len - 1);

						for (int kk = start; kk <= end; kk++)
						{
							//int p = ll - kk;

							conv += pSrc1[kk] * pSrc2[ll - kk];
						}

						pDst[ll] = conv;
					}

				}

				//pSrcDst[n] = pSrcDst[n] + pSrc[n]*val, 0 ≤ n < len
			__device__ void ippsAddProductC_32f(const float* pSrc, const float val, float* pSrcDst, int len)
				{
					for (int ll = 0; ll < len; ll++)
					{
						pSrcDst[ll] += val * pSrc[ll];
					}
				}


			__device__ void ippsMulC_32f_I(float val, float* pSrcDst, int length)
				{
					for (int ll = 0; ll < length; ll++)
					{
						pSrcDst[ll] *= val;
					}

				}



			__device__ CCTProfilsEvaluationSP_Device::CCTProfilsEvaluationSP_Device()
				{
					voxelStep = 0.25;
					//profile = NULL;
					memoryAllocated = false;
					length = 0;
					nProfils = 0;
					zeroIndex = 0;
					gaussCoeffs = 0;
					cannyCoeffs = 0;
					secDerCoeffs = 0;
					filterCoeffs = 0;
					tempVector = 0;
					sigma = 0.0;
					threshold = 0.0;
					voxelType = Void;
					searchRange = 20;
					searchRangeNeg = 0;
					tempConvLength = 0;
					tempConvProfile = 0;
					results = NULL;
					resCanny = NULL;
					resQuality = NULL;
					ptValid = NULL;
					rangeFactor = 3.5;
					nValid = 0;
				}

	   //     __device__ void CCTProfilsEvaluationSP_Device::Init()
				//{
				//	//assert(sigma > 0.4);
				//	dynSigma = sigma;
				//	shoch2 = dynSigma * dynSigma;
				//	gaussCoeffs = 0;
				//	cannyCoeffs = 0;
				//	secDerCoeffs = 0;
				//	filterCoeffs = 0;
				//	tempVector = 0;
				//	searchRangeNeg = searchRange;
				//	dynThresholdControl = false;
				//	dynThreshold = threshold;
				//	tempConvProfile = 0;
				//	tempConvLength = 0;
				//	coeffLength = 0;
				//	PreCalc();
				//	firstValid = -1;
				//	lastValid = -1;
				//	results = NULL;
				//	resCanny = NULL;
				//	resQuality = NULL;
				//	ptValid = NULL;
				//	nAngle = 0;
				//	rangeFactor = 3.5;
				//	nValid = 0;

				//}
			
			__device__ CCTProfilsEvaluationSP_Device::~CCTProfilsEvaluationSP_Device(void)
				{

					//delete[] gaussCoeffs;
					//delete[] cannyCoeffs;
					//delete[] secDerCoeffs;
					//delete[] filterCoeffs;
					//delete[] tempVector;
					////ZiTrace("del tempConvProfile Destruktor: %x alte Länge: %d\n",tempConvProfile,tempConvLength);
					//delete[] tempConvProfile;
					//if (memoryAllocated) delete[] profile;

					//delete[] results;
					//delete[] resCanny;
					//delete[] resQuality;
					//delete[] ptValid;
				}
				// Negativen Suchbereich abweichend von positivem setzen
			//__device__ void CCTProfilsEvaluationSP_Device::SetSearchRangeNeg(float srNeg)
			//	{
			//		if (srNeg == 0.0)
			//		{
			//			searchRangeNeg = searchRange;
			//		}
			//		else
			//		{
			//			searchRangeNeg = (int)ceil(srNeg / voxelStep);
			//		}

			//	}

			//	// Suchbereich  setzen
			//__device__ void CCTProfilsEvaluationSP_Device::SetSearchRange(float sr)
			//	{
			//		searchRange = (int)ceil(sr / voxelStep);

			//	}

			__device__ float Derivatives(float x, const ProfileEvaluationConstants& p, unsigned short* profile_16U, 
				float* filterCoeffs, float* tempVector , const float& dynSigma , const float& shoch2, int(*callback)(float*, float, int, float*))
			{
				//assert(sigma > 0.0);

				int actFilterLength = int(p.rangeFactor * dynSigma / p.voxelStep);

				//std::cout << "act filter length : " << actFilterLength<<" "<<dynSigma << std::endl;

				//assert(actFilterLength <= coeffLength);

				int filterIndex = int(floor(x / p.voxelStep)) + p.zeroIndex - actFilterLength;	// Index Beginn Filtermaske

				//assert(filterIndex >= 0 && filterIndex + 2 * actFilterLength + 1 < length);

				filterCoeffs[0] = (float)((filterIndex - p.zeroIndex + 0.5) * p.voxelStep - x);

				for (int ii = 1; ii < 2 * actFilterLength + 1; ii++)
				{
					filterCoeffs[ii] = filterCoeffs[ii - 1] + (float)p.voxelStep;

					//printf("%f ", filterCoeffs[ii]);
				}

				callback(filterCoeffs, shoch2, 2 * actFilterLength, tempVector);

				auto dat = profile_16U + filterIndex;
				ippsCopy_32f(profile_16U + filterIndex, tempVector, 2 * actFilterLength + 1);

				ippsSub_32f_I(profile_16U + filterIndex + 1, tempVector, 2 * actFilterLength + 1);

				float result = 0;

				ippsDotProd_32f(tempVector, filterCoeffs, 2 * actFilterLength, &result);

				return -result;
			}



				// Gauss-gefilterter Wert
				__device__ float CCTProfilsEvaluationSP_Device::Gauss(float x, int iProfil)
				{
					actFilterLength = int(rangeFactor * dynSigma / voxelStep);

					//assert(actFilterLength <= coeffLength);

					filterIndex = int(floor(x / voxelStep)) + zeroIndex - actFilterLength;	// Index Beginn Filtermaske

					if (x / voxelStep - floor(x / voxelStep) > 0.5)
						filterIndex++;

					//assert(filterIndex >= 0 && filterIndex + 2 * actFilterLength < length);

					filterCoeffs[0] = (float)((filterIndex - zeroIndex) * voxelStep - x);

					for (ii = 1; ii < 2 * actFilterLength + 1; ii++)
					{
						filterCoeffs[ii] = filterCoeffs[ii - 1] + (float)voxelStep;
					}

					fillGaussCoeffsCUDA(filterCoeffs, shoch2, 2 * actFilterLength + 1, tempVector);

					result = 0;

					ippsDotProd_32f(profile_16U + iProfil * length + filterIndex, filterCoeffs, 2 * actFilterLength + 1, &result);

					return voxelStep * result / dynSigma / sqrtf(2 * M_PI);
				}


				__device__ float Gauss(float x, const ProfileEvaluationConstants& p, unsigned short* profile_16U, float* filterCoeffs, float* tempVector, const float& dynSigma , const float& shoch2 )
				{
					int actFilterLength = int( p.rangeFactor * dynSigma / p.voxelStep);

					//assert(actFilterLength <= coeffLength);

					int filterIndex = int(floor(x / p.voxelStep)) + p.zeroIndex - actFilterLength;	// Index Beginn Filtermaske

					if (x / p.voxelStep - floor(x / p.voxelStep) > 0.5)
						filterIndex++;

					//assert(filterIndex >= 0 && filterIndex + 2 * actFilterLength < length);

					filterCoeffs[0] = (float)((filterIndex - p.zeroIndex) * p.voxelStep - x);

					for ( int ii = 1; ii < 2 * actFilterLength + 1; ii++)
					{
						filterCoeffs[ii] = filterCoeffs[ii - 1] + (float)p.voxelStep;
					}

					fillGaussCoeffsCUDA(filterCoeffs, shoch2, 2 * actFilterLength + 1, tempVector);

					float result = 0;

					ippsDotProd_32f( profile_16U +  filterIndex, filterCoeffs, 2 * actFilterLength + 1, &result);

					return p.voxelStep * result / dynSigma / sqrtf(2 * M_PI);
				}



				// Erste gefilterte Ableitung - Canny
				__device__ float CCTProfilsEvaluationSP_Device::Canny(float x, int iProfil)
				{
					//printf("[canny start gpu]\n");

					float c = Derivatives(x, iProfil, &fillGaussCoeffsCUDA);

					//printf("[Canny output %f]\n", c);

					return c;
				}

				__device__ float Canny(float x, const ProfileEvaluationConstants& p, unsigned short* profile_16U, float* filterCoeffs, float* tempVector, const float& dynSigma, const float& shoch2)
				{
					//printf("[canny start gpu]\n");

					float c = Derivatives(x, p, profile_16U, filterCoeffs, tempVector, dynSigma , shoch2 , &fillGaussCoeffsCUDA);

					//printf("[Canny output %f]\n", c);

					return c;
				}

				// Zweite gefilterte Ableitung - SecDer
				__device__ float CCTProfilsEvaluationSP_Device::SecondDer(float x, int iProfil)
				{
					return Derivatives(x, iProfil, &fillCannyCoeffsCUDA);
				}


				__device__ float SecondDer(float x, const ProfileEvaluationConstants& p, unsigned short* profile_16U, float* filterCoeffs, float* tempVector , const float& dynSigma , const float& shoch2 )
				{
					return Derivatives( x, p, profile_16U, filterCoeffs, tempVector , dynSigma , shoch2 , &fillCannyCoeffsCUDA);
				}


				// Dritte gefilterte Ableitung - ThirdDer
				__device__ float CCTProfilsEvaluationSP_Device::ThirdDer(float x, int iProfil)
				{
					return -Derivatives(x, iProfil, &fillSecDerCoeffsCUDA);
				}

				// Dritte gefilterte Ableitung - ThirdDer
				__device__ float  ThirdDer(float x, const ProfileEvaluationConstants& p, unsigned short* profile_16U , float* filterCoeffs , float* tempVector, const float& dynSigma, const float& shoch2)
				{
					return -Derivatives(x, p , profile_16U, filterCoeffs , tempVector, dynSigma , shoch2,  &fillSecDerCoeffsCUDA);
				}

				// Basisfunktion für gefilterte Ableitungen des Grauwertprofils

				//Basic function for filtered derivatives of the gray value profile
				__device__ float CCTProfilsEvaluationSP_Device::Derivatives(float x, int iProfil, int(*callback)(float*, float, int, float*))
				{
					//assert(sigma > 0.0);

					actFilterLength = int(rangeFactor * dynSigma / voxelStep);

					//std::cout << "act filter length : " << actFilterLength<<" "<<dynSigma << std::endl;

					//assert(actFilterLength <= coeffLength);

					filterIndex = int(floor(x / voxelStep)) + zeroIndex - actFilterLength;	// Index Beginn Filtermaske

					//assert(filterIndex >= 0 && filterIndex + 2 * actFilterLength + 1 < length);

					filterCoeffs[0] = (float)((filterIndex - zeroIndex + 0.5)*voxelStep - x);

					for (ii = 1; ii < 2 * actFilterLength + 1; ii++)
					{
						filterCoeffs[ii] = filterCoeffs[ii - 1] + (float)voxelStep;

						//printf("%f ", filterCoeffs[ii]);
					}

					callback(filterCoeffs, shoch2, 2 * actFilterLength, tempVector);

					auto dat = profile_16U + iProfil * length + filterIndex;
					ippsCopy_32f(profile_16U + iProfil * length + filterIndex, tempVector, 2 * actFilterLength + 1);

					ippsSub_32f_I(profile_16U + iProfil * length + filterIndex + 1, tempVector, 2 * actFilterLength + 1);

					result = 0;

					ippsDotProd_32f(tempVector, filterCoeffs, 2 * actFilterLength, &result);

					return -result;
				}




				__device__ float CCTProfilsEvaluationSP_Device::CannyOpt(int i, int iProfil)
				{
					//assert(i >= coeffLength && i + coeffLength < length);
					result = 0;
					ippsDotProd_32f(profile_16U + iProfil * length + i - coeffLength, gaussCoeffs, 2 * coeffLength + 1, &result);
					return result;
				}

				__device__ float CannyOpt(int i , const ProfileEvaluationConstants& p, unsigned short* profile_16U)
				{
					//assert(i >= coeffLength && i + coeffLength < length);
					float result = 0;
					
					ippsDotProd_32f(profile_16U + i - p.coeffLength, p.gaussCoeffs, 2 * p.coeffLength + 1, &result);
					
					return result;
				}


				__device__ float CCTProfilsEvaluationSP_Device::SecDerOpt(int i, int iProfil)
				{
					//assert(i >= coeffLength && i + coeffLength < length);
					result = 0;
					ippsDotProd_32f(profile_16U + iProfil * length + i - coeffLength, cannyCoeffs, 2 * coeffLength + 1, &result);
					return result;
				}

				__device__ float SecDerOpt(int i , const ProfileEvaluationConstants& p, unsigned short* profile_16U  )
				{
					//assert(i >= coeffLength && i + coeffLength < length);
					float result = 0;

					ippsDotProd_32f( profile_16U + i - p.coeffLength , p.cannyCoeffs , 2 * p.coeffLength + 1 , &result );
					
					return result;
				}

				__device__ int CCTProfilsEvaluationSP_Device::FoldCannyOpt(int iProfil, float *cannyProfile)
				{
					//assert(cannyProfile);
					//assert(zeroIndex - searchRangeNeg >= coeffLength && zeroIndex + searchRange + coeffLength < length);
					ippsConv_32f(profile_16U + iProfil * length + zeroIndex - searchRangeNeg - coeffLength, 2 * coeffLength + searchRange + searchRangeNeg + 1, gaussCoeffs, 2 * coeffLength + 1, cannyProfile);
					return searchRangeNeg + 2 * coeffLength; // Das ist der ZeroIndex
				}

				__device__ int FoldCannyOpt(const ProfileEvaluationConstants& p, unsigned short* profile_16U, float *cannyProfile)
				{
					//assert(cannyProfile);
					//assert(zeroIndex - searchRangeNeg >= coeffLength && zeroIndex + searchRange + coeffLength < length);
					ippsConv_32f( profile_16U + p.zeroIndex - p.searchRangeNeg - p.coeffLength,
						          2 * p.coeffLength + p.searchRange + p.searchRangeNeg + 1,
						          p.gaussCoeffs, 2 * p.coeffLength + 1, cannyProfile);
					
					return p.searchRangeNeg + 2 * p.coeffLength; // Das ist der ZeroIndex
				}

				__device__ int CCTProfilsEvaluationSP_Device::FoldSecDerOpt(int iProfil, float *secDerProfile)
				{
					//assert(secDerProfile);
					//assert(zeroIndex - searchRangeNeg >= coeffLength && zeroIndex + searchRange + coeffLength <= length);
					ippsConv_32f( profile_16U + iProfil * length + zeroIndex - searchRangeNeg - coeffLength , 
						          2 * coeffLength + searchRange + searchRangeNeg + 1, cannyCoeffs, 2 * coeffLength + 1, secDerProfile);

					//printf("%d %d %d \n", zeroIndex - searchRangeNeg - coeffLength, (2 * coeffLength + searchRange + searchRangeNeg + 1), 2 * coeffLength + 1);
					return searchRangeNeg + 2 * coeffLength; // Das ist der ZeroIndex
				}

				__device__ int FoldSecDerOpt( const ProfileEvaluationConstants& p,  unsigned short* profile_16U ,  float *secDerProfile)
				{
					//assert(secDerProfile);
					//assert(zeroIndex - searchRangeNeg >= coeffLength && zeroIndex + searchRange + coeffLength <= length);
					ippsConv_32f(profile_16U + p.zeroIndex - p.searchRangeNeg - p.coeffLength,
						2 * p.coeffLength + p.searchRange + p.searchRangeNeg + 1, p.cannyCoeffs, 2 * p.coeffLength + 1, secDerProfile);

					//printf("%d %d %d \n", zeroIndex - searchRangeNeg - coeffLength, (2 * coeffLength + searchRange + searchRangeNeg + 1), 2 * coeffLength + 1);
					return p.searchRangeNeg + 2 * p.coeffLength; // Das ist der ZeroIndex
				}


				__device__ int CCTProfilsEvaluationSP_Device::FoldThirdDerOpt(int iProfil, float *thirdDerProfile, int convRangeNeg, int convRangePos)
				{
					//assert(thirdDerProfile);

					if (!convRangeNeg || zeroIndex - convRangeNeg < coeffLength)
						convRangeNeg = zeroIndex - coeffLength;

					if (!convRangePos || zeroIndex + convRangePos + coeffLength >= length)
						convRangePos = length - coeffLength - zeroIndex - 1;

					//assert(zeroIndex - convRangeNeg >= coeffLength && zeroIndex + convRangePos + coeffLength < length);

					ippsConv_32f(profile_16U + iProfil * length + zeroIndex - convRangeNeg - coeffLength,
						2 * coeffLength + convRangePos + convRangeNeg + 1, secDerCoeffs,
						2 * coeffLength + 1, thirdDerProfile);

					return convRangeNeg + 2 * coeffLength; // Das ist der ZeroIndex
				}


				__device__ int FoldThirdDerOpt( const ProfileEvaluationConstants& p , unsigned short* profile_16U, float *thirdDerProfile, int convRangeNeg, int convRangePos)
				{
					//assert(thirdDerProfile);

					if (!convRangeNeg || p.zeroIndex - convRangeNeg < p.coeffLength)
						convRangeNeg = p.zeroIndex - p.coeffLength;

					if (!convRangePos || p.zeroIndex + convRangePos + p.coeffLength >= p.length)
						convRangePos = p.length - p.coeffLength - p.zeroIndex - 1;

					//assert(zeroIndex - convRangeNeg >= coeffLength && zeroIndex + convRangePos + coeffLength < length);

					ippsConv_32f( profile_16U + p.zeroIndex - convRangeNeg - p.coeffLength,
						          2 * p.coeffLength + convRangePos + convRangeNeg + 1, p.secDerCoeffs,
						          2 * p.coeffLength + 1, thirdDerProfile);

					return convRangeNeg + 2 * p.coeffLength; // Das ist der ZeroIndex
				}



				// direct put dyn Sigma
				__device__ void CCTProfilsEvaluationSP_Device::PutDynSigma(float newValue)
				{
					dynSigma = newValue;
					shoch2 = dynSigma * dynSigma;
				}



				__device__ void PutDynSigma( const ProfileEvaluationConstants&p , float newValue , float& dynSigma , float& shoch2 )
				{
					dynSigma = newValue;
					shoch2 = dynSigma * dynSigma;
				}


				// Dynamisches p.sigma begrenzen (kleiner als p.sigma und > 0.75)
				__device__ bool SetDynSigma( CCTProfilsEvaluationSP_Device& p , float x, int iProfil)
				{
					//	DPVector::const_iterator i;

					float curThreshold = -0.1f*p.Canny(x, iProfil);
					bool minBegrenzung = true, maxBegrenzung = true;
					float minIndex = x, maxIndex = x, xx;
					// Suche neg. Umkehrpunkt im Profil mit 10% Toleranz
					do
					{
						minIndex -= p.voxelStep / 4;
					} while (p.Canny(minIndex, iProfil) > curThreshold &&
						(minIndex - x < 4 * p.sigma) &&
						(minIndex / p.voxelStep > -p.searchRangeNeg));
					// Überprüfen auf reale Gegenflanke ab 50% Höhe
					xx = minIndex;

					do
					{
						xx -= p.voxelStep / 4;

						if (x - xx > 4 * p.sigma || (xx / p.voxelStep <= -p.searchRangeNeg))
							break;
					} while (minBegrenzung = (p.Canny(xx, iProfil) > 5 * curThreshold));


					// Suche pos. Umkehrpunkt im Profil mit 10% Toleranz
					curThreshold = -0.1f*p.Canny(x, iProfil);
					do
					{
						maxIndex += p.voxelStep / 4;
					} while (p.Canny(maxIndex, iProfil) > curThreshold &&
						(maxIndex - x < 4 * p.sigma) &&
						(maxIndex / p.voxelStep > p.searchRange));

					// Überprüfen auf reale Gegenflanke ab 50% Höhe
					xx = maxIndex;
					do
					{
						xx += p.voxelStep / 4;

						if (xx - x > 4 * p.sigma || xx / p.voxelStep >= p.searchRange)
							break;
					} while (maxBegrenzung = (p.Canny(xx, iProfil) > 5 * curThreshold));

					// Wenn Gegenflanke, p.sigma eingernzen auf Abstand zum Umkehrpunkt
					// DER FAKTOR 4.0 IST EXPERIMENTELL

					//	When counter - flanking, p.sigma is on the distance to the reversal point
					// THE FACTOR 4.0 IS EXPERIMENTAL
					if (!(minBegrenzung && maxBegrenzung))
						p.dynSigma = (float)((maxIndex - x) < (x - minIndex) ? (maxIndex - x) : (x - minIndex)) / 4.0f;
					else
					{
						p.dynSigma = p.sigma;
						p.shoch2 = p.dynSigma* p.dynSigma;

						return false;
					}

					// Bereich begrenzen
					if (p.dynSigma > p.sigma)
					{
						p.dynSigma = p.sigma;
						p.shoch2 = p.dynSigma* p.dynSigma;
						return false;
					}
					if (p.dynSigma < 0.35f)
						p.dynSigma = 0.35f;

					p.shoch2 = p.dynSigma* p.dynSigma;

					return true;

				}


				__device__ bool SetDynSigma(const ProfileEvaluationConstants& p, float x, unsigned short* profile_16U, float* filterBuffer, float* tempVector, float& dynSigma, float& shoch2 )
				{
					//	DPVector::const_iterator i;

					float curThreshold = -0.1f * Canny(x, p,  profile_16U, filterBuffer, tempVector , dynSigma , shoch2 ); //p.Canny(x, iProfil);
					bool minBegrenzung = true, maxBegrenzung = true;
					float minIndex = x, maxIndex = x, xx;
					// Suche neg. Umkehrpunkt im Profil mit 10% Toleranz
					do
					{
						minIndex -= p.voxelStep / 4;
					} while ( Canny(minIndex, p, profile_16U, filterBuffer, tempVector, dynSigma, shoch2) > curThreshold && //while (p.Canny(minIndex, iProfil) > curThreshold &&
						(minIndex - x < 4 * p.sigma) &&
						(minIndex / p.voxelStep > -p.searchRangeNeg));
					// Überprüfen auf reale Gegenflanke ab 50% Höhe
					xx = minIndex;

					do
					{
						xx -= p.voxelStep / 4;

						if (x - xx > 4 * p.sigma || (xx / p.voxelStep <= -p.searchRangeNeg))
							break;
					} while (minBegrenzung = (Canny(xx, p, profile_16U, filterBuffer, tempVector, dynSigma, shoch2) > 5 * curThreshold));


					// Suche pos. Umkehrpunkt im Profil mit 10% Toleranz
					curThreshold = -0.1f*Canny(x, p, profile_16U, filterBuffer, tempVector, dynSigma, shoch2);
					do
					{
						maxIndex += p.voxelStep / 4;
					} while (Canny(maxIndex, p, profile_16U, filterBuffer, tempVector, dynSigma, shoch2) > curThreshold &&
						(maxIndex - x < 4 * p.sigma) &&
						(maxIndex / p.voxelStep > p.searchRange));

					// Überprüfen auf reale Gegenflanke ab 50% Höhe
					xx = maxIndex;
					do
					{
						xx += p.voxelStep / 4;

						if (xx - x > 4 * p.sigma || xx / p.voxelStep >= p.searchRange)
							break;
					} while (maxBegrenzung = (Canny(xx, p, profile_16U, filterBuffer, tempVector, dynSigma, shoch2) > 5 * curThreshold));

					// Wenn Gegenflanke, p.sigma eingernzen auf Abstand zum Umkehrpunkt
					// DER FAKTOR 4.0 IST EXPERIMENTELL

					//	When counter - flanking, p.sigma is on the distance to the reversal point
					// THE FACTOR 4.0 IS EXPERIMENTAL
					if (!(minBegrenzung && maxBegrenzung))
						dynSigma = (float)((maxIndex - x) < (x - minIndex) ? (maxIndex - x) : (x - minIndex)) / 4.0f;
					else
					{
						dynSigma = p.sigma;
						shoch2 = dynSigma * dynSigma;

						return false;
					}

					// Bereich begrenzen
					if ( dynSigma > p.sigma)
					{
						dynSigma = p.sigma;
						shoch2 = dynSigma * dynSigma;
						return false;
					}
					if ( dynSigma < 0.35f)
						dynSigma = 0.35f;

					shoch2 = dynSigma * dynSigma;

					return true;

				}



				__device__ bool NewtonMax( CCTProfilsEvaluationSP_Device& p , float& x, int iProfil)
				{
					bool result = true;
					float start_x = x;
					float z;
					int	it = 0;
					float lastZ;

					//printf("start x : %f \n", start_x);



					do
					{
						z = p.ThirdDer(x, iProfil);

						if (z == 0) {
							result = false;
							break;
						}

						z = p.SecondDer(x, iProfil) / z; // Neue Schrittweite 

						//printf("z %f : ", z);

						if (it == 0 && fabs(z) > 1.0f)
							z *= 0.1f;
						
						if (fabs(z) > 3.0f)	// konvergiert offenbar nicht, empirisch gewonnen
						{
							result = false;
							break;
						}
						
						if (it > 0 && std::abs(z + lastZ) < 0.01f)
							z *= 0.5f;

						x = x - z;			// Korrektur anwenden

						//printf("%f ", x);

						lastZ = z;

						if (it++ > 25)			// Endlositeration
						{
							result = false;
							break;
						}

					} while (fabs(z) > 0.001);  // 0.001 bezieht sich auf Voxelmass und sollte ausreichen

					//printf("\n");

					if (!result)
						x = start_x;

					return result;

				}


				__device__ bool NewtonMax( const ProfileEvaluationConstants& p, float& x, unsigned short* profile_16U, float* filterBuffer, float* tempVector, const float& dynSigma, const float& shoch2)
				{
					bool result = true;
					float start_x = x;
					float z;
					int	it = 0;
					float lastZ;

					//printf("start x : %f \n", start_x);


					do
					{
						z = ThirdDer(x, p , profile_16U , filterBuffer , tempVector, dynSigma, shoch2);

						if (z == 0) {
							result = false;
							break;
						}

						z = SecondDer(x, p, profile_16U, filterBuffer, tempVector, dynSigma , shoch2) / z; //p.SecondDer(x, iProfil) / z; // Neue Schrittweite 

						if (it == 0 && fabs(z) > 1.0f)
							z *= 0.1f;

						if (fabs(z) > 3.0f)	// konvergiert offenbar nicht, empirisch gewonnen
						{
							result = false;
							break;
						}

						if (it > 0 && std::abs(z + lastZ) < 0.01f)
							z *= 0.5f;

						x = x - z;			// Korrektur anwenden

						//printf("%f ", x);

						lastZ = z;

						if (it++ > 25)			// Endlositeration
						{
							result = false;
							break;
						}

					} while (fabs(z) > 0.001);  // 0.001 bezieht sich auf Voxelmass und sollte ausreichen

					//printf("\n ", x);

					if (!result)
						x = start_x;

					return result;

				}




				__device__ float GradientLength(CCTProfilsEvaluationSP_Device& p, float x, int iProfil, float* gaussLow, float* gaussHigh, float xCanny)
				{
					if (xCanny == 0 && p.ptValid[iProfil])
						xCanny = p.resCanny[iProfil];
					int sign = 1;
					if (xCanny < 0) sign = -1;	// Sprung abwärts (interessant für Mehr-Material)
					// Suche des Parameters mit 50% xCanny (Maximalwert)
					int iLow = (int)floor((x) / p.voxelStep);
					int iBase = iLow;
					while (sign * p.SecDerOpt(iLow + p.zeroIndex, iProfil) > -0.25*sign * xCanny / p.dynSigma && (iBase - iLow) * p.voxelStep <= 5.0 && (iLow + p.zeroIndex > p.coeffLength))
						iLow--;

					if (!((iBase - iLow)*p.voxelStep <= 5.0))
						iLow = iBase - 1;
					int iHigh = iBase + 1;

					while (sign*p.SecDerOpt(iHigh + p.zeroIndex, iProfil) < 0.25*sign*xCanny / p.dynSigma && (iHigh - iBase)*p.voxelStep < 5.0 && (iHigh + p.zeroIndex < p.length - p.coeffLength - 1))
						iHigh++;

					if (!((iHigh - iBase)*p.voxelStep < 5.0))
						iHigh = iBase + 1;
					// Faltung dritte Ableitung +/- 10 Voxel um x
					int searchRangeRoot = int(10.0 / p.voxelStep);
					int coeffDistance = int(p.coeffLength / p.voxelStep);

					if (p.zeroIndex + iBase - searchRangeRoot <= coeffDistance)
						searchRangeRoot = p.zeroIndex + iBase - coeffDistance;

					if (p.zeroIndex + iBase + searchRangeRoot >= p.length - coeffDistance)
						searchRangeRoot = searchRangeRoot - (p.zeroIndex + iBase + coeffDistance);

					int foldZeroIndex = p.FoldThirdDerOpt(iProfil, p.tempConvProfile, -iBase + searchRangeRoot, iBase + searchRangeRoot);

					// Suche nach Nullstelle in dritter Ableitung Luftseite
					iHigh += foldZeroIndex;
					iLow += foldZeroIndex;
					iBase += foldZeroIndex;
					float x_vw = 0.0, x_rw = 0.0;						// Treffer der Vor- und Rückwärtssuche
					bool hit_vw = false, hit_rw = false;				// Indikatoren für Treffer mit Schwellwert
					// Loop mit gleichteitiger Vor- und Rückwärtssuceh
					while (1)
					{
						// Test Suchbereich und Vorzeichenwechsel 2.Abl.
						if ((iHigh - iBase) * p.voxelStep <= searchRangeRoot * p.voxelStep && sign*p.tempConvProfile[iHigh + 1] < 0 && sign*p.tempConvProfile[iHigh]>0)
						{
							// Interpolation Treffer vorwärts
							x_vw = (iHigh + p.tempConvProfile[iHigh] / (p.tempConvProfile[iHigh] - p.tempConvProfile[iHigh + 1]) - foldZeroIndex)*p.voxelStep;
							int iTest = (int)floor(x_vw / p.voxelStep + 0.5);
							float t = sign * p.CannyOpt(/*iHigh - foldZeroIndex*/iTest + p.zeroIndex, iProfil);
							if (t > 0.05*sign*xCanny && t<0.85*sign*xCanny && sign*p.SecDerOpt(/*iHigh - foldZeroIndex*/iTest + p.zeroIndex, iProfil)>0.15*sign*xCanny / p.dynSigma) hit_vw = true;
						}
						// Test Suchbereich und Vorzeichenwechsel 2.Abl.
						if ((iBase - iLow)*p.voxelStep <= searchRangeRoot * p.voxelStep && sign*p.tempConvProfile[iLow] > 0 && sign*p.tempConvProfile[iLow - 1] < 0)
						{
							// Interpolation Treffer rückwärts
							x_rw = (iLow - p.tempConvProfile[iLow] / (p.tempConvProfile[iLow] - p.tempConvProfile[iLow - 1]) - foldZeroIndex)*p.voxelStep;
							int iTest = (int)floor(x_rw / p.voxelStep + 0.5);
							float t = sign * p.CannyOpt(/*iLow - foldZeroIndex*/iTest + p.zeroIndex, iProfil);
							if (t > 0.05*sign*xCanny && t < 0.85*sign*xCanny && sign*p.SecDerOpt(/*iLow - foldZeroIndex*/iTest + p.zeroIndex, iProfil) < -0.15*sign*xCanny / p.dynSigma) hit_rw = true;
						}
						if (hit_vw && hit_rw)
							break;				// beide Grenzen gefunden
						if ((iBase - iLow)*p.voxelStep >= searchRangeRoot * p.voxelStep || (iHigh - iBase)*p.voxelStep >= searchRangeRoot * p.voxelStep)
							break;				// Suchbereich abgegrast
						iHigh++; iLow--;
					}
					if (hit_vw && hit_rw)
					{
						if (sign == -1)
						{
							if (gaussLow) *gaussLow = p.Gauss(x_vw, iProfil);
							if (gaussHigh) *gaussHigh = p.Gauss(x_rw, iProfil);
						}
						else
						{
							if (gaussLow) *gaussLow = p.Gauss(x_rw, iProfil);
							if (gaussHigh) *gaussHigh = p.Gauss(x_vw, iProfil);
						}
						return x_vw - x_rw;	// Differenz zwischen Wendepunkten ist gesuchte Kenngröße
					}
					else
					{
						if (gaussLow) *gaussLow = 0;
						if (gaussHigh) *gaussHigh = 0;
						return 0.0;
					}
				}



				__device__ float GradientLength( const ProfileEvaluationConstants& p, float x, unsigned short* profile_16U , float* tempConvProfile , float* filterBuffer, float* tempVector, 
					bool& ptValid , float& resCanny ,  float* gaussLow, float* gaussHigh, float xCanny, const float& dynSigma, const float& shoch2)
				{
					/*if (xCanny == 0 && p.ptValid[iProfil])
						xCanny = p.resCanny[iProfil];
*/
					if (xCanny == 0 && ptValid)
						xCanny = resCanny;

					int sign = 1;
					if (xCanny < 0) sign = -1;	// Sprung abwärts (interessant für Mehr-Material)
					// Suche des Parameters mit 50% xCanny (Maximalwert)
					int iLow = (int)floor((x) / p.voxelStep);
					int iBase = iLow;
					//while (sign * p.SecDerOpt(iLow + p.zeroIndex, iProfil) > -0.25*sign * xCanny / p.dynSigma && (iBase - iLow) * p.voxelStep <= 5.0 && (iLow + p.zeroIndex > p.coeffLength))
					while (sign * SecDerOpt(iLow + p.zeroIndex, p , profile_16U) > -0.25*sign * xCanny / dynSigma && (iBase - iLow) * p.voxelStep <= 5.0 && (iLow + p.zeroIndex > p.coeffLength))
					iLow--;

					if (!((iBase - iLow)*p.voxelStep <= 5.0))
						iLow = iBase - 1;
					int iHigh = iBase + 1;

					//while (sign*p.SecDerOpt(iHigh + p.zeroIndex, iProfil) < 0.25*sign*xCanny / p.dynSigma && (iHigh - iBase)*p.voxelStep < 5.0 && (iHigh + p.zeroIndex < p.length - p.coeffLength - 1))
					while ( sign * SecDerOpt( iHigh + p.zeroIndex, p, profile_16U ) < 0.25*sign*xCanny / dynSigma && (iHigh - iBase)*p.voxelStep < 5.0 && (iHigh + p.zeroIndex < p.length - p.coeffLength - 1))
					iHigh++;

					if (!((iHigh - iBase)*p.voxelStep < 5.0))
						iHigh = iBase + 1;
					// Faltung dritte Ableitung +/- 10 Voxel um x
					int searchRangeRoot = int(10.0 / p.voxelStep);
					int coeffDistance = int(p.coeffLength / p.voxelStep);

					if (p.zeroIndex + iBase - searchRangeRoot <= coeffDistance)
						searchRangeRoot = p.zeroIndex + iBase - coeffDistance;

					if (p.zeroIndex + iBase + searchRangeRoot >= p.length - coeffDistance)
						searchRangeRoot = searchRangeRoot - (p.zeroIndex + iBase + coeffDistance);

					int foldZeroIndex = FoldThirdDerOpt(p , profile_16U, tempConvProfile, -iBase + searchRangeRoot, iBase + searchRangeRoot); //p.FoldThirdDerOpt(iProfil, p.tempConvProfile, -iBase + searchRangeRoot, iBase + searchRangeRoot);

					// Suche nach Nullstelle in dritter Ableitung Luftseite
					iHigh += foldZeroIndex;
					iLow += foldZeroIndex;
					iBase += foldZeroIndex;
					float x_vw = 0.0, x_rw = 0.0;						// Treffer der Vor- und Rückwärtssuche
					bool hit_vw = false, hit_rw = false;				// Indikatoren für Treffer mit Schwellwert
					// Loop mit gleichteitiger Vor- und Rückwärtssuceh
					while (1)
					{
						// Test Suchbereich und Vorzeichenwechsel 2.Abl.
						if ((iHigh - iBase) * p.voxelStep <= searchRangeRoot * p.voxelStep && sign * tempConvProfile[iHigh + 1] < 0 && sign * tempConvProfile[iHigh]>0)
						{
							// Interpolation Treffer vorwärts
							x_vw = ( iHigh + tempConvProfile[iHigh] / ( tempConvProfile[iHigh] - tempConvProfile[iHigh + 1] ) - foldZeroIndex)*p.voxelStep;
							int iTest = (int)floor(x_vw / p.voxelStep + 0.5);
							float t = sign * CannyOpt(iTest + p.zeroIndex, p, profile_16U); //p.CannyOpt(/*iHigh - foldZeroIndex*/iTest + p.zeroIndex, iProfil);
							
							
							
							//if (t > 0.05*sign*xCanny && t<0.85*sign*xCanny && sign*p.SecDerOpt(/*iHigh - foldZeroIndex*/iTest + p.zeroIndex, iProfil)>0.15*sign*xCanny / p.dynSigma) 
							if (t > 0.05*sign*xCanny && t<0.85*sign*xCanny && sign * SecDerOpt(iTest + p.zeroIndex, p , profile_16U) > 0.15*sign*xCanny / dynSigma)
								hit_vw = true;
						}
						// Test Suchbereich und Vorzeichenwechsel 2.Abl.
						if ((iBase - iLow)*p.voxelStep <= searchRangeRoot * p.voxelStep && sign * tempConvProfile[iLow] > 0 && sign * tempConvProfile[iLow - 1] < 0)
						{
							// Interpolation Treffer rückwärts
							x_rw = (iLow - tempConvProfile[iLow] / ( tempConvProfile[iLow] - tempConvProfile[iLow - 1]) - foldZeroIndex)*p.voxelStep;
							int iTest = (int)floor(x_rw / p.voxelStep + 0.5);
							float t = sign * CannyOpt(iTest + p.zeroIndex, p, profile_16U); //p.CannyOpt(/*iLow - foldZeroIndex*/iTest + p.zeroIndex, iProfil);
							
							//if (t > 0.05*sign*xCanny && t < 0.85*sign*xCanny && sign*p.SecDerOpt(/*iLow - foldZeroIndex*/iTest + p.zeroIndex, iProfil) < -0.15*sign*xCanny / p.dynSigma) 
							if (t > 0.05*sign*xCanny && t < 0.85*sign*xCanny && sign * SecDerOpt(/*iLow - foldZeroIndex*/iTest + p.zeroIndex, p , profile_16U) < -0.15*sign*xCanny / dynSigma)
							hit_rw = true;
						}
						if (hit_vw && hit_rw)
							break;				// beide Grenzen gefunden
						if ((iBase - iLow)*p.voxelStep >= searchRangeRoot * p.voxelStep || (iHigh - iBase)*p.voxelStep >= searchRangeRoot * p.voxelStep)
							break;				// Suchbereich abgegrast
						iHigh++; iLow--;
					}
					if (hit_vw && hit_rw)
					{
						if (sign == -1)
						{
							if (gaussLow) *gaussLow = Gauss( x_vw, p, profile_16U, filterBuffer, tempVector , dynSigma , shoch2 ); //p.Gauss(x_vw, iProfil);
							if (gaussHigh) *gaussHigh = Gauss( x_rw, p, profile_16U, filterBuffer, tempVector, dynSigma , shoch2 ); ////p.Gauss(x_rw, iProfil);
						}
						else
						{
							if (gaussLow) *gaussLow = Gauss(x_rw, p, profile_16U, filterBuffer, tempVector , dynSigma , shoch2); // //p.Gauss(x_rw, iProfil);
							if (gaussHigh) *gaussHigh = Gauss(x_vw, p, profile_16U, filterBuffer, tempVector , dynSigma , shoch2); // //p.Gauss(x_vw, iProfil);
						}
						return x_vw - x_rw;	// Differenz zwischen Wendepunkten ist gesuchte Kenngröße
					}
					else
					{
						if (gaussLow) *gaussLow = 0;
						if (gaussHigh) *gaussHigh = 0;
						return 0.0;
					}
				}


				__device__ bool SearchAroundZero(CCTProfilsEvaluationSP_Device& p, float& x, int iProfil, float fSearchRange, float fSearchRangeNeg, float staticTest,
					float airPointsThresh, bool dynControl, int sign)
				{
					//std::cout << "range factor : " << p.rangeFactor << std::endl;

					bool result = true;
					//assert(p.threshold > 0.0);
					//assert(p.tempConvLength > 2 * p.coeffLength + p.searchRange + p.searchRangeNeg);
					//assert(p.dynSigma > 0.3);
					p.PutDynSigma(p.sigma);		// für jeden Punkt zurücksetzen !
					// Dyn. p.threshold evtl. rücksetzen
					if (!p.dynThresholdControl)
						p.dynThreshold = p.threshold;

					if (p.dynThreshold > p.threshold)
						p.dynThreshold = p.threshold;



					p.resQuality[iProfil] = -1.0;

					float x_vw = 0.0, x_rw = 0.0;						// Treffer der Vor- und Rückwärtssuche
					// Vorhandenes Resultat verwenden

					if (p.ptValid[iProfil] != true || p.results[iProfil] > 1e6)
					{
						p.ptValid[iProfil] = false;

						//Fold second derivative over entire search area
						p.convProfileZeroIndex = p.FoldSecDerOpt(iProfil, p.tempConvProfile);
						int i_vw = p.convProfileZeroIndex, i_rw = p.convProfileZeroIndex;	//Index of forward and backward search 
						bool hit_vw = false, hit_rw = false;		//Threshold hit indicators	


						//std::cout << "convolution profile : " << p.tempConvProfile[100] << " " << p.tempConvProfile[150] << " " << p.tempConvProfile[250] << std::endl;

						//printf("convolution profile gpu : %f %f %f \n : ", p.tempConvProfile[100], p.tempConvProfile[150], p.tempConvProfile[250]);

						//Loop with equal forward and reverse sweep
						while (1)
						{
							// Test search range and sign change 2.Abl.
							// It is tested until the successor of i_vw, if there is no sign change,
							// then no zero - at the whole coordinates of the opt-folding is exact!
							if (i_vw - p.convProfileZeroIndex < p.searchRange - 1 &&
								sign * p.tempConvProfile[i_vw + 1] > 0 &&
								sign * p.tempConvProfile[i_vw] < 0)
							{
								//Interpolation hits forward
								x_vw = (i_vw + p.tempConvProfile[i_vw] / (p.tempConvProfile[i_vw] - p.tempConvProfile[i_vw + 1]) - p.convProfileZeroIndex) * p.voxelStep;

								//printf(" canny vw : %f ", p.Canny(x_vw, iProfil));

								if (sign * p.Canny(x_vw, iProfil) > sign * p.dynThreshold)	// Schwellwertkriterium
								{
									if (!hit_vw && !hit_rw)
									{
										hit_vw = true;
										x = x_vw;
									}
									else
										p.resQuality[iProfil] = 50.0;
								}
							}



							//Test search range and sign change 2.Abl.
							if (p.convProfileZeroIndex - i_rw < p.searchRangeNeg - 1 && sign * p.tempConvProfile[i_rw] > 0 && sign * p.tempConvProfile[i_rw - 1] < 0)
							{

								//Interpolation hits backwards
								x_rw = (i_rw - p.tempConvProfile[i_rw] / (p.tempConvProfile[i_rw] - p.tempConvProfile[i_rw - 1]) - p.convProfileZeroIndex) * p.voxelStep;

								//printf(" canny : %f ", p.Canny(x_rw, iProfil));

								if (sign * p.Canny(x_rw, iProfil) > sign * p.dynThreshold)	//threshold criterion
									if (!hit_rw && !hit_vw)
									{
										hit_rw = true;
										x = x_rw;
									}
									else if (hit_vw && !hit_rw)
									{
										hit_rw = true;
										x = (x < -x_rw) ? x : x_rw;
									}
									else p.resQuality[iProfil] = 50.0;
							}




							if (!dynControl && (hit_vw || hit_rw))
								break;				//Landed hits

							i_vw++; i_rw--;

							if (i_vw - p.convProfileZeroIndex > p.searchRange && p.convProfileZeroIndex - i_rw > p.searchRangeNeg)
								break;				//Search area browsed
						}

						if (!hit_vw && !hit_rw)
							result = false;

						printf("\n hit found : %f %d %d %d \n", x_vw, hit_vw, hit_rw, result);
						printf("dynamic threshold %f %d %d %d %f \n", p.dynThreshold, sign, p.convProfileZeroIndex, p.searchRangeNeg , p.voxelStep);
					}
					else x = p.results[iProfil];

					if (result && dynControl)
						result = NewtonMax( p , x, iProfil);	// Punkt genau gefunden?? Ergebnis in x!!!

					printf("\n newton max : %f %d %f %f\n", x, result , p.dynSigma , p.shoch2);

					if (result)
						if (-x > fSearchRangeNeg || x > fSearchRange)
							result = false;

					while (result)	// Genaue Bestimmung Nulldurchgang erfolgreich
					{
						bool  dynCorr = false;

						if (dynControl)
							dynCorr = SetDynSigma( p , x, iProfil);

						if (dynCorr)
						{
							result = NewtonMax( p , x, iProfil);

							p.dynThreshold = p.dynSigma / p.sigma*p.threshold; // Auch den Schwellwert anpassen, heuristisch...

							if (!result)
								break;
						}

						p.resCanny[iProfil] = p.Canny(x, iProfil);

						if ((sign*p.resCanny[iProfil] < sign*p.dynThreshold)	// Gradientenschwellwert überschritten?
							|| (x > fSearchRange)
							|| (x < -fSearchRangeNeg))
						{
							result = false;
							break;
						}
						float actGradLength = 0;
						bool   staticChecked = false;
						// Überprüfung mit statischem Schwellwert
						if (dynControl)
						{
							float high, low;
							// Gradientensprunglänge und Endpunkte berechnen
							actGradLength = GradientLength( p , x, iProfil, &low, &high, p.resCanny[iProfil]);

							if (low > 0 && high > 0)
								staticChecked = true;

							if (staticChecked && staticTest > 0)
							{
								if (staticTest > high || staticTest < low)
								{
									result = false;
									break;
								}
							}
						}
						// Wenn die Berechnung der Gradientenlänge nicht funktioniert oder dynControl aus ist (Soll-Ist-Vergleich)
						if (!staticChecked && staticTest > 0)
						{
							float lowValue = p.Gauss(x - 2 * p.sigma, iProfil);
							float highValue = p.Gauss(x + 2 * p.sigma, iProfil);

							if (lowValue > staticTest || highValue < staticTest)
							{
								result = false;
								break;
							}
						}
						// Luftpunkttest
						if (airPointsThresh > 0)
						{
							float grayActual = p.Gauss(x, iProfil);

							if (grayActual < airPointsThresh)
							{
								result = false;
								break;
							}
						}


						// Dynamischen p.threshold auf 75% des Maximums dieses Punkts setzen


						//Set dynamic p.threshold to 75 % of the maximum of this point
						if (p.dynThresholdControl)
							p.dynThreshold = (float)fabs(p.Canny(x, iProfil)) * 3 / 4;

						// Aber nicht größer als vorgeg. Schwellwert
						//But not bigger than vorg. threshold
						if (p.dynThreshold > p.threshold)
							p.dynThreshold = p.threshold;

						p.ptValid[iProfil] = true;

						if (dynControl)
						{
							if (p.resQuality[iProfil] < 0)
								p.resQuality[iProfil] = 0.0;

							if (p.resCanny[iProfil] < 2 * p.threshold)
								p.resQuality[iProfil] += 25 * (2 * p.threshold - p.resCanny[iProfil]) / p.threshold;

							actGradLength = __min(actGradLength, 4.0f * p.dynSigma);

							if (actGradLength > 2 * p.dynSigma)
								p.resQuality[iProfil] += 12 * (actGradLength - 2 * p.dynSigma) / p.dynSigma;
						}

						p.results[iProfil] = x;

						break;
					}

					if (!result)
						p.ptValid[iProfil] = false;


					return result;
				}


				__device__ bool SearchAroundZero( const ProfileEvaluationConstants& p, unsigned short* profile_16U  , float* tempConvProfile, float* filterBuffer , float* tempVector , float& x, int iProfil, float fSearchRange, float fSearchRangeNeg, float staticTest,
					float airPointsThresh, bool dynControl, int sign , bool& ptValid , float& resCanny, float& resQuality , float& result, float& dynSigma, float& shoch2)
				{
					//std::cout << "range factor : " << p.rangeFactor << std::endl;

					bool isValid = true;
					//assert(p.threshold > 0.0);
					//assert(p.tempConvLength > 2 * p.coeffLength + p.searchRange + p.searchRangeNeg);
					//assert(p.dynSigma > 0.3);

					float dynThreshold = p.dynThreshold1;

					//p.PutDynSigma(p.sigma);		// TODO can be done a priori
					PutDynSigma(p, p.sigma, dynSigma, shoch2);
					//
					//							// Dyn. p.threshold evtl. rücksetzen
					if (!p.dynThresholdControl) // TODO can be done a priori
					 dynThreshold = p.threshold;

					if (dynThreshold > p.threshold) // TODO can be done a priori
						dynThreshold = p.threshold;



					resQuality = -1.0; // should be a parameter

					float x_vw = 0.0, x_rw = 0.0;						// Treffer der Vor- und Rückwärtssuche
					// Vorhandenes Resultat verwenden

					if ( ptValid != true || result > 1e6) // should be function parameter
					{
						ptValid = false;

						//Fold second derivative over entire search area
						int convProfileZeroIndex = FoldSecDerOpt( p , profile_16U , tempConvProfile );

						int i_vw = convProfileZeroIndex, i_rw = convProfileZeroIndex;	//Index of forward and backward search 
						bool hit_vw = false, hit_rw = false;		//Threshold hit indicators	


						//std::cout << "convolution profile : " << p.tempConvProfile[100] << " " << p.tempConvProfile[150] << " " << p.tempConvProfile[250] << std::endl;

						//printf("convolution profile gpu : %f %f %f \n : ", p.tempConvProfile[100], p.tempConvProfile[150], p.tempConvProfile[250]);

						//Loop with equal forward and reverse sweep
						while (1)
						{
							// Test search range and sign change 2.Abl.
							// It is tested until the successor of i_vw, if there is no sign change,
							// then no zero - at the whole coordinates of the opt-folding is exact!
							if ( i_vw - convProfileZeroIndex < p.searchRange - 1 &&
								sign * tempConvProfile[i_vw + 1] > 0 &&
								sign * tempConvProfile[i_vw] < 0)
							{
								//Interpolation hits forward
								x_vw = (i_vw + tempConvProfile[i_vw] / ( tempConvProfile[i_vw] - tempConvProfile[i_vw + 1]) - convProfileZeroIndex) * p.voxelStep;

								//printf(" canny vw : %f ", p.Canny(x_vw, iProfil));

								//if (sign * p.Canny(x_vw, iProfil) > sign * p.dynThreshold)	// Schwellwertkriterium
								
								if( sign * Canny( x_vw ,  p , profile_16U , filterBuffer , tempVector , dynSigma , shoch2) )
								{
									if (!hit_vw && !hit_rw)
									{
										hit_vw = true;
										x = x_vw;
									}
									else
										resQuality = 50.0;
								}
							}



							//Test search range and sign change 2.Abl.
							if ( convProfileZeroIndex - i_rw < p.searchRangeNeg - 1 && sign * tempConvProfile[i_rw] > 0 && sign * tempConvProfile[i_rw - 1] < 0)
							{

								//Interpolation hits backwards
								x_rw = (i_rw - tempConvProfile[i_rw] / ( tempConvProfile[i_rw] - tempConvProfile[i_rw - 1]) - convProfileZeroIndex) * p.voxelStep;

								//printf(" canny : %f ", p.Canny(x_rw, iProfil));

								if (sign *  Canny( x_rw, p, profile_16U, filterBuffer, tempVector , dynSigma , shoch2) > sign * dynThreshold)	//threshold criterion
									if (!hit_rw && !hit_vw)
									{
										hit_rw = true;
										x = x_rw;
									}
									else if (hit_vw && !hit_rw)
									{
										hit_rw = true;
										x = (x < -x_rw) ? x : x_rw;
									}
									else 
										resQuality = 50.0;
							}




							if (!dynControl && (hit_vw || hit_rw))
								break;				//Landed hits

							i_vw++; i_rw--;

							if (i_vw - convProfileZeroIndex > p.searchRange && convProfileZeroIndex - i_rw > p.searchRangeNeg)
								break;				//Search area browsed
						}

						if (!hit_vw && !hit_rw)
							isValid = false;

						//printf("\n hit found : %f %d %d %d \n", x_vw, hit_vw, hit_rw, isValid);
						//printf("dynamic threshold %f %d %d %d %f \n", dynThreshold, sign, convProfileZeroIndex, p.searchRangeNeg , p.voxelStep);
					}
					else x = result;

					if (isValid && dynControl)
						isValid = NewtonMax( p , x , profile_16U, filterBuffer , tempVector , dynSigma , shoch2 ); //NewtonMax(p, x, iProfil);	// Punkt genau gefunden?? Ergebnis in x!!!

					if (isValid)
					{
						result = x;
						
					}

					return isValid;

#if 0

					//printf("\n newton max : %f %d %f %f \n", x , isValid , dynSigma , shoch2);

					if (isValid)
						if (-x > fSearchRangeNeg || x > fSearchRange)
							isValid = false;

					while (isValid)	// Genaue Bestimmung Nulldurchgang erfolgreich
					{
						bool  dynCorr = false;

						if (dynControl)
							dynCorr = SetDynSigma(p, x, profile_16U, filterBuffer, tempVector , dynSigma , shoch2); //SetDynSigma(p, x, iProfil);

						if (dynCorr)
						{
							isValid = NewtonMax(p, x, profile_16U, filterBuffer, tempVector , dynSigma , shoch2); //NewtonMax(p, x, iProfil);

							dynThreshold = dynSigma / p.sigma * p.threshold; // Auch den Schwellwert anpassen, heuristisch...

							if (!isValid)
								break;
						}

						resCanny = Canny(x, p, profile_16U, filterBuffer, tempVector , dynSigma , shoch2); //p.Canny(x, iProfil);

						if ((sign * resCanny < sign * dynThreshold)	// Gradientenschwellwert überschritten?
							|| (x > fSearchRange)
							|| (x < -fSearchRangeNeg))
						{
							isValid = false;
							break;
						}
						float actGradLength = 0;
						bool   staticChecked = false;
						// Überprüfung mit statischem Schwellwert
						if (dynControl)
						{
							float high, low;
							// Gradientensprunglänge und Endpunkte berechnen
							actGradLength = GradientLength( p, x, profile_16U, tempConvProfile, filterBuffer, tempVector, ptValid, resCanny, &low, &high, resCanny , dynSigma , shoch2 );  //GradientLength(p, x, iProfil, &low, &high, p.resCanny[iProfil]);

							if (low > 0 && high > 0)
								staticChecked = true;

							if (staticChecked && staticTest > 0)
							{
								if (staticTest > high || staticTest < low)
								{
									isValid = false;
									break;
								}
							}
						}
						// Wenn die Berechnung der Gradientenlänge nicht funktioniert oder dynControl aus ist (Soll-Ist-Vergleich)
						if (!staticChecked && staticTest > 0)
						{
							float lowValue = Gauss(x - 2 * p.sigma, p, profile_16U, filterBuffer, tempVector , dynSigma , shoch2);  //p.Gauss(x - 2 * p.sigma, iProfil);
							float highValue = Gauss(x + 2 * p.sigma, p, profile_16U, filterBuffer, tempVector , dynSigma , shoch2);  //p.Gauss(x + 2 * p.sigma, iProfil);

							if (lowValue > staticTest || highValue < staticTest)
							{
								isValid = false;
								break;
							}
						}
						// Luftpunkttest
						if (airPointsThresh > 0)
						{
							float grayActual = Gauss( x , p, profile_16U, filterBuffer, tempVector , dynSigma , shoch2);  // //p.Gauss(x, iProfil);

							if (grayActual < airPointsThresh)
							{
								isValid = false;
								break;
							}
						}


						// Dynamischen p.threshold auf 75% des Maximums dieses Punkts setzen


						//Set dynamic p.threshold to 75 % of the maximum of this point
						if (p.dynThresholdControl)
							dynThreshold = (float)fabs(Canny(x, p, profile_16U, filterBuffer, tempVector , dynSigma , shoch2)) * 3 / 4; //(float)fabs(p.Canny(x, iProfil)) * 3 / 4;

						// Aber nicht größer als vorgeg. Schwellwert
						//But not bigger than vorg. threshold
						
						if (dynThreshold > p.threshold)
							dynThreshold = p.threshold;

						ptValid = true;

						if (dynControl)
						{
							if ( resQuality < 0)
								 resQuality = 0.0;

							if ( resCanny < 2 * p.threshold)
								 resQuality += 25 * (2 * p.threshold - resCanny ) / p.threshold;

							actGradLength = __min(actGradLength, 4.0f * dynSigma);

							if (actGradLength > 2 * dynSigma)
								resQuality += 12 * (actGradLength - 2 * dynSigma) / dynSigma;
						}

						result = x;

						break;
					}

					if ( !isValid )
						ptValid = false;


					return isValid;

#endif


				}




				__global__ void profileGradientMaxima_Kernel( unsigned short* prof, float* kernelData , 
					int profileSize , int kernelSize , int numProfiles , float fSearchRange , float fSearchRangeNeg  , int wB, float* results , bool* ptValid )
				{

					int y = threadIdx.y + blockIdx.y * blockDim.y;

					int profileId = (y * wB + blockIdx.x) * blockDim.x + threadIdx.x;

					if (profileId >= numProfiles)
						return;

					extern __shared__ float sharedKernelMemory[144];

					extern __shared__ ProfileEvaluationConstants profileEvalParams;

					//printf("%d \n", sizeof(ProfileEvaluationConstants));

					int* profBuff = (int*)&profileEvalParams;

					if (threadIdx.x < 16)
					{
						memcpy(sharedKernelMemory + 2 * threadIdx.x , constGaussKernelData.data + 2 * threadIdx.x , 2 * sizeof(float));
						memcpy(sharedKernelMemory + 48 + 2 * threadIdx.x, constCannyKernelData.data + 2 * threadIdx.x, 2 * sizeof(float));
						memcpy(sharedKernelMemory + 2 * 48 + 2 * threadIdx.x, constSecDerKernelData.data + 2 * threadIdx.x, 2 * sizeof(float));
					}
					else
					{
						memcpy(sharedKernelMemory + 16 + threadIdx.x, constGaussKernelData.data + 16 + threadIdx.x, sizeof(float));
						memcpy(sharedKernelMemory + 48 + 16 + threadIdx.x, constCannyKernelData.data + 16 + threadIdx.x, sizeof(float));
						memcpy(sharedKernelMemory + 2 * 48 + 16 + threadIdx.x, constSecDerKernelData.data + 16 + threadIdx.x, sizeof(float));
					}

					memcpy(profBuff + threadIdx.x * 2, profileEvaluatorData.data + 8 * threadIdx.x, 8);//copy 8 byte per threads

					__syncthreads();


					//printf(" %d \n", profileEvalParams.length);

					//CCTProfilsEvaluationSP_Device profileEvaluation;

					//auto ped = (unsigned char*)&profileEvaluation;

					//int evalSize = sizeof(CCTProfilsEvaluationSP_Device);

					//memcpy( ped , profileEvaluatorData.data , evalSize );

					extern __shared__ unsigned short profileData[];

					unsigned short* currentProfile = profileData + threadIdx.x * profileSize;

					//printf(" profile size :  %d \n", profileSize);

					memcpy( currentProfile , prof + profileId * profileSize, profileSize * sizeof(unsigned short));

					float* tempConvolutionData = (float*)( profileData + blockDim.x * profileSize );

					float *kernelDataShared = (float*)( tempConvolutionData + blockDim.x * profileEvalParams.tempConvLength );

					float* currentConvolutionData = tempConvolutionData + threadIdx.x * profileEvalParams.tempConvLength;

					float resQuality, resCanny , result;
					bool ptValidLocal;

					//profileEvaluation.resQuality = &resQuality;
					//profileEvaluation.resCanny = &resCanny;
					//profileEvaluation.ptValid = &ptValidLocal;
					//profileEvaluation.results = &result;

					//profileEvaluation.ptValid[0] = false;
					//profileEvaluation.results[0] = 0;

					//profileEvaluation.tempConvProfile = currentConvolutionData;
					//profileEvaluation.gaussCoeffs = sharedKernelMemory;
					//profileEvaluation.cannyCoeffs = sharedKernelMemory + 48; 
					//profileEvaluation.secDerCoeffs = sharedKernelMemory + 2 * 48; 

					//profileEvaluation.filterCoeffs = kernelDataShared + threadIdx.x * kernelSize * 2;
					//profileEvaluation.tempVector = profileEvaluation.filterCoeffs + kernelSize;
					//profileEvaluation.profile_16U = currentProfile;

					float xx = 0;

					float* filterCoeffs = kernelDataShared + threadIdx.x * kernelSize * 2;
					float* tempVector = filterCoeffs + kernelSize;

					//ptValid[profileId] = SearchAroundZero( profileEvaluation , xx, 0, fSearchRange, fSearchRangeNeg, -1, -1, true, true);

					float dynSigma = profileEvalParams.dynSigma1, shoch2 = profileEvalParams.shoch21;

					profileEvalParams.gaussCoeffs = sharedKernelMemory;
					profileEvalParams.cannyCoeffs = sharedKernelMemory + 48;
					profileEvalParams.secDerCoeffs = sharedKernelMemory + 2 * 48;


					//if (threadIdx.x == 0)
					//{

					//	ptValid[profileId] = SearchAroundZero(profileEvaluation, xx, 0, fSearchRange, fSearchRangeNeg, -1, -1, true, true);

					//	printf("value of xx1 : %f %d \n", xx, ptValid[profileId]);

						//float xx = 0;

						result = 0;
						ptValidLocal = false;

						ptValid[profileId] = SearchAroundZero(profileEvalParams, currentProfile, currentConvolutionData, filterCoeffs, tempVector, xx, 0, fSearchRange,
							fSearchRangeNeg, -1, -1, true, true, ptValidLocal, resCanny, resQuality, result, dynSigma, shoch2);

						results[profileId] = xx;

						//printf("value of xx2 : %f %d \n", xx, ptValid[profileId]);
					//}

			 
				}



				__global__ void Simple_Kernel()
				{
					printf("simple kernel \n");
				}




				void computeGradientBasedMaximaPoints( void* cpuProfiles , unsigned short* cpuProfileData , float* gaussKernelData , 
					                                   float* cannyKernelData , float *secDerKernelData , int numProfiles, int profileLength , 
					                                   int tempConvLength , int filterKernelSize  , int singleProfileEvaluatorSize , 
					                                   int coeffLength , int searchRangeNeg , int zeroIndex , int searchRange )
				{

					//global memory for storing the profiles
					unsigned short* profileMemoryDevice = 0;


					//printf(" single profile evaluator size : %d \n", singleProfileEvaluatorSize);


					//printf("gaussKernelData : %f %f %f %f %f \n", gaussKernelData[0], gaussKernelData[3], gaussKernelData[7], gaussKernelData[17], gaussKernelData[31]);
					//printf("cannyKernelData : %f %f %f %f %f \n", cannyKernelData[0], cannyKernelData[3], cannyKernelData[7], cannyKernelData[17], cannyKernelData[31]);
					//printf("secDerCoeffs : %f %f %f %f %f \n", secDerKernelData[0], secDerKernelData[3], secDerKernelData[7], secDerKernelData[17], secDerKernelData[31]);

					//printf(" single profile evaluator size  %d ", singleProfileEvaluatorSize);
					HANDLE_ERROR(cudaMemcpyToSymbol( profileEvaluatorData, cpuProfiles, singleProfileEvaluatorSize, 0 , cudaMemcpyHostToDevice ));
					HANDLE_ERROR(cudaMemcpyToSymbol( constGaussKernelData, gaussKernelData, filterKernelSize * sizeof(float), 0, cudaMemcpyHostToDevice));
					HANDLE_ERROR(cudaMemcpyToSymbol( constCannyKernelData, cannyKernelData, filterKernelSize * sizeof(float), 0, cudaMemcpyHostToDevice));
					HANDLE_ERROR(cudaMemcpyToSymbol( constSecDerKernelData, secDerKernelData, filterKernelSize * sizeof(float), 0, cudaMemcpyHostToDevice));


					int shift = zeroIndex - searchRangeNeg - coeffLength;
					int validLen = (2 * coeffLength + searchRange + searchRangeNeg + 1);

					unsigned short* validProfileData = new unsigned short[ validLen * numProfiles ];

					for ( int ii = 0; ii < numProfiles; ii++ )
					{
						memcpy(validProfileData + validLen * ii, cpuProfileData + ii * profileLength + shift , sizeof(unsigned short) * validLen);
					}

					//cudaMalloc((void**)&profileMemoryDevice, numProfiles * profileLength * sizeof(unsigned short));
					cudaMalloc((void**)&profileMemoryDevice, numProfiles * validLen * sizeof(unsigned short));
					//cudaMemcpy(profileMemoryDevice, cpuProfiles, singleProfileSize * numProfiles, cudaMemcpyHostToDevice);
					//cudaMemcpy(profileMemoryDevice, cpuProfileData, numProfiles * profileLength * sizeof(unsigned short), cudaMemcpyHostToDevice);
					cudaMemcpy( profileMemoryDevice, validProfileData, numProfiles * validLen * sizeof(unsigned short), cudaMemcpyHostToDevice);


					int groupSize =  32;

					dim3 threads(groupSize, 1);

					float* resultsGPU;
					bool* ptValidGPU;

					cudaMalloc( (void**)&resultsGPU, numProfiles * sizeof(float));
					cudaMalloc( (void**)&ptValidGPU, numProfiles * sizeof(bool));


					int wB = 1024;//1;//

					int nProfileSets = numProfiles / groupSize;

					int nXBatches = 1;

					if ( nProfileSets > wB)
					{
						nXBatches = nProfileSets % wB == 0 ? nProfileSets / wB : nProfileSets / wB + 1;
					}

					dim3 blocks(wB ,  nXBatches );//nXBatches


					//tempConvLength = std::max(tempConvLength, 474);

					printf("temp convolution length %d : \n", tempConvLength);

					int sharedMemorySize = ( ( validLen * sizeof(unsigned short) + tempConvLength * sizeof(float) + 2 * filterKernelSize * sizeof(float)) * groupSize ) + 48 * 3 + 256 ;

					printf("shared memory size  %d \n ", sharedMemorySize);

					//we need all the shared memory for computation
					
					float* variableKernelData;

					printf("number of blocks %d \n ", wB * nXBatches);


					//Simple_Kernel << <1, 1 >> > ();

					int* profileCountGPU;

					cudaMalloc( (void**)&profileCountGPU, sizeof(int));

					cudaMemset(profileCountGPU, 0, sizeof(int));

					profileGradientMaxima_Kernel <<< blocks, threads, sharedMemorySize >>> ( profileMemoryDevice , variableKernelData, validLen ,
						filterKernelSize, numProfiles, 40, 40, wB, resultsGPU, ptValidGPU);

					cudaError_t error = cudaGetLastError();

					if (error != cudaSuccess)
					{
						printf("cuda kernel failure\n");
					}
					else
					{
						printf("kernel executed successfully \n");
					}

					HANDLE_ERROR(error);

					//HANDLE_ERROR(cudaDeviceSynchronize());

					int profileCountCPU = 0;

					cudaMemcpy(&profileCountCPU, profileCountGPU, sizeof(int), cudaMemcpyDeviceToHost);

					printf("profile count gpu %d , actual number of profiles : %d : ", profileCountCPU, numProfiles);

				}

		}


	}

}