
#include "iostream"
#include "string"
#include "display3droutines.h"
#include "vtkPLYReader.h"
#include "vtkSmartPointer.h"
#include "vtkCutter.h"
#include "vtkPlane.h"
#include "opencvincludes.h"
#include "vtkModifiedCutter.h"
#include "tracer/bvh.h"
#include "embreeincludes.h"
#include "vtkCellLocator.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "queue"
#include "vtkRectilinearGrid.h"
#include "vtkDoubleArray.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkIncrementalPointLocator.h"
#include "vtkContourHelper.h"
#include "vtkOptimizedCutter.h"
#include "vtkSTLReader.h"
#include "vtkIdList.h"
#include "planeslicercpu.h"
#include "planeslicercuda.h"
#include "vtkSinglePrecisionCutter.h"
#include "cpuslicer.h"

/* error reporting function */
void error_handler(const RTCError code, const char* str)
{
	if (code == RTC_NO_ERROR)
		return;

	printf("Embree: ");
	switch (code) {
	case RTC_UNKNOWN_ERROR: printf("RTC_UNKNOWN_ERROR"); break;
	case RTC_INVALID_ARGUMENT: printf("RTC_INVALID_ARGUMENT"); break;
	case RTC_INVALID_OPERATION: printf("RTC_INVALID_OPERATION"); break;
	case RTC_OUT_OF_MEMORY: printf("RTC_OUT_OF_MEMORY"); break;
	case RTC_UNSUPPORTED_CPU: printf("RTC_UNSUPPORTED_CPU"); break;
	case RTC_CANCELLED: printf("RTC_CANCELLED"); break;
	default: printf("invalid error code"); break;
	}
	if (str) {
		printf(" (");
		while (*str) putchar(*str++);
		printf(")\n");
	}


	exit(1);
}


void meshSliceCPU();
void naiveSlicer(std::vector< Eigen::Vector3f >& points, std::vector< unsigned int >& indices , std::vector< unsigned int >& edges ,
	             std::vector< unsigned int >& triangleEdges, Eigen::Vector3f& pos, Eigen::Vector3f& normal);
void buildEdges(int nVertices, std::vector< unsigned int >& triangles, std::vector< unsigned int >& edges, std::vector< unsigned int >& triangleEdges);

void addMeshToScene();

void meshSliceSinglePrecision();

void meshSliceGPU();

void cpuSlicerDemo();

