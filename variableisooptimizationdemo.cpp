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
#include "CTPointCloudEvaluationDecorator.h"
#include "vtkDecimatePro.h"
#include "vtkPLYReader.h"


void testPointCloudEvaluationDecorator(int w, int h, int d, unsigned short isoValue, const Eigen::Vector3d& voxelSideLengths, std::vector< Eigen::Vector3d >& inPositions,
	std::vector< Eigen::Vector3d >& inDir, std::vector< Eigen::Vector3d >& outPositions,
	std::vector< Eigen::Vector3d >& outNormals, std::vector< double >& outQualities, unsigned short *volumeData);

void initAndRefinedMeshes( const std::string& initMeshPath , const std::string& refinedMeshPath  );


int main( int argc , char **argv )
{

	std::string path1 = "C:/DebugData/initMesh.dat", path2 = "C:/DebugData/refinedMesh.dat";

	initAndRefinedMeshes(path1, path2);

	return 0;
	
	std::cout << " variable iso optimization " << std::endl;
	
	QString rawVolumePath = "C:/Data/WrongHistogramDataset/multi_material_part_T2R2_11.uint16_scv";

	imt::volume::VolumeInfo volume;

	imt::volume::RawVolumeDataIO::readUint16SCV( rawVolumePath , volume );

	std::cout << volume.mWidth << " " << volume.mHeight << " " << volume.mDepth << std::endl;

	int isoThreshold = 7988;

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

	//marchingCubes->SetInputData(volumeData);

	//marchingCubes->SetValue(0, isoThreshold);

	//marchingCubes->ComputeNormalsOn();

	//marchingCubes->Update(0);

	vtkSmartPointer< vtkPolyData > isoSurface = vtkSmartPointer< vtkPolyData >::New();

	//isoSurface->DeepCopy(marchingCubes->GetOutput());


	vtkSmartPointer< vtkPLYReader > reader = vtkSmartPointer< vtkPLYReader >::New();

	reader->SetFileName( "C:/DebugData/1.6_mesh.ply" );

	reader->Update();

	isoSurface->DeepCopy(reader->GetOutput());

	//std::cout << " reducing mesh " << isoSurface->GetNumberOfPoints() << " " << isoSurface->GetNumberOfCells() << std::endl;

	//vtkSmartPointer< vtkDecimatePro > decimator = vtkSmartPointer< vtkDecimatePro >::New();

	//decimator->SetInputData(isoSurface);

	////decimator->SetPreserveTopology(1);
	//decimator->SetTargetReduction(0.2);
	//decimator->Update();

	//isoSurface->DeepCopy(decimator->GetOutput());

	//std::cout << " mesh reduced to : " << isoSurface->GetNumberOfPoints() << std::endl;

	auto vNormals = isoSurface->GetPointData()->GetNormals();


	if (!vNormals)
	{
		std::cout << " normals not found " << std::endl;
	}

	std::cout << " display surface " << std::endl;

	//tr::Display3DRoutines::displayPolyData(isoSurface);

	CTEvaluationPartProgress *pointOptimizer = new CTEvaluationPartProgress(0, 0, 0);

	unsigned int numVertices = isoSurface->GetNumberOfPoints();

	std::vector< Eigen::Vector3d > initVertices( numVertices ), initNormals( numVertices ), refinedVertices( numVertices ), refinedNormals( numVertices );

	std::vector< double > qualities(numVertices);

	vtkSmartPointer< vtkDataArray > vtkNormals = isoSurface->GetPointData()->GetNormals();


	std::cout << " number of vertices : " << numVertices << std::endl;
	std::cout << " number of tuples : " << vtkNormals->GetNumberOfTuples() << " , number of components : " << vtkNormals->GetNumberOfComponents() << std::endl;

	std::vector< Eigen::Vector3f > displayVertices(numVertices);

	for ( unsigned int vv = 0; vv < numVertices; vv++ )
	{
		isoSurface->GetPoint(vv, (double*)(initVertices.data() + vv));

		vtkNormals->GetTuple(vv, (double*)( initNormals.data() + vv ));

		displayVertices[vv] = initVertices[vv].cast<float>();
	}


	std::vector< Eigen::Vector3f > colors( numVertices , Eigen::Vector3f( 1 , 0 , 0 ) );

	tr::Display3DRoutines::displayPointSet(displayVertices, colors);

	std::cout << " apply variable iso optimization on the surface " << std::endl;

	testPointCloudEvaluationDecorator(volume.mWidth, volume.mHeight, volume.mDepth, isoThreshold, volume.mVoxelStep.cast<double>(),
		initVertices, initNormals, refinedVertices, refinedNormals, qualities, (unsigned short*)volume.mVolumeData);

	std::cout << " variable iso optimization finished " << std::endl;

	
	return 0;
}


