
#include <iostream>
#include "multimaterialsurfaceextractor.h"
#include "rawvolumedataio.h"
#include "src/plugins/Application/ZeissViewer/SegmentationInterface/ZeissSegmentationInterface.hpp"
#include "src/plugins/Application/ZeissViewer/SeparationInterface/ZeissSeparationInterface.hpp"
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
#include "gradientbasedsurfaceextractor.h"

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

	displayPolyData(isoSurface);
}



int main( int argc , char **argv )
{


	std::cout << " multi material surface extraction demo " << std::endl;


	QString volumeDataPath = "E:\\Projects\\DataSets\\separated_part_0.uint16_scv";
	//"C:\\Data\\Kupplung_4GB\\Test_Kupplung_4GB_Vx41 2015-6-2 13-46.uint16_scv";// 
	//"G:/Data/Multimaterial/T2_R2_BSC_M800_BHC_02_03.uint16_scv"; 
	//"G:\\Projects\\Wallthickness\\data\\Mobile charger\\Wall Thick_RnD_1 2016-10-13 15-10_sep1.uint16_scv";// "Fuel FlangemP375mCavity01.uint16mscv";//

	int w, h, d;

	float voxelStep;

	imt::volume::VolumeInfo volume;

	std::cout << "uint16scv file path : " << volumeDataPath.toStdString() << std::endl;

	imt::volume::RawVolumeDataIO::readUint16SCV(volumeDataPath, volume);

	Volume vol;

	vol.size[0] = volume.mWidth;
	vol.size[1] = volume.mHeight;
	vol.size[2] = volume.mDepth;

	vol.voxel_size[0] = volume.mVoxelStep(0);
	vol.voxel_size[1] = volume.mVoxelStep(1);
	vol.voxel_size[2] = volume.mVoxelStep(2);

	vol.data = (uint16_t*)volume.mVolumeData;

	std::vector<imt::volume::MultiMaterialSurfaceExtractor::MaterialRegion> materialRegions;

#if 1
	ZeissSegmentationInterface segmenter;

	SegmentationParameter param;
	param.max_memory = static_cast<size_t>(2048) * 1024 * 1024;
	param.output_material_index = false;
	segmenter.setParameter(param);
	
	segmenter.setInputVolume(vol);

	Materials materials = segmenter.getMaterialRegions();

	int numMaterialRegions = materials.regions.size();

	//numMaterialRegions = 2;

	materialRegions.resize(numMaterialRegions);

	for ( int mm = 0; mm < numMaterialRegions; mm++ )
	{
		materialRegions[mm]._StartValue = materials.regions[mm].lower_bound;
		materialRegions[mm]._EndValue = materials.regions[mm].upper_bound;
	}

#endif
	//std::cout << "material value : " << materialRegions[1]._StartValue << std::endl;

	//materialRegions.resize(1);

	int start = 9000;
	int end = 51000;//13000;////13000;//

	//materialRegions[0]._StartValue = start;
	//materialRegions[0]._EndValue = end;

	//int isoValue = (start + end) * 0.5; //10880;//materialRegions[1]._StartValue;


	//viewIsoSurface(volume, isoValue);

	std::cout << "number of materials : " << materialRegions.size() << std::endl;

	

	imt::volume::MultiMaterialSurfaceExtractor extractor( volume , materialRegions ) ;

	std::vector< Eigen::Vector3f > initialPoints, initialNormals;

	imt::volume::VolumeOctree vo(volume.mWidth, volume.mHeight, volume.mDepth, initialPoints, initialNormals);

	extractor.compute( start , end , materialRegions[1]._StartValue );





	//imt::volume::GradientBasedSurfaceExtractor isoSurfaceExtractor((unsigned short*)volume.mVolumeData, volume.mWidth,
	//	volume.mHeight, volume.mDepth, volume.mVoxelStep(0), volume.mVoxelStep(1), volume.mVoxelStep(2));

	//std::vector<double> vertices, normals;

	//std::vector<unsigned int> surfaceIndices;

	//isoSurfaceExtractor.compute(start, end, vertices, normals, surfaceIndices);




	
	return 0;
}