int main( int argc , char **argv )
{

	//meshSliceSinglePrecision();
	//meshSliceCPU();
	meshSliceGPU();

	cpuSlicerDemo();

	return 0;

	std::string meshPath = "C:/Users/INASING1/Documents/My Received Files/separated_part_0.stl";//"C:/projects/Wallthickness/data/engine.ply"; //

	vtkSmartPointer< vtkSTLReader > reader = vtkSmartPointer< vtkSTLReader >::New();
	//vtkSmartPointer< vtkPLYReader > reader = vtkSmartPointer< vtkPLYReader >::New();


	reader->SetFileName(meshPath.c_str());

	reader->Update();

	vtkSmartPointer< vtkPolyData > meshData = reader->GetOutput();

	std::cout << " number of vertices and triangles : " << meshData->GetNumberOfPoints() << " " << meshData->GetNumberOfCells() << std::endl;

	tr::Display3DRoutines::displayPolyData(meshData);

	double bounds[6];

	meshData->GetBounds(bounds);

	std::cout << bounds[0] << " " << bounds[1] << " " << bounds[2] << " " << bounds[3] << " " << bounds[4] << " " << bounds[5] << std::endl;

	vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();

	double position[3] , normal[3];

	position[0] = (bounds[0] + bounds[1]) * 0.5;
	position[1] = (bounds[2] + bounds[3]) * 0.5;
	position[2] = (bounds[4] + bounds[5]) * 0.5;

	plane->SetOrigin(position[0], position[1], position[2]);
	
	Eigen::Vector3f n(0.5, 0.5, 0.5), p(position[0], position[1], position[2]);

	n.normalize();

	plane->SetNormal(n(0), n(1), n(2));


	std::cout << " computing slice " << std::endl;

	double initT = cv::getTickCount();

	vtkSmartPointer< vtkCutter > cutter = vtkSmartPointer< vtkCutter >::New();
	cutter->SetCutFunction(plane);
	cutter->SetInputData(meshData);
	cutter->Update();

	std::cout << " time spent in computing the slice : " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;
	
	tr::Display3DRoutines::displayPolyData(cutter->GetOutput());

	initT = cv::getTickCount();

	vtkSmartPointer< CellLocatorVisitor > locatorVisitor = vtkSmartPointer< CellLocatorVisitor >::New();

	locatorVisitor->SetDataSet(meshData);

	locatorVisitor->Update();

	initT = cv::getTickCount();

	vtkSmartPointer< vtkOptimizedCutter > cutter2 = vtkSmartPointer< vtkOptimizedCutter >::New();
	cutter2->setLocatorVisitor(locatorVisitor);
	cutter2->SetCutFunction(plane);
	cutter2->SetInputData(meshData);
	cutter2->Update();

	std::cout << " time spent in computing the slice 2 : " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;

	initT = cv::getTickCount();

	//Mesh mesh(meshData, false);

	//Bvh bvh(mesh);

	std::cout << " bvh build time : " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;

	tr::Display3DRoutines::displayPolyData( cutter2->GetOutput() );

	int nVerts = meshData->GetNumberOfPoints();
	int nTris = meshData->GetNumberOfCells();

	std::vector< Eigen::Vector3f > vertices( nVerts );
	std::vector< unsigned int > indices(3 * nTris);

	for (int vv = 0; vv < nVerts; vv++)
	{
		double pt[3];

		meshData->GetPoint(vv, pt);

		vertices[vv](0) = pt[0];
		vertices[vv](1) = pt[1];
		vertices[vv](2) = pt[2];

	}

	vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();

	for ( int tt = 0; tt < nTris; tt++ )
	{
		meshData->GetCellPoints(tt, triangle);

		indices[3 * tt] = triangle->GetId(0);
		indices[3 * tt + 1] = triangle->GetId(1);
		indices[3 * tt + 2] = triangle->GetId(2);
	}

	std::vector< unsigned int > edges, triangleEdges;

	//buildEdges(meshData->GetNumberOfPoints(), indices, edges, triangleEdges);

	naiveSlicer( vertices , indices , edges , triangleEdges , p, n );

	//std::vector< Eigen::Vector3f >& points, std::vector< unsigned int >& indices, std::vector< unsigned int >& edges,
	//	std::vector< unsigned int >& triangleEdges, Eigen::Vector3f& pos, Eigen::Vector3f& normal


	/* for best performance set FTZ and DAZ flags in MXCSR control and status register */
	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
	_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

	/* create new Embree device and force bvh4.triangle4v hierarchy for triangles */
	RTCDevice device = rtcNewDevice("tri_accel=bvh4.triangle4v");
	rtcDeviceGetError(device);

	/* set error handler */
	rtcDeviceSetErrorFunction(device, error_handler);

	/* create scene */
	RTCScene scene = rtcDeviceNewScene(device, RTC_SCENE_STATIC, RTC_INTERSECT1);

	//BVH4* bvh4 = nullptr;

	///* if the scene contains only triangles, the BVH4 acceleration structure can be obtained this way */
	//AccelData* accel = ((Accel*)scene)->intersectors.ptr;
	//if (accel->type == AccelData::TY_BVH4)
	//	bvh4 = (BVH4*)accel;

	
	return 0;
}


void addMeshToScene( vtkSmartPointer< vtkPolyData > meshData )
{
	int numPoints = meshData->GetNumberOfPoints();
	int numCells = meshData->GetNumberOfCells();




}


