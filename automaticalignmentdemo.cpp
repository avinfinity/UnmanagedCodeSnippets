
#include <iostream>
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
#include "vtkImageData.h"
#include "vtkMarchingCubes.h"
#include "vtkPointData.h"
#include "vtkDoubleArray.h"
#include <vtkButterflySubdivisionFilter.h>
#include "MultiResolutionMarchingCubes.h"

#include "ipp.h"
#include "ippvm.h"
#include <random>

using namespace Zeiss::IMT::NG::NeoInsights::Volume::MeshGenerator;

void viewIsoSurface(imt::volume::VolumeInfo& volume, int isoVal);
void viewIsoSurface(unsigned char* data, int w, int h, int d, const Eigen::Vector3d& voxelSize, int isoVal);

void computeIsoSurface(imt::volume::VolumeInfo& volume, int isoVal, std::vector<Eigen::Vector3f>& points, std::vector<unsigned int>& surfaceIndices);
void displayWithNormal(std::vector<Eigen::Vector3d>& vertices, std::vector<Eigen::Vector3d>& normals,
	std::vector<unsigned int>& surfaceIndices);
double averagePointCountAreaRatio(std::vector<Eigen::Vector3d>& points, std::vector<std::size_t>& indices);

void resampleCADPoints(std::vector<Eigen::Vector3d>& cadPoints, std::vector<Eigen::Vector3d>& cadNormals,
	std::vector<std::size_t>& cadIndices, double pointAreaRatio, std::vector<Eigen::Vector3d>& resampledPoints,
	std::vector<Eigen::Vector3d>& resampledNormals);

template<typename TBuffer>
void CalculateIntertiaTensor(const long& width, const long& height, const long& depth, const Eigen::Vector3d& voxelSize, const INT64& threshold,
	const Eigen::Vector3d& centerOfGravityVoxel,const void* buffer, Eigen::Matrix3d& inertialTensor )
{
	const size_t sliceSZ = width * height;

	double sumxx = 0; double sumyy = 0; double sumzz = 0; double sumxy = 0; double sumxz = 0; double sumyz = 0;
	INT64  count = 0;

	const TBuffer* data = (const TBuffer*)buffer;

	for (long z = -(long)centerOfGravityVoxel(2), zi = 0; zi < depth; ++z, ++zi)
	{
		double zz = z * z;

		for (long y = -(long)centerOfGravityVoxel(1), yi = 0; yi < height; ++y, ++yi)
		{
			double yy = y * y;
			double yz = y * z;
			for (long x = -(long)centerOfGravityVoxel(0), xi = 0; xi < width; ++x, ++xi)
			{
				if (data[xi + yi * width + zi * sliceSZ] >= threshold)
				{
					sumxx += x * x;
					sumyy += yy;
					sumzz += zz;
					sumxy -= x * y;
					sumxz -= x * z;
					sumyz -= yz;
					++count;
				}
			}
		}
	}

	double denominator = count > 0 ? static_cast<double>(count) : 1;

	inertialTensor( 0 , 0 ) = double(sumyy) / denominator * voxelSize(1) * voxelSize(1) + 
		                      double(sumzz) / denominator * voxelSize(2) * voxelSize(2);

	inertialTensor(1, 1)/**/ = double(sumzz) / denominator * voxelSize(2) * voxelSize(2) + 
		                       double(sumxx) / denominator * voxelSize(0) * voxelSize(0);

	inertialTensor(2, 3)/*[2 * 3 + 2]*/ = double(sumxx) / denominator * voxelSize(0) * voxelSize(0) + 
		                                  double(sumyy) / denominator * voxelSize(1) * voxelSize(1);

	inertialTensor(0, 1)/*[1]*/ = double(sumxy) / denominator * voxelSize(0) * voxelSize(1);
	inertialTensor(1, 0)/*[1 * 3 + 0]*/ = inertialTensor(0, 1);/*[1];*/
	inertialTensor(0, 2)/*[2]*/ = double(sumxz) / denominator * voxelSize(0) * voxelSize(2);
	inertialTensor(2, 0)/*[2 * 3]*/ = inertialTensor(0, 2);/*[2];*/
	inertialTensor(1, 2)/*[1 * 3 + 2]*/ = double(sumyz) / denominator * voxelSize(1) * voxelSize(2);
	inertialTensor(2, 1)/*[2 * 3 + 1]*/ = inertialTensor(1, 2);/*[1 * 3 + 2];*/

}



