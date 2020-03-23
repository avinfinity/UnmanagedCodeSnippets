#include "CTPointCloudEvaluationCUDA.h"

#include "CTPointCloudEvaluation.h"
#include "ctprofilsevaluationsp.h"
#include "ctpointcloudevaluationsp.h"
#include "opencvincludes.h"
#include "display3droutines.h"
#include "ctprofileevaluationcpu.h"

#include "ctprofileevaluation.cuh"

#include "ipp.h"

namespace imt 
{

	namespace volume 
	{

		namespace cuda 
		{

			int Measure(bool wQualFactor, ICTEvaluationProgress* progress, CCTPointCloudEvaluation& evaluation);

			int Measure(bool wQualFactor, ICTEvaluationProgressSP* progress, CCTPointCloudEvaluationSP& evaluation);

			int MeasureCUDA(bool wQualFactor, ICTEvaluationProgressSP* progress, CCTPointCloudEvaluationSP& evaluation);

			void MeasureWithSingleProfile(bool wQualFactor, CCTPointCloudEvaluationSP& evaluation);

			CTPointCloudEvaluationCUDA::CTPointCloudEvaluationCUDA( imt::volume::VolumeInfo& volume, 
				std::vector<Eigen::Vector3f>& points, std::vector<Eigen::Vector3f>& normals, 
				std::vector<unsigned int>& surfaceIndices) : mVolumeInfo( volume ) , mPoints( points ), mNormals( normals ) , mSurfaceIndices(surfaceIndices)
			{
				int nPoints = mNormals.size();

				for (int pp = 0; pp < nPoints; pp++)
				{
					mNormals[pp] *= -1;
				}
			}


			void CTPointCloudEvaluationCUDA::compute()
			{


#if 1
				float initT = cv::getTickCount();

				int size[3] = { mVolumeInfo.mWidth , mVolumeInfo.mHeight , mVolumeInfo.mDepth };

				float sigma = 1.5;
				float gradientThreshold = 0.05;

				void** data = new void*[ mVolumeInfo.mDepth ];

				for (int i = 0; i < mVolumeInfo.mDepth; ++i)
				{
					data[i] = (((unsigned short*)mVolumeInfo.mVolumeData) + ((int64_t)i) * mVolumeInfo.mWidth * mVolumeInfo.mHeight);
				}

				CCTPointCloudEvaluationSP evaluation( data , CCTProfilsEvaluationSP::Short , size );
				evaluation.SetSigma(sigma);
				evaluation.SetThreshold(gradientThreshold);
				
				int count = mPoints.size();

				std::vector<d3d> posArr(count) , searchDirArr(count);

				for (int cc = 0; cc < count; cc++)
				{
					posArr[cc][0] = mPoints[cc](0) / mVolumeInfo.mVoxelStep(0);
					posArr[cc][1] = mPoints[cc](1) / mVolumeInfo.mVoxelStep(0);
					posArr[cc][2] = mPoints[cc](2) / mVolumeInfo.mVoxelStep(0);
				
					searchDirArr[cc][0] = mNormals[cc](0);
					searchDirArr[cc][1] = mNormals[cc](1);
					searchDirArr[cc][2] = mNormals[cc](2);
				
				}

				float searchRangeVoxel = 10  , searchRangeAirSideVoxel = 10 ;
				int interpolationMethod = IPPI_INTER_LINEAR;

				d3d* positions = posArr.data();
				d3d* searchDirections = searchDirArr.data();

				evaluation.extractProfiles( count , (d3d*)positions, (d3d*)searchDirections, searchRangeVoxel, searchRangeAirSideVoxel, interpolationMethod );


				std::cout << " time spent in profile extraction : " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;
				 
				bool useAngleCriterium = false;// true;

				float angleCriterium = M_PI * 0.25 ;

				if ( useAngleCriterium )
				{
					evaluation.SetAngleCriterium(180.0 * angleCriterium / M_PI);
				}


				//CCTPointCloudEvaluation pointCloudEvaluation;

				std::cout << evaluation.detectedMaterials << " " << evaluation.nMaterial << std::endl;


				bool useAutoParams = true;
				bool useBeamHardeningCorrection = true;

				if (useAutoParams || useBeamHardeningCorrection)
				{
					bool useAutoGrad = useAutoParams;
					
					// Bei sehr vielen Messpunkten Basis der Statistik etwas ausdünnen (Rechenzeit verringern!)
					int step = (count > 5000) ? count / 5000 : 1;
					
					if (evaluation.detectedMaterials > evaluation.nMaterial)
					{
						evaluation.SetThreshold(evaluation.globalGradThreshold / evaluation.upperLimit);
						
						useAutoGrad = false;
					}

					bool success = evaluation.AutoParam(step, useBeamHardeningCorrection, useBeamHardeningCorrection, useAutoParams, useAutoGrad);
					
					if (success)
						std::cout << "AutoParam successful!" << std::endl;
					else
						std::cout << "AutoParam failed!" << std::endl;

					sigma = evaluation.sigma;
					
					gradientThreshold = evaluation.relThreshold;					
				}

				bool usingQuality = true;// count <=  //qualityThreshold;


				initT = cv::getTickCount();

				Measure(usingQuality, 0, evaluation);

				//MeasureCUDA(usingQuality, 0, evaluation);

				//MeasureWithSingleProfile(usingQuality, evaluation);


				for (int i = 0; i < count; ++i)
				{
					d3d resultPoint, normal;
					float quality;

					if ( evaluation.getResult(i, resultPoint, normal, &quality) )					
					{
						mPoints[i](0) = resultPoint[0] * mVolumeInfo.mVoxelStep(0);
						mPoints[i](1) = resultPoint[1] * mVolumeInfo.mVoxelStep(1);
						mPoints[i](2) = resultPoint[2] * mVolumeInfo.mVoxelStep(2);
					}
				}

				std::cout << "time spent in measuring " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;

				tr::Display3DRoutines::displayMesh(mPoints, mSurfaceIndices);

#else


				std::vector<cpu::CCTProfilsEvaluation> profiles;

				int interpolationMethod = IPPI_INTER_LINEAR;

				double initT = cv::getTickCount();
				extractProfiles(profiles);

				std::cout << "time spent in extracting profiles : " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;

				float fSearchRange = 40;
				float fSearchRangeNeg = 40;

				int nProfileBlocks = profiles.size();

				int voxelDim[3] = { mVolumeInfo.mWidth , mVolumeInfo.mHeight , mVolumeInfo.mDepth };

				void** voxels = new void*[mVolumeInfo.mDepth];

				unsigned short* vData = (unsigned short*)mVolumeInfo.mVolumeData;

				int64_t zStep = mVolumeInfo.mWidth * mVolumeInfo.mHeight;

				for (int dd = 0; dd < mVolumeInfo.mDepth; dd++)
				{
					voxels[dd] = vData + zStep * dd;
				}

#pragma omp parallel default(shared) 
				// Ab hier laufen auf jedem Core ein Thread
				{
#pragma omp for
					for (int iSlice = 0; iSlice < nProfileBlocks; iSlice++)
					{
						    auto& profile = profiles[iSlice];
							cpu::CCTProfilsMeasure	pM(profiles[iSlice]);
							// Zähler zurücksetzen
							profile.nAngle = 0;
							profile.nValid = 0;
							// Schleife über alle Profile
							for (int i = 0; i < profile.nProfils; i++)
							{
								bool success;

								//if (nMaterial == 1)
								//{
									//if (profileCorrAir || profileCorrMat)
									//{
									//	float xx;

									//	if (profile.ptValid[i])
									//		xx = profile.results[i];
									//	else
									//		success = pM.SearchAroundZero(xx, i, float(fSearchRange), float(fSearchRangeNeg), -1, -1, false);

									//	if (profile.ptValid[i])
									//	{
									//		profile.ProfileCorrection(xx, i, profileCorrMat ? corrPolyMat : 0, profileCorrAir ? corrPolyAir : 0);

									//		if (!profileIsCorrected)
									//			profileIsCorrected = true;
									//	}
									//}

									success = pM.SearchAroundZero( profile.results[i], i, fSearchRange, fSearchRangeNeg, -1 , -1, true);
								
								
								if (!success)
								{
									profile.ptValid[i] = false;
									profile.results[i] = 1e7;
									continue;
								}

								profile.ptValid[i] = true;
								memcpy(resPoints[iSlice][i], cloudPoints[iSlice][i], sizeof(d3d));

								resPoints[iSlice][i][0] += profile.results[i] * cloudNormals[iSlice][i][0];
								resPoints[iSlice][i][1] += profile.results[i] * cloudNormals[iSlice][i][1];
								resPoints[iSlice][i][2] += profile.results[i] * cloudNormals[iSlice][i][2];

								pM.GradientDirection(voxels, voxelDim, cpu::CCTProfilsEvaluation::Short, interpolationMethod, resPoints[iSlice][i], sobelNormals[iSlice][i]);

								//if (angleCriterium)
								//{
								//	if (sobelNormals[iSlice][i][0] * cloudNormals[iSlice][i][0] +
								//		sobelNormals[iSlice][i][1] * cloudNormals[iSlice][i][1] +
								//		sobelNormals[iSlice][i][2] * cloudNormals[iSlice][i][2] < cosTest)
								//	{
								//		profile.ptValid[i] = false;
								//		profile.nAngle++;
								//	}
								//}

								if (profile.ptValid[i])
									profile.nValid++;
							}


						//}
					}
				}


				int nAngle = 0;
				int nValid = 0;
				int totalNumPoints = 0;

				for (int iSlice = 0; iSlice < nProfileBlocks; iSlice++)
				{
					nAngle += profiles[iSlice].nAngle;
					nValid += profiles[iSlice].nValid;

					totalNumPoints += profiles[iSlice].nProfils;
				}


				std::cout << "total and valid number of points : " << totalNumPoints << " " << nValid << std::endl;


				std::cout << "time spent in total processing : " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;

#endif
				
			}


