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



struct LineSegment
{

	Eigen::Vector3d mPoint1, mPoint2;
};

struct AABB
{
	Eigen::Vector3d mOrigin;
	Eigen::Vector3d mDim;
};


void testNormals();

void sliceRendering();

void minEnclosingAABB(Eigen::Vector3d& volumeDim, Eigen::Vector3d& startPosition,
	Eigen::Vector3d& xAxis, Eigen::Vector3d& yAxis, Eigen::Vector3d& zAxis, Eigen::Vector3d cuboidDim,
	Eigen::Vector3d& meaabbStartPos, Eigen::Vector3d& meaabbDim);
void createCuboidCage(const Eigen::Vector3d& origin, const Eigen::Vector3d& xAxis, const Eigen::Vector3d& yAxis,
	const Eigen::Vector3d& zAxis, double w, double h, double d,
	vtkSmartPointer<vtkPoints> points, vtkSmartPointer<vtkPolyData> wireframe,
	vtkSmartPointer<vtkUnsignedCharArray> cellColors, int& pointId, const Eigen::Vector3f& color);

void minimumEnclosingCuboid();

bool intersect(LineSegment& segment, AABB& aabb, double& t1, double& t2);



int main(int argc, char** argv)
{

	//A* x = new A();

	//*x = 10;

	//{
		//std::shared_ptr<A> xptr2(x);
	//}

	//std::shared_ptr<A> xptr(x);

	//std::cout << *xptr << std::endl;


	//minimumEnclosingCuboid();

	//return 0;

	sliceRendering();

	return 0;

	//testNormals();

	QString volumeFilePath = "C:\\Test Data\\Multimaterial_Stecker 2018-10-15 16-7-27.uint16_scv";

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

	unsigned short* vData = (unsigned short*)volInfo.mVolumeData;

	for (int64_t zz = 0; zz < volInfo.mDepth; zz++)
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

	imt::volume::MultiResMarchingCubes mrcm(volInfo, iso50Th, 2);

	//unsigned short* volumeData, int64_t volumeWidth, int64_t volumeHeight, int64_t volumeDepth,
	//	double voxelStepX, double voxelStepY, double voxelStepZ

	Zeiss::IMT::NG::NeoInsights::Volume::MeshGenerator::MultiResolutionMarchingCubes marchingCubes((unsigned short*)volInfo.mVolumeData, volInfo.mWidth,
		volInfo.mHeight, volInfo.mDepth, volInfo.mVoxelStep(0), volInfo.mVoxelStep(1), volInfo.mVoxelStep(2));


	Zeiss::IMT::NG::NeoInsights::Volume::MeshGenerator::MultiResolutionMarchingCubes2 marchingCubes2((unsigned short*)volInfo.mVolumeData, volInfo.mWidth,
		volInfo.mHeight, volInfo.mDepth, volInfo.mVoxelStep(0), volInfo.mVoxelStep(1), volInfo.mVoxelStep(2));


	std::vector<double> vertices, vertexNormals;
	std::vector<unsigned int> surfaceIndices;



	marchingCubes.compute(iso50Th, vertices, vertexNormals, surfaceIndices);

	std::vector<std::size_t> indices_6_bit(surfaceIndices.size());

	for (int ii = 0; ii < surfaceIndices.size(); ii++)
		indices_6_bit[ii] = surfaceIndices[ii];

	QString path = "E:\\Projects\\DataSets\\modelfile_kupplung.dat";

	FILE* cadFile;

	fopen_s(&cadFile, path.toStdString().c_str(), "wb");

	std::size_t nPositions = vertices.size() / 3;
	std::size_t nIndices = surfaceIndices.size();// = facetData.Triangles().size();
	fwrite(&nPositions, sizeof(std::size_t), 1, cadFile);
	fwrite(&nIndices, sizeof(std::size_t), 1, cadFile);



	fwrite(vertices.data(), sizeof(double) * 3, nPositions, cadFile);
	fwrite(vertexNormals.data(), sizeof(double) * 3, nPositions, cadFile);
	fwrite(indices_6_bit.data(), sizeof(std::size_t), nIndices, cadFile);

	fclose(cadFile);






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

	vc::MESHViewer::viewMesh(visualizationVertices, visualizationNormals, surfaceIndices, &volInfo);

	return 0;
}

#define norm3(x) sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2])


