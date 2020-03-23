#pragma once

#include "CTPointCloudEvaluation.h"

#include "stdafx.h"
#include "loxx.h"
#include "LxManager.h"

using namespace  LoXX;

typedef  double d3d[3];
typedef  long l3d[3];

class CTEvaluationPartProgress : public ICTEvaluationProgress
{
public:
	CTEvaluationPartProgress(ICTEvaluationProgress* progress, double start, double end)
		: _Progress(progress), _Start(start), _End(end)
	{
		
	}
	
	~CTEvaluationPartProgress()
	{
		if(_Progress != nullptr)
			_Progress->Report(_End);

		_Progress = nullptr;
	}

	virtual void Report( double progress)
	{
		if(_Progress != nullptr)
		{
			double reportedValue = _Start + (_End - _Start) * progress;
			if (reportedValue < 0)
				reportedValue = 0;
			else if (reportedValue > 1)
				reportedValue = 1;

			_Progress->Report(reportedValue);
		}
	}

	virtual bool IsCanceled()
	{
		if(_Progress == nullptr)
			return false;
		return _Progress->IsCanceled();
	}
private:

	ICTEvaluationProgress* _Progress;
	double _Start;
	double _End;
};

class CCTPointCloudEvaluationDecorator
{
public:

	static double CalculateSignedSquaredDeviation(const d3d nominal, const d3d nominalNormal, const d3d actual, const d3d actualNormal)
	{
		if (actualNormal[0] == 0 && actualNormal[1] == 0 && actualNormal[2] == 0)
			return std::numeric_limits<double>::quiet_NaN();
		d3d deviationVector = {actual[0] - nominal[0], actual[1] - nominal[1], actual[2] - nominal[2]};
		double dev_squaredNorm = deviationVector[0] * deviationVector[0] + deviationVector[1] * deviationVector[1] + deviationVector[2] * deviationVector[2];

		//Determine if the nominal-vector and the deviationVector have the same direction
		double scp = nominalNormal[0] * deviationVector[0] + nominalNormal[1] * deviationVector[1] + nominalNormal[2] * deviationVector[2];

		double sign = scp >= 0 ? 1.0 : -1.0;

		return sign * dev_squaredNorm;
	}