			void CTPointCloudEvaluationCUDA::extractProfiles( std::vector<cpu::CCTProfilsEvaluation>& profiles , unsigned short*& profileGrayValues )
			{
				int nPoints = mPoints.size();

				int nPointsPerBlock = 16;

				int nProfiles = nPoints % nPointsPerBlock == 0 ? nPoints / nPointsPerBlock : nPoints / nPointsPerBlock + 1;

				float voxelStep = 0.25;

				profiles.resize(nProfiles);

				int nSlices = nProfiles;

				float fSearchRange = 10.0f , fSearchRangeNeg = 10.0f;

				float sigmaMax, sigma;

				//threshold = 0.25;			// Gradientenschwellwert Standard
				sigma = 1.5;
				sigmaMax = 8.0;

				int voxelDim[3] = { mVolumeInfo.mWidth , mVolumeInfo.mHeight , mVolumeInfo.mDepth };

				int interpolationMethod = IPPI_INTER_LINEAR;

				cloudPoints.resize(nSlices);
				cloudNormals.resize(nSlices);
				resPoints.resize(nSlices);
				sobelNormals.resize(nSlices);


				//profileGrayValues = new un

				void** voxels = new void*[mVolumeInfo.mDepth];

				unsigned short* vData = (unsigned short*)mVolumeInfo.mVolumeData;

				int64_t zStep = mVolumeInfo.mWidth * mVolumeInfo.mHeight;

				for ( int dd = 0; dd < mVolumeInfo.mDepth; dd++ )
				{
					voxels[dd] = vData + zStep * dd;
				}

				int isoThreshold = mVolumeInfo.mAirVoxelFilterVal;

				int zeroIndex = int( profiles[0].rangeFactor * sigmaMax / profiles[0].voxelStep) + profiles[0].searchRangeNeg;
				int length = zeroIndex + int( profiles[0].rangeFactor * sigmaMax / profiles[0].voxelStep ) + profiles[0].searchRange;
				//profiles[0].SetSigma(sigma);

				profileGrayValues = new unsigned short[(int64_t)length * (int64_t)nPoints];

#pragma omp parallel default(shared) 
				// Ab hier laufen auf jedem Core ein Thread
				{
#pragma omp for
					for (int iSlice = 0; iSlice < nSlices; iSlice++)
					{
						profiles[iSlice].voxelStep = voxelStep;
						profiles[iSlice].searchRange = (int)ceil(fSearchRange / voxelStep);
						profiles[iSlice].searchRangeNeg = (int)ceil(fSearchRangeNeg / voxelStep);

						profiles[iSlice].nProfils = nPointsPerBlock;


						if ( iSlice == nSlices - 1 ) 
						profiles[iSlice].nProfils = nPoints - iSlice * nPointsPerBlock;

						profiles[iSlice].zeroIndex = int(profiles[iSlice].rangeFactor * sigmaMax / profiles[iSlice].voxelStep) + profiles[iSlice].searchRangeNeg;
						profiles[iSlice].length = profiles[iSlice].zeroIndex + int(profiles[iSlice].rangeFactor * sigmaMax / profiles[iSlice].voxelStep) + profiles[iSlice].searchRange;
						profiles[iSlice].SetSigma(sigma);
						
						if ( 4 * profiles[iSlice].coeffLength + profiles[iSlice].length + 
							 profiles[iSlice].searchRange + profiles[iSlice].searchRangeNeg + 1 > profiles[iSlice].tempConvLength)
						{

							delete[] profiles[iSlice].tempConvProfile;
							profiles[iSlice].tempConvLength = 4 * profiles[iSlice].coeffLength + profiles[iSlice].length + profiles[iSlice].searchRange + profiles[iSlice].searchRangeNeg + 1;
							profiles[iSlice].tempConvProfile = new float[profiles[iSlice].tempConvLength];
						}

						float* pxMap;
						float* pyMap;
						float* pzMap;
						float* steps;

						try
						{
							// px,py,pzMaps
							pxMap = new float[profiles[iSlice].nProfils*profiles[iSlice].length];
							pyMap = new float[profiles[iSlice].nProfils*profiles[iSlice].length];
							pzMap = new float[profiles[iSlice].nProfils*profiles[iSlice].length];
							// Schrittweitenvektor
							steps = new float[profiles[iSlice].length];
							// Speicher für Profile allokieren
							profiles[iSlice].profile = new float[profiles[iSlice].nProfils*profiles[iSlice].length];
							profiles[iSlice].memoryAllocated = true;
							
							// Speicher für Ergebnisse allokieren
							//if (!cloudPtAlloc)
							{
								cloudPoints[iSlice] = new cpu::f3f[profiles[iSlice].nProfils];
								cloudNormals[iSlice] = new cpu::f3f[profiles[iSlice].nProfils];
								resPoints[iSlice] = new cpu::f3f[profiles[iSlice].nProfils];
								sobelNormals[iSlice] = new cpu::f3f[profiles[iSlice].nProfils];
							}


							profiles[iSlice].ptValid = new bool[profiles[iSlice].nProfils];
							profiles[iSlice].results = new float[profiles[iSlice].nProfils];
							profiles[iSlice].resCanny = new float[profiles[iSlice].nProfils];
							profiles[iSlice].resQuality = new float[profiles[iSlice].nProfils];
							memset(profiles[iSlice].ptValid, 0, profiles[iSlice].nProfils * sizeof(bool));
						}
						catch (...)
						{
							//_Log->LogDebug( "Error: no memory available");
							throw "Exception memory allocation";
						}
						
						//if (!cloudPtAlloc)
						{
							memcpy(cloudPoints[iSlice], mPoints.data() + iSlice * nPointsPerBlock, profiles[iSlice].nProfils * sizeof(cpu::f3f));
							memcpy(cloudNormals[iSlice], mNormals.data() + iSlice * nPointsPerBlock, profiles[iSlice].nProfils * sizeof(cpu::f3f));
						}


						steps[0] = -(float)profiles[iSlice].voxelStep*profiles[iSlice].zeroIndex;
						for (int i = 1; i < profiles[iSlice].length; i++) steps[i] = steps[i - 1] + (float)profiles[iSlice].voxelStep;
						// Füllen der Maps punktweise
						for (int i = 0; i < profiles[iSlice].nProfils; i++)
						{
							// X-Koordinaten
							ippsSet_32f((float)cloudPoints[iSlice][i][0], pxMap + i * profiles[iSlice].length, profiles[iSlice].length);
							ippsAddProductC_32f(steps, (float)cloudNormals[iSlice][i][0], pxMap + i * profiles[iSlice].length, profiles[iSlice].length);
							// Y-Koordinaten
							ippsSet_32f((float)cloudPoints[iSlice][i][1], pyMap + i * profiles[iSlice].length, profiles[iSlice].length);
							ippsAddProductC_32f(steps, (float)cloudNormals[iSlice][i][1], pyMap + i * profiles[iSlice].length, profiles[iSlice].length);
							// Z-Koordinaten
							if (voxelDim[2] == 1)
								ippsSet_32f(0.0, pzMap + i * profiles[iSlice].length, profiles[iSlice].length);
							else
							{
								ippsSet_32f((float)cloudPoints[iSlice][i][2], pzMap + i * profiles[iSlice].length, profiles[iSlice].length);
								ippsAddProductC_32f(steps, (float)cloudNormals[iSlice][i][2], pzMap + i * profiles[iSlice].length, profiles[iSlice].length);
							}
						}
						delete[] steps;
						IpprVolume srcVolume = { voxelDim[0],voxelDim[1],voxelDim[2] };
						IpprCuboid srcVoi = { 0,0,0,voxelDim[0],voxelDim[1],voxelDim[2] };
						IpprVolume dstVolume = { profiles[iSlice].length,profiles[iSlice].nProfils,1 };	// Ein "Bild" mit nProfils Zeilen == Profilen

						IppStatus sts;
						float	tmin, tmax;
						try
						{
								unsigned short* temp = new unsigned short[profiles[iSlice].nProfils*profiles[iSlice].length];
								
								Ipp16s val = -1;
								
								ippsSet_16s(val, (short*)temp, profiles[iSlice].nProfils*profiles[iSlice].length);
								
								sts = ipprRemap_16u_C1PV((Ipp16u**)voxels, srcVolume, sizeof(Ipp16u)*voxelDim[0], srcVoi,
									&pxMap, &pyMap, &pzMap, sizeof(Ipp32f)*profiles[iSlice].length,
									&temp, sizeof(Ipp16u)*profiles[iSlice].length, dstVolume, interpolationMethod);
								
								ippsConvert_16u32f(temp, profiles[iSlice].profile, profiles[iSlice].nProfils*profiles[iSlice].length);

								memcpy( profileGrayValues + length * iSlice * nPointsPerBlock , temp , profiles[iSlice].nProfils * profiles[iSlice].length * sizeof(unsigned short) );
								
								//if (iSlice == 0)
								//{
								//	lowerLimit = 0;
								//	upperLimit = float(0xffff);
								//	threshold = (float)relThreshold * upperLimit;
								//}

								delete[] temp;
							
						}
						catch (...)
						{
							//_Log->LogDebug( "Error: Box size");
							throw "Error: Box size";
						}

						delete[] pxMap;
						delete[] pyMap;
						delete[] pzMap;
						if (sts != ippStsNoErr)
						{
							//_Log->LogDebug( "Error: VoxelMapping");
							throw  "Error: VoxelMapping";
						}
						// Profilbereiche außerhalb des srcVoi sind mit Initval markiert und  müssen mit dem 
						// letzten Grauwert aufgefüllt werden, damit eine Integration bis zum Rand möglich wird
						float initVal = -99999.9f;

						//if (profiles[iSlice].voxelType != CCTProfilsEvaluation::Float32)
						//	initVal = upperLimit;

						// Alle Profile durchsuchen
						profiles[iSlice].nOutside = 0;
						for (int i = 0; i < profiles[iSlice].nProfils; i++)
						{
							int last = 0;
							if (profiles[iSlice].profile[i*profiles[iSlice].length] == initVal)
							{
								while (profiles[iSlice].profile[i*profiles[iSlice].length + last] == initVal)
								{
									if (last >= profiles[iSlice].length) break;
									last++;
								}
								if (last < profiles[iSlice].length)
									ippsSet_32f(profiles[iSlice].profile[i*profiles[iSlice].length + last], profiles[iSlice].profile + i * profiles[iSlice].length, last);
							}
							if (i == 0)
								profiles[iSlice].firstValid = last;
							if (last >= profiles[iSlice].length) profiles[iSlice].nOutside++;
							last = profiles[iSlice].length - 1;
							if (profiles[iSlice].profile[i*profiles[iSlice].length + last] == initVal)
							{
								while (profiles[iSlice].profile[i*profiles[iSlice].length + last] == initVal)
								{
									if (last <= 0) break;
									last--;
								}
								if (last > 0)
									ippsSet_32f(profiles[iSlice].profile[i*profiles[iSlice].length + last], profiles[iSlice].profile + i * profiles[iSlice].length + last, profiles[iSlice].length - last);
							}
							if (i == 0)
								profiles[iSlice].lastValid = last;
						}
						profiles[iSlice].dynThresholdControl = false;
						profiles[iSlice].SetThreshold(0.15);
					}
				}

			}