void testNormals()
{

	int volumeWidth = 128, volumeHeight = 128, volumeDepth = 128;
	double voxelSize = 1.0;

	double minRadius = 25, maxRadius = 45;

	int centerX = volumeWidth * 0.5, centerY = volumeHeight * 0.5, centerZ = volumeDepth * 0.5;

	unsigned short* volumeData = new unsigned short[volumeWidth * volumeHeight * volumeDepth];

	memset(volumeData, 0, volumeWidth * volumeDepth * volumeHeight * sizeof(unsigned short));

	long long zStep = volumeWidth * volumeHeight;
	long long yStep = volumeWidth;

	for (int zz = 0; zz < volumeDepth; zz++)
		for (int yy = 0; yy < volumeHeight; yy++)
			for (int xx = 0; xx < volumeWidth; xx++)
			{

				double dx = xx - centerX;
				double dy = yy - centerY;
				double dz = zz - centerZ;

				double distFromCenter = sqrt(dx * dx + dy * dy + dz * dz);

				long long id = zz * zStep + yy * yStep + xx;

				//if (distFromCenter < 45 && distFromCenter > 25)
				//{
				volumeData[id] = (unsigned short)(distFromCenter * 50);
				//}
				//else
				//{
				//	volumeData[id] = rand() % 20 + 1; //random air values
				//}

			}



	Zeiss::IMT::NG::NeoInsights::Volume::MeshGenerator::MultiResolutionMarchingCubes mcmc(volumeData, volumeWidth, volumeHeight, volumeDepth, voxelSize, voxelSize, voxelSize);

	std::vector<double> vertices, normals;
	std::vector<unsigned int> surfaceIndices;

	mcmc.compute(40 * 50, vertices, normals, surfaceIndices);


	////test the normal consistency
	int numComputedVertices = vertices.size() / 3;

	bool areNormalConsistent = true;

	for (int vv = 0; vv < numComputedVertices; vv++)
	{
		double nx = normals[3 * vv];
		double ny = normals[3 * vv + 1];
		double nz = normals[3 * vv + 2];

		double expectedNormal[3] = { vertices[3 * vv] - centerX , vertices[3 * vv + 1] - centerY , vertices[3 * vv + 2] - centerZ };

		double norm = norm3(expectedNormal);

		double ex = expectedNormal[0] / norm;
		double ey = expectedNormal[1] / norm;
		double ez = expectedNormal[2] / norm;

		double cosVal = nx * ex + ny * ey + nz * ez;

		if (cosVal < 0.95)
		{
			areNormalConsistent = false;

			break;
		}
	}
	std::vector<Eigen::Vector3f> visualizationVertices, visualizationNormals;

	int numVertices = vertices.size() / 3;

	visualizationVertices.resize(numVertices);
	visualizationNormals.resize(numVertices);

	for (int vv = 0; vv < numVertices; vv++)
	{
		visualizationVertices[vv](0) = vertices[3 * vv];
		visualizationVertices[vv](1) = vertices[3 * vv + 1];
		visualizationVertices[vv](2) = vertices[3 * vv + 2];

		visualizationNormals[vv](0) = normals[3 * vv];
		visualizationNormals[vv](1) = normals[3 * vv + 1];
		visualizationNormals[vv](2) = normals[3 * vv + 2];
	}

	tr::Display3DRoutines::displayMesh(visualizationVertices, surfaceIndices);

}


