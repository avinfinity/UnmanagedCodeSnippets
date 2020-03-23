
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
#include "meshviewer.h"

//#include "ipp.h"



#include "stdio.h"
#include "stdlib.h"


void createArrow(Eigen::Vector3f& origin, Eigen::Vector3f dir, float length, float shaftRadius, float arrowRadius,
	int shaftResolution,  std::vector< Eigen::Vector3f >& points, std::vector< unsigned int >& faceIndices);
void viewIsoSurface(imt::volume::VolumeInfo& volume, int isoVal);


int main(int argc, char **argv)
{
	//viewCopiedImage();

	//	return 0;

	//engineDataIsoSurfaceDemo();

	//return 0;


	QString filePath = "C:/projects/Wallthickness/data/bottle/bottle.wtadat";//"C:/projects/Wallthickness/data/fanuc_engine_cover/engine_cover.wtadat";//	

	QString fileName =

		//"C:/Data/CAD, Object data/Aluminium_1";
		//"C:\\Data\\VolumeNotLoading\\sphere_simulation_noScat_noBH_50kV_1ray";
		//"C:\\Projects\\Wallthickness\\data\\Datasets for Multi-material\\MultiMaterial\\MAR_Ref 2012-8-30 15-13";

		//"C:\\Projects\\Wallthickness\\data\\CT multi\\Hemanth_ Sugar free 2016-6-6 11-33"; //
		//"C:/Projects/Wallthickness/data/separated_part_7";
		// "C:/Data/ZxoData/T2_R2_BSC_M800_BHC_02_03";//"C:/Data/WallthicknessData/10.03.17/Mobile cover_Static 2017-3-10 12-44";//"C:/Data/WallthicknessData/11.1.17/Bizerba_2 2016-8-25 17-50-31";//"C:/Data/WallthicknessData/11.1.17/Blister_Single 2016-10-19 8-55";// "C:/projects/Wallthickness/data/CT multi/Calliper Assy 2016-6-9 10-12";
		//"C:/projects/Wallthickness/data/Datasets for Multi-material/MultiMaterial/MAR_Ref 2012-8-30 15-13"; 
		//"C:/projects/Wallthickness/data/Mobile charger/Wall Thick_RnD_1 2016-10-13 15-10_sep1";
		//"C:/projects/Wallthickness/data/Mobile charger/Wall Thick_RnD_1 2016-10-13 15-10_sep1"; 
		////""C:/projects/datasets/Bitron/separated_part_0";
		//"C:/projects/Wallthickness/data/CT multi/Hemanth_ Sugar free 2016-6-6 11-33";
		//"C: / projects / Wallthickness / data / CT multi / Fuel_filter_0km_PR_1 2016 - 5 - 30 12 - 54";//;
		//"C:/projects/Wallthickness/data/Mobile charger/Wall Thick_RnD_1 2016-10-13 15-10_sep1";
		"C:/Projects/Data/separated_part_0";
	//"C:/projects/datasets/Bitron/separated_part_7";
	//"C:/projects/datasets/bug_38950/Sample H wo frame_1 2012-9-10 6-27";//
	QString scvFilePath = fileName + ".uint16_scv";
	QString vgiFilePath = fileName + ".vgi";
	QString stlFilePath = "C:/projects/Wallthickness/data/Mobile _ Charger.stl";

	//viewSTLFile(stlFilePath);

	//return 0;

	int w, h, d;

	float voxelStep;

	imt::volume::VolumeInfo volume;

	//readVGI(vgiFilePath , w , h , d , voxelStep );



	//readUint16_SCV(scvFilePath , w , h , d , volume );

	imt::volume::RawVolumeDataIO::readUint16SCV(scvFilePath, volume);

	std::cout << " air voxel filter threshold : " << volume.mAirVoxelFilterVal << std::endl;


	imt::volume::HistogramFilter filter(&volume);

	std::vector< long > histogram(std::numeric_limits< unsigned short >::max(), 0);

	unsigned short *vData = (unsigned short*)volume.mVolumeData;

	for (int zz = 0; zz < volume.mDepth; zz++)
		for (int yy = 0; yy < volume.mHeight; yy++)
			for (int xx = 0; xx < volume.mWidth; xx++)
			{
				histogram[vData[zz * volume.mWidth * volume.mHeight + yy * volume.mWidth + xx]] += 1;
			}


	long maxHistogramVal = *(std::max_element(histogram.begin(), histogram.end()));

	//tr::Display3DRoutines::displayHistogram(histogram, 0, maxHistogramVal);
	//filter.plotHistogram(histogram);


	long iso50Th = filter.ISO50Threshold(histogram);
	long otsuTh = filter.OtsuThreshold(histogram);

	//viewIsoSurface(volume, iso50Th);


	std::cout << " iso 50 threshold : " << iso50Th << std::endl;
	std::cout << " otsu threshold : " << otsuTh << std::endl;
	std::cout << " volume graphics threshold : " << volume.mAirVoxelFilterVal << std::endl;

	//setup the volume

	Volume vol;

	vol.size[0] = volume.mWidth;
	vol.size[1] = volume.mHeight;
	vol.size[2] = volume.mDepth;

	vol.voxel_size[0] = volume.mVoxelStep(0);
	vol.voxel_size[1] = volume.mVoxelStep(1);
	vol.voxel_size[2] = volume.mVoxelStep(2);

	vol.data = (uint16_t*)volume.mVolumeData;

	std::cout << volume.mWidth << " " << volume.mHeight << " " << volume.mDepth << std::endl;

	std::cout << volume.mWidth << " " << volume.mHeight << " " << volume.mDepth << std::endl;


	ZeissSegmentationInterface segmenter;

	SegmentationParameter param;
	param.max_memory = static_cast<size_t>(2048) * 1024 * 1024;
	param.output_material_index = false;
	segmenter.setParameter(param);

	segmenter.setInputVolume(vol);

	Materials materials = segmenter.getMaterialRegions();

	std::cout << materials.regions[0].lower_bound << " " << materials.regions[1].lower_bound << " " << materials.regions[1].upper_bound << std::endl;

	viewIsoSurface(volume, materials.regions[1].lower_bound);

	return 0;

}