void buildEdges( int nVertices , std::vector< unsigned int >& triangles, std::vector< unsigned int >& edges, std::vector< unsigned int >& triangleEdges)
{
	std::vector< std::vector< unsigned int > > vertexAdjacency( nVertices ) , edgeIds(nVertices) , triangleIds( nVertices );

	int nTris = triangles.size() / 3;

	for ( unsigned int tt = 0; tt < nTris; tt++ )
	{
		unsigned int vid1 = triangles[ 3 * tt];
		unsigned int vid2 = triangles[ 3 * tt + 1];
		unsigned int vid3 = triangles[ 3 * tt + 2];

		vertexAdjacency[vid1].push_back(vid2);
		vertexAdjacency[vid1].push_back(vid3);

		vertexAdjacency[vid2].push_back(vid1);
		vertexAdjacency[vid2].push_back(vid3);
		
		vertexAdjacency[vid3].push_back(vid1);
		vertexAdjacency[vid3].push_back(vid2);

		triangleIds[vid1].push_back(tt);
		triangleIds[vid1].push_back(tt);

		triangleIds[vid2].push_back(tt);
		triangleIds[vid2].push_back(tt);

		triangleIds[vid3].push_back(tt);
		triangleIds[vid3].push_back(tt);
	}

	for (unsigned int vv = 0; vv < nVertices; vv++)
	{
		edgeIds[vv].resize( vertexAdjacency[vv].size() , -1 );
	}



	unsigned int edgeId = 0;

	edges.reserve(nTris * 3);

//#pragma omp parallel for
	for ( int vv = 0; vv < nVertices; vv++ )
	{
		for ( auto it = vertexAdjacency[vv].begin() , it2 = edgeIds[vv].begin(); it != vertexAdjacency[vv].end(); it++ , it2++ )
		{
			if ( *it > vv )
			{
				int vid1 = vv;
				int vid2 = *it;

				edges.push_back(vid1);
				edges.push_back(vid2);

				*it2 = edgeId;

				edgeId++;
			}
			else
			{
				//find the edge id
				unsigned int vid = *it;

				for ( unsigned int vv2 = 0; vv2 < vertexAdjacency[vid].size() ; vv2++ )
				{
					unsigned int vid2 = vertexAdjacency[vid][vv2];

					if ( vid2 == vv )
					{
						*it2 = edgeIds[vid][vv2];
					}
				}

			}

		}
	}


	triangleEdges.resize(3 * nTris);

	for (unsigned int tt = 0; tt < nTris; tt++)
	{
		unsigned int vid1 = triangles[3 * tt];
		unsigned int vid2 = triangles[3 * tt + 1];
		unsigned int vid3 = triangles[3 * tt + 2];

		for ( int tt2 = 0; tt2 != triangleIds[vid1].size() ; tt2++ )
		{
			if (triangleIds[vid1][ tt2 ] == tt )
			{
				triangleEdges[ 3 * tt ] = edgeIds[vid1][tt2];
			}
		}

		for ( int tt2 = 0; tt2 != triangleIds[vid2].size(); tt2++ )
		{
			if (triangleIds[vid2][tt2] == tt)
			{
				triangleEdges[ 3 * tt + 1 ] = edgeIds[vid2][tt2];
			}
		}

		for (int tt2 = 0; tt2 != triangleIds[vid3].size(); tt2++)
		{
			if (triangleIds[vid3][tt2] == tt)
			{
				triangleEdges[ 3 * tt + 2 ] = edgeIds[vid3][tt2];
			}
		}

	}


}


