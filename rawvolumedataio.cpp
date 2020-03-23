#include "rawvolumedataio.h"	
#include "iostream"	
#include "stdio.h"


namespace imt{

	namespace volume{








	RawVolumeDataIO::RawVolumeDataIO()
	{
		
		
	}

	void RawVolumeDataIO::readUint16SCV(QString filePath, imt::volume::VolumeInfo& info)
	{

		FILE *file2 = fopen(filePath.toStdString().c_str(), "rb");

		char buffer[1024];

		fread(buffer, 1, 1024, file2);

		int *bf = (int*)buffer;

		int headerSize = *bf;

		std::cout << " header size : " << headerSize << std::endl;

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


		std::cout <<" volume dimension : "<< volumeSizeX << " " << volumeSizeY << " " << volumeSizeZ << std::endl;

		info.mWidth = volumeSizeX;
		info.mHeight = volumeSizeY;
		info.mDepth = volumeSizeZ;

		info.mVoxelStep(0) = voxelSizeX;
		info.mVoxelStep(1) = voxelSizeY;
		info.mVoxelStep(2) = voxelSizeZ;

		long long w = info.mWidth;
		long long h = info.mHeight;
		long long d = info.mDepth;

		long long size = w * h * d ;

		info.mVolumeData = (unsigned char*)(new unsigned short[ size ] );
		fread((char*)info.mVolumeData , 2 , size, file2);

		fclose(file2);


	}


	void RawVolumeDataIO::readUint16SCV(QString filePath, VolumeInfo& info, CPUBuffer& cpuBuffer)
	{
		FILE *file2 = fopen(filePath.toStdString().c_str(), "rb");

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

		info.mWidth = volumeSizeX;
		info.mHeight = volumeSizeY;
		info.mDepth = volumeSizeZ;

		info.mVoxelStep(0) = voxelSizeX;
		info.mVoxelStep(1) = voxelSizeY;
		info.mVoxelStep(2) = voxelSizeZ;

		long long w = info.mWidth;
		long long h = info.mHeight;
		long long d = info.mDepth;

		long long size = w * h * d;

		cpuBuffer.resize(size * sizeof(unsigned short));

		//info.mVolumeData = (unsigned char*)(new unsigned short[size]);
		fread((char*)cpuBuffer.data(), 2, size, file2);

		fclose(file2);

	}

	}



}
