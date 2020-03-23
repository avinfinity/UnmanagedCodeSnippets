

#include "iostream"
#include <pcl/common/PCLPointCloud2.h>
#include <pcl/surface/marching_cubes_hoppe.h>
#include <pcl/surface/marching_cubes_rbf.h>
#include "wallthicknessestimator.h"
#include "volumeinfo.h"
#include "rawvolumedataio.h"
#include "iostream"
#include "vtkImageMarchingCubes.h"
#include "vtkSmartPointer.h"
#include "vtkPolyData.h"
#include "vtkImageData.h"
#include <vtkStructuredGrid.h>
#include "vtkPointData.h"
#include "display3droutines.h"
#include "vtkMarchingCubes.h"
#include "vtkWindowedSincPolyDataFilter.h"
#include "poissonmeshgenerator.h"
#include <vtkSmoothPolyDataFilter.h>
#include "vtkPolyDataNormals.h"
#include "vtkPLYReader.h"
#include "vtkPolyDataNormals.h"
#include <vtkImageImport.h>
#include <vtkDecimatePro.h>
#include <vtkQuadricClustering.h>
#include <vtkQuadricClustering.h>
//#include <vtkQuadricDecimationBoundary.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkCommand.h>
#include <vtkCleanPolyData.h>
#include <vtkAppendPolyData.h>
#include "FullBlockVolumeDataProvider.h"
#include <algorithm>
// #include ""

using namespace pcl;

float default_iso_level = 0.0f;
int default_hoppe_or_rbf = 0;
float default_extend_percentage = 0.0f;
int default_grid_res = 50;
float default_off_surface_displacement = 0.01f;




void writeMeshData(std::string filePath, std::vector< Eigen::Vector3f >& vertices, std::vector< Eigen::Vector3f >& normals, std::vector< unsigned int >& indices)
{

	std::fstream writer;

	writer.open(filePath, std::ios::binary | std::ios::out);

	int numVertices = vertices.size();
	int numFaces = indices.size() / 3;

	writer.write((char*)&numVertices, sizeof(int));
	writer.write((char*)&numFaces, sizeof(int));

	vertices.resize(numVertices * 3);
	normals.resize(numVertices * 3);
	indices.resize(numFaces * 3);

	writer.write((char*)vertices.data(), numVertices * 3 * sizeof(float));
	writer.write((char*)normals.data(), numVertices * 3 * sizeof(float));
	writer.write((char*)indices.data(), sizeof(unsigned int) * 3 * numFaces);

	writer.close();
}



void convertToVertexAndIndices(vtkSmartPointer< vtkPolyData > mesh, std::vector< Eigen::Vector3f >& vertices, std::vector< Eigen::Vector3f >& normals, std::vector< unsigned int >& indices)
{
	vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();

	vertices.resize(mesh->GetNumberOfPoints());
	normals.resize(mesh->GetNumberOfPoints());
	indices.resize(mesh->GetNumberOfCells() * 3);

	int nPoints = mesh->GetNumberOfPoints();

	vtkSmartPointer< vtkDataArray > vtknormals = mesh->GetPointData()->GetNormals();

	for (int pp = 0; pp < nPoints; pp++)
	{

		double pt[3];

		mesh->GetPoint(pp, pt);

		vertices[pp](0) = pt[0];
		vertices[pp](1) = pt[1];
		vertices[pp](2) = pt[2];


		double n[3];

		vtknormals->GetTuple(pp, n);

		normals[pp](0) = n[0];
		normals[pp](1) = n[1];
		normals[pp](2) = n[2];

	}


	int numTriangles = mesh->GetNumberOfCells();

	for (int cc = 0; cc < numTriangles; cc++)
	{
		mesh->GetCellPoints(cc, triangle);

		indices[3 * cc] = triangle->GetId(0);
		indices[3 * cc + 1] = triangle->GetId(1);
		indices[3 * cc + 2] = triangle->GetId(2);
	}



}


void viewMesh(std::string meshPath)
{
	vtkSmartPointer< vtkPLYReader > reader = vtkSmartPointer< vtkPLYReader >::New();

	reader->SetFileName(meshPath.c_str());
	reader->Update();

	vtkSmartPointer< vtkPolyData > meshData = vtkSmartPointer< vtkPolyData >::New();

	meshData->DeepCopy(reader->GetOutput());

	std::vector< Eigen::Vector3f > vertices, normals;
	std::vector< unsigned int > meshIndices;

	vtkSmartPointer< vtkPolyDataNormals > normalEstimator = vtkSmartPointer< vtkPolyDataNormals >::New();

	normalEstimator->SetInputData(meshData);

	normalEstimator->ComputePointNormalsOn();
	normalEstimator->ComputeCellNormalsOn();

	normalEstimator->Update();

	meshData->DeepCopy(normalEstimator->GetOutput());

	convertToVertexAndIndices(meshData, vertices, normals, meshIndices);

	std::cout << " size of vertices and normals : " << vertices.size() << " " << normals.size() << std::endl;

	writeMeshData("C:\\Projects\\Wallthickness\\data\\msp.meshdata" , vertices , normals , meshIndices);


	tr::Display3DRoutines::displayPolyData(meshData);



}