template<typename TBuffer>
void CalculateCenterOfGravityAndInertiaTensor(const long& width, const long& height, const long& numSlices, const Eigen::Vector3d & voxelSize,
	const int64_t& threshold, const void* buffer, Eigen::Vector3d& centerOGravityWorld, Eigen::Matrix3d& inertiaTensor)
{
	auto centersOfGravity = CalculateCenterOfGravity<TBuffer>(width, height, numSlices, voxelSize, threshold, buffer);

	std::cout << "center of gravity : " << centersOfGravity.second.transpose() << std::endl;
	std::cout << "center of gravity mm : " << centersOfGravity.first.transpose() << std::endl;

	//void CalculateIntertiaTensor(const long& width, const long& height, const long& depth, Eigen::Vector3d & voxelSize, const INT64 & threshold,
		//Eigen::Vector3d & centerOfGravityVoxel, void* buffer, Eigen::Matrix3d & inertialTensor)

	centerOGravityWorld = centersOfGravity.first;
	CalculateIntertiaTensor<TBuffer>(width, height, numSlices, voxelSize, threshold, centersOfGravity.second, buffer,  inertiaTensor);
}

template<typename TBuffer>
std::pair<Eigen::Vector3d, Eigen::Vector3d> CalculateCenterOfGravity(const long& width, const long& height, const long& numSlices,
	const Eigen::Vector3d& voxelSize, const int64_t& threshold, const void* buffer)
{
	const long sliceSZ = width * height;
	TBuffer* voxelData = (TBuffer*)buffer;

	INT64 sum[3] = { 0, 0, 0 };
	INT64  count = 0;
	for (long z = 0; z < numSlices; ++z)
	{
		for (long y = 0; y < height; ++y)
		{
			for (long x = 0; x < width; ++x)
			{
				if (voxelData[x + y * width + z * sliceSZ] >= threshold)
				{
					sum[0] += x;
					sum[1] += y;
					sum[2] += z;
					++count;
				}
			}
		}
	}

	Eigen::Vector3d centerOfGravityWorld, centerOfGravityVoxel;
	std::array<double, 3> voxelOrigin{ 0.0 };  // = 0, in der Originalversion scheint das immer so gesetzt zu sein. Checken u. ggf. weglassen
	for (long i = 0; i < 3; ++i)
	{
		INT64 denominator = count > 0 ? count : 1;
		centerOfGravityVoxel(i) = static_cast<long>(sum[i] / denominator);
		centerOfGravityWorld(i) = static_cast<double>(sum[i]) / static_cast<double>(denominator) * voxelSize(i) - voxelOrigin[i];
	}

	return std::make_pair(centerOfGravityWorld, centerOfGravityVoxel);
}


Eigen::Matrix4d computeTransform( const Eigen::Vector3d& cadCoG, const Eigen::Matrix3d& cadEigenVecs,
	                              const  Eigen::Vector3d& volumeCoG, const Eigen::Matrix3d& volumeEigenVecs);



