

#include "iostream"
#include "src/plugins/Application/ZeissViewer/SegmentationInterface/ZeissSegmentationInterface.hpp"
#include "src/plugins/Application/ZeissViewer/SeparationInterface/ZeissSeparationInterface.hpp"
#include "QFile"
#include "iostream"
#include "wallthicknessestimator.h"
#include "QFile"
#include "QDataStream"
#include "QTextStream"
#include "QByteArray"
#include "volumeinfo.h"
#include "display3droutines.h"
#include "vtkImageData.h"
#include "vtkMarchingCubes.h"
#include "histogramfilter.h"
#include "QDebug"
#include "volumesegmenter.h"
#include "vtkSTLReader.h"
#include <vtkColorTransferFunction.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolumeProperty.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPLYWriter.h>
#include "wallthicknessestimator.h"
#include "rawvolumedataio.h"
#include "overlappedvoxelsegmentation.h"
#include <vtkAppendPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkInteractorStyleTrackballCamera.h>

//#include "ipp.h"



#include "stdio.h"
#include "stdlib.h"
#include "ipp.h"


static void HandleIppError(IppStatus err,
	const char *file,
	int line) {
	if (err != ippStsNoErr) {
		printf("IPP Error code %d in %s at line %d %s\n", err,
			file, line, ippGetStatusString(err));
		exit(EXIT_FAILURE);
	}
}

#define ippSafeCall( err ) (HandleIppError( err, __FILE__, __LINE__ ))


void resizeVolume_Uint16C1(unsigned short *inputVolume, int iWidth, int iHeight, int iDepth, unsigned short *outputVolume, int oWidth, int oHeight, int oDepth, double resizeRatio)
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

	ippSafeCall(ipprResizeGetBufSize(srcVoi, dstVoi, 1, interpolation, &bufferSize));

	computationBuffer = new Ipp8u[bufferSize];


	ippSafeCall(ipprResize_16u_C1V(inputVolume, inputVolumeSize, srcStep, srcPlaneStep, srcVoi, outputVolume, dstStep,
		dstPlaneStep, dstVoi, xFactor, yFactor, zFactor, xShift, yShift,
		zShift, interpolation, computationBuffer));

}


void resizeVolume_Uint16C1_MT(unsigned short *inputVolume, int iWidth, int iHeight, int iDepth, unsigned short *outputVolume, int oWidth,
	int oHeight, int oDepth, double resizeRatio)
{
	int bandWidth = 32;

	int numBands = oDepth % 32 == 0 ? oDepth / bandWidth : oDepth / bandWidth + 1;

	int extension = (1.0 / resizeRatio) + 1;

	if (numBands > 1)
	{
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

			ippSafeCall(ipprResizeGetBufSize(srcVoi, dstVoi, 1, interpolation, &bufferSize));

			computationBuffer = new Ipp8u[bufferSize];

			ippSafeCall(ipprResize_16u_C1V(inputVolume + sourceDataShift, inputVolumeSize, srcStep, srcPlaneStep,
				srcVoi, outputVolume + destDataShift, dstStep, dstPlaneStep, dstVoi,
				xFactor, yFactor, zFactor, xShift, yShift, zShift, interpolation, computationBuffer));

			delete[] computationBuffer;
		}
	}
	else
	{
		resizeVolume_Uint16C1(inputVolume, iWidth, iHeight, iDepth, outputVolume, oWidth, oHeight, oDepth, resizeRatio);
	}

}


void viewIsoSurface(imt::volume::VolumeInfo& volume, int isoVal);