			int Measure( bool wQualFactor, ICTEvaluationProgress* progress , CCTPointCloudEvaluation& evaluation)
			{


#if 1
				if (evaluation.resPoints == NULL)
					return 0;   // Keine Profile extrahiert

				clock_t ticks = clock();

				float cosTest = 0;
				if (evaluation.angleCriterium)
				{
					cosTest = cos(evaluation.angleCriterium / 180 * M_PI);
					std::stringstream msg;
					msg << "PointCloud - max. angular difference: " << evaluation.angleCriterium << "°";
					//_Log->LogDebug(msg.str());
				}

				float staticTest = -1;
				if (evaluation.checkStaticThreshold4Measure)
				{
					staticTest = evaluation.staticThreshold;
					std::stringstream msg;
					msg << "PointCloud - Static threshold for measurement: " << staticTest;
					//_Log->LogDebug(msg.str());
				}
				const int step = (evaluation.nSlices < 100) ? 1 : evaluation.nSlices / 100;   //Fortschrittsanzeige in 1% Schritten
				int currentSteps = 0;
				bool abort = false;

#pragma omp parallel default(shared) 
				// Ab hier laufen auf jedem Core ein Thread
				{
#pragma omp for
					for (int iSlice = 0; iSlice < evaluation.nSlices; iSlice++)
					{
						//std::cout << "dyn sigma for slice " << iSlice << " : " << profile.dynSigma << std::endl;

#pragma omp flush(abort)
						if (!abort)
						{
							auto& profile = evaluation.pSlice[iSlice];

							CCTProfilsMeasure	pM(profile);
							// Zähler zurücksetzen
							profile.nAngle = 0;
							profile.nValid = 0;
							// Schleife über alle Profile
							for (int i = 0; i < profile.nProfils; i++)
							{
								bool success = pM.SearchAroundZero( profile.results[i], i, float(evaluation.fSearchRange),
										                            float(evaluation.fSearchRangeNeg), staticTest, 
									                                evaluation.airPointsThreshold, wQualFactor);

								if (!success)
								{
									profile.ptValid[i] = false;
									profile.results[i] = 1e7;
									continue;
								}

								profile.ptValid[i] = true;
								memcpy(evaluation.resPoints[iSlice][i], evaluation.cloudPoints[iSlice][i], sizeof(d3d));

								evaluation.resPoints[iSlice][i][0] += profile.results[i] * evaluation.cloudNormals[iSlice][i][0];
								evaluation.resPoints[iSlice][i][1] += profile.results[i] * evaluation.cloudNormals[iSlice][i][1];
								evaluation.resPoints[iSlice][i][2] += profile.results[i] * evaluation.cloudNormals[iSlice][i][2];

								pM.GradientDirection( evaluation.voxels, evaluation.voxelDim, evaluation.voxelType, evaluation.interpolationMethod, 
									                  evaluation.resPoints[iSlice][i], evaluation.sobelNormals[iSlice][i]);

								if (evaluation.angleCriterium)
								{
									if (evaluation.sobelNormals[iSlice][i][0] * evaluation.cloudNormals[iSlice][i][0] +
										evaluation.sobelNormals[iSlice][i][1] * evaluation.cloudNormals[iSlice][i][1] +
										evaluation.sobelNormals[iSlice][i][2] * evaluation.cloudNormals[iSlice][i][2] < cosTest)
									{
										profile.ptValid[i] = false;
										profile.nAngle++;
									}
								}

								if (profile.ptValid[i])
									profile.nValid++;

							}

							if (progress != nullptr)
							{
								if (progress->IsCanceled())
									abort = true;
								if (iSlice % step == 0)
								{
#pragma omp critical 
									{
										progress->Report(0.01 * (float)currentSteps);
										currentSteps++;
									}
								}
							}
						}
					}
				}

				int nAngle = 0;
				int nValid = 0;
				int totalNumPoints = 0;

				for (int iSlice = 0; iSlice < evaluation.nSlices; iSlice++)
				{
					auto& profile = evaluation.pSlice[iSlice];

					nAngle += profile.nAngle;
					nValid += profile.nValid;

					totalNumPoints += profile.nProfils;
				}

				std::cout << "number of input and valid points : " << nValid << " " << totalNumPoints <<" "<<nAngle<< std::endl;

				return nValid;
#endif

				return 0;
			}