void naiveSlicer( std::vector< Eigen::Vector3f >& points, std::vector< unsigned int >& indices, std::vector< unsigned int >& edges, 
	              std::vector< unsigned int >& triangleEdges, Eigen::Vector3f& pos, Eigen::Vector3f& normal )
{

	double initT = cv::getTickCount();

	int nTris = indices.size() / 3;

	int nIntersections = 0;

	std::vector< std::pair< Eigen::Vector3f, Eigen::Vector3f > > intersectedEdges;
	
	std::vector< Eigen::Vector3f > edgeVertices;

	edgeVertices.reserve(nTris / 5);

#pragma omp parallel for
	for ( int tt = 0; tt < nTris; tt++ )
	{
		int p = 0, n = 0;

		float f1 = ( -points[ indices[3 * tt] ] + pos ).dot(normal);
		float f2 = ( -points[ indices[3 * tt + 1] ] + pos ).dot(normal);
		float f3 = ( -points[ indices[3 * tt + 2] ] + pos ).dot(normal);

		p += f1 > 0;
		p += f2 > 0;
		p += f3 > 0;

		n += f1 < 0;
		n += f2 < 0;
		n += f3 < 0;

		if ( p < 3 && p > 0 )
		{
			nIntersections++;

			float l1 = f2 / ( points[ indices[3 * tt] ] - points[ indices[ 3 * tt + 1 ] ] ).dot(normal);
			float l2 = f3 / ( points[ indices[3 * tt + 1] ] - points[ indices[ 3 * tt + 2 ] ]).dot(normal);
			float l3 = f1 / ( points[ indices[3 * tt + 2] ] - points[ indices[ 3 * tt ] ] ).dot(normal);

			Eigen::Vector3f p1, p2;

			std::pair< unsigned int, unsigned int > edgeIdPair;

			bool p1Found = false , p2Found = false;

			if (l1 >= 0 && l1 <= 1)
			{
				p1 = l1 * points[ indices[3 * tt] ] + ( 1 - l1 ) * points[indices[3 * tt + 1]];

				p1Found = true;

				edgeIdPair.first = triangleEdges[3 * tt];
			}

			if ( l2 >= 0 && l2 <= 1 )
			{
				if (!p1Found)
				{
					p1 = l2 * points[indices[3 * tt + 1]] + ( 1 - l2 )  * points[indices[3 * tt + 2]];

					p1Found = true;

					edgeIdPair.first = triangleEdges[3 * tt + 1];
				}
				else
				{
					p2 = l2 * points[indices[3 * tt + 1]] + ( 1 - l2 )  * points[indices[3 * tt + 2]];

					edgeIdPair.second = triangleEdges[3 * tt + 1];

					p2Found = true;
				}
			}

			if (l3 >= 0 && l3 <= 1)
			{
				if ( !p2Found )				
				{
					p2 = l3 * points[ indices[ 3 * tt + 2 ] ] + ( 1 - l3 ) * points[ indices[ 3 * tt ] ];

					edgeIdPair.second = triangleEdges[3 * tt + 2];

					p2Found = true;
				}
			}

#pragma omp critical
			{
				edgeVertices.push_back(p1);
				edgeVertices.push_back(p2);
			}

		}
			
	}


	vtkSmartPointer< vtkPolyData > edgeData = vtkSmartPointer< vtkPolyData >::New();

	edgeData->Allocate(edgeVertices.size() / 2);

	vtkSmartPointer< vtkPoints > edgePoints = vtkSmartPointer< vtkPoints >::New();

	edgePoints->Allocate( edgeVertices.size() );

	int nEdges = edgeVertices.size() / 2;

	vtkSmartPointer< vtkIdList > edge = vtkSmartPointer< vtkIdList >::New();
	

	for (int ee = 0; ee < nEdges; ee++)
	{
		edge->Reset();

		edgePoints->InsertNextPoint( edgeVertices[2 * ee](0), edgeVertices[2 * ee](1), edgeVertices[2 * ee](2) );
		edgePoints->InsertNextPoint( edgeVertices[2 * ee + 1](0), edgeVertices[2 * ee + 1](1), edgeVertices[2 * ee + 1](2));

		edge->InsertNextId( 2 * ee );
		edge->InsertNextId( 2 * ee + 1 );

		edgeData->InsertNextCell(VTK_LINE, edge);
	}

	edgeData->SetPoints( edgePoints );

	std::cout << " time spent in naive slicer  : " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;

	std::cout << " number of edges cpu : " << nEdges << std::endl;

	tr::Display3DRoutines::displayPolyData(edgeData);

	std::vector< Eigen::Vector3f > edgeVertexColors( edgeVertices.size() , Eigen::Vector3f( 1 , 1 , 1 ) );

	tr::Display3DRoutines::displayPointSet(edgeVertices, edgeVertexColors);

}


void meshSliceCPU()
{
	std::string meshPath = "C:/Users/INASING1/Documents/My Received Files/separated_part_0.stl";//"C:/projects/Wallthickness/data/engine.ply"; //

	vtkSmartPointer< vtkSTLReader > reader = vtkSmartPointer< vtkSTLReader >::New();
	//vtkSmartPointer< vtkPLYReader > reader = vtkSmartPointer< vtkPLYReader >::New();

	reader->SetFileName(meshPath.c_str());

	reader->Update();

	vtkSmartPointer< vtkPolyData > meshData = reader->GetOutput();

	PlaneSlicerCPU planeSlicer;

	planeSlicer.setMeshData(meshData);

	float planePositions[3], planeNormals[3];

	double bounds[6];

	meshData->GetBounds(bounds);
	float position[3], normal[3];

	position[0] = (bounds[0] + bounds[1]) * 0.5;
	position[1] = (bounds[2] + bounds[3]) * 0.5;
	position[2] = (bounds[4] + bounds[5]) * 0.5;

	Eigen::Vector3f n(0.5, 0.5, 0.5), p(position[0], position[1], position[2]);

	n.normalize();

	memcpy(normal, n.data(), 3 * sizeof(float));

	std::cout << normal[0] << " " << normal[1] << " " << normal[2] << std::endl;

	std::vector< float > computedEdges;

	double t = cv::getTickCount();

	planeSlicer.computeSlice(position, normal, computedEdges);

	std::cout << " time spent : " << (cv::getTickCount() - t) / cv::getTickFrequency() << std::endl;

	std::cout << " num edges cpu : " << computedEdges.size() / 2 << std::endl;


}