void sliceRendering()
{
	FILE* file = NULL;

	fopen_s(&file, "D:\\extractedData1.dat", "rb");

	int ws = 0, hs = 0;

	int volumeWidth = 0, volumeHeight = 0, volumeDepth = 0;


	fread(&ws, sizeof(int), 1, file);
	fread(&hs, sizeof(int), 1, file);
	fread(&volumeWidth, sizeof(int), 1, file);
	fread(&volumeHeight, sizeof(int), 1, file);
	fread(&volumeDepth, sizeof(int), 1, file);
	cv::Mat sliceS(hs, ws, CV_8UC4);

	std::vector<float> xMapS(ws * hs), yMapS(ws * hs), zMapS(ws * hs);

	std::cout << "width and height : " << ws << " " << hs << std::endl;
	std::cout << "volume width , height , depth : " << volumeWidth << " " << volumeHeight << " " << volumeDepth << std::endl;

	fread(sliceS.data, sizeof(unsigned char), ws * hs * 4, file);

	fread(xMapS.data(), sizeof(float), ws * hs, file);
	fread(yMapS.data(), sizeof(float), ws * hs, file);
	fread(zMapS.data(), sizeof(float), ws * hs, file);

	std::vector<float> colorMapS(4 * 65536, 0);
	fread(colorMapS.data(), sizeof(float), 65536 * 4, file);

	Eigen::Vector3d minS, maxS, xAxisS, yAxisS, zAxisS;

	fread(&minS, sizeof(double), 3, file);
	fread(&maxS, sizeof(double), 3, file);
	fread(&xAxisS, sizeof(double), 3, file);
	fread(&yAxisS, sizeof(double), 3, file);
	fread(&zAxisS, sizeof(double), 3, file);

	std::cout << minS.transpose() << " -- " << maxS.transpose() << std::endl;

	std::cout << xAxisS.transpose() << std::endl;
	std::cout << yAxisS.transpose() << std::endl;
	std::cout << zAxisS.transpose() << std::endl;


	std::vector<Eigen::Vector3f> slicePoints(ws * hs);

	for (int ii = 0; ii < ws * hs; ii++)
	{
		slicePoints[ii] = Eigen::Vector3f(xMapS[ii], yMapS[ii], zMapS[ii]);
	}

	std::vector<Eigen::Vector3f> slicePointColors(ws * hs, Eigen::Vector3f(1, 1, 1));

	for (int ii = 0; ii < 2; ii++)
		for (int jj = 0; jj < 2; jj++)
			for (int kk = 0; kk < 2; kk++)
			{
				slicePoints.push_back(Eigen::Vector3f(ii * volumeWidth, jj * volumeHeight, kk * volumeDepth));

				slicePointColors.push_back(Eigen::Vector3f(0, 1, 0));
			}

	slicePoints.push_back(minS.cast<float>());
	slicePoints.push_back(maxS.cast<float>());

	slicePointColors.push_back(Eigen::Vector3f(1, 0, 0));
	slicePointColors.push_back(Eigen::Vector3f(0, 0, 1));

	tr::Display3DRoutines::displayPointSet(slicePoints, slicePointColors);

	fclose(file);

	cv::namedWindow("slice", 0);
	cv::imshow("slice", sliceS);
	cv::waitKey();


	return;



	QString volumeFilePath = "C:\\Test Data\\10GB CAD Cube\\Test cube_10gb_BHC 2018-1-25 9-51-35.uint16_scv";//"C:\\Test Data\\Multimaterial_Stecker 2018-10-15 16-7-27.uint16_scv";

	//first load the volume
	imt::volume::RawVolumeDataIO io;

	imt::volume::VolumeInfo volInfo;

	io.readUint16SCV(volumeFilePath, volInfo);

	volInfo.mZStep = volInfo.mWidth * volInfo.mHeight;
	volInfo.mYStep = volInfo.mWidth;

	volInfo.mVolumeDataU16 = (unsigned short*)volInfo.mVolumeData;

	imt::volume::HistogramFilter filter(&volInfo);

	std::vector< int64_t > histogram(std::numeric_limits< unsigned short >::max(), 0);

	unsigned short* vData = (unsigned short*)volInfo.mVolumeData;

	for (int64_t zz = 0; zz < volInfo.mDepth; zz++)
		for (int64_t yy = 0; yy < volInfo.mHeight; yy++)
			for (int64_t xx = 0; xx < volInfo.mWidth; xx++)
			{
				histogram[vData[zz * volInfo.mWidth * volInfo.mHeight + yy * volInfo.mWidth + xx]] += 1;
			}


	long maxHistogramVal = *(std::max_element(histogram.begin(), histogram.end()));

	//tr::Display3DRoutines::displayHistogram(histogram, 0, maxHistogramVal);
	//filter.plotHistogram(histogram);

	std::vector<double> transformationData{ -0.998721905621282,-0.00149459127925789,-0.050520505036104,14.1617949155217 ,
			0.0170719278270519,	-0.950791484734876,	-0.309360795570346,	125.112662495486,
			-0.0475720980456293,	-0.309827885692295,	0.949601799037354,	-105.893721671161,
			0,	0,	0,	1 };

	Eigen::Matrix4d transformation;

	for (int rr = 0; rr < 4; rr++)
		for (int cc = 0; cc < 4; cc++)
		{
			transformation(rr, cc) = transformationData[4 * rr + cc];
		}

	Eigen::Matrix4d invTransformation = transformation.inverse();

	double wTransformed = 0, hTransformed = 0, dTransformed = 0;

	int w = volInfo.mWidth;
	int h = volInfo.mHeight;
	int d = volInfo.mDepth;

	double minX = FLT_MAX, maxX = -FLT_MAX, minY = FLT_MAX, maxY = -FLT_MAX, minZ = FLT_MAX, maxZ = -FLT_MAX;

	for (int ii = 0; ii < 2; ii++)
		for (int jj = 0; jj < 2; jj++)
			for (int kk = 0; kk < 2; kk++)
			{
				Eigen::Vector3d corner(ii * w, jj * h, kk * d);

				Eigen::Vector4d transformedCorners = (transformation * corner.homogeneous());

				minX = std::min(minX, transformedCorners(0));
				maxX = std::max(maxX, transformedCorners(0));

				minY = std::min(minY, transformedCorners(1));
				maxY = std::max(maxY, transformedCorners(1));

				minZ = std::min(minZ, transformedCorners(2));
				maxZ = std::max(maxZ, transformedCorners(2));

				//std::cout << transformedCorners.transpose() << std::endl;
			}

	Eigen::Vector3d startPos(minX, minY, minZ);

	Eigen::Vector4d transformedStartPos = invTransformation * startPos.homogeneous();

	std::cout << " transformed start pos : " << transformedStartPos.transpose() << std::endl;

	Eigen::Vector3d xAxis = invTransformation.row(0).transpose().block(0, 0, 3, 1);
	Eigen::Vector3d yAxis = invTransformation.row(1).transpose().block(0, 0, 3, 1);
	Eigen::Vector3d zAxis = invTransformation.row(2).transpose().block(0, 0, 3, 1);

	std::cout << xAxis.transpose() << std::endl;
	std::cout << yAxis.transpose() << std::endl;
	std::cout << zAxis.transpose() << std::endl;


	int transformedW = maxX - minX;
	int transformedH = maxY - minY;
	int transformedD = maxZ - minZ;




	long iso50Th = filter.ISO50Threshold(histogram);

	std::cout << " min x , y , z : " << minX * volInfo.mVoxelStep(0) << " , "
		<< minY * volInfo.mVoxelStep(0) << " , " << minZ * volInfo.mVoxelStep(0) << std::endl;

	std::cout << "width , height , depth : " << (maxX - minX) * volInfo.mVoxelStep(0)
		<< " , " << (maxY - minY) * volInfo.mVoxelStep(0)
		<< " , " << (maxZ - minZ) * volInfo.mVoxelStep(0) << std::endl;

	std::cout << "iso 50 value : " << iso50Th << std::endl;

	FILE* reader;
	fopen_s(&reader, "D:\\transferfunction_slice.dat", "rb");

	int nGrayValues = 0;// _NativeTransferFunction->NumberOfGreyscales();

	fread(&nGrayValues, sizeof(int), 1, reader);
	std::vector<float> colorMap(nGrayValues * 4);

	fread(colorMap.data(), sizeof(float), nGrayValues * 4, reader);
	fclose(reader);

	std::cout << "colormap size : " << nGrayValues << " " << nGrayValues * 4 * sizeof(float) << std::endl;

	cv::Mat slice(volInfo.mHeight, volInfo.mWidth, CV_8UC3);

	int64_t z = volInfo.mDepth * 0.5;

	unsigned char* sliceData = slice.data;

	int64_t planeStep = volInfo.mWidth * volInfo.mHeight;

	double initT = cv::getTickCount();

	for (int yy = 0; yy < volInfo.mHeight; yy++)
		for (int xx = 0; xx < volInfo.mWidth; xx++)
		{
			unsigned short grayValue = vData[planeStep * z + volInfo.mWidth * yy + xx];

			unsigned char* data = sliceData + 3 * volInfo.mWidth * yy + 3 * xx;

			data[0] = colorMap[4 * grayValue] * 255;
			data[1] = colorMap[4 * grayValue + 1] * 255;
			data[2] = colorMap[4 * grayValue + 2] * 255;
		}


	double zShift = transformedD * 0.5;

	Eigen::Vector3d transformedOrigin = transformedStartPos.hnormalized();


	std::vector<Eigen::Vector3d> slicePixelPositions(transformedH * transformedW);

	int64_t nPixels = slicePixelPositions.size();

	float* xMap = new float[nPixels];
	float* yMap = new float[nPixels];
	float* zMap = new float[nPixels];

	initT = cv::getTickCount();


	for (int yy = 0; yy < transformedH; yy++)
		for (int xx = 0; xx < transformedW; xx++)
		{
			Eigen::Vector3d point = transformedOrigin + xx * xAxis + yy * yAxis + zShift * zAxis;

			slicePixelPositions[yy * transformedW + xx] = point;

			xMap[yy * transformedW + xx] = point(0);
			yMap[yy * transformedW + xx] = point(1);
			zMap[yy * transformedW + xx] = point(2);

			//unsigned short grayValue = vData[planeStep * z + volInfo.mWidth * yy + xx];

			//unsigned char* data = sliceData + 3 * volInfo.mWidth * yy + 3 * xx;

			//data[0] = colorMap[4 * grayValue] * 255;
			//data[1] = colorMap[4 * grayValue + 1] * 255;
			//data[2] = colorMap[4 * grayValue + 2] * 255;
		}

	std::vector<unsigned short*> volumeSlices(d);

	for (int zz = 0; zz < d; zz++)
	{
		volumeSlices[zz] = vData + zz * planeStep;
	}

	IpprVolume srcVolume = { w , h , d };
	IpprCuboid srcVoi = { 0,0,0, w, h , d };
	IpprVolume dstVolume = { nPixels , 1 , 1 };

	unsigned short* temp;

	temp = new unsigned short[nPixels];

	ippsSet_16s(0, (short*)temp, nPixels);



	auto sts = ipprRemap_16u_C1PV(volumeSlices.data(), srcVolume, sizeof(Ipp16u) * w, srcVoi,
		&xMap, &yMap, &zMap, sizeof(Ipp32f) * nPixels, &temp,
		sizeof(Ipp16u) * nPixels, dstVolume, IPPI_INTER_LINEAR);

	if (sts != 0)
	{
		std::cout << "error in ipp " << std::endl;
	}

	cv::Mat slantedSlice(transformedH, transformedW, CV_8UC3);

	slantedSlice.setTo(cv::Scalar(0, 0, 0));

	for (int yy = 0; yy < transformedH; yy++)
		for (int xx = 0; xx < transformedW; xx++)
		{

			unsigned short grayValue = temp[transformedW * yy + xx];

			unsigned char* data = slantedSlice.data + 3 * transformedW * yy + 3 * xx;

			data[0] = colorMap[4 * grayValue] * 255;
			data[1] = colorMap[4 * grayValue + 1] * 255;
			data[2] = colorMap[4 * grayValue + 2] * 255;
		}


	//sts = ipprRemap_16u_C1PV((Ipp16u * *)voxels, srcVolume, sizeof(Ipp16u) * voxelDim[0], srcVoi,
	//	&xMap, &yMap, &zMap, sizeof(Ipp32f) * 27,
	//	&temp, sizeof(Ipp16u) * 27, dstVolume, interpolationMethod);


	std::cout << "time spent in slice computation" << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;

	cv::imwrite("sliceImage.jpg", slantedSlice);

	cv::namedWindow("slice", 0);
	cv::imshow("slice", slantedSlice);
	cv::waitKey();






}