	static long DoMeasureData(
					void** data, unsigned int bitDepth, l3d size, d3d voxelSize, 
					int interpolationMethod,
					bool useAutoParams, double& gradientThreshold, double& sigma,
					bool useBeamHardeningCorrection,
					bool considerStaticThreshold, long staticThreshold,
					bool useAngleCriterium, double angleCriterium,
					unsigned long long qualityThreshold,
					double searchRange, double searchRangeAirSide, 
					long count, d3d const* positions, d3d const* searchDirections, d3d* results, d3d* normals, double* qualities,
					ICTEvaluationProgress* progress = nullptr
					)
	{
		if(count == 0 || searchRange == 0.0 || searchRangeAirSide == 0.0)
			return 0;

		unsigned int const byteSize = bitDepth / 8;
		
		double const voxelSizeMax = std::max(voxelSize[0], std::max(voxelSize[1], voxelSize[2]));
		
		double const origSearchRangeVoxel = searchRange / voxelSizeMax;	
		double const searchRangeVoxel = std::max(origSearchRangeVoxel, 10.0);
		double const origSearchRangeAirSideVoxel = searchRangeAirSide / voxelSizeMax;
		double const searchRangeAirSideVoxel = std::max(origSearchRangeAirSideVoxel, 10.0);
		
#ifdef WRITE_DEBUG_FILE
		FILE* pDimensionsBin;
		fopen_s(&pDimensionsBin, "C:\\Temp\\dimensions.bin", "wb");
		fwrite(size, sizeof(long), 3, pDimensionsBin);
		fwrite(&voxelSizeMax, sizeof(double), 1, pDimensionsBin);
		fclose(pDimensionsBin);

		FILE* pParamsBin;
		fopen_s(&pParamsBin, "C:\\Temp\\params.bin", "wb");
		fwrite(&origSearchRangeVoxel, sizeof(double), 1, pParamsBin);
		fwrite(&origSearchRangeAirSideVoxel, sizeof(double), 1, pParamsBin);
		fwrite(&sigma, sizeof(double), 1, pParamsBin);
		fwrite(&useAngleCriterium, sizeof(bool), 1, pParamsBin);
		fwrite(&angleCriterium, sizeof(double), 1, pParamsBin);
		fwrite(&useBeamHardeningCorrection, sizeof(bool), 1, pParamsBin);
		fwrite(&useAutoParams, sizeof(bool), 1, pParamsBin);
		fwrite(&gradientThreshold, sizeof(double), 1, pParamsBin);
		fwrite(&qualityThreshold, sizeof(unsigned long long), 1, pParamsBin);
		fwrite(&considerStaticThreshold, sizeof(bool), 1, pParamsBin);
		fwrite(&staticThreshold, sizeof(long), 1, pParamsBin);
		fclose(pParamsBin);

		FILE* pVoxBin;
		fopen_s(&pVoxBin, "C:\\Temp\\vox.bin", "wb");
		int sliceSize = size[0] * size[1];
		for (int i = 0; i < size[2]; i++)
		{
			if (!fwrite(data[i], sizeof(short), sliceSize, pVoxBin))
			{
				break;
			}
		}

		fclose(pVoxBin);

		long countLong = static_cast<long>(count);

		FILE* pVPointsAndNormalsBin;
		fopen_s(&pVPointsAndNormalsBin, "C:\\Temp\\pointsAndNormals.bin", "wb");
		fwrite(&countLong, sizeof(long), 1, pVPointsAndNormalsBin);
		fwrite(positions, 3*sizeof(double), count, pVPointsAndNormalsBin);
		fwrite(searchDirections, 3*sizeof(double), count, pVPointsAndNormalsBin);
		fclose(pVPointsAndNormalsBin);
#endif //WRITE_DEBUG_FILE
		
		LXLogger* log = LXManager::Manager().GetLogger("CCTPointCloudEvaluationDecorator");
		std::stringstream msg;	
		
		CCTPointCloudEvaluation evaluation(data, (CCTProfilsEvaluation::vxType)byteSize, size);
		evaluation.SetSigma(sigma);
		evaluation.SetThreshold(gradientThreshold);
		bool usingQuality = count <= qualityThreshold;

		if((!usingQuality || evaluation.nMaterial > 1) && evaluation.detectedMaterials < evaluation.nMaterial)
		{
			unsigned int const shortestDimension = std::min(std::min(size[0], size[1]), size[2]);
			unsigned int const SliceSkipRate = 25;
			int evpts = evaluation.MaterialAnalysis(1, shortestDimension / SliceSkipRate);
			
			msg << "MaterialAnalysis with " << evpts << " evaluated points found " << evaluation.detectedMaterials << " materials and optimal static threshold: " << evaluation.staticThreshold;
			log->LogDebug(msg.str());
			msg.str("");
			//log->Info(gcnew System::String("MaterialAnalysis: air threshold: ") + evaluation.materialThresholds[0].ToString() + " gradient noise: " + evaluation.materialGradientThresholds[0].ToString());
			msg << "MaterialAnalysis: air threshold: " << evaluation.materialThresholds[0] << " gradient noise: " << evaluation.materialGradientThresholds[0];
			log->LogDebug(msg.str());
			msg.str("");
			
			for(int i=1; i<=evaluation.detectedMaterials; i++)
			{
				msg << "MaterialAnalysis: " << i << ". Material threshold: " << evaluation.materialThresholds[i] << " gradient threshold: " << evaluation.materialGradientThresholds[i];
				log->LogDebug(msg.str());
				msg.str("");				
			}
			if(evaluation.detectedMaterials > 1)
			{
				msg << "Material analysis detected " << evaluation.detectedMaterials << " materials. Using the global threshold!";
				log->LogDebug(msg.str());
				msg.str("");				
			}
		}

		//log->Info(System::String::Format("Trying to extract profiles ({0}|{1})", searchRangeVoxel, searchRangeAirSideVoxel));
		evaluation.extractProfiles(count, (d3d*)positions, (d3d*)searchDirections, searchRangeVoxel,  searchRangeAirSideVoxel, interpolationMethod);

		if(progress != nullptr)
		{
			if(progress->IsCanceled())
				return 0;
			progress->Report(0.05);
		}

		if (useAngleCriterium)
		{
			msg << "Using Angle criteria ("<< angleCriterium << ")";
			log->LogDebug(msg.str());
			msg.str("");	
			evaluation.SetAngleCriterium(180.0 * angleCriterium / M_PI);
		}

		if (useAutoParams || useBeamHardeningCorrection)
		{
			bool useAutoGrad = useAutoParams;
			// Bei sehr vielen Messpunkten Basis der Statistik etwas ausdünnen (Rechenzeit verringern!)
			long step = (count > 5000) ? count / 5000 : 1;
			msg << "Using " << step << " as step size";
			log->LogDebug(msg.str());
			msg.str("");

			if(evaluation.detectedMaterials > evaluation.nMaterial)
			{
				evaluation.SetThreshold(evaluation.globalGradThreshold/evaluation.upperLimit);
				useAutoGrad = false;
			}

			bool success = evaluation.AutoParam(step, useBeamHardeningCorrection, useBeamHardeningCorrection, useAutoParams, useAutoGrad);
			if(success)
				log->LogDebug("AutoParam successful!");
			else
				log->LogDebug("AutoParam failed!");
			
			sigma = evaluation.sigma;
			msg << "Sigma = " << evaluation.sigma;
			log->LogDebug(msg.str());
			msg.str("");
			
			gradientThreshold = evaluation.relThreshold;
			msg << "GradientThreshold = " << evaluation.relThreshold;
			log->LogDebug(msg.str());
			msg.str("");
		}

		if(progress != nullptr)
		{
			if(progress->IsCanceled())
				return 0;
			progress->Report(0.15);
		}

		if(considerStaticThreshold)
		{
			evaluation.checkStaticThreshold4Measure = true;
			evaluation.staticThreshold = static_cast<double>(staticThreshold);
		}

		ICTEvaluationProgress* measurementProgress = nullptr;

		if(progress != nullptr)
		{
			if(progress->IsCanceled())
				return 0;

			measurementProgress = new CTEvaluationPartProgress(progress, 0.15, 1.0);
		}

		msg << "Starting to measure " << count << " points (Quality: " << usingQuality ? "true)..." : "false)...";
		log->LogDebug(msg.str());
		msg.str("");

		long valid = evaluation.Measure(usingQuality, measurementProgress);
		msg << valid << " points grabbed from volume";
		log->LogDebug(msg.str());
		msg.str("");

		double const origSearchRangeVoxelSquared = origSearchRangeVoxel * origSearchRangeVoxel;
		double const origSearchRangeAirSideVoxelSquared = origSearchRangeAirSideVoxel * origSearchRangeAirSideVoxel;
		for (int i = 0; i < count; ++i)
		{	
			if ((measurementProgress != nullptr && measurementProgress->IsCanceled()) || !evaluation.getResult(i, results[i], normals[i], &qualities[i]))
			{
				//log->Error(System::String::Format("Invalid Point {0}: {1} {2} {3} | {4} {5} {6}", i, positions[i][0], positions[i][1], positions[i][2], searchDirections[i][0], searchDirections[i][1], searchDirections[i][2] ));

				results[i][0] = 0.0;
				results[i][1] = 0.0;
				results[i][2] = 0.0;

				normals[i][0] = 0.0;
				normals[i][1] = 0.0;
				normals[i][2] = 0.0;

				qualities[i] = 0.0;
			}
			else if (origSearchRangeVoxel != searchRangeVoxel || origSearchRangeAirSideVoxel != searchRangeAirSideVoxel)
			{
				//Removing points which are not in range [-origSearchRangeAirSideVoxel, origSearchRangeVoxel]
				double squaredDev = CalculateSignedSquaredDeviation(positions[i], searchDirections[i], results[i], normals[i]);
				if(squaredDev == std::numeric_limits<double>::quiet_NaN() || squaredDev > origSearchRangeVoxelSquared || -squaredDev > origSearchRangeAirSideVoxelSquared)
				{
					results[i][0] = 0.0;
					results[i][1] = 0.0;
					results[i][2] = 0.0;

					normals[i][0] = 0.0;
					normals[i][1] = 0.0;
					normals[i][2] = 0.0;

					qualities[i] = 0.0;
				}
			}
		}

		if(measurementProgress != nullptr)
		{
			delete measurementProgress;
			measurementProgress = nullptr;
		}
		
		return valid;
	}
};