			int Measure(bool wQualFactor, ICTEvaluationProgressSP* progress, CCTPointCloudEvaluationSP& evaluation)
			{


#if 1
				if (evaluation.resPoints == NULL)
					return 0;   // Keine Profile extrahiert

				clock_t ticks = clock();

				float cosTest = 0;
				if (evaluation.angleCriterium)
				{
					cosTest = cos(evaluation.angleCriterium / 180 * M_PI);
					std::stringstream msg;
					msg << "PointCloud - max. angular difference: " << evaluation.angleCriterium << "°";
					//_Log->LogDebug(msg.str());
				}

				float staticTest = -1;
				if (evaluation.checkStaticThreshold4Measure)
				{
					staticTest = evaluation.staticThreshold;
					std::stringstream msg;
					msg << "PointCloud - Static threshold for measurement: " << staticTest;
					//_Log->LogDebug(msg.str());
				}
				const int step = (evaluation.nSlices < 100) ? 1 : evaluation.nSlices / 100;   //Fortschrittsanzeige in 1% Schritten
				int currentSteps = 0;
				bool abort = false;

				int maxConvLen = 21 * 2 + 1;

#pragma omp parallel default(shared) 
				// Ab hier laufen auf jedem Core ein Thread
				{
#pragma omp for
					for (int iSlice = 0; iSlice < evaluation.nSlices; iSlice++)
					{
						//std::cout << "dyn sigma for slice " << iSlice << " : " << profile.dynSigma << std::endl;

#pragma omp flush(abort)
						if (!abort)
						{
							auto& profile = evaluation.pSlice[iSlice];

							imt::volume::CCTProfilsMeasureSP	pM(profile);
							// Zähler zurücksetzen
							profile.nAngle = 0;
							profile.nValid = 0;
							// Schleife über alle Profile
							for (int i = 0; i < profile.nProfils; i++)
							{
								bool success;

								float* tempConvProfile = new float[profile.tempConvLength];
								float* gaussCoeff = new float[maxConvLen];
								float* cannyCoeff = new float[maxConvLen];
								float* filterCoeff = new float[maxConvLen];
								float* secondDerCoeff = new float[maxConvLen];
								float* tempVector = new float[maxConvLen];

								float *bTCP = profile.tempConvProfile, *bGC = profile.gaussCoeffs, *bCC = profile.cannyCoeffs, 
									  *bFC = profile.filterCoeffs, *bSDC = profile.secDerCoeffs, *bTV = profile.tempVector ;

								memcpy(gaussCoeff, profile.gaussCoeffs,  ( 2 * profile.coeffLength + 1) * sizeof(float));
								memcpy(cannyCoeff, profile.cannyCoeffs, (2 * profile.coeffLength + 1) * sizeof(float));
								memcpy(secondDerCoeff, profile.secDerCoeffs, (2 * profile.coeffLength + 1) * sizeof(float));

								profile.gaussCoeffs = gaussCoeff;
								profile.cannyCoeffs = cannyCoeff;
								profile.secDerCoeffs = secondDerCoeff;
								profile.tempVector = tempVector;
								profile.filterCoeffs = filterCoeff;

								profile.tempConvProfile = tempConvProfile;

								//if (evaluation.nMaterial == 1)
								//{
									//if (evaluation.profileCorrAir || evaluation.profileCorrMat)
									//{
									//	float xx;

									//	if (profile.ptValid[i])
									//		xx = profile.results[i];
									//	else
									//		success = pM.SearchAroundZero(xx, i, float(evaluation.fSearchRange), float(evaluation.fSearchRangeNeg), -1, -1, false);

									//	if (profile.ptValid[i])
									//	{
									//		profile.ProfileCorrection(xx, i, evaluation.profileCorrMat ? evaluation.corrPolyMat : 0, evaluation.profileCorrAir ? evaluation.corrPolyAir : 0);

									//		if (!evaluation.profileIsCorrected)
									//			evaluation.profileIsCorrected = true;
									//	}
									//}

									success = pM.SearchAroundZero2(profile.results[i], i, float(evaluation.fSearchRange),
									float(evaluation.fSearchRangeNeg), staticTest, evaluation.airPointsThreshold, wQualFactor);
								//}
								//else
									//success = pM.SearchAroundZeroMixedMat(profile.results[i], i, evaluation.globalGradThreshold, evaluation.materialThresholds, evaluation.materialMasks);
								
								if (!success)
								{
									profile.ptValid[i] = false;
									profile.results[i] = 1e7;
									continue;
								}

								profile.ptValid[i] = true;
								memcpy(evaluation.resPoints[iSlice][i], evaluation.cloudPoints[iSlice][i], sizeof(d3d));

								evaluation.resPoints[iSlice][i][0] += profile.results[i] * evaluation.cloudNormals[iSlice][i][0];
								evaluation.resPoints[iSlice][i][1] += profile.results[i] * evaluation.cloudNormals[iSlice][i][1];
								evaluation.resPoints[iSlice][i][2] += profile.results[i] * evaluation.cloudNormals[iSlice][i][2];

								pM.GradientDirection(evaluation.voxels, evaluation.voxelDim, evaluation.voxelType, evaluation.interpolationMethod, evaluation.resPoints[iSlice][i], evaluation.sobelNormals[iSlice][i]);

								if (evaluation.angleCriterium)
								{
									if (evaluation.sobelNormals[iSlice][i][0] * evaluation.cloudNormals[iSlice][i][0] +
										evaluation.sobelNormals[iSlice][i][1] * evaluation.cloudNormals[iSlice][i][1] +
										evaluation.sobelNormals[iSlice][i][2] * evaluation.cloudNormals[iSlice][i][2] < cosTest)
									{
										profile.ptValid[i] = false;
										profile.nAngle++;
									}
								}

								if (profile.ptValid[i])
									profile.nValid++;
								//#pragma omp critical
								//			{
								//				nValid++;
								//			}


								profile.tempConvProfile = bTCP;
								profile.gaussCoeffs = bGC;
								profile.cannyCoeffs = bCC; 
								profile.filterCoeffs = bFC;
								profile.secDerCoeffs = bSDC;
								profile.tempVector = bTV;

								delete[] tempConvProfile;
								delete[]  gaussCoeff;
								delete[]  cannyCoeff;
								delete[]  filterCoeff;
								delete[]  secondDerCoeff;
								delete[]  tempVector;


							}

							if (progress != nullptr)
							{
								if (progress->IsCanceled())
									abort = true;
								if (iSlice % step == 0)
								{
#pragma omp critical 
									{
										progress->Report(0.01 * (float)currentSteps);
										currentSteps++;
									}
								}
							}
						}
					}
				}

				int nAngle = 0;
				int nValid = 0;
				int totalNumPoints = 0;

				for (int iSlice = 0; iSlice < evaluation.nSlices; iSlice++)
				{
					auto& profile = evaluation.pSlice[iSlice];

					nAngle += profile.nAngle;
					nValid += profile.nValid;

					totalNumPoints += profile.nProfils;
				}

				std::stringstream msg;
				std::cout << "PointCloud - Measured " << nValid << " points out of " << evaluation.allPoints << " nominal points";
				if (evaluation.angleCriterium) msg << ", " << nAngle << " points rejected due to angle criterion";
				//_Log->LogDebug(msg.str());
				std::stringstream msg2;
				msg2 << "Measure: Milliseconds spent " << int((1000 * clock() - ticks) / CLOCKS_PER_SEC);
				//_Log->LogDebug(msg2.str());

				std::cout << "number of input and valid points : " << nValid << " " << totalNumPoints << " " << nAngle << std::endl;

				return nValid;
#endif

				return 0;
			}