void loadAndView(std::string filePath)
{
	FILE *file = fopen(filePath.c_str(), "rb");

	int size = 4977072 / 8;

	std::vector< double > data(size);

	fread(data.data(), 8, size, file);

	std::vector< Eigen::Vector3f > vertices(size / 3);

	for (int ss = 0; ss < size / 9; ss++)
	{
		vertices[3 * ss](0) = data[9 * ss];
		vertices[3 * ss + 1](0) = data[9 * ss + 1];
		vertices[3 * ss + 2](0) = data[9 * ss + 2];


		vertices[3 * ss](1) = data[9 * ss + 3];
		vertices[3 * ss + 1](1) = data[9 * ss + 4];
		vertices[3 * ss + 2](1) = data[9 * ss + 5];

		vertices[3 * ss ](2) = data[9 * ss + 6];
		vertices[3 * ss + 1](2) = data[9 * ss + 7];
		vertices[3 * ss + 2](2) = data[9 * ss + 8];

	}

	std::vector< unsigned int > indices(size / 3);

	for (int ss = 0; ss < size / 3; ss++)
		indices[ss] = ss;


	tr::Display3DRoutines::displayMesh(vertices, std::vector< Eigen::Vector3f >(vertices.size(), Eigen::Vector3f(1, 1, 1)), indices);
}

unsigned int GeneratateForTriangleCountEstimation(FullBlockVolumeDataProvider* volumeDataProvider, imt::volume::VolumeInfo& volInfo, size_t isoThreshold);


int main( int argc , char **argv )
{

	vtkObject::GlobalWarningDisplayOff();
	
	QString volumeFilePath = "C:/Projects/Data/Test_Kupplung_1GB_Vx64µm 2015-6-2 9-10.uint16_scv";

	CPUBuffer cpuBuffer;

	imt::volume::VolumeInfo volumeInfo;

	//imt::volume::RawVolumeDataIO::readUint16SCV(volumeFilePath, volumeInfo, cpuBuffer);

	QString filePath = "C:\\Projects\\Data\\TestMeshUnClean.ply";

	vtkSmartPointer<vtkPLYReader> reader = vtkSmartPointer<vtkPLYReader>::New();

	reader->SetFileName(filePath.toStdString().c_str());

	reader->Update();

	vtkSmartPointer<vtkPolyData> inputMesh = reader->GetOutput();


	tr::Display3DRoutines::displayPolyData(inputMesh);

	vtkSmartPointer<vtkCleanPolyData> filter1 = vtkSmartPointer<vtkCleanPolyData>::New();

	filter1->SetInputData(inputMesh);

	filter1->PointMergingOn();

	filter1->SetTolerance(1.0 / 10000);

	filter1->Update();

	inputMesh = filter1->GetOutput();
	tr::Display3DRoutines::displayPolyData(inputMesh);

	vtkSmartPointer<vtkPolyDataConnectivityFilter> filter = vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();

	filter->SetInputData(inputMesh);

	filter->SetExtractionModeToLargestRegion();

	filter->ScalarConnectivityOn();

	filter->FullScalarConnectivityOff();

	filter->SetScalarRange(100.0, 100.0);

	filter->Update();

	int numRegions = filter->GetNumberOfExtractedRegions();

	std::cout << "num of extracted regions : " << numRegions << std::endl;

	vtkSmartPointer<vtkPolyData> outputMesh = filter->GetOutput();


	tr::Display3DRoutines::displayPolyData(outputMesh);


	return 0;
}

unsigned int GeneratateForTriangleCountEstimation(FullBlockVolumeDataProvider* volumeDataProvider , imt::volume::VolumeInfo& volInfo , size_t isoThreshold )//progress extractOnlyLargestSurface
{
	size_t refW = volInfo.mWidth;
	size_t refH = volInfo.mHeight;
	size_t refD = volInfo.mDepth;

	size_t volumeSize = refW * refH * refD / (1024 * 1024) * 2;

	size_t maxMemory = 1024;

	int targetLevel = 0;

	auto tempVolSize = volumeSize;

	unsigned int targetWidth = refW;
	unsigned int targetHeight = refH;
	unsigned int targetDepth = refD;

	while (tempVolSize > maxMemory)
	{
		tempVolSize /= 8;

		targetLevel++;

		targetWidth /= 2;
		targetHeight /= 2;
		targetDepth /= 2;
	}

	FullBlockVolumeDataProvider::RoiData roiData;

	roiData.Width = targetWidth;
	roiData.Height = targetWidth;
	roiData.Depth = targetWidth;

	roiData._Buffer = new CPUBuffer;
	roiData._Buffer->resize(roiData.Width * roiData.Height * roiData.Depth * sizeof(unsigned short));

	volumeDataProvider->GetRoiBuffer(roiData);

	int width = roiData.Width;
	int height = roiData.Height;
	int depth = roiData.Depth;


	// get access to the volume as pointer to data array
	unsigned short* data = static_cast<unsigned short*>(roiData._Buffer->data());

	vtkSmartPointer<vtkImageImport> import = vtkSmartPointer<vtkImageImport>::New();
	import->SetReleaseDataFlag(1);
	import->SetImportVoidPointer(static_cast<void*>(data));
	import->SetDataExtent(0, width - 1, 0, height - 1, 0, depth - 1);
	import->SetWholeExtent(0, width - 1, 0, height - 1, 0, depth - 1);
	import->SetDataSpacing(volInfo.mVoxelStep(0), volInfo.mVoxelStep(1), volInfo.mVoxelStep(2));
	import->SetDataOrigin(0.0, 0.0, 0.0);

	import->SetDataScalarTypeToUnsignedShort();

	//// generate iso surface
	vtkSmartPointer< vtkMarchingCubes > iso = vtkSmartPointer< vtkMarchingCubes >::New();
	iso->SetInputConnection(import->GetOutputPort());
	iso->SetValue(0, (double)isoThreshold);
	iso->ComputeNormalsOn();
	iso->ComputeGradientsOn();
	iso->ComputeScalarsOff();

	iso->Update();

	unsigned int count = (unsigned int)((unsigned int)iso->GetOutput()->GetNumberOfCells() * (1 << targetLevel) * (1 << targetLevel));

	roiData._Buffer->resize(0);

	return count;
}