void minimumEnclosingCuboid()
{
	FILE* reader;
	fopen_s(&reader, "D:\\roidata.dat", "rb");

	Eigen::Vector3d startLocation, xAxis, yAxis, zAxis;
	double roiW, roiH, roiD;

	fread(startLocation.data(), sizeof(double), 3, reader);
	fread(xAxis.data(), sizeof(double), 3, reader);
	fread(yAxis.data(), sizeof(double), 3, reader);
	fread(zAxis.data(), sizeof(double), 3, reader);

	fread(&roiW, sizeof(double), 1, reader);
	fread(&roiH, sizeof(double), 1, reader);
	fread(&roiD, sizeof(double), 1, reader);

	Eigen::Vector3d volSize;

	fread(volSize.data(), sizeof(double), 3, reader);

	fclose(reader);


	std::cout << startLocation.transpose() << std::endl;
	std::cout << xAxis.transpose() << std::endl;
	std::cout << yAxis.transpose() << std::endl;
	std::cout << zAxis.transpose() << std::endl;
	std::cout << roiW << " " << roiH << " " << roiD << std::endl;
	std::cout << volSize.transpose() << std::endl;

	vtkSmartPointer<vtkPolyData> cuboidData = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

	cuboidData->Allocate(24);
	points->Allocate(16);

	//set vertex (cell) normals
  // Setup the colors array
	vtkSmartPointer<vtkUnsignedCharArray> colors =
		vtkSmartPointer<vtkUnsignedCharArray>::New();
	colors->SetNumberOfComponents(3);
	colors->SetName("Colors");

	//cuboidData->GetCellData()->

	int pointId = 0;

	Eigen::Vector3d roiDim(roiW, roiH, roiD);

	Eigen::Vector3d enclosingAABBStartPos, enclosingAABBDim;

	minEnclosingAABB(volSize, startLocation, xAxis, yAxis, zAxis, roiDim, enclosingAABBStartPos, enclosingAABBDim);

	createCuboidCage(startLocation, xAxis, yAxis, zAxis, roiW, roiH, roiD, points, cuboidData, colors, pointId, Eigen::Vector3f(0, 1, 0));
	createCuboidCage(Eigen::Vector3d(0, 0, 0), Eigen::Vector3d(1, 0, 0), Eigen::Vector3d(0, 1, 0), Eigen::Vector3d(0, 0, 1),
		volSize(0), volSize(1), volSize(2), points, cuboidData, colors, pointId, Eigen::Vector3f(0, 0, 1));
	createCuboidCage(enclosingAABBStartPos, Eigen::Vector3d(1, 0, 0), Eigen::Vector3d(0, 1, 0), Eigen::Vector3d(0, 0, 1),
		enclosingAABBDim(0), enclosingAABBDim(1), enclosingAABBDim(2), points, cuboidData, colors, pointId, Eigen::Vector3f(1, 0, 0));
	cuboidData->GetCellData()->SetScalars(colors);
	cuboidData->SetPoints(points);

	tr::Display3DRoutines::displayPolyData(cuboidData);

}

