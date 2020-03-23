

#include "Uint16SCVTiledReader.h"
#include <algorithm>
#include <LDM/SoLDMTopoOctree.h>

#include <stdio.h>
#include <stdlib.h>
#include <ipp.h>
#include <omp.h>
#include <queue>


static void HandleIppError(IppStatus err,
	const char *file,
	int line) {
	if (err != ippStsNoErr) {
		printf("IPP Error code %d in %s at line %d\n", err,
			file, line);
		exit(EXIT_FAILURE);
	}
}

#define ippSafeCall( err ) (HandleIppError( err, __FILE__, __LINE__ ))

void resizeVolume_Uint16C1( unsigned short *inputVolume, int iWidth, int iHeight, int iDepth, unsigned short *outputVolume, int oWidth, int oHeight, int oDepth , double resizeFactor )
{
	IpprVolume inputVolumeSize, outputVolumeSize;

	int srcStep = iWidth * sizeof(unsigned short);
	int srcPlaneStep = iWidth * iHeight * sizeof(unsigned short);
	IpprCuboid srcVoi;

	int dstStep = oWidth * sizeof(unsigned short);
	int dstPlaneStep = oWidth * oHeight * sizeof(unsigned short);
	IpprCuboid dstVoi;

	double xFactor = resizeFactor, yFactor = resizeFactor, zFactor = resizeFactor;
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

	ippSafeCall(ipprResizeGetBufSize(srcVoi, dstVoi, 1, interpolation, &bufferSize));

	computationBuffer = new Ipp8u[bufferSize];

	ippSafeCall(ipprResize_16u_C1V(inputVolume, inputVolumeSize, srcStep, srcPlaneStep, srcVoi, outputVolume, dstStep,
		dstPlaneStep, dstVoi, xFactor, yFactor, zFactor, xShift, yShift,
		zShift, interpolation, computationBuffer));

	delete[] computationBuffer;

}





	Uint16SCVTiledReader::Uint16SCVTiledReader()
	{
		_TopoTree = new imt::volume::VolumeTopoOctree();
	}


	void Uint16SCVTiledReader::setVolumeInfo(int w, int h, int d, float voxelSizeX, float voxelSizeY, float voxelSizeZ, int bitDepth, int rawHeaderOffset)
	{
		_Header.volumeSizeX = w;
		_Header.volumeSizeY = h;
		_Header.volumeSizeZ = d;
		_Header.voxelSizeX = voxelSizeX;
		_Header.voxelSizeY = voxelSizeY;
		_Header.voxelSizeZ = voxelSizeZ;
		_Header._TileDim = 256;
		_Header.numBitsPerVoxel = bitDepth;

		_RawFileSkipSize = rawHeaderOffset;

		imt::volume::VolumeTopoOctree::Vec3i dim;

		dim._X = _Header.volumeSizeX;
		dim._Y = _Header.volumeSizeY;
		dim._Z = _Header.volumeSizeZ;

		int minDim = INT_MAX;

		int maxDim = 0;

		minDim = std::min(minDim, _Header.volumeSizeX);
		minDim = std::min(minDim, _Header.volumeSizeY);
		minDim = std::min(minDim, _Header.volumeSizeZ);

		maxDim = std::max(maxDim, _Header.volumeSizeX);
		maxDim = std::max(maxDim, _Header.volumeSizeY);
		maxDim = std::max(maxDim, _Header.volumeSizeZ);


		if (minDim > 256)
		{
			_Header._TileDim = 256;
		}
		else if (minDim > 128)
		{
			_Header._TileDim = 128;
		}
		else
		{
			_Header._TileDim = 64;
		}

		if (maxDim <= 512)
		{
			_Header._TileDim = std::min(_Header._TileDim, 128);
		}


		_TopoTree->init(dim, _Header._TileDim, 0);
	
	}


	double Uint16SCVTiledReader::computeOptimalResizeFactor(double memoryCap)
	{
		double optimalResizeFactor = 1.0;

		double step = 0.001;

		SoLDMTopoOctree topoTree;

		for (int ii = 0; ii < 1000; ii++)
		{
			double currentResizeFactor = 1.0 - ii * step;
		
			unsigned int d1 = _Header.volumeSizeX * currentResizeFactor;
			unsigned int d2 = _Header.volumeSizeY * currentResizeFactor;
			unsigned int d3 = _Header.volumeSizeZ * currentResizeFactor;

			int tileDim = 256;

			double tileMemory = tileDim * tileDim * tileDim / (1024.0 * 1024.0);

			unsigned int resizedW = d1 % tileDim == 0 ? d1 : (d1 / tileDim + 1) * tileDim;
			unsigned int resizedH = d2 % tileDim == 0 ? d2 : (d2 / tileDim + 1) * tileDim;
			unsigned int resizedD = d3 % tileDim == 0 ? d3 : (d3 / tileDim + 1) * tileDim;

			SbVec3i32 volumeSize( resizedW , resizedH , resizedD );

			topoTree.init(volumeSize, tileDim);

			unsigned int numFileIds = topoTree.getNumFileIDs();

			double memoryNeeded = numFileIds * tileMemory;

			if (memoryNeeded < memoryCap)
			{
				optimalResizeFactor = currentResizeFactor;
			}

		}

		return optimalResizeFactor;	
	}



	struct ImageDataReader
	{

		FILE *_RawDataFile;

		//std::queue< ImageResizer >& _ResizeJobQ;


	};


	struct ImageResizer {
		
		unsigned short *inputVolume, *outputVolume;

		int iWidth, iHeight, iDepth;
		int oWidth, oHeight, oDepth;

		double resizeFactor;
		
		
		void operator()()
		{
			resizeVolume_Uint16C1( inputVolume, iWidth, iHeight, iDepth, outputVolume,  oWidth, oHeight,  oDepth, resizeFactor);		
		}
	};

	
	
	void Uint16SCVTiledReader::init( std::wstring , size_t memoryCap )
	{
		
		double optimalResizeFactor = computeOptimalResizeFactor( memoryCap );

		unsigned int rw = _Header.volumeSizeX * optimalResizeFactor;
		unsigned int rh = _Header.volumeSizeY * optimalResizeFactor;
		unsigned int rd = _Header.volumeSizeZ * optimalResizeFactor;

		unsigned short *resizedVolumePtr = new unsigned short[ rw * rh * rd ];

		int bleedBoundaryLength = (int)(3 / optimalResizeFactor) + 1;

		int bandWidth = bleedBoundaryLength * 20;

		double resizeMemoryUsage = 8192;

		double initVolumeSize = _Header.volumeSizeX * _Header.volumeSizeY * _Header.volumeSizeZ * sizeof( unsigned short ) / ( 1024.0 * 1024.0 );

		int numThreads = omp_get_num_procs();

		int numThreadsForProcessing = numThreads - 2;

		std::queue< ImageResizer > resizeJobQ;

		if ( resizeMemoryUsage > initVolumeSize )
		{

			int numStepts = _Header.volumeSizeZ / bandWidth;

			unsigned short *intermediateBuffer = new unsigned short[rw * rh * (bandWidth + 2 * bleedBoundaryLength)];

			memset(intermediateBuffer, 0, rw * rh * (bandWidth + 2 * bleedBoundaryLength) * sizeof(unsigned short));

			int resizedBandWidth;
			int stripWidth, stripHeight;
			int xShift;

			//run out of core method to resize the volume
			//we need to overlap reading and resizing for faster performance
			for ( int ss = 0; ss < numStepts; ss++ )
			{
				if ( ss == 0 )
				{
					resizeVolume_Uint16C1( intermediateBuffer, stripWidth, stripHeight, bandWidth, 
						                   resizedVolumePtr, rw, rh, resizedBandWidth , optimalResizeFactor );
				}
				else
				{

				}
			}

		}
		else
		{
			//run in core method to resize
		}


	}


	void Uint16SCVTiledReader::resizeVolumeMT()
	{
	    
		
	
	}