void testPointCloudEvaluationDecorator( int w, int h, int d , unsigned short isoValue , const Eigen::Vector3d& voxelSideLengths, std::vector< Eigen::Vector3d >& inPositions,
	                                    std::vector< Eigen::Vector3d >& inDir , std::vector< Eigen::Vector3d >& outPositions , 
										std::vector< Eigen::Vector3d >& outNormals , std::vector< double >& outQualities, unsigned short *volumeData)
{

	//unsigned short *volumeData = volume.data();

	void **sliceData = new void *[d];

	for (int zz = 0; zz < d; zz++)
	{
		sliceData[zz] = volumeData + (w * h * zz);
	}

	unsigned int bitDepth = 16;

	l3d size;
	d3d voxelSize;
	int interpolationMethod = 2;
	bool useAutoParams;
	double gradientThreshold = 0.05, sigma = 1.5;
	bool useBeamHardeningCorrection = false, considerStaticThreshold = false;
	long staticThreshold = isoValue;
	bool useAngleCriteria = false;
	double angleCriteria = 0 ;
	unsigned long long qualityThreshold = 100000;
	double searchRange = 5, searchRangeAirSide = 5;
	long count = inPositions.size();

	voxelSize[0] = voxelSideLengths(0);
	voxelSize[1] = voxelSideLengths(1);
	voxelSize[2] = voxelSideLengths(2);

	d3d *nominalPositions = (d3d*)inPositions.data(), *searchDirections = (d3d*)inDir.data(), 
		*results = (d3d*)outPositions.data(), *normals = (d3d*)outNormals.data();

	double *qualities = outQualities.data();

	ICTEvaluationProgress *progress = 0;

	size[0] = w;
	size[1] = h;
	size[2] = d;

	voxelSize[0] = voxelSideLengths(0);
	voxelSize[1] = voxelSideLengths(1);
	voxelSize[2] = voxelSideLengths(2);

	//interpolation methods

	//IPPI_INTER_NN - nearest neighbor interpolation,
	//IPPI_INTER_LINEAR - trilinear interpolation,
	//IPPI_INTER_CUBIC - tricubic interpolation,
	//IPPI_INTER_CUBIC2P_BSPLINE - B - spline,
	//IPPI_INTER_CUBIC2P_CATMULLROM - Catmull - Rom spline,
	//IPPI_INTER_CUBIC2P_B05C03 - special two - parameters filter(1 / 2, 3 / 10).

	interpolationMethod = IPPI_INTER_LINEAR;

	int nSets = count / 99000 + 1;

	int offSet = 0;

	for (int ss = 0; ss < nSets; ss++)
	{
		int setSize = std::min(99000, (int)(count - 99000 * ss));

		CCTPointCloudEvaluationDecorator::DoMeasureData( sliceData, bitDepth, size, voxelSize, interpolationMethod, useAutoParams, gradientThreshold,
			                                             sigma, useBeamHardeningCorrection, considerStaticThreshold, staticThreshold, useAngleCriteria,
														 angleCriteria, qualityThreshold, searchRange, searchRangeAirSide, setSize, nominalPositions + offSet, searchDirections + offSet,
														 results + offSet, normals + offSet, qualities + offSet, progress);

		offSet += setSize;

		std::cout << " set : " << ss << " ,  " << nSets << std::endl;

	}




}



void readMesh( const std::string& path, std::vector< Eigen::Vector3f >& meshPoints, std::vector< Eigen::Vector3f >& meshNormals, std::vector< unsigned int >& meshIndices)
{
	std::fstream reader(path, std::ios::in | std::ios::binary);

	unsigned int numVertices = 0, numTriangles = 0;

	reader.read((char*)&numVertices, sizeof(unsigned int));
	reader.read((char*)&numTriangles, sizeof(unsigned int));

	std::cout << " num verices and triangles : " << numVertices << " " << numTriangles << std::endl;

	std::vector< Eigen::Vector3d > points( numVertices ), normals( numVertices );
	std::vector< size_t > indices( numTriangles * 3 );

	reader.read((char*)points.data(), sizeof(Eigen::Vector3d) * numVertices);
	reader.read((char*)normals.data(), sizeof(Eigen::Vector3d) * numVertices);
	reader.read((char*)indices.data(), sizeof(size_t) * numTriangles * 3);

	meshPoints.resize(numVertices);
	meshNormals.resize(numVertices);

	for (int vv = 0; vv < numVertices; vv++)
	{
		meshPoints[vv] = points[vv].cast<float>();
		meshNormals[vv] = normals[vv].cast<float>();
	}

	meshIndices.resize(3 * numTriangles);

	for (int ff = 0; ff < numTriangles; ff++)
	{
		meshIndices[3 * ff] = indices[3 * ff];
		meshIndices[3 * ff + 1] = indices[3 * ff + 1];
		meshIndices[3 * ff + 2] = indices[3 * ff + 2];
	}


	reader.close();
}

void initAndRefinedMeshes( const std::string& initMeshPath , const std::string& refinedMeshPath )
{
	
	std::vector< Eigen::Vector3f > points1, points2, normals1, normals2;
	std::vector< unsigned int > indices1, indices2;

	readMesh(initMeshPath, points1, normals1, indices1);
	readMesh(refinedMeshPath, points2, normals2, indices2);

	std::vector< std::vector< Eigen::Vector3f > > verticesSets;
	std::vector< std::vector<unsigned int> > indicesSets;

	verticesSets.push_back(points1);
	verticesSets.push_back(points2);

	indicesSets.push_back(indices1);
	indicesSets.push_back(indices2);

	tr::Display3DRoutines::displayMeshes2ViewPort(verticesSets, indicesSets);


}