void createCuboidCage(const Eigen::Vector3d& origin, const Eigen::Vector3d& xAxis, const Eigen::Vector3d& yAxis,
	const Eigen::Vector3d& zAxis, double w, double h, double d,
	vtkSmartPointer<vtkPoints> points, vtkSmartPointer<vtkPolyData> wireframe,
	vtkSmartPointer<vtkUnsignedCharArray> wireColors, int& pointId, const Eigen::Vector3f& color)
{
	Eigen::Vector3d corners[8];

	corners[0] = origin;
	corners[1] = origin + xAxis * w;
	corners[2] = origin + xAxis * w + yAxis * h;
	corners[3] = origin + yAxis * h;

	corners[4] = origin + d * zAxis;
	corners[5] = origin + xAxis * w + d * zAxis;
	corners[6] = origin + xAxis * w + yAxis * h + d * zAxis;
	corners[7] = origin + yAxis * h + d * zAxis;

	for (int ii = 0; ii < 8; ii++)
	{
		points->InsertNextPoint(corners[ii].data());
	}

	vtkSmartPointer<vtkIdList> edge = vtkSmartPointer<vtkIdList>::New();

	unsigned char colorVal[] = { color(0) * 255.0 , color(1) * 255.0 , color(2) * 255.0 };



	for (int ii = 0; ii < 4; ii++)
	{
		//int id = ii + pointId;

		wireColors->InsertNextTypedTuple(colorVal);
		wireColors->InsertNextTypedTuple(colorVal);
		wireColors->InsertNextTypedTuple(colorVal);

		edge->InsertNextId(ii + pointId);
		edge->InsertNextId((ii + 1) % 4 + pointId);

		wireframe->InsertNextCell(VTK_LINE, edge);

		edge->Reset();

		edge->InsertNextId(ii + 4 + pointId);
		edge->InsertNextId((ii + 1) % 4 + 4 + pointId);

		wireframe->InsertNextCell(VTK_LINE, edge);

		edge->Reset();

		edge->InsertNextId(ii + pointId);
		edge->InsertNextId(ii + 4 + pointId);

		wireframe->InsertNextCell(VTK_LINE, edge);

		edge->Reset();
	}


	pointId += 8;
}



