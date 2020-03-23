#include "volumeanalytics.h"
#include "histogramprocessor.h"
#include <stdio.h>
#include <ipp.h>
#include <iostream>
#include "src/plugins/Application/ZeissViewer/SegmentationInterface/ZeissSegmentationInterface.hpp"
#include "src/plugins/Application/ZeissViewer/SeparationInterface/ZeissSeparationInterface.hpp"



namespace imt {

	namespace volume {




		// Type of interpolation, the following values are possible :

		// IPPI_INTER_NN - nearest neighbor interpolation,

		//	IPPI_INTER_LINEAR - trilinear interpolation,

		//	IPPI_INTER_CUBIC - tricubic interpolation,

		//	IPPI_INTER_CUBIC2P_BSPLINE - B - spline,

		//	IPPI_INTER_CUBIC2P_CATMULLROM - Catmull - Rom spline,

		//	IPPI_INTER_CUBIC2P_B05C03 - special two - parameters filter(1 / 2, 3 / 10).


		void VolumeAnalytics::resizeVolume_Uint16C1(unsigned short *inputVolume, int iWidth, int iHeight, int iDepth, unsigned short *outputVolume, int oWidth,
			int oHeight, int oDepth, double resizeRatio)
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

			int interpolation = IPPI_INTER_LINEAR;//IPPI_INTER_CUBIC2P_B05C03;//

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



			ipprResizeGetBufSize(srcVoi, dstVoi, 1, interpolation, &bufferSize);

			computationBuffer = new Ipp8u[bufferSize];

			ipprResize_16u_C1V(inputVolume, inputVolumeSize, srcStep, srcPlaneStep, srcVoi, outputVolume, dstStep,
				dstPlaneStep, dstVoi, xFactor, yFactor, zFactor, xShift, yShift, zShift, interpolation, computationBuffer);

			delete[] computationBuffer;

		}