int main(int argc, char** argv)
{
	QString filePath =
		//"G:\\Data\\CAD, Object data\\tube.uint16_scv";
		//"G:\\Data\\CAD, Object data\\Aluminium_1.uint16_scv";
		"G:\\Data\\VolumeVisualizationBug\\fEDER_CLIP.uint16_scv";
		"G:\\Data\\ZxoData\\07.06.17\\Test Pice 2017-9-7 10-16.uint16_scv";
		//"G:\\Data\\Kupplung_4GB\\Test_Kupplung_4GB_Vx41 2015-6-2 13-46.uint16_scv";

	imt::volume::VolumeInfo volume , resizedVolume;

	imt::volume::RawVolumeDataIO::readUint16SCV(filePath, volume);

	imt::volume::HistogramFilter filter(&volume);

	//filter.ISO50Threshold(  )

	double resizeCoefficient = 0.4;


	resizedVolume.mWidth = volume.mWidth * resizeCoefficient;
	resizedVolume.mHeight = volume.mHeight * resizeCoefficient;
	resizedVolume.mDepth = volume.mDepth * resizeCoefficient;

	resizedVolume.mVoxelStep = volume.mVoxelStep / resizeCoefficient;

	resizedVolume.mVolumeData = (unsigned char*)(new unsigned short[(int64_t)resizedVolume.mWidth * (int64_t)resizedVolume.mHeight * (int64_t)resizedVolume.mDepth] );

	resizeVolume_Uint16C1_MT((unsigned short*)volume.mVolumeData, volume.mWidth, volume.mHeight, volume.mDepth , 
		(unsigned short*)resizedVolume.mVolumeData, resizedVolume.mWidth, resizedVolume.mHeight, resizedVolume.mDepth, resizeCoefficient);

	std::vector< int64_t > histogram(std::numeric_limits< unsigned short >::max(), 0) , resizedHistogram(std::numeric_limits< unsigned short >::max(), 0);

	unsigned short *vResizedData = (unsigned short*)resizedVolume.mVolumeData;

	for (int zz = 0; zz < resizedVolume.mDepth; zz++)
		for (int yy = 0; yy < resizedVolume.mHeight; yy++)
			for (int xx = 0; xx < resizedVolume.mWidth; xx++)
			{
				resizedHistogram[vResizedData[zz * resizedVolume.mWidth * resizedVolume.mHeight + yy * resizedVolume.mWidth + xx]] += 1;
			}


	unsigned short *vData = (unsigned short*)volume.mVolumeData;

	for (int zz = 0; zz < volume.mDepth; zz++)
		for (int yy = 0; yy < volume.mHeight; yy++)
			for (int xx = 0; xx < volume.mWidth; xx++)
			{
				histogram[vData[zz * volume.mWidth * volume.mHeight + yy * volume.mWidth + xx]] += 1;
			}


	int isoValue = filter.ISO50Threshold(histogram);
	int isoValueResized = filter.ISO50Threshold(resizedHistogram);

	int isoValueFraunhoffer = filter.fraunhoufferThreshold(resizedVolume.mWidth, resizedVolume.mHeight, resizedVolume.mDepth, resizedVolume.mVoxelStep(0) , 
		resizedVolume.mVoxelStep(1) , resizedVolume.mVoxelStep(2), (unsigned short*)resizedVolume.mVolumeData);

	//int isoValueFraunhoffer = filter.fraunhoufferThreshold(volume.mWidth, volume.mHeight, volume.mDepth, volume.mVoxelStep(0),
	//	volume.mVoxelStep(1), volume.mVoxelStep(2), (unsigned short*)volume.mVolumeData);

	std::cout << "iso value and resized iso value : " << isoValue << " " << isoValueResized <<" "<<isoValueFraunhoffer<< std::endl;
	 
	//viewIsoSurface(resizedVolume, isoValue);
	viewIsoSurface(volume, isoValueFraunhoffer);


	return 0;
}



void viewIsoSurface(imt::volume::VolumeInfo& volume, int isoVal)
{
	vtkSmartPointer< vtkImageData > volumeData = vtkSmartPointer< vtkImageData >::New();


	//volumeData->SetOrigin(volume.mVolumeOrigin(0), volume.mVolumeOrigin(1), volume.mVolumeOrigin(2));
	//volumeData->SetSpacing(volume.mVoxelStep(0), volume.mVoxelStep(1), volume.mVoxelStep(2));
	volumeData->SetDimensions(volume.mWidth, volume.mHeight, volume.mDepth);
	volumeData->SetExtent(0, volume.mWidth - 1, 0, volume.mHeight - 1, 0, volume.mDepth - 1);
	volumeData->AllocateScalars(VTK_UNSIGNED_SHORT, 1);

	long int nPoints = volumeData->GetNumberOfPoints();


	long int zStep = volume.mHeight * volume.mWidth;
	long int yStep = volume.mWidth;

	unsigned char *ptr = (unsigned char *)volumeData->GetScalarPointer();

	memcpy(ptr, volume.mVolumeData, volume.mWidth * volume.mHeight * volume.mDepth * 2);

	vtkSmartPointer< vtkMarchingCubes > marchingCubes = vtkSmartPointer< vtkMarchingCubes >::New();

	marchingCubes->SetInputData(volumeData);

	marchingCubes->SetValue(0, isoVal);

	marchingCubes->ComputeNormalsOn();

	marchingCubes->Update(0);

	vtkSmartPointer< vtkPolyData > isoSurface = vtkSmartPointer< vtkPolyData >::New();

	isoSurface->DeepCopy(marchingCubes->GetOutput());

	tr::Display3DRoutines::displayPolyData(isoSurface);

}