int main(int argc, char** argv)
{

	std::string cadVoxelDataPath = "D:\\Data\\cadvoxel_kupplung.dat";

	int cadVoxelDim[3];
	Eigen::Vector3d cadVoxelSize;

	FILE* cadVoxelFile = fopen(cadVoxelDataPath.c_str(), "rb");

	fread(cadVoxelDim, sizeof(int), 3, cadVoxelFile);
	fread(cadVoxelSize.data(), sizeof(double), 3, cadVoxelFile);

	size_t bufferSize = (size_t)cadVoxelDim[0] * (size_t)cadVoxelDim[1]
		* (size_t)cadVoxelDim[2];

	unsigned char* cadVoxelData = new unsigned char[bufferSize];

	fread(cadVoxelData, sizeof(unsigned char), bufferSize , cadVoxelFile);
	fclose(cadVoxelFile);

	//viewIsoSurface(cadVoxelData, cadVoxelDim[0], cadVoxelDim[1], cadVoxelDim[2], cadVoxelSize, 255);

	std::cout << cadVoxelDim[0] << " " << cadVoxelDim[1] << " " << cadVoxelDim[2] << std::endl;
	std::cout << " cad voxel size : " << cadVoxelSize(0) << " " << cadVoxelSize(1) << " " << cadVoxelSize(2) << std::endl;

	std::cout << "automatic alignment demo " << std::endl;

	std::string cadFilePath = "D:\\Data\\caddat_kupplung.dat";

	FILE* file = fopen(cadFilePath.c_str(), "rb");

	QString volumeFilePath = "C:\\Test Data\\5.Kupplung-Performance test data\\1GB\\Test_Kupplung_1GB_Vx64_2015-6-2 9-10.uint16_scv";

	imt::volume::RawVolumeDataIO io;

	imt::volume::VolumeInfo volInfo;

	io.readUint16SCV(volumeFilePath, volInfo);

	volInfo.mZStep = volInfo.mWidth * volInfo.mHeight;
	volInfo.mYStep = volInfo.mWidth;

	std::cout << "volume voxel size : " << volInfo.mVoxelStep.transpose() << std::endl;

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

	long iso50Th = filter.ISO50Threshold(histogram);

	std::vector<Eigen::Vector3f> volumeSurfacePoints;
	std::vector<unsigned int> volumeSurfaceIndices;

	computeIsoSurface(volInfo, iso50Th, volumeSurfacePoints, volumeSurfaceIndices);


	std::cout << "iso 50 threshold : " << iso50Th << std::endl;


	//const long& width, const long& height, const long& numSlices, const std::array<double, 3> & voxelSize,
		//const int64_t& threshold, const void* buffer, Eigen::Vector3d& centerOGravityWorld, Eigen::Matrix3d& inertiaTensor

	Eigen::Vector3d volumeVoxelSize = volInfo.mVoxelStep.cast<double>();//{ volInfo.mVoxelStep(0) , volInfo.mVoxelStep(1) , volInfo.mVoxelStep(2) };

	Eigen::Vector3d cogVolume, cogCAD;
	Eigen::Matrix3d cogInertialTensor, cogInertialTensorCAD;

	CalculateCenterOfGravityAndInertiaTensor<unsigned short>(volInfo.mWidth, volInfo.mHeight,
		volInfo.mDepth, volumeVoxelSize, iso50Th, vData, cogVolume, cogInertialTensor);

	std::cout << "volume center of gravity : " << cogVolume.transpose() << std::endl;

	std::cout << " inertial tensor :  " << cogInertialTensor << std::endl;

	CalculateCenterOfGravityAndInertiaTensor<unsigned char>(cadVoxelDim[0], cadVoxelDim[1],
		cadVoxelDim[2], cadVoxelSize, 255, cadVoxelData, cogCAD, cogInertialTensorCAD);

	std::cout << " volume c o g :  " << cogVolume.transpose() << std::endl;


	std::cout << " center of gravity CAD :  " << cogCAD.transpose() << std::endl;
	std::cout << " inertial tensor CAD :  " << cogInertialTensorCAD << std::endl;


	Eigen::EigenSolver<Eigen::Matrix3d> eigenSolver;

	auto eigenSolutionVolume = eigenSolver.compute(cogInertialTensor);
	auto eigenSolutionCAD = eigenSolver.compute(cogInertialTensorCAD);

	auto eigenVectorsVolume = eigenSolutionVolume.eigenvectors().real();
	auto eigenValuesVolume = eigenSolutionVolume.eigenvalues().real();

	auto eigenVectorsCAD = eigenSolutionCAD.eigenvectors().real();
	auto eigenValuesCAD = eigenSolutionCAD.eigenvalues().real();

	//std::cout << " volume eigen vectors : " << std::endl;
	//std::cout << eigenVectorsVolume << std::endl;
	//std::cout << "volume eigen values : " << eigenVectorsVolume.eigenvalues().transpose() << std::endl;

	//std::cout << " volume c o g :  " << cogVolume.transpose() << std::endl;

	typedef std::pair<Eigen::Vector3d, double> VecValuePair;

	std::vector<VecValuePair> eigenVectorValuePairsCAD(3), eigenVectorValuePairsVolume(3);

	for (int ii = 0; ii < 3; ii++)
	{
		eigenVectorValuePairsCAD[ii].first = eigenVectorsCAD.row(ii);
		eigenVectorValuePairsCAD[ii].second = eigenValuesCAD(ii);
	
		eigenVectorValuePairsVolume[ii].first = eigenVectorsVolume.row(ii);
		eigenVectorValuePairsVolume[ii].second = eigenValuesVolume(ii);
	}

	std::sort(eigenVectorValuePairsCAD.begin(), eigenVectorValuePairsCAD.end(), [](const VecValuePair& left, const VecValuePair& right)->bool {
		
		return left.second > right.second;
		});

	std::sort(eigenVectorValuePairsVolume.begin(), eigenVectorValuePairsVolume.end(), [](const VecValuePair& left, const VecValuePair& right)->bool {

		return left.second > right.second;
		});

	Eigen::Matrix3d volumeEigenVecs, cadEigenVecs;

	for (int ii = 0; ii < 3; ii++)
	{
		//eigenVectorValuePairsCAD[ii].first = eigenVectorsCAD.row(ii);
		//eigenVectorValuePairsCAD[ii].second = eigenValuesCAD(ii);

		//eigenVectorValuePairsVolume[ii].first = eigenVectorsVolume.row(ii);
		//eigenVectorValuePairsVolume[ii].second = eigenValuesVolume(ii);

		std::cout << eigenVectorValuePairsCAD[ii].first.transpose() << "  --  " << eigenVectorValuePairsCAD[ii].second << std::endl;

		cadEigenVecs.row(ii) = eigenVectorValuePairsCAD[ii].first;
	}

	for (int ii = 0; ii < 3; ii++)
	{
		//eigenVectorValuePairsCAD[ii].first = eigenVectorsCAD.row(ii);
		//eigenVectorValuePairsCAD[ii].second = eigenValuesCAD(ii);

		//eigenVectorValuePairsVolume[ii].first = eigenVectorsVolume.row(ii);
		//eigenVectorValuePairsVolume[ii].second = eigenValuesVolume(ii);

		std::cout << eigenVectorValuePairsVolume[ii].first.transpose() << "  --  " << eigenVectorValuePairsVolume[ii].second << std::endl;

		volumeEigenVecs.row(ii) = eigenVectorValuePairsVolume[ii].first;
	}

	std::cout << " volume c o g :  " << cogVolume.transpose() << std::endl;

	Eigen::Matrix4d alignmentTransform = computeTransform(cogCAD, cadEigenVecs, cogVolume, volumeEigenVecs);

	std::cout << "automatic alignment transform : " << alignmentTransform << std::endl;

	//viewIsoSurface(volInfo, iso50Th);


	//size_t nComponents = 0;//bodies.Count;

	//fread(&nComponents, sizeof(size_t), 1, file);

	//std::cout << "number of components : " << nComponents << std::endl;


	//for (int cc = 0; cc < nComponents; cc++)
	{
		size_t num_points, num_indices;
		fread(&num_points, sizeof(size_t), 1, file);
		fread(&num_indices, sizeof(size_t), 1, file);

		std::vector<Eigen::Vector3d> vertices(num_points) , normals(num_points);
		std::vector<size_t> indices(num_indices);

		fread(vertices.data(), sizeof(Eigen::Vector3d), num_points, file);
		fread(normals.data(), sizeof(Eigen::Vector3d), num_points, file);
		fread(indices.data(), sizeof(size_t), num_indices, file);

		std::vector<Eigen::Vector3f> verticesf(num_points);
		std::vector<unsigned int> displayIndices(num_indices);

		for (int vv = 0; vv < num_points; vv++)
		{
			verticesf[vv] = vertices[vv].cast<float>();
		}

		for (int ii = 0; ii < num_indices; ii++)
		{
			displayIndices[ii] = indices[ii];
		}

		verticesf.insert( verticesf.end() , volumeSurfacePoints.begin(), volumeSurfacePoints.end());

		displayIndices.insert(displayIndices.end(), volumeSurfaceIndices.begin(), volumeSurfaceIndices.end());

		for (int ii = num_points; ii < verticesf.size(); ii++)
		{
			Eigen::Vector4d transformedPoint = alignmentTransform * verticesf[ii].homogeneous().cast<double>();

			verticesf[ii] = transformedPoint.hnormalized().cast<float>();
		}

		for (int ii = num_indices; ii < displayIndices.size(); ii++)
		{
			displayIndices[ii] += num_points;
		}

		tr::Display3DRoutines::displayMesh(verticesf, displayIndices);

		//displayWithNormal(vertices, normals, displayIndices);

		double pointAreaRatio = averagePointCountAreaRatio(vertices, indices);

		std::cout << " point area raio : " << pointAreaRatio << std::endl;

		std::vector<Eigen::Vector3d> resampledPoints, resampledNormals;

		resampleCADPoints(vertices, normals, indices, pointAreaRatio, resampledPoints, resampledNormals);

		std::vector<Eigen::Vector3f> resampledPointsF(resampledPoints.size()), resampledNormalsF(resampledPoints.size());

		for (int pp = 0; pp < resampledPoints.size(); pp++)
		{
			resampledPointsF[pp] = resampledPoints[pp].cast<float>();
		}

		tr::Display3DRoutines::displayPointSet(resampledPointsF, std::vector<Eigen::Vector3f>(resampledPointsF.size(), Eigen::Vector3f(0, 1, 0)));
	}

	fclose(file);

	

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

	unsigned char* ptr = (unsigned char*)volumeData->GetScalarPointer();

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

void viewIsoSurface(unsigned char* data, int w, int h, int d, const Eigen::Vector3d& voxelSize, int isoVal)
{
	vtkSmartPointer< vtkImageData > volumeData = vtkSmartPointer< vtkImageData >::New();


	//volumeData->SetOrigin(volume.mVolumeOrigin(0), volume.mVolumeOrigin(1), volume.mVolumeOrigin(2));
	//volumeData->SetSpacing(volume.mVoxelStep(0), volume.mVoxelStep(1), volume.mVoxelStep(2));
	volumeData->SetDimensions(w, h, d);
	volumeData->SetExtent(0, w - 1, 0, h - 1, 0, d - 1);
	volumeData->AllocateScalars(VTK_UNSIGNED_CHAR, 1);

	long int nPoints = volumeData->GetNumberOfPoints();


	long int zStep = h * w;
	long int yStep = w;

	unsigned char* ptr = (unsigned char*)volumeData->GetScalarPointer();

	memcpy(ptr, data, w * h * d);

	vtkSmartPointer< vtkMarchingCubes > marchingCubes = vtkSmartPointer< vtkMarchingCubes >::New();

	marchingCubes->SetInputData(volumeData);

	marchingCubes->SetValue(0, isoVal);

	marchingCubes->ComputeNormalsOn();

	marchingCubes->Update(0);

	vtkSmartPointer< vtkPolyData > isoSurface = vtkSmartPointer< vtkPolyData >::New();

	isoSurface->DeepCopy(marchingCubes->GetOutput());

	tr::Display3DRoutines::displayPolyData(isoSurface);
}

void displayWithNormal(std::vector<Eigen::Vector3d>& vertices , std::vector<Eigen::Vector3d>& normals ,
	std::vector<unsigned int>& surfaceIndices)
{
	vtkSmartPointer< vtkPolyData > dataSet = vtkSmartPointer< vtkPolyData >::New();
	vtkSmartPointer< vtkPoints > points = vtkSmartPointer< vtkPoints >::New();
	vtkSmartPointer< vtkUnsignedCharArray > vColors = vtkSmartPointer< vtkUnsignedCharArray >::New();
	vtkSmartPointer< vtkDoubleArray > vNormals = vtkSmartPointer< vtkDoubleArray >::New();

	int numVerts = vertices.size();
	int numIndices = surfaceIndices.size() / 3;

	points->Allocate(numVerts);
	dataSet->Allocate(numIndices);

	vColors->SetNumberOfComponents(3);
	vColors->SetName("Colors");

	vNormals->SetNumberOfComponents(3);

	// Add the three colors we have created to the array
	vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();

	int id = 0;

	for (int vv = 0; vv < numVerts; vv++)
	{
		unsigned char col[] = { 255 , 255 , 255 };

		vColors->InsertNextTypedTuple(col);

		points->InsertNextPoint(vertices[vv](0), vertices[vv](1), vertices[vv](2));

		vNormals->InsertNextTypedTuple(normals[vv].data());

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
	dataSet->GetPointData()->SetScalars(vColors);
	dataSet->GetPointData()->SetNormals(vNormals);

	//vtkSmartPointer< vtkButterflySubdivisionFilter> subdivider = vtkSmartPointer< vtkButterflySubdivisionFilter>::New();

	//subdivider->SetInputData(dataSet);
	//subdivider->SetNumberOfSubdivisions(2);
	//subdivider->Update();

	//dataSet->DeepCopy(subdivider->GetOutput());

	tr::Display3DRoutines::displayPolyData(dataSet);
}


double averagePointCountAreaRatio(std::vector<Eigen::Vector3d>& points, std::vector<std::size_t>& indices)
{
	int nPoints = points.size();
	int nIndices = indices.size() / 3;

	double totalArea = 0;

	for (int ii = 0; ii < nIndices; ii++)
	{
		int id1 = indices[3 * ii];
		int id2 = indices[3 * ii + 1];
		int id3 = indices[3 * ii + 2];

		Eigen::Vector3d v1 = points[id1];
		Eigen::Vector3d v2 = points[id2];
		Eigen::Vector3d v3 = points[id3];

		double a = (v1 - v2).norm();
		double b = (v2 - v3).norm();
		double c = (v3 - v1).norm();

		double s = (a + b + c) * 0.5;

		double area = sqrt(s * (s - a) * (s - b) * (s - c));

		totalArea += area;
	}

	double pointsAreaRatio = points.size() / totalArea;

	return pointsAreaRatio;
}


void resampleCADPoints(std::vector<Eigen::Vector3d>& cadPoints, std::vector<Eigen::Vector3d>& cadNormals,
	std::vector<std::size_t>& cadIndices, double pointAreaRatio, std::vector<Eigen::Vector3d>& resampledPoints, 
	std::vector<Eigen::Vector3d>& resampledNormals)
{
	int nCadTriangles = cadIndices.size();

	std::default_random_engine generator;
	std::normal_distribution<double> distribution(1.0, 0.0);

	std::mt19937_64 rng;
	// initialize the random number generator with time-dependent seed
	uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::seed_seq ss{ uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed >> 32) };
	rng.seed(ss);
	// initialize a uniform distribution between 0 and 1
	std::uniform_real_distribution<double> unif(0, 1);

	for (int ii = 0; ii < 10; ii++)
	{
		std::cout << unif(rng) << " ";
	}


	std::cout << std::endl;

	std::vector<Eigen::Vector3f> sampledPoints;

	int nTriangles = cadIndices.size() / 3;

	for (int ii = 0; ii < nTriangles; ii++)
	{
		int id1 = cadIndices[3 * ii];
		int id2 = cadIndices[3 * ii + 1];
		int id3 = cadIndices[3 * ii + 2];

		Eigen::Vector3d v1 = cadPoints[id1];
		Eigen::Vector3d v2 = cadPoints[id2];
		Eigen::Vector3d v3 = cadPoints[id3];

		Eigen::Vector3d vec1 = v2 - v1;
		Eigen::Vector3d vec2 = v3 - v2;

		Eigen::Vector3d n = (vec1.cross(vec2)).normalized();

		double a = (v1 - v2).norm();
		double b = (v2 - v3).norm();
		double c = (v3 - v1).norm();

		double s = (a + b + c) * 0.5;

		double area = sqrt(s * (s - a) * (s - b) * (s - c));

		int nPoints = floor(pointAreaRatio * area);

		for (int pp = 0; pp < nPoints; pp++)
		{
			double t1 = unif(rng);
			double t2 = unif(rng);
			double t3 = unif(rng);

			Eigen::Vector3d sampledPoint = (t1 * v1 + t2 * v2 + t3 * v3) / (t1 + t2 + t3);

			sampledPoints.push_back(sampledPoint.cast<float>());

			resampledPoints.push_back(sampledPoint);
			resampledNormals.push_back(n);

		}
	}

	//tr::Display3DRoutines::displayPointSet(sampledPoints);

}


bool calculatePrincipalAxes( Eigen::Matrix3d& inertiaTensor, Eigen::Vector3d& eigenVector1, 
	Eigen::Vector3d& eigenVector2, Eigen::Vector3d& eigenVector3)
{
	Eigen::Vector3d eval;
	Eigen::Matrix3d evectors;

	Eigen::EigenSolver<Eigen::Matrix3d> eigenSolver;
	//Zeiss::IMT::Numerics::Solvers::SVDSolver svdsolv;
	//svdsolv.SVD(intertiaTensor, eval, evectors);

	auto eigenVectors = eigenSolver.compute(inertiaTensor);
	
	//if (svdsolv.Error())
		//return false;

	// sort the eigenvalues in descending order and sort the eigenvectors accordingly
	int n = 3;
	bool swapped;
	do
	{
		swapped = false;
		for (int i = 0; i < 2; ++i)
		{
			if (eval(i) < eval(i + 1))
			{
				double tmp = eval(i);
				eval(i) = eval(i + 1);
				eval(i + 1) = tmp;
				double tmpV1 = evectors(1, i);
				double tmpV2 = evectors(2, i);
				double tmpV3 = evectors(3, i);
				evectors(1, i) = evectors(1, i + 1);
				evectors(2, i) = evectors(2, i + 1);
				evectors(3, i) = evectors(3, i + 1);
				evectors(1, i + 1) = tmpV1;
				evectors(2, i + 1) = tmpV2;
				evectors(3, i + 1) = tmpV3;
				swapped = true;
			}
		}
		n--;
	} while (swapped && (n > 0));

	//check if we have already a right system
	double det = evectors(1, 1) * evectors(2, 2) * evectors(3, 3) +
		evectors(2, 1) * evectors(3, 2) * evectors(1, 3) +
		evectors(3, 1) * evectors(1, 2) * evectors(2, 3) -
		evectors(3, 1) * evectors(2, 2) * evectors(1, 3) -
		evectors(2, 1) * evectors(1, 2) * evectors(3, 3) -
		evectors(1, 1) * evectors(3, 2) * evectors(2, 3);

	// if not we make one
	if (det < 0.0)
	{
		evectors(1, 1) *= -1.0;
		evectors(2, 1) *= -1.0;
		evectors(3, 1) *= -1.0;
	}

	eigenVector1 = Eigen::Vector3d(evectors(1, 1), evectors(2, 1), evectors(3, 1));
	eigenVector2 = Eigen::Vector3d(evectors(1, 2), evectors(2, 2), evectors(3, 2));
	eigenVector3 = Eigen::Vector3d(evectors(1, 3), evectors(2, 3), evectors(3, 3));

	return true;//eigenVector1 != nullptr && eigenVector2 != nullptr && eigenVector3 != nullptr;
}


Eigen::Matrix4d computeTransform( const Eigen::Vector3d& cadCoG, const Eigen::Matrix3d& cadEigenVecs, 
	                   const  Eigen::Vector3d& volumeCoG, const Eigen::Matrix3d& volumeEigenVecs)
{

	std::cout << " CAD cog :  "<<cadCoG.transpose() << std::endl;
	std::cout << " volume cog : " <<volumeCoG.transpose() << std::endl;

	Eigen::Vector3d translation = cadCoG - volumeCoG;

	std::cout << " translation :  " << translation.transpose() << std::endl;

	Eigen::Vector3d cadXAxis = cadEigenVecs.row(0).normalized().transpose();
	Eigen::Vector3d cadYAxis = cadEigenVecs.row(1).normalized().transpose();
	Eigen::Vector3d cadZAxis = cadEigenVecs.row(2).normalized().transpose();

	Eigen::Vector3d volumeXAxis = volumeEigenVecs.row(0).normalized().transpose();
	Eigen::Vector3d volumeYAxis = volumeEigenVecs.row(1).normalized().transpose();
	Eigen::Vector3d volumeZAxis = volumeEigenVecs.row(2).normalized().transpose();

	Eigen::Matrix3d rCad = cadEigenVecs, rVolume = volumeEigenVecs;

	//rCad.row(0) = cadXAxis;
	//rCad.row(1) = cadYAxis;
	//rCad.row(2) = cadZAxis;

	//rVolume.row(0) = volumeXAxis;
	//rVolume.row(1) = volumeYAxis;
	//rVolume.row(2) = volumeZAxis;

	Eigen::Matrix3d rotation = rCad * rVolume.transpose();

	Eigen::Matrix4d transformation , translationMatrix , rotationMatrix;

	transformation.setIdentity();
	translationMatrix.setIdentity();
	rotationMatrix.setIdentity();

	rotationMatrix.block(0, 0, 3, 3) = rotation;
	translationMatrix.block(0, 3, 3, 1) = translation;

	transformation = rotationMatrix * translationMatrix;

	return transformation;
}



void computeIsoSurface( imt::volume::VolumeInfo& volume, int isoVal, std::vector<Eigen::Vector3f>& points, std::vector<unsigned int>& surfaceIndices )
{

	//unsigned short* volumeData, int32_t volumeWidth, int32_t volumeHeight, int32_t volumeDepth,
		//double voxelStepX, double voxelStepY, double voxelStepZ

	MultiResolutionMarchingCubes marchingCubes( (unsigned short*)volume.mVolumeData , volume.mWidth , volume.mHeight , 
		volume.mDepth , volume.mVoxelStep(0) , volume.mVoxelStep(1) , volume.mVoxelStep(2) );

	std::vector<double> vertices, normals;

	marchingCubes.compute(isoVal, vertices, normals, surfaceIndices);

	int nVertices = vertices.size() / 3;

	points.resize(nVertices);

	for (int vv = 0; vv < nVertices; vv++)
	{
		points[vv] = Eigen::Vector3f(vertices[3 * vv], vertices[3 * vv + 1], vertices[3 * vv + 2]);
	}

	//tr::Display3DRoutines::displayMesh(points, surfaceIndices);

}



