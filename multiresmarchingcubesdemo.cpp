#include <iostream>
#include "volumeinfo.h"
#include "rawvolumedataio.h"
#include "volumesegmenter.h"
#include "display3droutines.h"
#include "histogramfilter.h"
#include "marchingcubes.h"
#include "stdio.h"
#include <fstream>
#include "meshviewer.h"
#include "multiresmarchingcubes.h"
#include "MultiResolutionMarchingCubes.h"
#include "MultiResolutionMarchingCubes2.h"
#include <vtkFillHolesFilter.h>
#include <vtkPLYWriter.h>
#include "vtkCellData.h"
#include "ipp.h"
#include "ippvm.h"


void testNormals();



int main(int argc, char **argv)
{



	//testNormals();

	QString volumeFilePath = "C:\\Test Data\\MSP\\MSB_Demo part_BHC 2016-10-21 12-24.uint16_scv";
		
		// "C:\\Test Data\\BMWTestData\\ZK_B48B20_Test_9channel_101_defect-005-vol.uint16_scv";//"C:\\Test Data\\Multimaterial_Stecker 2018-10-15 16-7-27.uint16_scv";
		
		"E:\\Projects\\DataSets\\SymmetricPartAlignment\\Test_Kupplung_1GB_Vx64_2015-6-2 9-10.uint16_scv";//"E:/Projects/DataSets/separated_part_0.uint16_scv"; //"E:\\Projects\\DataSets\\cube.uint16_scv";//
	//"G:\\Data\\07.06.17\\Test Pice_BHC 2017-9-7 12-40.uint16_scv"; 
	//"G:\\Projects\\Data\\Connector\\Connector_1.uint16_scv"; 
	//"G:\\Projects\\Data\\Kupplung\\VV_Test_Kupplung_7GB_Vx31 2015-6-19 9-16.uint16_scv"; 
	//"G:\\Projects\\Data\\CADCube\\Test cube_10gb_BHC 2018-1-25 9-51-35.uint16_scv"; 
	//"C:/Data/Kupplung_4GB/Test_Kupplung_4GB_Vx41 2015-6-2 13-46.uint16_scv";
	//"G:\\Projects\\Data\\Connector\\Connector_1.uint16_scv";//
	//"G:\\Data\\07.06.17\\Test Pice_BHC 2017-9-7 12-40.uint16_scv";//
	//"G:/Projects/Data/CADCube_10GB/Test cube_10gb_BHC 2018-1-25 9-51-35.uint16_scv";
	//"G:\\Data\\CAD, Object data\\cube.uint16_scv"; //// ////;


	//first load the volume
	imt::volume::RawVolumeDataIO io;

	imt::volume::VolumeInfo volInfo;

	io.readUint16SCV(volumeFilePath, volInfo);

	volInfo.mZStep = volInfo.mWidth * volInfo.mHeight;
	volInfo.mYStep = volInfo.mWidth;

	volInfo.mVolumeDataU16 = (unsigned short*)volInfo.mVolumeData;

	imt::volume::HistogramFilter filter(&volInfo);

	std::vector< int64_t > histogram(std::numeric_limits< unsigned short >::max(), 0);

	unsigned short *vData = (unsigned short*)volInfo.mVolumeData;

	for (int64_t zz = 0; zz <volInfo.mDepth; zz++)
		for (int64_t yy = 0; yy < volInfo.mHeight; yy++)
			for (int64_t xx = 0; xx < volInfo.mWidth; xx++)
			{
				histogram[vData[zz * volInfo.mWidth * volInfo.mHeight + yy * volInfo.mWidth + xx]] += 1;
			}


	long maxHistogramVal = *(std::max_element(histogram.begin(), histogram.end()));

	//tr::Display3DRoutines::displayHistogram(histogram, 0, maxHistogramVal);
	//filter.plotHistogram(histogram);


long iso50Th = filter.ISO50Threshold(histogram); 	//fraunhoufferThreshold(volInfo.mWidth, volInfo.mHeight, volInfo.mDepth,
	//	volInfo.mVoxelStep(0), volInfo.mVoxelStep(1),
	//	volInfo.mVoxelStep(2), (unsigned short*)volInfo.mVolumeData);//


	std::cout << " iso 50 threshold : " << iso50Th << std::endl;

	double coeff = 1024.0 * 1024.0 * 1024.0 / ((double)volInfo.mWidth * (double)volInfo.mHeight * (double)volInfo.mDepth * 2.0);

	coeff = std::pow(coeff, 0.3333333);

	std::cout << " volume reduction coefficient : " << coeff << std::endl;

	imt::volume::MultiResMarchingCubes mrcm( volInfo , iso50Th , 2);

	//unsigned short* volumeData, int64_t volumeWidth, int64_t volumeHeight, int64_t volumeDepth,
	//	double voxelStepX, double voxelStepY, double voxelStepZ

	Zeiss::IMT::NG::NeoInsights::Volume::MeshGenerator::MultiResolutionMarchingCubes marchingCubes( (unsigned short*)volInfo.mVolumeData , volInfo.mWidth ,
		volInfo.mHeight , volInfo.mDepth , volInfo.mVoxelStep(0) , volInfo.mVoxelStep(1) , volInfo.mVoxelStep(2) );


	Zeiss::IMT::NG::NeoInsights::Volume::MeshGenerator::MultiResolutionMarchingCubes2 marchingCubes2((unsigned short*)volInfo.mVolumeData, volInfo.mWidth,
		volInfo.mHeight, volInfo.mDepth, volInfo.mVoxelStep(0), volInfo.mVoxelStep(1), volInfo.mVoxelStep(2));


	std::vector<double> vertices, vertexNormals;
	std::vector<unsigned int> surfaceIndices;

	

	//marchingCubes.compute( iso50Th, vertices, vertexNormals, surfaceIndices );

	//std::vector<std::size_t> indices_6_bit(surfaceIndices.size());

	//for (int ii = 0; ii < surfaceIndices.size(); ii++)
	//	indices_6_bit[ii] = surfaceIndices[ii];

	//QString path = "E:\\Projects\\DataSets\\modelfile_kupplung.dat";

	//FILE *cadFile;

	//fopen_s(&cadFile, path.toStdString().c_str(), "wb");

	//std::size_t nPositions = vertices.size() / 3;
	//std::size_t nIndices = surfaceIndices.size();// = facetData.Triangles().size();
	//fwrite(&nPositions, sizeof(std::size_t), 1, cadFile);
	//fwrite(&nIndices, sizeof(std::size_t), 1, cadFile);

	//

	//fwrite(vertices.data(), sizeof(double) * 3, nPositions, cadFile);
	//fwrite(vertexNormals.data(), sizeof(double) * 3, nPositions, cadFile);
	//fwrite(indices_6_bit.data(), sizeof(std::size_t), nIndices, cadFile);

	//fclose(cadFile);






	double initT = cv::getTickCount();

	//marchingCubes2.compute(iso50Th , vertices , vertexNormals , surfaceIndices);

	marchingCubes2.computeWithGridHierarchy(iso50Th, vertices, vertexNormals, surfaceIndices);

	std::vector< Zeiss::IMT::NG::NeoInsights::Volume::MeshGenerator::MultiResolutionMarchingCubes::ConnectedComponent > connectedComponents;

	//marchingCubes.generateConnectedComponents(iso50Th, connectedComponents);

	std::cout << "number of vertices and faces : " << vertices.size() / 3 << " " << surfaceIndices.size() / 3 << std::endl;

	std::cout << "time spent in running marching cubes : " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;

	std::vector<Eigen::Vector3f> visualizationVertices, visualizationNormals;

	int numVertices = vertices.size() / 3;

	visualizationVertices.resize(numVertices);
	visualizationNormals.resize(numVertices);

	for (int vv = 0; vv < numVertices; vv++)
	{
		visualizationVertices[vv](0) = vertices[3 * vv];
		visualizationVertices[vv](1) = vertices[3 * vv + 1];
		visualizationVertices[vv](2) = vertices[3 * vv + 2];
	
		visualizationNormals[vv](0) = vertexNormals[3 * vv];
		visualizationNormals[vv](1) = vertexNormals[3 * vv + 1];
		visualizationNormals[vv](2) = vertexNormals[3 * vv + 2];
	}


	vtkSmartPointer< vtkPolyData > dataSet = vtkSmartPointer< vtkPolyData >::New();
	vtkSmartPointer< vtkPoints > points = vtkSmartPointer< vtkPoints >::New();
	//vtkSmartPointer< vtkUnsignedCharArray > vColors = vtkSmartPointer< vtkUnsignedCharArray >::New();

	int numVerts = vertices.size() / 3;
	int numIndices = surfaceIndices.size() / 3;

	points->Allocate(numVerts);
	dataSet->Allocate(numIndices);

	//vColors->SetNumberOfComponents(3);
	//vColors->SetName("Colors");

	// Add the three colors we have created to the array
	vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();

	int id = 0;

	for (int vv = 0; vv < numVerts; vv++)
	{
		//unsigned char col[] = { 255 , 255 , 255 };

		//vColors->InsertNextTypedTuple(col);

		points->InsertNextPoint(vertices[3 * vv], vertices[3 * vv + 1], vertices[3 * vv + 2]);

	}

	for (int ii = 0; ii < numIndices; ii++)
	{
		triangle->Reset();

		triangle->InsertNextId(surfaceIndices[3 * ii]);
		triangle->InsertNextId(surfaceIndices[3 * ii + 1]);
		triangle->InsertNextId(surfaceIndices[3 * ii + 2]);

		dataSet->InsertNextCell(VTK_TRIANGLE, triangle);
	}

	dataSet->SetPoints(points);
	//dataSet->GetPointData()->SetScalars(vColors);


	//vtkSmartPointer<vtkFillHolesFilter> holeFiller = vtkSmartPointer<vtkFillHolesFilter>::New();

	//holeFiller->SetInputData(dataSet);

	//holeFiller->Update();

	//dataSet->DeepCopy(holeFiller->GetOutput());

	vtkSmartPointer<vtkPLYWriter> writer = vtkSmartPointer<vtkPLYWriter>::New();

	writer->SetFileName("C:\\Data\\mesh.ply");

	writer->SetInputData(dataSet);

	writer->Update();

	tr::Display3DRoutines::displayMesh(visualizationVertices, surfaceIndices);

	vc::MESHViewer::viewMesh(visualizationVertices , visualizationNormals,  surfaceIndices , &volInfo);

	return 0;
}

#define norm3(x) sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2])







