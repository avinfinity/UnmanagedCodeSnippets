
#include "iostream"
#include "anglesearchtree.h"
#include "volumeutility.h"
#include "rawvolumedataio.h"
#include "volumeinfo.h"
#include "display3droutines.h"
#include "vtkSmartPointer.h"
#include "vtkImageData.h"
#include "vtkMarchingCubes.h"
#include "vtkIdList.h"
#include "vtkPointData.h"
#include "wallthicknessestimator.h"
#include "anglesearchtree.h"
#include "pointcloudsearchoctree.h"
#include "meshviewer.h"

void viewIsoSurface(imt::volume::VolumeInfo& volume, int isoVal);

void convertToVertexAndIndices(vtkSmartPointer< vtkPolyData > mesh, std::vector< Eigen::Vector3f >& vertices, std::vector< Eigen::Vector3f >& normals, std::vector< unsigned int >& indices);

void octreeSearchDemo(std::vector< Eigen::Vector3d >& pointCloud , double resolution);

int main( int argc , char **argv )
{
	
	imt::volume::AngleSearchTree *searchTree;// = new imt::volume::AngleSearchTree(;

	QString dataPath = "G:/projects/Wallthickness/data/separated_part_7.uint16_scv";// "C:/projects/Wallthickness/data/CT multi/Calliper Assy 2016-6-9 10-12.uint16_scv"; //;

	imt::volume::RawVolumeDataIO io;

	imt::volume::VolumeInfo volInfo;

	//io.readHeader(dataPath, volInfo);

	io.readUint16SCV(dataPath, volInfo);

	std::cout << volInfo.mWidth << " " << volInfo.mHeight << " " << volInfo.mHeight << std::endl;

	std::cout << " voxel step : " << volInfo.mVoxelStep.transpose() << std::endl;

	std::vector< __int64 > histogram;// (USHRT_MAX);
	int minVal, maxVal;

	int isoThreshold = 0;

	imt::volume::VolumeUtility::computeISOThreshold(volInfo, isoThreshold, histogram, minVal, maxVal);

	std::cout << " iso threshold : " << isoThreshold << std::endl;

	imt::volume::VolumeUtility::extractIsoSurface(volInfo, isoThreshold);

	//std::cout << " num vertices : " << volInfo.mVertices.size() << " , num faces :  " << volInfo.mFaceIndices.size() / 3 << std::endl;

	viewIsoSurface(volInfo, isoThreshold);

	//compute wallthickness along the opposite of surface normals
	imt::volume::WallthicknessEstimator wte(volInfo);

	wte.computeBrepRayTraceThickness(volInfo);


	std::vector< Eigen::Vector3d > dVertices( volInfo.mVertices.size() ) , oppositeEnds(volInfo.mVertices.size());

	std::vector<unsigned int> oppositeFaceIndices(volInfo.mOppositeFaceIndices.size(), -1);

	for ( int vv = 0; vv < volInfo.mVertices.size(); vv++ )
	{
		dVertices[vv] = volInfo.mVertices[vv].cast< double >();
		oppositeEnds[vv] = volInfo.mOppVertices[vv].cast<double>();
		oppositeFaceIndices[vv] = volInfo.mOppositeFaceIndices[vv];
	}

	Eigen::Vector3d dVoxelStep = volInfo.mVoxelStep.cast< double >();

	//vc::MESHViewer::viewMesh(volInfo.mVertices, volInfo.mFaceIndices, &volInfo);

	imt::volume::AngleSearchTree ast(dVertices, volInfo.mFaceIndices, dVoxelStep);

	

	//ast.computeOppositeEnds(dVertices, oppositeFaceIndices , oppositeEnds);

	Eigen::Vector3d queryPoint(0, -2, 5) , foundPoint;

	unsigned int oppFaceIndex;

	//ast.findClosestPoint(queryPoint, oppFaceIndex, foundPoint);

	int inputVertexId = 0;

	queryPoint = dVertices[inputVertexId];//Eigen::Vector3d inputPoint

	oppFaceIndex = oppositeFaceIndices[inputVertexId];

	foundPoint = oppositeEnds[inputVertexId];


	Eigen::Vector3d coneAxis = foundPoint - queryPoint;

	coneAxis.normalize();

	float searchAngle = CV_PI / 6;

	Eigen::Vector3d closestPoint;

	unsigned int closestFaceIndex = 0;

	std::cout << "opposite face index : " << oppFaceIndex << std::endl;

	std::cout << " closest point   " << foundPoint.transpose() <<" "<<ast.geometricCenter(oppFaceIndex).transpose()<< std::endl;
	
	
	// searching in cone beam 
	ast.findClosestPoint(queryPoint, coneAxis, oppFaceIndex, searchAngle, closestPoint, closestFaceIndex);

	

	std::cout << " found closest point : " << closestPoint.transpose() << std::endl;

	//tr::Display3DRoutines::displayMesh(volInfo.mVertices, volInfo.mRayVertexColors, volInfo.mFaceIndices);

	std::cout << " octree search closest point  "<<foundPoint.transpose() << std::endl;

	//octreeSearchDemo(dVertices, volInfo.mVoxelStep(0) * 3);
	
	tr::Display3DRoutines::displayMesh(volInfo.mVertices, volInfo.mFaceIndices);


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

	long int zStep = volume.mHeight * volume.mWidth;
	long int yStep = volume.mWidth;

	unsigned char *ptr = (unsigned char *)volumeData->GetScalarPointer();

	std::cout << volume.mWidth << " " << volume.mHeight << " " << volume.mDepth << std::endl;

	memcpy(ptr, volume.mVolumeData, volume.mWidth * volume.mHeight * volume.mDepth * 2);

	vtkSmartPointer< vtkMarchingCubes > marchingCubes = vtkSmartPointer< vtkMarchingCubes >::New();

	marchingCubes->SetInputData(volumeData);

	marchingCubes->SetValue(0, isoVal);

	marchingCubes->ComputeNormalsOn();

	marchingCubes->Update(0);

	vtkSmartPointer< vtkPolyData > isoSurface = vtkSmartPointer< vtkPolyData >::New();

	isoSurface->DeepCopy(marchingCubes->GetOutput());

	//polulate the vertices and indices 
	convertToVertexAndIndices(isoSurface, volume.mVertices, volume.mVertexNormals , volume.mFaceIndices);

	//tr::Display3DRoutines::displayPolyData(isoSurface);


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



void octreeSearchDemo( std::vector< Eigen::Vector3d >& pointCloud , double resolution )
{
	imt::volume::PointCloudSearchOctree octree( pointCloud , resolution );

	Eigen::Vector3d queryPoint(0 , -2 , 5);

	octree.addPointsFromInputCloud();

	std::vector< int > closestIndices;
	std::vector< float > sqrDistances;

	octree.nearestKSearch( queryPoint, 6, closestIndices, sqrDistances);

	std::cout << " size of closest indices : " << closestIndices.size() << std::endl;


	//octree.getKNearestNeighborRecursive(queryPoint , 5 , )

}