void minEnclosingAABB(Eigen::Vector3d& volumeDim, Eigen::Vector3d& startPosition,
	Eigen::Vector3d& xAxis, Eigen::Vector3d& yAxis, Eigen::Vector3d& zAxis, Eigen::Vector3d cuboidDim,
	Eigen::Vector3d& meaabbStartPos, Eigen::Vector3d& meaabbDim)
{
	std::vector<Eigen::Vector3d> cuboidCorners(8);

	cuboidCorners[0] = startPosition;
	cuboidCorners[1] = startPosition + xAxis * cuboidDim(0);
	cuboidCorners[2] = startPosition + xAxis * cuboidDim(0) + yAxis * cuboidDim(1);
	cuboidCorners[3] = startPosition + yAxis * cuboidDim(1);

	cuboidCorners[4] = startPosition + cuboidDim(2) * zAxis;
	cuboidCorners[5] = startPosition + xAxis * cuboidDim(0) + cuboidDim(2) * zAxis;
	cuboidCorners[6] = startPosition + xAxis * cuboidDim(0) + yAxis * cuboidDim(1) + cuboidDim(2) * zAxis;
	cuboidCorners[7] = startPosition + yAxis * cuboidDim(1) + cuboidDim(2) * zAxis;

	std::vector<Eigen::Vector3f> cuboidCornersF(8);

	for (int ii = 0; ii < 8; ii++)
	{
		cuboidCornersF[ii] = cuboidCorners[ii].cast<float>();
	}

	//tr::Display3DRoutines::displayPointSet(cuboidCornersF, std::vector<Eigen::Vector3f>(cuboidCornersF.size(), Eigen::Vector3f(1, 1, 1)));

	AABB aabb;

	aabb.mOrigin = Eigen::Vector3d(0, 0, 0);
	aabb.mDim = volumeDim;//Eigen::Vector3d(  )

	int nIntersections = 0;

	std::vector<Eigen::Vector3d> intersectionPoints;

	//12 line segment intersections
	for (int ii = 0; ii < 4; ii++)
	{
		LineSegment seg1, seg2, seg3;

		seg1.mPoint1 = cuboidCorners[ii];
		seg1.mPoint2 = cuboidCorners[(ii + 1) % 4];

		seg2.mPoint1 = cuboidCorners[ii + 4];
		seg2.mPoint2 = cuboidCorners[(ii + 1) % 4 + 4];

		seg3.mPoint1 = cuboidCorners[ii];
		seg3.mPoint2 = cuboidCorners[ii + 4];

		double t1 = -1, t2 = -1;

		if (intersect(seg1, aabb, t1, t2))
		{
			if (t1 >= 0 && t1 <= 1)
			{
				nIntersections++;

				Eigen::Vector3d pt = seg1.mPoint1 * t1 + (1 - t1) * seg1.mPoint2;

				intersectionPoints.push_back(pt);
			}

			if (t2 >= 0 && t2 <= 1)
			{
				nIntersections++;

				Eigen::Vector3d pt = seg1.mPoint1 * (1 - t2) + t2 * seg1.mPoint2;

				intersectionPoints.push_back(pt);
			}
		}

		t1 = -1;
		t2 = -1;

		if (intersect(seg2, aabb, t1, t2))
		{
			if (t1 >= 0 && t1 <= 1)
			{
				nIntersections++;

				Eigen::Vector3d pt = seg2.mPoint1 * t1 + (1 - t1) * seg2.mPoint2;

				intersectionPoints.push_back(pt);
			}

			if (t2 >= 0 && t2 <= 1)
			{
				nIntersections++;

				Eigen::Vector3d pt = seg2.mPoint1 * (1 - t2) + t2 * seg2.mPoint2;

				intersectionPoints.push_back(pt);
			}
		}

		t1 = -1;
		t2 = -1;

		if (intersect(seg3, aabb, t1, t2))
		{
			if (t1 >= 0 && t1 <= 1)
			{
				nIntersections++;

				Eigen::Vector3d pt = seg3.mPoint1 * t1 + (1 - t1) * seg3.mPoint2;

				intersectionPoints.push_back(pt);
			}

			if (t2 >= 0 && t2 <= 1)
			{
				nIntersections++;

				Eigen::Vector3d pt = seg3.mPoint1 * (1 - t2) + t2 * seg3.mPoint2;

				intersectionPoints.push_back(pt);
			}
		}


		double xMin = FLT_MAX, yMin = FLT_MAX, zMin = FLT_MAX,
			xMax = -FLT_MAX, yMax = -FLT_MAX, zMax = -FLT_MAX;

		int nIntersectionPoints = intersectionPoints.size();

		for (int ii = 0; ii < nIntersectionPoints; ii++)
		{
			Eigen::Vector3d ip = intersectionPoints[ii];

			xMin = std::min(ip(0), xMin);
			yMin = std::min(ip(1), yMin);
			zMin = std::min(ip(2), zMin);

			xMax = std::max(ip(0), xMax);
			yMax = std::max(ip(1), yMax);
			zMax = std::max(ip(2), zMax);
		}

		meaabbStartPos(0) = xMin;
		meaabbStartPos(1) = yMin;
		meaabbStartPos(2) = zMin;

		meaabbDim(0) = xMax - xMin;
		meaabbDim(1) = yMax - yMin;
		meaabbDim(2) = zMax - zMin;


	}


	std::cout << "number of intersection points : " << intersectionPoints.size() << std::endl;

	std::vector<Eigen::Vector3f> displayPoints(intersectionPoints.size());

	for (int pp = 0; pp < intersectionPoints.size(); pp++)
	{
		displayPoints[pp] = intersectionPoints[pp].cast<float>();
	}

	//displayPoints.insert(displayPoints.end(), cuboidCornersF.begin(), cuboidCornersF.end());

	//tr::Display3DRoutines::displayPointSet(displayPoints, std::vector<Eigen::Vector3f>(displayPoints.size() , Eigen::Vector3f(1,1,1)));


	//std::cout << "number of total intersections : " << nIntersections << std::endl;

}