			int MeasureCUDA(bool wQualFactor, ICTEvaluationProgressSP* progress, CCTPointCloudEvaluationSP& evaluation)
			{

#if 1
				if (evaluation.resPoints == NULL)
					return 0;   // Keine Profile extrahiert

				clock_t ticks = clock();

				float cosTest = 0;
				if (evaluation.angleCriterium)
				{
					cosTest = cos(evaluation.angleCriterium / 180 * M_PI);
					std::stringstream msg;
					msg << "PointCloud - max. angular difference: " << evaluation.angleCriterium << "°";
					//_Log->LogDebug(msg.str());
				}

				float staticTest = -1;
				if (evaluation.checkStaticThreshold4Measure)
				{
					staticTest = evaluation.staticThreshold;
					std::stringstream msg;
					msg << "PointCloud - Static threshold for measurement: " << staticTest;
					//_Log->LogDebug(msg.str());
				}
				const int step = (evaluation.nSlices < 100) ? 1 : evaluation.nSlices / 100;   //Fortschrittsanzeige in 1% Schritten
				int currentSteps = 0;
				bool abort = false;


				unsigned short* profileGrayValues = evaluation.profileGrayValues;

				int numPoints = evaluation.allPoints;

				//void* cpuProfileEvaluators, unsigned short* cpuProfileData, int numProfiles, int profileLength,
					//int tempConvLength, int filterKernelSize, int singleProfileEvaluatorSize

				std::cout << "size of profile evaluation : " << sizeof(CCTProfilsEvaluationSP) << std::endl;

				auto& cpuProfileEvaluator = evaluation.pSlice[0];

				int maxConvLen = 21 * 2 + 1;

				float* tempConvProfile = new float[cpuProfileEvaluator.tempConvLength];
				float* gaussCoeff = new float[maxConvLen];
				float* cannyCoeff = new float[maxConvLen];
				float* filterCoeff = new float[maxConvLen];
				float* secondDerCoeff = new float[maxConvLen];
				float* tempVector = new float[maxConvLen];

				//float *bTCP = profile.tempConvProfile, *bGC = profile.gaussCoeffs, *bCC = profile.cannyCoeffs,
				//	*bFC = profile.filterCoeffs, *bSDC = profile.secDerCoeffs, *bTV = profile.tempVector;

				memcpy(gaussCoeff, cpuProfileEvaluator.gaussCoeffs, (2 * cpuProfileEvaluator.coeffLength + 1) * sizeof(float));
				memcpy(cannyCoeff, cpuProfileEvaluator.cannyCoeffs, (2 * cpuProfileEvaluator.coeffLength + 1) * sizeof(float));
				memcpy(secondDerCoeff, cpuProfileEvaluator.secDerCoeffs, (2 * cpuProfileEvaluator.coeffLength + 1) * sizeof(float));

				std::cout << "cpu profile sigma : " << cpuProfileEvaluator.sigma << std::endl;


				auto& slice0 = evaluation.pSlice[0];

				std::cout << "slice 0 temp convolution length : " << slice0.tempConvLength << std::endl;

				double initT = cv::getTickCount();

				numPoints = ( numPoints / 32 ) * 32;

				int tempConvLen = slice0.tempConvLength;
				int coeffLength = slice0.coeffLength;
				int searchRangeNegative = slice0.searchRangeNeg;
				int searchRange = slice0.searchRange;
				int zeroIndex = slice0.zeroIndex;

				int profileLength = slice0.length;

				slice0.length = (2 * coeffLength + searchRange + searchRangeNegative + 1);
				slice0.zeroIndex = searchRangeNegative + coeffLength;
				slice0.tempConvLength = 4 * coeffLength + searchRange + searchRangeNegative + 2;


				slice0.PutDynSigma(slice0.sigma);		// TODO can be done a priori

							// Dyn. p.threshold evtl. rücksetzen
				if (!slice0.dynThresholdControl) // TODO can be done a priori
					slice0.dynThreshold = slice0.threshold;

				if (slice0.dynThreshold > slice0.threshold) // TODO can be done a priori
					slice0.dynThreshold = slice0.threshold;

				computeGradientBasedMaximaPoints( (void*)&slice0 , profileGrayValues ,gaussCoeff , cannyCoeff , secondDerCoeff,  numPoints , profileLength , evaluation.pSlice[0].tempConvLength , 
					maxConvLen , sizeof(CCTProfilsEvaluationSP) , coeffLength , searchRangeNegative , zeroIndex, searchRange );

				std::cout << "time spent in gpu based computation " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;

				return 0;

				//computeGradientBasedMaximaPoints(  )

				//imt::volume::cuda::

//#pragma omp parallel default(shared) 
				// Ab hier laufen auf jedem Core ein Thread
				{
//#pragma omp for
					for (int iSlice = 0; iSlice < evaluation.nSlices; iSlice++)
					{
						//std::cout << "dyn sigma for slice " << iSlice << " : " << profile.dynSigma << std::endl;

//#pragma omp flush(abort)
						if (!abort)
						{
							auto& profile = evaluation.pSlice[iSlice];

							imt::volume::CCTProfilsMeasureSP	pM(profile);
							// Zähler zurücksetzen
							profile.nAngle = 0;
							profile.nValid = 0;



							// Schleife über alle Profile
							//for (int i = 0; i < profile.nProfils; i++)
							//{
							//	bool success;

							//	float* tempConvProfile = new float[profile.tempConvLength];
							//	float* gaussCoeff = new float[maxConvLen];
							//	float* cannyCoeff = new float[maxConvLen];
							//	float* filterCoeff = new float[maxConvLen];
							//	float* secondDerCoeff = new float[maxConvLen];
							//	float* tempVector = new float[maxConvLen];

							//	float *bTCP = profile.tempConvProfile, *bGC = profile.gaussCoeffs, *bCC = profile.cannyCoeffs,
							//		*bFC = profile.filterCoeffs, *bSDC = profile.secDerCoeffs, *bTV = profile.tempVector;

							//	memcpy(gaussCoeff, profile.gaussCoeffs, (2 * profile.coeffLength + 1) * sizeof(float));
							//	memcpy(cannyCoeff, profile.cannyCoeffs, (2 * profile.coeffLength + 1) * sizeof(float));
							//	memcpy(secondDerCoeff, profile.secDerCoeffs, (2 * profile.coeffLength + 1) * sizeof(float));

							//	profile.gaussCoeffs = gaussCoeff;
							//	profile.cannyCoeffs = cannyCoeff;
							//	profile.secDerCoeffs = secondDerCoeff;
							//	profile.tempVector = tempVector;
							//	profile.filterCoeffs = filterCoeff;

							//	profile.tempConvProfile = tempConvProfile;




							//	if (evaluation.nMaterial == 1)
							//	{
							//		if (evaluation.profileCorrAir || evaluation.profileCorrMat)
							//		{
							//			float xx;

							//			if (profile.ptValid[i])
							//				xx = profile.results[i];
							//			else
							//				success = pM.SearchAroundZero(xx, i, float(evaluation.fSearchRange), float(evaluation.fSearchRangeNeg), -1, -1, false);

							//			if (profile.ptValid[i])
							//			{
							//				profile.ProfileCorrection(xx, i, evaluation.profileCorrMat ? evaluation.corrPolyMat : 0, evaluation.profileCorrAir ? evaluation.corrPolyAir : 0);

							//				if (!evaluation.profileIsCorrected)
							//					evaluation.profileIsCorrected = true;
							//			}
							//		}

							//		success = pM.SearchAroundZero(profile.results[i], i, float(evaluation.fSearchRange),
							//			float(evaluation.fSearchRangeNeg), staticTest, evaluation.airPointsThreshold, wQualFactor);
							//	}
							//	else
							//		success = pM.SearchAroundZeroMixedMat(profile.results[i], i, evaluation.globalGradThreshold, evaluation.materialThresholds, evaluation.materialMasks);
							//	if (!success)
							//	{
							//		profile.ptValid[i] = false;
							//		profile.results[i] = 1e7;
							//		continue;
							//	}

							//	profile.ptValid[i] = true;
							//	memcpy(evaluation.resPoints[iSlice][i], evaluation.cloudPoints[iSlice][i], sizeof(d3d));

							//	evaluation.resPoints[iSlice][i][0] += profile.results[i] * evaluation.cloudNormals[iSlice][i][0];
							//	evaluation.resPoints[iSlice][i][1] += profile.results[i] * evaluation.cloudNormals[iSlice][i][1];
							//	evaluation.resPoints[iSlice][i][2] += profile.results[i] * evaluation.cloudNormals[iSlice][i][2];

							//	pM.GradientDirection(evaluation.voxels, evaluation.voxelDim, evaluation.voxelType, evaluation.interpolationMethod, evaluation.resPoints[iSlice][i], evaluation.sobelNormals[iSlice][i]);

							//	if (evaluation.angleCriterium)
							//	{
							//		if (evaluation.sobelNormals[iSlice][i][0] * evaluation.cloudNormals[iSlice][i][0] +
							//			evaluation.sobelNormals[iSlice][i][1] * evaluation.cloudNormals[iSlice][i][1] +
							//			evaluation.sobelNormals[iSlice][i][2] * evaluation.cloudNormals[iSlice][i][2] < cosTest)
							//		{
							//			profile.ptValid[i] = false;
							//			profile.nAngle++;
							//		}
							//	}

							//	if (profile.ptValid[i])
							//		profile.nValid++;
							//	//#pragma omp critical
							//	//			{
							//	//				nValid++;
							//	//			}


							//	profile.tempConvProfile = bTCP;
							//	profile.gaussCoeffs = bGC;
							//	profile.cannyCoeffs = bCC;
							//	profile.filterCoeffs = bFC;
							//	profile.secDerCoeffs = bSDC;
							//	profile.tempVector = bTV;

							//	delete[] tempConvProfile;
							//	delete[]  gaussCoeff;
							//	delete[]  cannyCoeff;
							//	delete[]  filterCoeff;
							//	delete[]  secondDerCoeff;
							//	delete[]  tempVector;


							//}

							if (progress != nullptr)
							{
								if (progress->IsCanceled())
									abort = true;
								if (iSlice % step == 0)
								{
#pragma omp critical 
									{
										progress->Report(0.01 * (float)currentSteps);
										currentSteps++;
									}
								}
							}
						}
					}
				}

				int nAngle = 0;
				int nValid = 0;
				int totalNumPoints = 0;

				for (int iSlice = 0; iSlice < evaluation.nSlices; iSlice++)
				{
					auto& profile = evaluation.pSlice[iSlice];

					nAngle += profile.nAngle;
					nValid += profile.nValid;

					totalNumPoints += profile.nProfils;
				}

				std::stringstream msg;
				std::cout << "PointCloud - Measured " << nValid << " points out of " << evaluation.allPoints << " nominal points";
				if (evaluation.angleCriterium) msg << ", " << nAngle << " points rejected due to angle criterion";
				//_Log->LogDebug(msg.str());
				std::stringstream msg2;
				msg2 << "Measure: Milliseconds spent " << int((1000 * clock() - ticks) / CLOCKS_PER_SEC);
				//_Log->LogDebug(msg2.str());

				std::cout << "number of input and valid points : " << nValid << " " << totalNumPoints << " " << nAngle << std::endl;

				return nValid;
#endif

				return 0;
			}