		void VolumeAnalytics::resizeVolume_Uint16C1_MT(unsigned short *inputVolume, int iWidth, int iHeight, int iDepth, unsigned short *outputVolume, int oWidth,
			int oHeight, int oDepth, double resizeRatio)
		{
			int bandWidth = 32;

			int numBands = oDepth % 32 == 0 ? oDepth / bandWidth : oDepth / bandWidth + 1;

			int extension = (1.0 / resizeRatio) + 1;

			if (numBands > 1)
			{

				double progress = 0;

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

					ipprResizeGetBufSize(srcVoi, dstVoi, 1, interpolation, &bufferSize);

					computationBuffer = new Ipp8u[bufferSize];

					ipprResize_16u_C1V(inputVolume + sourceDataShift, inputVolumeSize, srcStep, srcPlaneStep,
						srcVoi, outputVolume + destDataShift, dstStep, dstPlaneStep, dstVoi,
						xFactor, yFactor, zFactor, xShift, yShift, zShift, interpolation, computationBuffer);

					delete[] computationBuffer;


				}

			}
			else
			{
				resizeVolume_Uint16C1(inputVolume, iWidth, iHeight, iDepth, outputVolume, oWidth, oHeight, oDepth, resizeRatio);

			}

		}





		VolumeAnalytics::VolumeAnalytics()
		{

		}

		void VolumeAnalytics::readVolumeData(std::string& path, VolumeData& volume)
		{
			FILE *file2 = fopen(path.c_str(), "rb");

			char buffer[1024];

			fread(buffer, 1, 1024, file2);

			int *bf = (int*)buffer;

			int headerSize = *bf;

			bf++;

			int mirrorZ = *bf;

			bf++;

			int numBitsPerVoxel = *bf;

			bf++;

			int volumeSizeX = *bf;

			bf++;

			int volumeSizeY = *bf;

			bf++;

			int volumeSizeZ = *bf;

			bf++;

			double *fbf = (double*)bf;

			double voxelSizeX = *fbf;

			fbf++;

			double voxelSizeY = *fbf;

			fbf++;

			double voxelSizeZ = *fbf;

			fbf++;

			double resultBitDepthMinVal = *fbf;

			fbf++;

			double resultBitDepthmaxVal = *fbf;

			fbf++;

			double tubePosX = *fbf;

			fbf++;

			double tubePosY = *fbf;

			fbf++;

			double tubePosZ = *fbf;

			fbf++;

			bf = (int*)fbf;

			int tubeCurrent = *bf;

			bf++;

			int tubeVoltage = *bf;

			bf++;

			fbf = (double*)bf;

			double rtPosX = *fbf;

			fbf++;

			double rtPosY = *fbf;

			fbf++;

			double rtPosZ = *fbf;

			fbf++;

			double detectorIntegrationTime = *fbf; //int mili seconds

			fbf++;

			double detectorGain = *fbf;

			fbf++;

			double detectorXPos = *fbf;

			fbf++;

			double detectorYPos = *fbf;

			fbf++;

			double detectorZPos = *fbf;

			fbf++;

			float *sbf = (float*)fbf;

			float detectorPixelWidth = *sbf;

			sbf++;

			float detectorPixelHeight = *sbf;

			sbf++;

			bf = (int*)sbf;

			int imageBitDepth = *bf;

			bf++;

			int detectorWidth = *bf;

			bf++;

			int detectorHeight = *bf;

			bf++;

			int detectorImageWidth = *bf;

			bf++;

			int detectorImageHeight = *bf;

			bf++;

			unsigned int* ubf = (unsigned int*)bf;

			int numberOfProjections = *ubf;

			ubf++;

			bf = (int*)ubf;

			int roiX = *bf;

			bf++;

			int roiY = *bf;

			bf++;

			int roiWidth = *bf;

			bf++;

			int roiHeight = *bf;

			bf++;

			fbf = (double*)bf;

			double noiseSuppressionFilter = *fbf;

			fbf++;

			double voxelreductionFactor = *fbf;

			fbf++;

			sbf = (float*)fbf;

			float gain = *sbf;

			sbf++;

			float binningMode = *sbf;

			sbf++;

			char prefilterData[128];

			memcpy(prefilterData, sbf, 128);

			fbf = (double*)(sbf + 32);

			double volumeStartPosX = *fbf;

			fbf++;

			double volumeStartPosY = *fbf;

			fbf++;

			double volumeStartPosZ = *fbf;

			fbf++;

			sbf = (float*)fbf;

			float minValue = *sbf;

			sbf++;

			float maxValue = *sbf;

			sbf++;

			float volumeDefinitionAngle = *sbf;

			sbf++;

			bool *bbf = (bool*)sbf;

			bool volumeMerge = *bbf;

			volume._Width = volumeSizeX;
			volume._Height = volumeSizeY;
			volume._Depth = volumeSizeZ;

			volume._VoxelSizeX = voxelSizeX;
			volume._VoxelSizeY = voxelSizeY;
			volume._VoxelSizeZ = voxelSizeZ;

			int64_t size = volume._Width * volume._Height * volume._Depth;

			volume._VolumeBuffer = new unsigned short[size];

			fread(volume._VolumeBuffer, 2, size, file2);

			fclose(file2);


		}

		unsigned short VolumeAnalytics::computeIsoThreshold(VolumeAnalytics::VolumeData& volume)
		{
			unsigned short isoThreshold = 0;

			HistogramProcessor histogramFilter;

			std::vector<int64_t> histogram(USHRT_MAX, 0);

			int64_t volumeSize = volume._Width * volume._Height * volume._Depth;

			for (int64_t vv = 0; vv < volumeSize; vv++)
			{
				int grayValue = volume._VolumeBuffer[vv];

				histogram[grayValue]++;
			}

			isoThreshold = histogramFilter.ISO50Threshold(histogram);



			Volume vol;

			vol.size[0] = volume._Width;
			vol.size[1] = volume._Height;
			vol.size[2] = volume._Depth;

			vol.voxel_size[0] = volume._VoxelSizeX;
			vol.voxel_size[1] = volume._VoxelSizeY;
			vol.voxel_size[2] = volume._VoxelSizeZ;

			vol.data = (uint16_t*)volume._VolumeBuffer;

			ZeissSegmentationInterface segmenter;

			SegmentationParameter param;
			param.max_memory = static_cast<size_t>(2048) * 1024 * 1024;
			param.output_material_index = false;
			segmenter.setParameter(param);

			segmenter.setInputVolume(vol);

			Materials materials = segmenter.getMaterialRegions();

			isoThreshold = materials.regions[materials.first_material_index].lower_bound;
			
			return isoThreshold;
		}


		void VolumeAnalytics::generateMesh(const VolumeData& volume, int isoThreshold, SurfaceMesh& surfaceMesh)
		{

		}


		void VolumeAnalytics::reduceVolume(const VolumeData& inputVolume, VolumeData& targetVolume, double reductionCoeff)//each side is reduced by this coefficient
		{
			if (reductionCoeff >= 1.0)
			{
				std::cout << "failed volume reduction , reduction coefficient should be less than 1 " << std::endl;

				return ;
			}

			targetVolume._Width = inputVolume._Width * reductionCoeff;
			targetVolume._Height = inputVolume._Height * reductionCoeff;
			targetVolume._Depth = inputVolume._Depth * reductionCoeff;

			targetVolume._VolumeBuffer = (new unsigned short[targetVolume._Width * targetVolume._Height * targetVolume._Depth]);

			targetVolume._VoxelSizeX = targetVolume._VoxelSizeX / reductionCoeff;
			targetVolume._VoxelSizeY = targetVolume._VoxelSizeY / reductionCoeff;
			targetVolume._VoxelSizeZ = targetVolume._VoxelSizeZ / reductionCoeff;

			memset(targetVolume._VolumeBuffer, 0, (size_t)targetVolume._Width * (size_t)targetVolume._Height * (size_t)targetVolume._Depth * sizeof(unsigned short));

			resizeVolume_Uint16C1_MT((unsigned short*)inputVolume._VolumeBuffer, inputVolume._Width, inputVolume._Height, inputVolume._Depth,
				(unsigned short*)targetVolume._VolumeBuffer, targetVolume._Width, targetVolume._Height, targetVolume._Depth, reductionCoeff);
		}



	}



}