bool intersect(LineSegment& segment, AABB& aabb, double& t1, double& t2)
{
	// parametric representation a * t + (1 - t) * b

	//intersect the line segment with 6 planes

	//a line segment can intersect a cuboid at atmost two points
	t1 = -1, t2 = -1;

	bool p1Inside = (segment.mPoint1(0) >= aabb.mOrigin(0) && segment.mPoint1(0) <= (aabb.mOrigin(0) + aabb.mDim(0))) &&
		(segment.mPoint1(1) >= aabb.mOrigin(1) && segment.mPoint1(1) <= (aabb.mOrigin(1) + aabb.mDim(1))) &&
		(segment.mPoint1(2) >= aabb.mOrigin(2) && segment.mPoint1(2) <= (aabb.mOrigin(2) + aabb.mDim(2)));


	bool p2Inside = (segment.mPoint2(0) >= aabb.mOrigin(0) && segment.mPoint2(0) <= (aabb.mOrigin(0) + aabb.mDim(0))) &&
		(segment.mPoint2(1) >= aabb.mOrigin(1) && segment.mPoint2(1) <= (aabb.mOrigin(1) + aabb.mDim(1))) &&
		(segment.mPoint2(2) >= aabb.mOrigin(2) && segment.mPoint2(2) <= (aabb.mOrigin(2) + aabb.mDim(2)));

	if (p1Inside)
	{
		t1 = 1;
	}

	if (p2Inside)
	{
		t2 = 1;
	}

	if (p1Inside && p2Inside)
	{
		return true;
	}

	double xDenom = segment.mPoint1(0) - segment.mPoint2(0);
	double yDenom = segment.mPoint1(1) - segment.mPoint2(1);
	double zDenom = segment.mPoint1(2) - segment.mPoint2(2);

	double xMin = aabb.mOrigin(0), xMax = aabb.mOrigin(0) + aabb.mDim(0);
	double yMin = aabb.mOrigin(1), yMax = aabb.mOrigin(1) + aabb.mDim(1);
	double zMin = aabb.mOrigin(2), zMax = aabb.mOrigin(2) + aabb.mDim(2);

	double minDenom = FLT_EPSILON;

	double t1Best = -1, t2Best = -1;

	if (std::abs(xDenom) > minDenom)
	{
		double pit1 = (xMin - segment.mPoint2(0)) / xDenom;
		double pit2 = (xMax - segment.mPoint2(0)) / xDenom;

		if (pit1 >= 0 && pit1 <= 1)
		{
			if (t1Best < 0)
			{
				t1Best = pit1;
			}
			else
			{
				t2Best = pit1;
			}
		}

		if (pit2 >= 0 && pit2 <= 1)
		{
			if (t1Best < 0)
			{
				t1Best = pit2;
			}
			else
			{
				t2Best = pit2;
			}
		}
	}

	if (std::abs(yDenom) > minDenom)
	{
		double pit1 = (yMin - segment.mPoint2(1)) / yDenom;
		double pit2 = (yMax - segment.mPoint2(1)) / yDenom;

		if (pit1 >= 0 && pit1 <= 1)
		{
			if (t1Best < 0)
			{
				t1Best = pit1;
			}
			else
			{
				t2Best = pit1;
			}
		}

		if (pit2 >= 0 && pit2 <= 1)
		{
			if (t1Best < 0)
			{
				t1Best = pit2;
			}
			else
			{
				t2Best = pit2;
			}
		}
	}

	if (std::abs(zDenom) > minDenom)
	{
		double pit1 = (zMin - segment.mPoint2(2)) / zDenom;
		double pit2 = (zMax - segment.mPoint2(2)) / zDenom;

		if (pit1 >= 0 && pit1 <= 1)
		{
			if (t1Best < 0)
			{
				t1Best = pit1;
			}
			else
			{
				t2Best = pit1;
			}
		}

		if (pit2 >= 0 && pit2 <= 1)
		{
			if (t1Best < 0)
			{
				t1Best = pit2;
			}
			else
			{
				t2Best = pit2;
			}
		}
	}

	if (t1Best < 0)
		return false;


	if (t2Best >= 0)
	{
		if (t1Best > t2Best)
			std::swap(t1Best, t2Best);
	}

	if (p1Inside)
	{
		t2 = 1 - t1Best;

		return true;
	}

	if (p2Inside)
	{
		t1 = t1Best;

		return true;
	}


	t1 = t1Best;
	t2 = 1 - t2Best;

	return true;
}






