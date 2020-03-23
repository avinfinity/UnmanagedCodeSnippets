#include "vtkImageData.h"
#include "vtkMarchingCubes.h"
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
#include "overlappedvoxelsegmentation.h"
#include <vtkAppendPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkInteractorStyleTrackballCamera.h>

#include "volumeanalytics.h"

void displayPolyData(vtkSmartPointer< vtkPolyData > mesh);
void viewIsoSurface(imt::volume::VolumeAnalytics::VolumeData& volume, int isoVal);

int main(int argc, char** argv)
{

	std::string volumeFilePath = "C:\\Data\\Kupplung_4GB\\Test_Kupplung_4GB_Vx41 2015-6-2 13-46.uint16_scv";//"G:\\Data\\separated_part_0.uint16_scv";

	imt::volume::VolumeAnalytics volumeAnalytics;

	imt::volume::VolumeAnalytics::VolumeData fullvolume , volume;

	volumeAnalytics.readVolumeData(volumeFilePath, fullvolume);

	volumeAnalytics.reduceVolume(fullvolume, volume, 0.5);

	int isoThreshold = volumeAnalytics.computeIsoThreshold(volume);

	viewIsoSurface(volume, isoThreshold);

	return 0;
}


void displayPolyData(vtkSmartPointer< vtkPolyData > mesh)
{
	vtkSmartPointer< vtkRenderer > renderer = vtkSmartPointer< vtkRenderer >::New();

	vtkSmartPointer< vtkPolyDataMapper > meshMapper = vtkSmartPointer< vtkPolyDataMapper >::New();

	meshMapper->SetInputData(mesh);

	vtkSmartPointer< vtkActor > meshActor = vtkSmartPointer< vtkActor >::New();

	meshActor->GetProperty()->SetColor(1.0, 0, 0);

	//meshActor->GetProperty()->BackfaceCullingOn();

	meshActor->SetMapper(meshMapper);

	renderer->AddActor(meshActor);

	//   meshActor->GetProperty()->SetAmbient(1.0);
	//   meshActor->GetProperty()->SetDiffuse(0.0);
	//   meshActor->GetProperty()->SetSpecular(0.0);

	vtkSmartPointer< vtkRenderWindow > window = vtkSmartPointer< vtkRenderWindow >::New();
	vtkSmartPointer< vtkRenderWindowInteractor > interactor = vtkSmartPointer< vtkRenderWindowInteractor >::New();
	vtkSmartPointer< vtkInteractorStyleTrackballCamera > style = vtkSmartPointer< vtkInteractorStyleTrackballCamera >::New();

	window->AddRenderer(renderer);
	window->SetInteractor(interactor);
	interactor->SetInteractorStyle(style);

	window->Render();

	interactor->Start();
}





void viewIsoSurface(imt::volume::VolumeAnalytics::VolumeData& volume , int isoVal )
{
	vtkSmartPointer< vtkImageData > volumeData = vtkSmartPointer< vtkImageData >::New();


	//volumeData->SetOrigin(volume.mVolumeOrigin(0), volume.mVolumeOrigin(1), volume.mVolumeOrigin(2));
	//volumeData->SetSpacing(volume.mVoxelStep(0), volume.mVoxelStep(1), volume.mVoxelStep(2));
	volumeData->SetDimensions(volume._Width, volume._Height, volume._Depth);
	volumeData->SetExtent(0, volume._Width - 1, 0, volume._Height - 1, 0, volume._Depth - 1);
	volumeData->AllocateScalars(VTK_UNSIGNED_SHORT, 1);

	long int nPoints = volumeData->GetNumberOfPoints();


	long int zStep = volume._Height * volume._Width;
	long int yStep = volume._Width;

	unsigned char *ptr = (unsigned char *)volumeData->GetScalarPointer();

	memcpy(ptr, volume._VolumeBuffer, volume._Width * volume._Height * volume._Depth * 2);

	vtkSmartPointer< vtkMarchingCubes > marchingCubes = vtkSmartPointer< vtkMarchingCubes >::New();

	marchingCubes->SetInputData(volumeData);

	marchingCubes->SetValue(0, isoVal);

	marchingCubes->ComputeNormalsOn();

	marchingCubes->Update(0);

	vtkSmartPointer< vtkPolyData > isoSurface = vtkSmartPointer< vtkPolyData >::New();

	isoSurface->DeepCopy(marchingCubes->GetOutput());

	displayPolyData(isoSurface);
}
