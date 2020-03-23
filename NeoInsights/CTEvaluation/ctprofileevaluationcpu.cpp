#include "ctprofileevaluationcpu.h"
#include "string.h"
#include <algorithm>
#include <iostream>
#include <assert.h>
#include <ippr.h>

namespace imt 
{

	namespace volume 
	{

		namespace cpu
		{


			void ippsConvert_16u32f(unsigned short* pSrc, float* pDst, int len)
			{
				for (int ll = 0; ll < len; ll++)
				{
					pDst[ll] = pSrc[ll];
				}

			}


			void ippsSet_16s(short value, short* arr, int len)
			{
				for (int ll = 0; ll < len; ll++)
				{
					arr[ll] = value;
				}
			}


			void ippsNorm_L2_32f(float* arr, int len, float* norm)
			{
				*norm = 0;

				for (int ll = 0; ll < len; ll++)
				{
					*norm += arr[ll] * arr[ll];
				}

				*norm = std::sqrt(*norm);

			}



			void ippsSqr_32f_I( float* coeffs , int length )
			{
				for (int ii = 0; ii < length; ii++)
				{
					coeffs[ii] = coeffs[ii] * coeffs[ii];
				}
			}


			void ippsDivC_32f_I( float denom , float* arr , int  length )
			{
				float invDenom = 1.0f / denom;

				for (int ii = 0; ii < length; ii++)
				{
					arr[ii] *= invDenom; ///= denom; //can use fast inbuilt division function
				}
			}


			 void ippsExp_32f_I( float* arr , int length )
			{
				for (int ii = 0; ii < length; ii++)
				{
					arr[ii] = std::expf(arr[ii]);
				}

			}


			 void ippsCopy_32f(float *src, float* dst, int len)
			{
				memcpy(dst, src, len * sizeof(float));
			}


			 void ippsMul_32f_I(const float* pSrc, float* pSrcDst, int len)
			{
				for (int ii = 0; ii < len; ii++)
				{
					pSrcDst[ii] = pSrcDst[ii] * pSrc[ii];
				}

			}


			 void ippsAddC_32f_I(float val, float *srcDst, int length)
			{
				for (int ll = 0; ll < length; ll++)
				{
					srcDst[ll] += val;
				}
			}



			 int fillGaussCoeffs(float* gaussCoeffs, float shoch2, int length, float* tempVector)
			{
				ippsSqr_32f_I(gaussCoeffs, length);
				ippsDivC_32f_I(-2.0f * shoch2, gaussCoeffs, length);
				ippsExp_32f_I(gaussCoeffs, length);


				return 0;
			}

			 int fillCannyCoeffs(float* cannyCoeffs, float shoch2, int length, float* t)
			{
				ippsCopy_32f(cannyCoeffs, t, length);
				ippsSqr_32f_I(cannyCoeffs, length);
				ippsDivC_32f_I(-2.0f*shoch2, cannyCoeffs, length);
				ippsExp_32f_I(cannyCoeffs, length);
				ippsDivC_32f_I(-shoch2, cannyCoeffs, length);
				ippsMul_32f_I(t, cannyCoeffs, length);

				return 0;
			}


			 int fillSecDerCoeffs(float* secDerCoeffs, float shoch2, int length, float* t)
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



			 void ippsDotProd_32f(float* src1, float* src2, int len, float* result)
			{
				for (int ll = 0; ll < len; ll++)
				{
					*result += src1[ll] * src2[ll];
				}
			}


			 void ippsSub_32f_I(float* pSrc, float* pSrcDst, int length)
			{
				for (int ll = 0; ll < length; ll++)
				{
					pSrcDst[ll] -= pSrc[ll];
				}
			}



			void ippsConv_32f( const float* pSrc1 , int src1Len , const float* pSrc2 , int src2Len , float* pDst )
			{
				int dstLen = src1Len + src2Len - 1;

				for (int ll = 0; ll < dstLen; ll++)
				{
					float conv = 0;

					int start = std::max(0, ll - src2Len + 1);
					int end = std::min(ll, src1Len - 1);

					for (int kk = start; kk <= end; kk++)
					{
						int p = ll - kk;

						conv += pSrc1[kk] * pSrc2[ll - kk];
					}

					pDst[ll] = conv;
				}

			}

			//pSrcDst[n] = pSrcDst[n] + pSrc[n]*val, 0 ≤ n < len
			 void ippsAddProductC_32f(const float* pSrc, const float val, float* pSrcDst, int len)
			{
				for (int ll = 0; ll < len; ll++)
				{
					pSrcDst[ll] += val * pSrc[ll];
				}
			}


			 void ippsMulC_32f_I( float val , float* pSrcDst, int length)
			 {
				 for ( int ll = 0; ll < length; ll++ )
				 {
					 pSrcDst[ll] *= val;
				 }

			 }