void meshSliceGPU()
{
	std::string meshPath = "C:/projects/Wallthickness/data/engine.ply"; //"C:/Users/INASING1/Documents/My Received Files/separated_part_0.stl";//

	//vtkSmartPointer< vtkSTLReader > reader = vtkSmartPointer< vtkSTLReader >::New();
	vtkSmartPointer< vtkPLYReader > reader = vtkSmartPointer< vtkPLYReader >::New();

	reader->SetFileName(meshPath.c_str());

	reader->Update();

	vtkSmartPointer< vtkPolyData > meshData = reader->GetOutput();

	PlaneSlicerCUDA planeSlicer;

	planeSlicer.setMeshData(meshData);

	float planePositions[3], planeNormals[3];

	double bounds[6];

	meshData->GetBounds(bounds);
	float position[3], normal[3];

	std::cout << " mesh stats : "<< meshData->GetNumberOfCells() << " " << meshData->GetNumberOfPoints() << std::endl;

	position[0] = (bounds[0] + bounds[1]) * 0.5;
	position[1] = (bounds[2] + bounds[3]) * 0.5;
	position[2] = (bounds[4] + bounds[5]) * 0.5;

	Eigen::Vector3f n(0.5, 0.5, 0.5), p(position[0], position[1], position[2]);

	n.normalize();

	memcpy( normal , n.data() , 3 * sizeof( float ) );

	std::cout << normal[0] << "  ----- " << normal[1] << " " << normal[2] << std::endl;

	std::vector< float > computedEdges;

	double t = cv::getTickCount();

	planeSlicer.computeSlice( position , normal , computedEdges );

	std::vector< Eigen::Vector3f > points( computedEdges.size() / 3 );

	memcpy(points.data(), computedEdges.data(), computedEdges.size() * sizeof(float));

	std::vector< Eigen::Vector3f > colors( points.size() , Eigen::Vector3f(1, 0, 0) );

	//tr::Display3DRoutines::displayPointSet(points, colors);


	std::cout << " time spent : " << (cv::getTickCount() - t) / cv::getTickFrequency() << std::endl;

	std::cout << " num edges gpu : " << computedEdges.size() / 2 << std::endl;


	vtkSmartPointer< vtkPolyData > edgeData = vtkSmartPointer< vtkPolyData >::New();

	edgeData->Allocate( points.size() / 2 );

	vtkSmartPointer< vtkPoints > edgePoints = vtkSmartPointer< vtkPoints >::New();

	edgePoints->Allocate( points.size() );

	int nEdges = points.size() / 2;

	vtkSmartPointer< vtkIdList > edge = vtkSmartPointer< vtkIdList >::New();


	for (int ee = 0; ee < nEdges; ee++)
	{
		edge->Reset();

		edgePoints->InsertNextPoint(points[2 * ee](0), points[2 * ee](1), points[2 * ee](2));
		edgePoints->InsertNextPoint(points[2 * ee + 1](0), points[2 * ee + 1](1), points[2 * ee + 1](2));

		edge->InsertNextId(2 * ee);
		edge->InsertNextId(2 * ee + 1);

		edgeData->InsertNextCell(VTK_LINE, edge);
	}

	edgeData->SetPoints(edgePoints);

	
	tr::Display3DRoutines::displayPolyData(edgeData);

}


void meshSliceSinglePrecision()
{
	std::string meshPath = "C:/Users/INASING1/Documents/My Received Files/separated_part_0.stl";//"C:/projects/Wallthickness/data/engine.ply"; //

	vtkSmartPointer< vtkSTLReader > reader = vtkSmartPointer< vtkSTLReader >::New();
	//vtkSmartPointer< vtkPLYReader > reader = vtkSmartPointer< vtkPLYReader >::New();

	reader->SetFileName(meshPath.c_str());

	reader->Update();

	vtkSmartPointer< vtkPolyData > meshData = reader->GetOutput();

	PlaneSlicerCPU planeSlicer;

	planeSlicer.setMeshData(meshData);

	float planePositions[3], planeNormals[3];

	double bounds[6];

	meshData->GetBounds(bounds);
	float position[3], normal[3];

	position[0] = (bounds[0] + bounds[1]) * 0.5;
	position[1] = (bounds[2] + bounds[3]) * 0.5;
	position[2] = (bounds[4] + bounds[5]) * 0.5;

	Eigen::Vector3f n(0.5, 0.5, 0.5), p(position[0], position[1], position[2]);

	n.normalize();

	memcpy(normal, n.data(), 3 * sizeof(float));

	vtkSmartPointer< vtkSinglePrecisionCutter > cutter = vtkSmartPointer< vtkSinglePrecisionCutter >::New();

	cutter->setPlaneSlicer( &planeSlicer );

	cutter->setPlaneInfo( position , normal );

	cutter->Update();

	vtkSmartPointer< vtkPolyData > outputEdges = cutter->GetOutput();

	tr::Display3DRoutines::displayPolyData( outputEdges );



}