			void MeasureWithSingleProfile(bool wQualFactor,  CCTPointCloudEvaluationSP& evaluation)
			{

				int nSlices = evaluation.nSlices;

				int kernelSize = 43;

				auto& slice0 = evaluation.pSlice[0];

				unsigned short* prof = evaluation.profileGrayValues;

				int profileId = 0;

				int profileSize = slice0.length;

				std::vector<bool> ptValid(evaluation.allPoints , false);
				std::vector<float> results(evaluation.allPoints, 0);

				std::cout << "temp conv length : " << slice0.tempConvLength <<" "<<evaluation.allPoints<< std::endl;

				for(int ss = 0; ss < nSlices; ss++)
				{ 

					auto& slice = evaluation.pSlice[ss];

					CCTProfilsEvaluationSP profileEvaluation;

				    auto ped = (unsigned char*)&profileEvaluation;

				    int evalSize = sizeof(CCTProfilsEvaluationSP);

				    memcpy(ped, &slice , evalSize);

					int nProfils = slice.nProfils;

					if ( profileId >= 1)
						break;
					

					for (int pp = 0; pp < nProfils; pp++ , profileId++)
					{
						if (profileId >= 1)
							break;


						//constant kernels are put onto registers
						float* gaussCoeffs = new float[kernelSize];
						float* cannyCoeffs = new float[kernelSize];
						float* secDerCoeffs = new float[kernelSize];

						memcpy(gaussCoeffs, slice0.gaussCoeffs, kernelSize * sizeof(float));
						memcpy(cannyCoeffs, slice0.cannyCoeffs, kernelSize * sizeof(float));
						memcpy(secDerCoeffs, slice0.secDerCoeffs, kernelSize * sizeof(float));


						unsigned short *profileData = new unsigned short[slice0.length];

						memcpy(profileData, prof + profileId * profileSize, profileSize * sizeof(unsigned short));


						//slice.tempConvLength = 473;
						float* tempConvolutionData = new float[474]; //(float*)(profileData + blockDim.x * profileSize);

						float *kernelDataShared = new float[2 * kernelSize]; //(float*)(tempConvolutionData + blockDim.x * profileEvaluation.tempConvLength);

						unsigned short* currentProfile = profileData;// +threadIdx.x * profileSize;
						float* currentConvolutionData = tempConvolutionData;// +threadIdx.x * profileEvaluation.tempConvLength;

						for (int ii = 0; ii < profileEvaluation.tempConvLength; ii++)
						{
							currentConvolutionData[ii] = 0;
						}

						profileEvaluation.resQuality = new float[1];
						profileEvaluation.resCanny = new float[1];
						profileEvaluation.ptValid = new bool[1];
						profileEvaluation.results = new float[1];

						profileEvaluation.ptValid[0] = false;
						profileEvaluation.results[0] = 0;

						profileEvaluation.tempConvProfile = currentConvolutionData;
						profileEvaluation.gaussCoeffs = gaussCoeffs;
						profileEvaluation.cannyCoeffs = cannyCoeffs;
						profileEvaluation.secDerCoeffs = secDerCoeffs;

						profileEvaluation.filterCoeffs = kernelDataShared ;
						profileEvaluation.tempVector = profileEvaluation.filterCoeffs + kernelSize;
						profileEvaluation.profile_16U = currentProfile;

						CCTProfilsMeasureSP measure(profileEvaluation);



						float xx = 0;

						bool res = measure.SearchAroundZero2(xx, 0, 40, 40, -1, -1, true, true);

						results[profileId] = xx;

						printf("cpu value of xx : %f %d \n", xx , res);


						delete[] gaussCoeffs;
						delete[] cannyCoeffs;
						delete[] secDerCoeffs;
						delete[] profileEvaluation.resCanny;
						delete[] profileEvaluation.resQuality;
						delete[] profileEvaluation.ptValid;
						delete[] profileEvaluation.results;

					}


				}


			}
		



         }




	}



}