			 CCTProfilsEvaluation::CCTProfilsEvaluation()
			 {
				 voxelStep = 0.25;
				 profile = NULL;
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


			 CCTProfilsEvaluation::CCTProfilsEvaluation(float* data, int n, int datalength, int zeroIdx, float thresh, float srchRange, float sig, float vxStep)
			 {
				 voxelType = Float32;
				 voxelStep = vxStep;
				 profile = data;
				 memoryAllocated = false;
				 length = datalength;
				 nProfils = n;
				 zeroIndex = zeroIdx;
				 searchRange = (int)ceil(srchRange / voxelStep);
				 sigma = sig;
				 threshold = thresh;
				 Init();
			 }
			 void CCTProfilsEvaluation::Init()
			 {
				 assert(sigma > 0.4);
				 dynSigma = sigma;
				 shoch2 = dynSigma * dynSigma;
				 gaussCoeffs = 0;
				 cannyCoeffs = 0;
				 secDerCoeffs = 0;
				 filterCoeffs = 0;
				 tempVector = 0;
				 searchRangeNeg = searchRange;
				 dynThresholdControl = false;
				 dynThreshold = threshold;
				 tempConvProfile = 0;
				 tempConvLength = 0;
				 coeffLength = 0;
				 PreCalc();
				 firstValid = -1;
				 lastValid = -1;
				 results = NULL;
				 resCanny = NULL;
				 resQuality = NULL;
				 ptValid = NULL;
				 nAngle = 0;
				 rangeFactor = 3.5;
				 nValid = 0;

			 }
			 CCTProfilsEvaluation::CCTProfilsEvaluation(unsigned short* data, int n, int datalength, int zeroIdx, float thresh, float srchRange, float sig, float vxStep)
			 {
				 tempConvLength = 0;
				 voxelType = Short;
				 voxelStep = vxStep;
				 assert(thresh > 0 && thresh < 1.0);
				 profile = new float[n*datalength];
				 memoryAllocated = true;
				 length = datalength;
				 //ippsConvert_16u32f(data,profile,n*length);
				 nProfils = n;
				 zeroIndex = zeroIdx;
				 searchRange = (int)ceil(srchRange / voxelStep);
				 sigma = sig;
				 threshold = thresh * 0xffff;
				 Init();
			 }
			 CCTProfilsEvaluation::CCTProfilsEvaluation(unsigned char* data, int n, int datalength, int zeroIdx, float thresh, float srchRange, float sig, float vxStep)
			 {
				 voxelType = Char;
				 voxelStep = vxStep;
				 assert(thresh > 0 && thresh < 1.0);
				 profile = new float[n*datalength];
				 memoryAllocated = true;
				 length = datalength;
				 //ippsConvert_8u32f(data,profile,n*length);
				 nProfils = n;
				 zeroIndex = zeroIdx;
				 searchRange = (int)ceil(srchRange / voxelStep);
				 sigma = sig;
				 threshold = thresh * 0xff;
				 Init();
			 }
			 CCTProfilsEvaluation::~CCTProfilsEvaluation(void)
			 {

				 delete[] gaussCoeffs;
				 delete[] cannyCoeffs;
				 delete[] secDerCoeffs;
				 delete[] filterCoeffs;
				 delete[] tempVector;
				 //ZiTrace("del tempConvProfile Destruktor: %x alte Länge: %d\n",tempConvProfile,tempConvLength);
				 delete[] tempConvProfile;
				 if (memoryAllocated) delete[] profile;

				 delete[] results;
				 delete[] resCanny;
				 delete[] resQuality;
				 delete[] ptValid;
			 }
			 // Negativen Suchbereich abweichend von positivem setzen
			 void CCTProfilsEvaluation::SetSearchRangeNeg(float srNeg)
			 {
				 if (srNeg == 0.0)
				 {
					 searchRangeNeg = searchRange;
				 }
				 else
				 {
					 searchRangeNeg = (int)ceil(srNeg / voxelStep);
				 }

			 }

			 // Suchbereich  setzen
			 void CCTProfilsEvaluation::SetSearchRange(float sr)
			 {
				 searchRange = (int)ceil(sr / voxelStep);

			 }


			 bool CCTProfilsEvaluation::SetSigma(float sig)
			 {
				 coeffLen = int(rangeFactor * sig / voxelStep);
				 sigma = (float)sig;
				 PutDynSigma(sigma);

				 if (nProfils)	// Wenn schon Profile extrahiert sind, testen, ob ihre länge reicht
				 {
					 if (ptValid) memset(ptValid, 0, nProfils * sizeof(bool));
					 if (coeffLen + searchRangeNeg > zeroIndex || coeffLen + searchRange > length - zeroIndex)
						 return false;	// Die extrahierte Profillänge reicht nicht aus!!!
				 }
				 PreCalc();
				 return true;
			 }
			 void CCTProfilsEvaluation::SetThreshold(float thr)
			 {
				 threshold = (float)thr;
				 dynThreshold = threshold;
			 }


			 void CCTProfilsEvaluation::PreCalc()
			 {
				 assert(voxelStep > 0);
				 coeffLength = int(rangeFactor * sigma / voxelStep);

				 delete[] gaussCoeffs;
				 delete[] cannyCoeffs;
				 delete[] secDerCoeffs;
				 delete[] filterCoeffs;
				 delete[] tempVector;
				 gaussCoeffs = new float[2 * coeffLength + 1];
				 cannyCoeffs = new float[2 * coeffLength + 1];
				 secDerCoeffs = new float[2 * coeffLength + 1];
				 filterCoeffs = new float[2 * coeffLength + 1];
				 tempVector = new float[2 * coeffLength + 1];
				 // Genug Platz für ein ganzes Profil vorsehen, doppelte Sigma-Länge 8* statt 4*
				 int newLength = 8 * coeffLength + length + 1;
				 if (newLength > tempConvLength)
				 {
					 //	ZiTrace("del tempConvProfile PreCalc: %x alte Länge: %d\n",tempConvProfile,tempConvLength);
					 delete[] tempConvProfile;
					 tempConvProfile = new float[newLength];
					 tempConvLength = newLength;
					 //	ZiTrace("new tempConvProfile PreCalc: %x neue Länge: %d\n",tempConvProfile,tempConvLength);
				 }
				 gaussCoeffs[0] = (float)((-coeffLength + 0.5)*voxelStep);
				 for (ii = 1; ii < 2 * coeffLength; ii++)
				 {
					 gaussCoeffs[ii] = gaussCoeffs[ii - 1] + (float)voxelStep;
				 }
				 ippsCopy_32f(gaussCoeffs, cannyCoeffs, 2 * coeffLength);	// Parameter kopieren für CannyListe
				 ippsCopy_32f(gaussCoeffs, secDerCoeffs, 2 * coeffLength);	// Parameter kopieren für Liste für 3.Ableitung
				 fillGaussCoeffs(gaussCoeffs, shoch2, 2 * coeffLength, tempVector);		// Gauss-Koeff. berechnen
				 gaussCoeffs[2 * coeffLength] = 0;							// Der Koeff[0] muss nach Subtraktion den Wert für (-coeffLength+0.5)voxelstep enthalten
				 ippsCopy_32f(gaussCoeffs, tempVector + 1, 2 * coeffLength);	// um 1 Index verschoben kopieren
				 tempVector[0] = 0;										// Der Koeff[2coeffLength] muss nach Subtraktion den Wert für (coeffLength-0.5)voxelstep enthalten
				 ippsSub_32f_I(tempVector, gaussCoeffs, 2 * coeffLength + 1);	// Differenz bilden
				 ippsMulC_32f_I(-1.0f, gaussCoeffs, 2 * coeffLength + 1);		// Vorzeichenfehler korrigieren
				 // Canny-Koeffizienten
				 fillCannyCoeffs(cannyCoeffs, shoch2, 2 * coeffLength, tempVector);		// Canny-Koeff. berechnen
				 cannyCoeffs[2 * coeffLength] = 0;
				 ippsCopy_32f(cannyCoeffs, tempVector + 1, 2 * coeffLength);	// um 1 Index verschoben kopieren
				 tempVector[0] = 0;
				 ippsSub_32f_I(tempVector, cannyCoeffs, 2 * coeffLength + 1);	// Differenz bilden
				 ippsMulC_32f_I(-1.0f, cannyCoeffs, 2 * coeffLength + 1);		// Vorzeichenfehler korrigieren
				 // SecDer-Koeffizienten
				 fillSecDerCoeffs(secDerCoeffs, shoch2, 2 * coeffLength, tempVector);		// secDerCoeffs-Koeff. berechnen
				 secDerCoeffs[2 * coeffLength] = 0;
				 ippsCopy_32f(secDerCoeffs, tempVector + 1, 2 * coeffLength);	// um 1 Index verschoben kopieren
				 tempVector[0] = 0;
				 ippsSub_32f_I(tempVector, secDerCoeffs, 2 * coeffLength + 1);	// Differenz bilden
				 ippsMulC_32f_I(-1.0f, secDerCoeffs, 2 * coeffLength + 1);		// Vorzeichenfehler korrigieren

			 }

			 // Gauss-gefilterter Wert
			 float CCTProfilsEvaluation::Gauss(float x, int iProfil)
			 {
				 actFilterLength = int(rangeFactor * dynSigma / voxelStep);

				 assert(actFilterLength <= coeffLength);

				 filterIndex = int(floor(x / voxelStep)) + zeroIndex - actFilterLength;	// Index Beginn Filtermaske

				 if (x / voxelStep - floor(x / voxelStep) > 0.5)
					 filterIndex++;

				 assert(filterIndex >= 0 && filterIndex + 2 * actFilterLength < length);

				 filterCoeffs[0] = (float)((filterIndex - zeroIndex) * voxelStep - x);

				 for (ii = 1; ii < 2 * actFilterLength + 1; ii++)
				 {
					 filterCoeffs[ii] = filterCoeffs[ii - 1] + (float)voxelStep;
				 }

				 fillGaussCoeffs(filterCoeffs, shoch2, 2 * actFilterLength + 1, tempVector);

				 result = 0;

				 ippsDotProd_32f(profile + iProfil * length + filterIndex, filterCoeffs, 2 * actFilterLength + 1, &result);

				 return voxelStep * result / dynSigma / sqrt(2 * M_PI);
			 }
			 // Erste gefilterte Ableitung - Canny
			 float CCTProfilsEvaluation::Canny(float x, int iProfil)
			 {
				 return Derivatives(x, iProfil, &fillGaussCoeffs);
			 }
			 // Zweite gefilterte Ableitung - SecDer
			 float CCTProfilsEvaluation::SecondDer(float x, int iProfil)
			 {
				 return Derivatives(x, iProfil, &fillCannyCoeffs);
			 }
			 // Dritte gefilterte Ableitung - ThirdDer
			 float CCTProfilsEvaluation::ThirdDer(float x, int iProfil)
			 {
				 return -Derivatives(x, iProfil, &fillSecDerCoeffs);
			 }

			 // Basisfunktion für gefilterte Ableitungen des Grauwertprofils

			 //Basic function for filtered derivatives of the gray value profile
			 float CCTProfilsEvaluation::Derivatives(float x, int iProfil, int(*callback)(float*, float, int, float*))
			 {
				 assert(sigma > 0.0);

				 actFilterLength = int(rangeFactor * dynSigma / voxelStep);

				 //std::cout << "act filter length : " << actFilterLength<<" "<<dynSigma << std::endl;

				 assert(actFilterLength <= coeffLength);

				 filterIndex = int(floor(x / voxelStep)) + zeroIndex - actFilterLength;	// Index Beginn Filtermaske

				 assert(filterIndex >= 0 && filterIndex + 2 * actFilterLength + 1 < length);

				 filterCoeffs[0] = (float)((filterIndex - zeroIndex + 0.5)*voxelStep - x);

				 for (ii = 1; ii < 2 * actFilterLength + 1; ii++)
				 {
					 filterCoeffs[ii] = filterCoeffs[ii - 1] + (float)voxelStep;
				 }

				 callback(filterCoeffs, shoch2, 2 * actFilterLength, tempVector);

				 ippsCopy_32f(profile + iProfil * length + filterIndex, tempVector, 2 * actFilterLength + 1);
				 ippsSub_32f_I(profile + iProfil * length + filterIndex + 1, tempVector, 2 * actFilterLength + 1);

				 result = 0;

				 ippsDotProd_32f(tempVector, filterCoeffs, 2 * actFilterLength, &result);

				 return -result;
			 }


			 float CCTProfilsEvaluation::CannyOpt(int i, int iProfil)
			 {
				 assert(i >= coeffLength && i + coeffLength < length);
				 result = 0;
				 ippsDotProd_32f(profile + iProfil * length + i - coeffLength, gaussCoeffs, 2 * coeffLength + 1, &result);
				 return result;
			 }

			 float CCTProfilsEvaluation::SecDerOpt(int i, int iProfil)
			 {
				 assert(i >= coeffLength && i + coeffLength < length);
				 result = 0;
				 ippsDotProd_32f(profile + iProfil * length + i - coeffLength, cannyCoeffs, 2 * coeffLength + 1, &result);
				 return result;
			 }

			 int CCTProfilsEvaluation::FoldCannyOpt(int iProfil, float *cannyProfile)
			 {
				 assert(cannyProfile);
				 assert(zeroIndex - searchRangeNeg >= coeffLength && zeroIndex + searchRange + coeffLength < length);
				 ippsConv_32f(profile + iProfil * length + zeroIndex - searchRangeNeg - coeffLength, 2 * coeffLength + searchRange + searchRangeNeg + 1, gaussCoeffs, 2 * coeffLength + 1, cannyProfile);
				 return searchRangeNeg + 2 * coeffLength; // Das ist der ZeroIndex
			 }
			 int CCTProfilsEvaluation::FoldSecDerOpt(int iProfil, float *secDerProfile)
			 {
				 assert(secDerProfile);
				 assert(zeroIndex - searchRangeNeg >= coeffLength && zeroIndex + searchRange + coeffLength <= length);
				 ippsConv_32f(profile + iProfil * length + zeroIndex - searchRangeNeg - coeffLength, 2 * coeffLength + searchRange + searchRangeNeg + 1, cannyCoeffs, 2 * coeffLength + 1, secDerProfile);
				 return searchRangeNeg + 2 * coeffLength; // Das ist der ZeroIndex
			 }

			 int CCTProfilsEvaluation::FoldThirdDerOpt(int iProfil, float *thirdDerProfile, int convRangeNeg, int convRangePos)
			 {
				 assert(thirdDerProfile);

				 if (!convRangeNeg || zeroIndex - convRangeNeg < coeffLength)
					 convRangeNeg = zeroIndex - coeffLength;

				 if (!convRangePos || zeroIndex + convRangePos + coeffLength >= length)
					 convRangePos = length - coeffLength - zeroIndex - 1;

				 assert(zeroIndex - convRangeNeg >= coeffLength && zeroIndex + convRangePos + coeffLength < length);

				 ippsConv_32f(profile + iProfil * length + zeroIndex - convRangeNeg - coeffLength,
					 2 * coeffLength + convRangePos + convRangeNeg + 1, secDerCoeffs,
					 2 * coeffLength + 1, thirdDerProfile);

				 return convRangeNeg + 2 * coeffLength; // Das ist der ZeroIndex
			 }

			 // Profil mit Strahlaufhärtungskorrektur versehen
			 bool CCTProfilsEvaluation::ProfileCorrection(float x, int iProfil, float* corrPolyMat, float* corrPolyAir)
			 {
				 ret = false;
				 //if(!ptValid[iProfil])
				 //	x = SearchAroundZero(iProfil,float(searchRange/voxelStep),float(searchRangeNeg/voxelStep),false);
				 //else
				 //	x = results[iProfil];
				 assert(tempConvLength > length);

				 ix = (int)ceil(x / voxelStep) + zeroIndex;

				 if (corrPolyMat)
				 {
					 tempConvProfile[0] = float(voxelStep*(ix - zeroIndex) - x);

					 for (ii = 1; ii < length - ix; ii++)
						 tempConvProfile[ii] = float(tempConvProfile[ii - 1] + voxelStep);

					 ippsAddProductC_32f(tempConvProfile, -(float)corrPolyMat[1], profile + iProfil * length + ix, length - ix);

					 ippsSqr_32f_I(tempConvProfile, length - ix);

					 ippsAddProductC_32f(tempConvProfile, -(float)corrPolyMat[2], profile + iProfil * length + ix, length - ix);

					 ret = true;
				 }
				 //Luftseite, Index dekrementieren
				 ix--;

				 if (corrPolyAir)
				 {
					 tempConvProfile[ix] = float(voxelStep*(ix - zeroIndex) - x);

					 for (ii = ix - 1; ii >= 0; ii--)
						 tempConvProfile[ii] = (float)(tempConvProfile[ii + 1] - voxelStep);

					 ippsAddProductC_32f(tempConvProfile, -(float)corrPolyAir[1], profile + iProfil * length, ix + 1);

					 ippsSqr_32f_I(tempConvProfile, ix + 1);

					 ippsAddProductC_32f(tempConvProfile, -(float)corrPolyAir[2], profile + iProfil * length, ix + 1);

					 ret = true;
				 }

				 return ret;
			 }
			 // direct put dyn Sigma
			 void CCTProfilsEvaluation::PutDynSigma(float newValue)
			 {
				 dynSigma = newValue;
				 shoch2 = dynSigma * dynSigma;
			 }




			 CCTProfilsMeasure::CCTProfilsMeasure(CCTProfilsEvaluation &peval) :p(peval)
			 {
				 xMap = new float[27];
				 yMap = new float[27];
				 zMap = new float[27];
				 extract = new float[27];
			 }

			 CCTProfilsMeasure::~CCTProfilsMeasure(void)
			 {
				 delete[] xMap;
				 delete[] yMap;
				 delete[] zMap;
				 delete[] extract;
			 }




			 // Dynamisches p.sigma begrenzen (kleiner als p.sigma und > 0.75)
			 bool CCTProfilsMeasure::SetDynSigma(float x, int iProfil)
			 {
				 //	DPVector::const_iterator i;

				 float curThreshold = -0.1*p.Canny(x, iProfil);
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
				 curThreshold = -0.1*p.Canny(x, iProfil);
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
			 // Newton-Verfahren für Gradienten-Maximum
			 bool CCTProfilsMeasure::NewtonMax(float& x, int iProfil)
			 {
				 bool result = true;
				 float start_x = x;
				 float z;
				 int	it = 0;
				 float lastZ;
				 do
				 {
					 z = p.ThirdDer(x, iProfil);

					 if (z == 0) {
						 result = false;
						 break;
					 }

					 z = p.SecondDer(x, iProfil) / z; // Neue Schrittweite 
					 if (it == 0 && fabs(z) > 1.0)
						 z *= 0.1;
					 if (fabs(z) > 3.0)	// konvergiert offenbar nicht, empirisch gewonnen
					 {
						 result = false;
						 break;
					 }
					 if (it > 0 && std::abs(z + lastZ) < 0.01)
						 z *= 0.5;

					 x = x - z;			// Korrektur anwenden

					 lastZ = z;

					 if (it++ > 25)			// Endlositeration
					 {
						 result = false;
						 break;
					 }
				 } while (fabs(z) > 0.001);  // 0.001 bezieht sich auf Voxelmass und sollte ausreichen

				 if (!result)
					 x = start_x;
				 return result;

			 }

			 /*!
			 * \brief
			 *  SearchAroundZero bestimmt auf dem Grauwertprofil den am nächsten zum Bezugspunkt ("Zero") gelegenen Grauwertsprung, der die
			 * Bedingungen für einen Messpunkt erfüllt.
			 *
			 * \param x
			 * Position des gefundenen Gradientenmaximums im Voxelmass bezogen auf den Nullpunkt des Profils (= Lage des Vorgabepunkts).
			 *
			 * \param iProfil
			 * Profilnummer.
			 *
			 * \param fSearchRange
			 * Suchbereich in positive Richtung oder in beide, wenn fSearchRangeNeg Null ist.
			 *
			 * \param fSearchRangeNeg
			 * Suchbereich in negative Richtung.
			 *
			 * \param staticTest
			 * Dieser Schwellwert muss überschritten werden, damit der Messpunkt gültig ist. Wird für Werte < 0 nicht berücksichtigt.
			 *
			 * \param dynControl
			 * volle Auswertung des Bereichs, um Qualitätskennzahl zu erhalten.
			 *
			 * \param int sign = 1,000000
			 * Such-Richtung.
			 *
			 * \returns
			 * Gültiges Gradientenmaximum gefunden = true
			 *
			 *
			 * \see
			 * SearchGlobally | SearchAroundZeroMixedMat.
			 */
			 bool	CCTProfilsMeasure::SearchAroundZero(float& x, int iProfil, float fSearchRange, float fSearchRangeNeg, float staticTest,
				 float airPointsThresh, bool dynControl, int sign)
			 {


				 //std::cout << "range factor : " << p.rangeFactor << std::endl;

				 bool result = true;
				 assert(p.threshold > 0.0);
				 assert(p.tempConvLength > 2 * p.coeffLength + p.searchRange + p.searchRangeNeg);
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

					 // Zweite Ableitung über gesamten Suchbereich falten
					 //Fold second derivative over entire search area
					 p.convProfileZeroIndex = p.FoldSecDerOpt(iProfil, p.tempConvProfile);
					 int i_vw = p.convProfileZeroIndex, i_rw = p.convProfileZeroIndex;	// Index der Vor- und Rückwärtssuche
					 bool hit_vw = false, hit_rw = false;				// Indikatoren für Treffer mit Schwellwert

					 // Loop mit gleichteitiger Vor- und Rückwärtssuceh
					 while (1)
					 {
						 // Test Suchbereich und Vorzeichenwechsel 2.Abl.
						 // Es wird bis zum Nachfolger von i_vw getestet, wenn dort kein Vorzeichenwechsel, 
						 //dann auch keine Nullstelle - an den ganzen Koordinaten ist dei Opt-Faltung exakt!
						 if (i_vw - p.convProfileZeroIndex < p.searchRange - 1 &&
							 sign * p.tempConvProfile[i_vw + 1] > 0 &&
							 sign * p.tempConvProfile[i_vw] < 0)
						 {
							 // Interpolation Treffer vorwärts
							 x_vw = (i_vw + p.tempConvProfile[i_vw] / (p.tempConvProfile[i_vw] - p.tempConvProfile[i_vw + 1]) - p.convProfileZeroIndex) * p.voxelStep;

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
						 // Test Suchbereich und Vorzeichenwechsel 2.Abl.
						 if (p.convProfileZeroIndex - i_rw < p.searchRangeNeg - 1 && sign * p.tempConvProfile[i_rw] > 0 && sign * p.tempConvProfile[i_rw - 1] < 0)
						 {
							 // Interpolation Treffer rückwärts
							 x_rw = (i_rw - p.tempConvProfile[i_rw] / (p.tempConvProfile[i_rw] - p.tempConvProfile[i_rw - 1]) - p.convProfileZeroIndex) * p.voxelStep;

							 if (sign*p.Canny(x_rw, iProfil) > sign*p.dynThreshold)	// Schwellwertkriterium
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
							 break;				// Treffer gelandet

						 i_vw++; i_rw--;

						 if (i_vw - p.convProfileZeroIndex > p.searchRange && p.convProfileZeroIndex - i_rw > p.searchRangeNeg)
							 break;				// Suchbereich abgegrast
					 }

					 if (!hit_vw && !hit_rw)
						 result = false;
				 }
				 else x = p.results[iProfil];

				 if (result && dynControl)
					 result = NewtonMax(x, iProfil);	// Punkt genau gefunden?? Ergebnis in x!!!

				 if (result)
					 if (-x > fSearchRangeNeg || x > fSearchRange)
						 result = false;

				 while (result)	// Genaue Bestimmung Nulldurchgang erfolgreich
				 {
					 bool  dynCorr = false;

					 if (dynControl)
						 dynCorr = SetDynSigma(x, iProfil);

					 if (dynCorr)
					 {
						 result = NewtonMax(x, iProfil);

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
						 actGradLength = GradientLength(x, iProfil, &low, &high, p.resCanny[iProfil]);

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

						 actGradLength = std::min(actGradLength, 4.0f * p.dynSigma);

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




			 bool	CCTProfilsMeasure::SearchAroundZero2(float& x, int iProfil, float fSearchRange, float fSearchRangeNeg, float staticTest,
				 float airPointsThresh, bool dynControl, int sign)
			 {
				 bool result = true;
				 assert(p.threshold > 0.0);
				 assert(p.tempConvLength > 2 * p.coeffLength + p.searchRange + p.searchRangeNeg);
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

					 // Zweite Ableitung über gesamten Suchbereich falten
					 //Fold second derivative over entire search area
					 p.convProfileZeroIndex = p.FoldSecDerOpt(iProfil, p.tempConvProfile);
					 int i_vw = p.convProfileZeroIndex, i_rw = p.convProfileZeroIndex;	// Index der Vor- und Rückwärtssuche
					 bool hit_vw = false, hit_rw = false;				// Indikatoren für Treffer mit Schwellwert

					 // Loop mit gleichteitiger Vor- und Rückwärtssuceh
					 while (1)
					 {
						 // Test Suchbereich und Vorzeichenwechsel 2.Abl.
						 // Es wird bis zum Nachfolger von i_vw getestet, wenn dort kein Vorzeichenwechsel, 
						 //dann auch keine Nullstelle - an den ganzen Koordinaten ist dei Opt-Faltung exakt!
						 if (i_vw - p.convProfileZeroIndex < p.searchRange - 1 &&
							 sign * p.tempConvProfile[i_vw + 1] > 0 &&
							 sign * p.tempConvProfile[i_vw] < 0)
						 {
							 // Interpolation Treffer vorwärts
							 x_vw = (i_vw + p.tempConvProfile[i_vw] / (p.tempConvProfile[i_vw] - p.tempConvProfile[i_vw + 1]) - p.convProfileZeroIndex) * p.voxelStep;

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
						 // Test Suchbereich und Vorzeichenwechsel 2.Abl.
						 if (p.convProfileZeroIndex - i_rw < p.searchRangeNeg - 1 && sign * p.tempConvProfile[i_rw] > 0 && sign * p.tempConvProfile[i_rw - 1] < 0)
						 {
							 // Interpolation Treffer rückwärts
							 x_rw = (i_rw - p.tempConvProfile[i_rw] / (p.tempConvProfile[i_rw] - p.tempConvProfile[i_rw - 1]) - p.convProfileZeroIndex) * p.voxelStep;

							 if (sign*p.Canny(x_rw, iProfil) > sign*p.dynThreshold)	// Schwellwertkriterium
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
							 break;				// Treffer gelandet

						 i_vw++; i_rw--;

						 if (i_vw - p.convProfileZeroIndex > p.searchRange && p.convProfileZeroIndex - i_rw > p.searchRangeNeg)
							 break;				// Suchbereich abgegrast
					 }

					 if (!hit_vw && !hit_rw)
						 result = false;
				 }
				 else x = p.results[iProfil];

				 if (result && dynControl)
					 result = NewtonMax(x, iProfil);	//Exactly found ?? Result in x  // Punkt genau gefunden?? Ergebnis in x!!!

				 if (result)
					 if (-x > fSearchRangeNeg || x > fSearchRange)
						 result = false;

				 while (result)	// Genaue Bestimmung Nulldurchgang erfolgreich
				 {
					 bool  dynCorr = false;

					 if (dynControl)
						 dynCorr = SetDynSigma(x, iProfil);

					 if (dynCorr)
					 {
						 result = NewtonMax(x, iProfil);

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
						 actGradLength = GradientLength(x, iProfil, &low, &high, p.resCanny[iProfil]);

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
					 if (p.dynThresholdControl) p.dynThreshold = (float)fabs(p.Canny(x, iProfil)) * 3 / 4;
					 // Aber nicht größer als vorgeg. Schwellwert
					 if (p.dynThreshold > p.threshold) p.dynThreshold = p.threshold;
					 p.ptValid[iProfil] = true;
					 if (dynControl)
					 {
						 if (p.resQuality[iProfil] < 0)
							 p.resQuality[iProfil] = 0.0;

						 if (p.resCanny[iProfil] < 2 * p.threshold)
							 p.resQuality[iProfil] += 25 * (2 * p.threshold - p.resCanny[iProfil]) / p.threshold;

						 actGradLength = std::min(actGradLength, 4.0f * p.dynSigma);

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






			 // p.length of gradient Step in Voxel
			 // zur Abschätzung des optimalen p.sigma wird der Abstand zwischen den zwei Wendepunkten der Gradientenfunktion ermittelt
			 // Diese wird dann in CCTProfilsMeasure::AutoSigma statistisch ausgewertet

			 //to estimate the optimal p.sigma, the distance between the two inflection points of the gradient function is determined
			 // This is then evaluated statistically in CCTProfilsMeasure :: AutoSigma
			 float CCTProfilsMeasure::GradientLength(float x, int iProfil, float* gaussLow, float* gaussHigh, float xCanny)
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

			 // calculates real gradient direction by Sobel filter 
			 float CCTProfilsMeasure::GradientDirection(void** voxels, int voxelDim[3], CCTProfilsEvaluation::vxType voxelType, int interpolationMethod, f3f point, f3f sobel)
			 {
				 float normal[3];
				 float sobelx[27] = { -1, 0, 1,-3, 0, 3,-1, 0, 1,
					 -3, 0, 3,-6, 0, 6,-3, 0, 3,
					 -1, 0, 1,-3, 0, 3,-1, 0, 1 };
				 float sobely[27] = { -1, -3, -1,0, 0, 0,1, 3, 1,
					 -3, -6, -3, 0, 0, 0,3, 6, 3,
					 -1, -3, -1,0, 0, 0, 1, 3, 1 };
				 float sobelz[27] = { -1, -3, -1,-3, -6, -3,-1, -3, -1,
					 0, 0, 0, 0, 0, 0, 0, 0, 0,
					 1, 3, 1,3, 6, 3, 1, 3, 1 };
				 // Koordinatentabellen für die 27 Gitterpunkte erstellen
				 for (int i = 0; i < 3; i++)
				 {
					 for (int j = 0; j < 3; j++)
					 {
						 for (int k = 0; k < 3; k++)
						 {
							 xMap[9 * k + 3 * j + i] = float(point[0]);
							 if (float(point[0]) + i - 1 >= 0 && float(point[0]) + i - 1 < voxelDim[0])
								 xMap[9 * k + 3 * j + i] += i - 1;
							 yMap[9 * k + 3 * j + i] = float(point[1]) + j - 1;
							 if (float(point[1]) + j - 1 >= 0 && float(point[1]) + j - 1 < voxelDim[1])
								 yMap[9 * k + 3 * j + i] += j - 1;
							 if (voxelDim[2] < 3)
								 zMap[9 * k + 3 * j + i] = 0.0;
							 else
							 {
								 zMap[9 * k + 3 * j + i] = float(point[2]);
								 if (float(point[2]) + k - 1 >= 0 && float(point[2]) + k - 1 < voxelDim[2])
									 zMap[9 * k + 3 * j + i] += k - 1;
							 }
						 }
					 }
				 }
				 IpprVolume srcVolume = { voxelDim[0],voxelDim[1],voxelDim[2] };
				 IpprCuboid srcVoi = { 0,0,0,voxelDim[0],voxelDim[1],voxelDim[2] };
				 IpprVolume dstVolume = { 27,1,1 };	// Ein "Bild" mit 1 Zeile mit 27 Einzelpunkten


				 // Extraktion der 27 Werte
				 IppStatus sts;
				 switch (voxelType)
				 {
				 case CCTProfilsEvaluation::Char:
				 {
					 //unsigned char* temp;
					 //temp = new unsigned char[27];
					 //ippsSet_8u(0,temp,27);
					 //sts = ipprRemap_8u_C1PV((Ipp8u**)voxels,srcVolume,sizeof(Ipp8u)*voxelDim[0],srcVoi,
					 //	&xMap,&yMap,&zMap,sizeof(Ipp32f)*27,
					 //	&temp,sizeof(Ipp8u)*27,dstVolume,interpolationMethod);
					 //ippsConvert_8u32f(temp,extract,27);
					 //delete[] temp;
					 break;
				 }
				 case CCTProfilsEvaluation::Short:
				 {
					 unsigned short *temp;
					 temp = new unsigned short[27];
					 ippsSet_16s(0, (short*)temp, 27);
					 sts = ipprRemap_16u_C1PV((Ipp16u**)voxels, srcVolume, sizeof(Ipp16u)*voxelDim[0], srcVoi,
						 &xMap, &yMap, &zMap, sizeof(Ipp32f) * 27,
						 &temp, sizeof(Ipp16u) * 27, dstVolume, interpolationMethod);
					 ippsConvert_16u32f(temp, extract, 27);
					 delete[] temp;
					 break;
				 }
				 case CCTProfilsEvaluation::Float32:
					 //ippsSet_32f(0.0f,extract,27);
					 //sts = ipprRemap_32f_C1PV((Ipp32f**)voxels,srcVolume,sizeof(Ipp32f)*voxelDim[0],srcVoi,
					 //	&xMap,&yMap,&zMap,sizeof(Ipp32f)*27,
					 //	&extract,sizeof(Ipp32f)*27,dstVolume,interpolationMethod);
					 break;
				 }

				 if (sts != ippStsNoErr)
				 {
					 // Keine Normale, da keine Daten!
					 sobel[0] = 0;
					 sobel[1] = 0;
					 sobel[2] = 0;
					 return 0;
				 }
				 // Faltung mit den drei Richtungsoperatoren
				 ippsDotProd_32f(sobelx, extract, 27, normal);
				 ippsDotProd_32f(sobely, extract, 27, normal + 1);
				 ippsDotProd_32f(sobelz, extract, 27, normal + 2);

				 // Normierung der Richtung Ausgabe
				 float norm;
				 ippsNorm_L2_32f(normal, 3, &norm);
				 sobel[0] = normal[0] / norm;
				 sobel[1] = normal[1] / norm;
				 sobel[2] = normal[2] / norm;
				 // Achtung, die Höhe des Gradientenbetrags ist mit Faktor 27 (???) versehen.
				 return norm;
			 }




		}




	}



}