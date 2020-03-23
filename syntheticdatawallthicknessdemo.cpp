
#include "iostream"
#include "implicitsurfacegenerator.h"
#include "volumeinfo.h"
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

int main( int argc , char **argv )
{
	
	imt::volume::ImplicitFunction imf;

	std::vector< unsigned short > volumeDataV( 512 * 512 * 512 );

	int zStep = 512 * 512;
	int yStep = 512;

	for ( int zz = 0; zz < 512; zz++ )
		for ( int yy = 0; yy < 512; yy++ )
			for ( int xx = 0; xx < 512; xx++ )
			{
				volumeDataV[zStep * zz + yStep * yy + xx] = imf.value( xx , yy , zz );
			}


	

	imt::volume::VolumeInfo volInfo;

	volInfo.mVolumeData = (unsigned char*)volumeDataV.data();

	volInfo.mVolumeOrigin.setZero();
	
	volInfo.mWidth = 512;
	volInfo.mHeight = 512;
	volInfo.mDepth = 512;

	volInfo.mVoxelStep(0) = 1;
	volInfo.mVoxelStep(1) = 1;
	volInfo.mVoxelStep(2) = 1;

	int objTh = 5000;

	vtkSmartPointer< vtkImageData > volumeData = vtkSmartPointer< vtkImageData >::New();


	volumeData->SetOrigin(volInfo.mVolumeOrigin(0), volInfo.mVolumeOrigin(1), volInfo.mVolumeOrigin(2));
	volumeData->SetSpacing(volInfo.mVoxelStep(0), volInfo.mVoxelStep(1), volInfo.mVoxelStep(2));
	volumeData->SetDimensions(volInfo.mWidth, volInfo.mHeight, volInfo.mDepth);
	volumeData->SetExtent(0, volInfo.mWidth - 1, 0, volInfo.mHeight - 1, 0, volInfo.mDepth - 1);
	volumeData->AllocateScalars(VTK_UNSIGNED_SHORT, 1);

	unsigned char *ptr = (unsigned char *)volumeData->GetScalarPointer();

	memcpy(ptr, volInfo.mVolumeData, volInfo.mWidth * volInfo.mHeight * volInfo.mDepth * 2);

	vtkSmartPointer< vtkMarchingCubes > marchingCubes = vtkSmartPointer< vtkMarchingCubes >::New();

	marchingCubes->SetInputData(volumeData);

	marchingCubes->SetValue(0, objTh);

	marchingCubes->ComputeNormalsOn();

	marchingCubes->Update(0);

	vtkSmartPointer< vtkPolyData > isoSurface = vtkSmartPointer< vtkPolyData >::New();

	isoSurface->DeepCopy(marchingCubes->GetOutput());

	tr::Display3DRoutines::displayPolyData(isoSurface);


	
	return 0;
}