#ifndef __UIN16SCVTILEDREADER_H__
#define __UIN16SCVTILEDREADER_H__

#include <string>
#include "volumetopooctree.h"


class Uint16SCVTiledReader
{
	
	public:



		struct Uint16SCVHeader
		{

			int _Major, _Minor;
			int _TileDim;
			int headerSize;
			int mirrorZ;
			int numBitsPerVoxel;
			int volumeSizeX;
			int volumeSizeY;
			int volumeSizeZ;
			double voxelSizeX;
			double voxelSizeY;
			double voxelSizeZ;
			double resultBitDepthMinVal;
			double resultBitDepthmaxVal;
			double tubePosX;
			double tubePosY;
			double tubePosZ;
			int tubeCurrent;
			int tubeVoltage;
			double rtPosX;
			double rtPosY;
			double rtPosZ;
			double detectorIntegrationTime; //int mili seconds
			double detectorGain;
			double detectorXPos;
			double detectorYPos;
			double detectorZPos;
			float detectorPixelWidth;
			float detectorPixelHeight;
			int imageBitDepth;
			int detectorWidth;
			int detectorHeight;
			int detectorImageWidth;
			int detectorImageHeight;
			int numberOfProjections;
			int roiX;
			int roiY;
			int roiWidth;
			int roiHeight;
			double noiseSuppressionFilter;
			double voxelreductionFactor;
			float gain;
			float binningMode;
			char prefilterData[128];
			double volumeStartPosX;
			double volumeStartPosY;
			double volumeStartPosZ;
			float minValue;
			float maxValue;
			float volumeDefinitionAngle;
			bool volumeMerge;
		};



	
	Uint16SCVTiledReader();
	
	//generates tiled data based on memory cap. note that for quality visualization the iso50 threshold or multi material thresholds should be computed on 
	//the reduced data
	void init( std::wstring , size_t memoryCap );

	void setVolumeInfo( int w, int h, int d, float voxelSizeX, float voxelSizeY, float voxelSizeZ, int bitDepth, int rawHeaderOffset );


protected:


	double computeOptimalResizeFactor( double memoryCap );

	void resizeVolumeMT( );



protected:

	Uint16SCVHeader _Header;

	size_t _RawFileSkipSize;

	imt::volume::VolumeTopoOctree *_TopoTree;

	
};





#endif