void cpuSlicerDemo()
{
	std::string meshPath = "C:/projects/Wallthickness/data/engine.ply"; //"C:/Users/INASING1/Documents/My Received Files/separated_part_0.stl";//

	//vtkSmartPointer< vtkSTLReader > reader = vtkSmartPointer< vtkSTLReader >::New();
	vtkSmartPointer< vtkPLYReader > reader = vtkSmartPointer< vtkPLYReader >::New();

	reader->SetFileName(meshPath.c_str());

	reader->Update();

	vtkSmartPointer< vtkPolyData > meshData = reader->GetOutput();

	float planePositions[3], planeNormals[3];

	double bounds[6];

	meshData->GetBounds(bounds);
	float position[3], normal[3];

	position[0] = (bounds[0] + bounds[1]) * 0.5;
	position[1] = (bounds[2] + bounds[3]) * 0.5;
	position[2] = (bounds[4] + bounds[5]) * 0.5;

	Eigen::Vector3f n(0.5, 0.5, 0.5), p(position[0], position[1], position[2]);

	n.normalize();

	memcpy(normal, n.data(), 3 * sizeof(float));

	int nVerts = meshData->GetNumberOfPoints();

	float *vertData = new float[7 * nVerts];
	int *indices = new int[3 * meshData->GetNumberOfCells()];

	int nTris = meshData->GetNumberOfCells();

	vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();

	for (int tt = 0; tt < nTris; tt++)
	{
		meshData->GetCellPoints(tt, triangle);



		indices[3 * tt] = triangle->GetId(0);
		indices[3 * tt + 1] = triangle->GetId(1);
		indices[3 * tt + 2] = triangle->GetId(2);

	}

	for (int pp = 0; pp < meshData->GetNumberOfPoints(); pp++)
	{
		double p[3];

		meshData->GetPoint( pp , p );

		vertData[7 * pp] = p[0];
		vertData[7 * pp + 1] = p[1];
		vertData[7 * pp + 2] = p[2];
	}

	CPUSlicer slicer( vertData , nVerts , indices , nTris );

	float a = n(0), b = n(1), c = n(2), d = -(a * p(0) + b * p(1) + c * p(2) );

	float *resultArray = new float[4 * nVerts];

	int nOutputEdges = 0;

	double initT = cv::getTickCount();
	slicer.computeSlice(a, b, c, d, resultArray, nOutputEdges);

	std::cout << " time spent : " << (cv::getTickCount() - initT) / cv::getTickFrequency() * 1000 << std::endl;

	vtkSmartPointer< vtkPolyData > edgeData = vtkSmartPointer< vtkPolyData >::New();

	edgeData->Allocate( nOutputEdges );

	vtkSmartPointer< vtkPoints > edgePoints = vtkSmartPointer< vtkPoints >::New();

	edgePoints->Allocate( nOutputEdges * 2);

	int nEdges = nOutputEdges;

	vtkSmartPointer< vtkIdList > edge = vtkSmartPointer< vtkIdList >::New();


	for (int ee = 0; ee < nEdges; ee++)
	{
		edge->Reset();

		edgePoints->InsertNextPoint(resultArray[14 * ee], resultArray[14 * ee + 1], resultArray[14 * ee + 2]);
		edgePoints->InsertNextPoint(resultArray[14 * ee + 7], resultArray[14 * ee + 8], resultArray[14 * ee + 9]);

		edge->InsertNextId(2 * ee);
		edge->InsertNextId(2 * ee + 1);

		edgeData->InsertNextCell(VTK_LINE, edge);
	}

	edgeData->SetPoints(edgePoints);


	tr::Display3DRoutines::displayPolyData(edgeData);

}