void viewIsoSurface(imt::volume::VolumeInfo& volume, int isoVal)
{
	vtkSmartPointer< vtkImageData > volumeData = vtkSmartPointer< vtkImageData >::New();

	volumeData->SetOrigin(volume.mVolumeOrigin(0), volume.mVolumeOrigin(1), volume.mVolumeOrigin(2));
	volumeData->SetSpacing(volume.mVoxelStep(0), volume.mVoxelStep(1), volume.mVoxelStep(2));
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

	//tr::Display3DRoutines::displayPolyData(isoSurface);

	std::vector< Eigen::Vector3f > vertices, normals;
	std::vector< unsigned int > faceIndices;

	vertices.resize(isoSurface->GetNumberOfPoints());
	normals.resize(isoSurface->GetNumberOfPoints());
	faceIndices.resize(isoSurface->GetNumberOfCells() * 3);

	vtkSmartPointer< vtkPoints > pointsVtk = isoSurface->GetPoints();

	size_t numFaces = isoSurface->GetNumberOfCells();
	size_t numPoints = isoSurface->GetNumberOfPoints();

	vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();

	for (size_t ff = 0; ff < numFaces; ff++)
	{
		triangle->Reset();

		isoSurface->GetCellPoints(ff, triangle);

		faceIndices[ 3 * ff] = triangle->GetId(0);
		faceIndices[3 * ff + 1] = triangle->GetId(1);
		faceIndices[3 * ff + 2] = triangle->GetId(2);
	}

	for (size_t pp = 0; pp < numPoints; pp++)
	{
		double pt[3];
		pointsVtk->GetPoint(pp, pt);

		vertices[pp](0) = pt[0];
		vertices[pp](1) = pt[1];
		vertices[pp](2) = pt[2];
	}

	vc::MESHViewer::viewMesh(vertices, faceIndices, &volume);

	//tr::Display3DRoutines::displayMesh(vertices, faceIndices);

	//std::vector< Eigen::Vector3f > arrowPoints;
	//std::vector< unsigned int > arrowFaceIndices;

	//Eigen::Vector3f dir(1, 0, 0);
	//createArrow(vertices[0], dir, 1, 0.2 , 0.3 , 10 , arrowPoints , arrowFaceIndices);
	//vc::MESHViewer::viewMesh(arrowPoints, arrowFaceIndices);

}


