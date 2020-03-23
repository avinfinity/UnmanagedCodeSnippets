

#include "iostream"
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
#include "wallthicknessestimator.h"
#include "atomic"
#include "embreeincludes.h"
#include "vtkIdList.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Triangulation_data_structure_2.h>
#include <CGAL/Triangulation_2.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Triangulation_vertex_base_with_info_2<int, K>    Vb;
typedef CGAL::Triangulation_data_structure_2<Vb>                    Tds;
typedef CGAL::Delaunay_triangulation_2<K, Tds>         Triangulation;
typedef Triangulation::Vertex_circulator Vertex_circulator;
typedef Triangulation::Point             Point;

typedef CGAL::Exact_predicates_tag                               Itag;
typedef CGAL::Constrained_Delaunay_triangulation_2<K, CGAL::Default, Itag> CDT;
typedef CDT::Point          ConstrainedPoint;

bool computeIntersectionWithPlane(Eigen::Vector3f& pos1, Eigen::Vector3f& pos2, Eigen::Vector3f& pos3, Eigen::Vector3f& n1, Eigen::Vector3f& n2, Eigen::Vector3f& n3,
	Eigen::Vector3f& wt, Eigen::Vector3f& planeCoeffs, int planeType, Eigen::Vector3f& end1, Eigen::Vector3f& end2, Eigen::Vector3f& end3, Eigen::Vector3f& end4 , Eigen::Vector3f& eN1 , Eigen::Vector3f& eN2);


class BAH{


public:

	BAH( std::vector< Eigen::Vector2f >& edges );

	bool findIntersection( const Eigen::Vector2f& pos, const Eigen::Vector2f& dir)
	{

	}

protected:

	void buildQuadTree();


protected:

	std::vector< Eigen::Vector2f >& mEdges;



};

void computeBrepRayTraceThickness(imt::volume::VolumeInfo& volinfo, RTCScene& scene, RTCDevice& device);

void buildTracer(imt::volume::VolumeInfo& volume, RTCDevice& mDevice, RTCScene& mScene);


void computeOppositeEndsOnPlane(std::vector< Eigen::Vector3f >& edgeEnds, std::vector< Eigen::Vector3f >& edgeNormals,
	std::vector< Eigen::Vector3f >& oppositeEnds, std::vector< bool >& oppositeEndFound,
	RTCDevice& device, RTCScene& scene, float vstep);

void computeTriangulation(std::vector< Eigen::Vector3f >& points, std::vector< Eigen::Vector3f >& triangulations);

int main( int argc , char **argv )
{

	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
	_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

	rtcInit(NULL);

	QString filePath = "C:/projects/Wallthickness/data/bottle/bottle.wtadat";//"C:/projects/Wallthickness/data/fanuc_engine_cover/engine_cover.wtadat";//	

	QString fileName = "C:/projects/Wallthickness/data/CT multi/Calliper Assy 2016-6-9 10-12";//"C:/projects/Wallthickness/data/Datasets for Multi-material/MultiMaterial/MAR_Ref 2012-8-30 15-13"; //"C:/projects/Wallthickness/data/CT multi/Hemanth_ Sugar free 2016-6-6 11-33";//"C: / projects / Wallthickness / data / CT multi / Fuel_filter_0km_PR_1 2016 - 5 - 30 12 - 54";//;
	//"C:/projects/Wallthickness/data/Mobile charger/Wall Thick_RnD_1 2016-10-13 15-10_sep1"; 
	QString scvFilePath = fileName + ".uint16_scv";

	QString vgiFilePath = fileName + ".vgi";

	QString stlFilePath = "C:/projects/Wallthickness/data/Mobile _ Charger.stl";

	imt::volume::VolumeInfo volume;

	volume.loadVolume(filePath);

	tr::Display3DRoutines::displayMesh(volume.mVertices, volume.mFaceIndices);

	imt::volume::WallthicknessEstimator wte( volume );

	std::cout << " estimating wall thickness " << std::endl;

	wte.computeBrepRayTraceThickness( volume );

	RTCDevice mDevice = wte.mDevice;

	RTCScene mScene = wte.mScene;

	//buildTracer(volume, mDevice, mScene);

	//computeBrepRayTraceThickness(volume, mScene, mDevice);
	

	std::cout << " wall thickness estimated " << volume.mWallthickness.size() << std::endl;

	int nTris = volume.mFaceIndices.size() / 3;

	Eigen::Vector3f planeCoeffs;

	planeCoeffs(0) = volume.mVolumeOrigin(0) + volume.mVoxelStep(0) * volume.mWidth / 2.0;
	planeCoeffs(1) = volume.mVolumeOrigin(1) + volume.mVoxelStep(1) * volume.mHeight / 2.0;
	planeCoeffs(2) = volume.mVolumeOrigin(2) + volume.mVoxelStep(2) * volume.mDepth / 2.0;

	int nTotalSegments = 0;

	std::vector< Eigen::Vector3f > edges , edgeNormals, facePointsE;


	for ( int tt = 0; tt < nTris; tt++ )
	{
		Eigen::Vector3f v1, v2, v3;

		int id1 = volume.mFaceIndices[3 * tt];
		int id2 = volume.mFaceIndices[3 * tt + 1];
		int id3 = volume.mFaceIndices[3 * tt + 2];

		v1 = volume.mVertices[id1];
		v2 = volume.mVertices[id2];
		v3 = volume.mVertices[id3];

		Eigen::Vector3f n1 = volume.mVertexNormals[id1];
		Eigen::Vector3f n2 = volume.mVertexNormals[id2];
		Eigen::Vector3f n3 = volume.mVertexNormals[id3];

		Eigen::Vector3f wt;

		wt(0) = volume.mWallthickness[id1];
		wt(1) = volume.mWallthickness[id2];
		wt(2) = volume.mWallthickness[id3];

		int planeType = 0; 

		Eigen::Vector3f end1, end2 , end3 , end4;

		if (wt(0) < 0 || wt(1) < 0 || wt(2) < 0  )
			continue;

		if ( n1.norm() > 1.1 || n2.norm() > 1.1 || n3.norm() > 1.1 || n1.norm() < 0.9 || n2.norm() < 0.9 || n3.norm() < 0.9 )
		{
			std::cout << " bad normal " << std::endl;
		}

		Eigen::Vector3f eN1, eN2;
		

		if ( computeIntersectionWithPlane( v1 , v2, v3, n1, n2, n3, wt, planeCoeffs, planeType, end1, end2, end3, end4 , eN1 , eN2 ) )
		{
			nTotalSegments++;

			edges.push_back(end1);
			edges.push_back(end2);

			facePointsE.push_back( end1 );
			facePointsE.push_back( end2 );
			facePointsE.push_back( end3 );
			facePointsE.push_back( end4 );

			eN1(0) = 0;
			eN2(0) = 0;

			eN1.normalize();
			eN2.normalize();

			edgeNormals.push_back(eN1); //(n1);//
			edgeNormals.push_back(eN2); //(n1); //
       }


	}

	std::vector< Eigen::Vector3f > oppEnds( edges.size() , Eigen::Vector3f(0 , 0 , 0));

	std::vector< bool > oppositeEndsFound(edges.size(), false);

	std::vector< Eigen::Vector3f > triangulatedPoints;

	computeTriangulation(edges , triangulatedPoints);

	computeOppositeEndsOnPlane(edges, edgeNormals, oppEnds, oppositeEndsFound , mDevice , mScene, volume.mVoxelStep(0));

	std::vector< Eigen::Vector3f > endColors(edges.size(), Eigen::Vector3f(1, 0, 0));

	//tr::Display3DRoutines::displayPointSet(oppEnds, endColors);


	std::vector< Eigen::Vector3f > colors(facePointsE.size(), Eigen::Vector3f(1, 0, 0));

	//tr::Display3DRoutines::displayPointSet(facePointsE, colors);

	vtkSmartPointer< vtkPolyData > edgeData = vtkSmartPointer< vtkPolyData >::New();

	vtkSmartPointer< vtkPolyData > faceData = vtkSmartPointer< vtkPolyData >::New();

	edgeData->Allocate(nTotalSegments);

	faceData->Allocate(nTotalSegments * 2);

	vtkSmartPointer< vtkPoints > edgePoints = vtkSmartPointer< vtkPoints >::New();

	edgePoints->Allocate(nTotalSegments * 2);

	vtkSmartPointer< vtkPoints > facePoints = vtkSmartPointer< vtkPoints >::New();

	edgePoints->Allocate(nTotalSegments * 4);

	int nEdges = nTotalSegments;

	vtkSmartPointer< vtkIdList > edge = vtkSmartPointer< vtkIdList >::New();

	vtkSmartPointer< vtkIdList > face = vtkSmartPointer< vtkIdList >::New();


	for (int ee = 0; ee < nEdges; ee++)
	{
		edge->Reset();

		edgePoints->InsertNextPoint(edges[2 * ee](0), edges[2 * ee](1), edges[2 * ee](2));
		edgePoints->InsertNextPoint(edges[2 * ee + 1](0), edges[2 * ee + 1](1), edges[2 * ee + 1](2));

		edge->InsertNextId(2 * ee);
		edge->InsertNextId(2 * ee + 1);

		edgeData->InsertNextCell(VTK_LINE, edge);


		face->Reset();

		//facePoints->InsertNextPoint( facePointsE[4 * ee](0), facePointsE[4 * ee](1), facePointsE[4 * ee](2));
		//facePoints->InsertNextPoint( facePointsE[4 * ee + 1](0), facePointsE[4 * ee + 1](1), facePointsE[4 * ee + 1](2));
		//facePoints->InsertNextPoint( facePointsE[4 * ee + 2](0), facePointsE[4 * ee + 2](1), facePointsE[4 * ee + 2](2));
		//facePoints->InsertNextPoint( facePointsE[4 * ee + 3](0), facePointsE[4 * ee + 3](1), facePointsE[4 * ee + 3](2));

		if (!oppositeEndsFound[2 * ee] || !oppositeEndsFound[2 * ee + 1])
		{
			continue;
		}

		facePoints->InsertNextPoint(edges[2 * ee](0), edges[2 * ee](1), edges[2 * ee](2));
		facePoints->InsertNextPoint(edges[2 * ee + 1](0), edges[2 * ee + 1](1), edges[2 * ee + 1](2));
		facePoints->InsertNextPoint(oppEnds[2 * ee](0), oppEnds[2 * ee](1), oppEnds[2 * ee](2));
		facePoints->InsertNextPoint(oppEnds[2 * ee + 1](0), oppEnds[2 * ee + 1](1), oppEnds[2 * ee + 1](2));

		face->InsertNextId(4 * ee);
		face->InsertNextId(4 * ee + 1);
		face->InsertNextId(4 * ee + 2);

		faceData->InsertNextCell( VTK_TRIANGLE, face);

		face->Reset();

		face->InsertNextId(4 * ee + 2);
		face->InsertNextId(4 * ee + 1);
		face->InsertNextId(4 * ee + 3);

		faceData->InsertNextCell( VTK_TRIANGLE, face);

	}

	edgeData->SetPoints(edgePoints);

	faceData->SetPoints(facePoints);

	tr::Display3DRoutines::displayPolyData(edgeData);

	tr::Display3DRoutines::displayPolyData(faceData);


	
	return 0;
}


void computeBrepRayTraceThickness(imt::volume::VolumeInfo& volinfo, RTCScene& scene, RTCDevice& device  )
{


#if 0
	unsigned int geomID;

	scene = 0;

	device = rtcNewDevice(NULL);

	long int numTriangles = volinfo.mFaceIndices.size() / 3;
	long int numVertices = volinfo.mVertices.size();

	volinfo.mOppVertices.resize(numVertices);

	scene = rtcDeviceNewScene(device, RTC_SCENE_DYNAMIC, RTC_INTERSECT8);

	geomID = rtcNewTriangleMesh(scene, RTC_GEOMETRY_DYNAMIC, numTriangles, numVertices, 1);

	EmbreeTriangle* triangles = (EmbreeTriangle*)rtcMapBuffer(scene, geomID, RTC_INDEX_BUFFER);

	rtcUnmapBuffer(scene, geomID, RTC_INDEX_BUFFER);

	EmbreeVertex* vertices = (EmbreeVertex*)rtcMapBuffer(scene, geomID, RTC_VERTEX_BUFFER);

	std::vector< unsigned int > indices;

	for (int tt = 0; tt < numTriangles; tt++)
	{

		triangles[tt].v0 = volinfo.mFaceIndices[3 * tt];
		triangles[tt].v1 = volinfo.mFaceIndices[3 * tt + 1];
		triangles[tt].v2 = volinfo.mFaceIndices[3 * tt + 2];

	}

	for (int pp = 0; pp < numVertices; pp++)
	{

		vertices[pp].x = volinfo.mVertices[pp](0);
		vertices[pp].y = volinfo.mVertices[pp](1);
		vertices[pp].z = volinfo.mVertices[pp](2);
		vertices[pp].a = 1.0;

	}

	rtcUnmapBuffer(scene, geomID, RTC_VERTEX_BUFFER);

	rtcCommit(scene);
#endif

	long int numTriangles = volinfo.mFaceIndices.size() / 3;
	long int numVertices = volinfo.mVertices.size();

	float maxLength = volinfo.mVoxelStep.norm() * volinfo.mWidth;

	//std::vector< float > wallThickness(numVertices, -1);

	volinfo.mWallthickness.resize(numVertices);

	std::fill(volinfo.mWallthickness.begin(), volinfo.mWallthickness.end(), -1);

	double initT = cv::getTickCount();

	float near = volinfo.mVoxelStep.norm() * 0.01;

#pragma omp parallel for
	for (int vv = 0; vv < numVertices + 8; vv += 8)
	{
		RTCRay8 ray;


		for (int ii = 0; ii < 8; ii++)
		{
			int id;

			if ((vv + ii) >= numVertices)
			{
				id = numVertices - 1;
			}
			else
			{
				id = vv + ii;
			}

			ray.orgx[ii] = volinfo.mVertices[id](0);
			ray.orgy[ii] = volinfo.mVertices[id](1);
			ray.orgz[ii] = volinfo.mVertices[id](2);
		}


		for (int ii = 0; ii < 8; ii++)
		{
			Eigen::Vector3f dir;

			int id;

			if ((vv + ii) >= numVertices)
			{
				id = numVertices - 1;
			}
			else
			{
				id = vv + ii;
			}

			ray.dirx[ii] = -volinfo.mVertexNormals[id](0);
			ray.diry[ii] = -volinfo.mVertexNormals[id](1);
			ray.dirz[ii] = -volinfo.mVertexNormals[id](2);

			ray.geomID[ii] = RTC_INVALID_GEOMETRY_ID;
			ray.instID[ii] = RTC_INVALID_GEOMETRY_ID;
			ray.primID[ii] = RTC_INVALID_GEOMETRY_ID;

			ray.tnear[ii] = near;
			ray.tfar[ii] = maxLength;

			ray.mask[ii] = 0xFFFFFFFF;
			ray.time[ii] = 0.f;
		}

		__aligned(32) int valid8[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

		rtcIntersect8(valid8, scene, ray);

		for (int ii = 0; ii < 8; ii++)
		{
			if ((vv + ii) >= numVertices)
			{
				continue;
			}

			if (ray.primID[ii] != RTC_INVALID_GEOMETRY_ID)
			{
				volinfo.mWallthickness[vv + ii] = ray.tfar[ii];

				volinfo.mOppVertices[vv + ii](0) = ray.orgx[ii] + ray.tfar[ii] * ray.dirx[ii];
				volinfo.mOppVertices[vv + ii](1) = ray.orgy[ii] + ray.tfar[ii] * ray.diry[ii];
				volinfo.mOppVertices[vv + ii](2) = ray.orgz[ii] + ray.tfar[ii] * ray.dirz[ii];
			}
		}

	}


	volinfo.mVertexColors.resize(numVertices, Eigen::Vector3f(1, 1, 1));
	volinfo.mRayVertexColors.resize(numVertices, Eigen::Vector3f(1, 1, 1));

	std::fill(volinfo.mVertexColors.begin(), volinfo.mVertexColors.end(), Eigen::Vector3f(1, 1, 1));
	std::fill(volinfo.mRayVertexColors.begin(), volinfo.mRayVertexColors.end(), Eigen::Vector3f(1, 1, 1));

	if (volinfo.mRayVertexColors.size() != volinfo.mSphereVertexColors.size())
	{
		volinfo.mSphereVertexColors.resize(volinfo.mRayVertexColors.size(), Eigen::Vector3f(1, 1, 1));
	}

	double thickness = 0;
	int count = 0;

	for (int vv = 0; vv < numVertices; vv++)
	{
		if (volinfo.mWallthickness[vv] < 0)
			continue;

		if (volinfo.mWallthickness[vv] < 2.5)
		{
			volinfo.mVertexColors[vv](0) = 1 - volinfo.mWallthickness[vv] / 2.5;
			volinfo.mVertexColors[vv](1) = volinfo.mWallthickness[vv] / 2.5;
			volinfo.mVertexColors[vv](2) = 0;

			volinfo.mRayVertexColors[vv](0) = 1 - volinfo.mWallthickness[vv] / 2.5;
			volinfo.mRayVertexColors[vv](1) = volinfo.mWallthickness[vv] / 2.5;
			volinfo.mRayVertexColors[vv](2) = 0;


		}
		else if (volinfo.mWallthickness[vv] > 2.5 && volinfo.mWallthickness[vv] < 5)
		{
			volinfo.mVertexColors[vv](0) = 0;
			volinfo.mVertexColors[vv](1) = 1 - volinfo.mWallthickness[vv] / 2.5;
			volinfo.mVertexColors[vv](2) = volinfo.mWallthickness[vv] / 2.5;


			volinfo.mRayVertexColors[vv](0) = 0;
			volinfo.mRayVertexColors[vv](1) = 1 - volinfo.mWallthickness[vv] / 2.5;
			volinfo.mRayVertexColors[vv](2) = volinfo.mWallthickness[vv] / 2.5;
		}

		thickness += volinfo.mWallthickness[vv];
		count++;

	}

	volinfo.mWallthicknessDataChanged = true;

	volinfo.updateSurfaceData();

}





void buildTracer(imt::volume::VolumeInfo& volume, RTCDevice& mDevice, RTCScene& mScene)
{
	unsigned int mGeomID;

	mDevice = rtcNewDevice(NULL);

	mScene = rtcDeviceNewScene(mDevice, RTC_SCENE_STATIC, RTC_INTERSECT1);

	long int numTriangles = volume.mFaceIndices.size() / 3;
	long int numVertices = volume.mVertices.size();

	std::cout << " num verts and faces : " << numVertices << " " << numTriangles << std::endl;

	struct EmbreeVertex
	{
		float x, y, z, a;
	};

	struct EmbreeTriangle { int v0, v1, v2; };


	mGeomID = rtcNewTriangleMesh(mScene, RTC_GEOMETRY_STATIC, numTriangles, numVertices, 1);

	EmbreeTriangle* triangles = (EmbreeTriangle*)rtcMapBuffer(mScene, mGeomID, RTC_INDEX_BUFFER);

	for (int tt = 0; tt < numTriangles; tt++)
	{
		triangles[tt].v0 = volume.mFaceIndices[3 * tt];
		triangles[tt].v1 = volume.mFaceIndices[3 * tt + 1];
		triangles[tt].v2 = volume.mFaceIndices[3 * tt + 2];
	}

	rtcUnmapBuffer(mScene, mGeomID, RTC_INDEX_BUFFER);

	EmbreeVertex* vertices = (EmbreeVertex*)rtcMapBuffer(mScene, mGeomID, RTC_VERTEX_BUFFER);

	int numBasePoints = numVertices;

	for (int pp = 0; pp < numBasePoints; pp++)
	{
		vertices[pp].x = volume.mVertices[pp](0);
		vertices[pp].y = volume.mVertices[pp](1);
		vertices[pp].z = volume.mVertices[pp](2);
		vertices[pp].a = 1.0;
	}

	rtcUnmapBuffer(mScene, mGeomID, RTC_VERTEX_BUFFER);

	rtcCommit(mScene);
}


void computeOppositeEndsOnPlane( std::vector< Eigen::Vector3f >& edgeEnds, std::vector< Eigen::Vector3f >& edgeNormals ,
	                             std::vector< Eigen::Vector3f >& oppositeEnds , std::vector< bool >& oppositeEndFound ,
								 RTCDevice& device , RTCScene& scene , float vstep )
{

	std::vector< Eigen::Vector3f > colors(edgeEnds.size(), Eigen::Vector3f(1, 0, 0));

	oppositeEndFound.resize(edgeEnds.size(), false);

	std::fill( oppositeEndFound.begin() , oppositeEndFound.end(), false);

	//tr::Display3DRoutines::displayPointSet(edgeEnds , colors);

	float maxLength = FLT_MAX;

	double initT = cv::getTickCount();

	float near = vstep * 0.01;

	std::cout << " vstep : " << vstep << std::endl;

	int numVertices = edgeEnds.size();

	std::atomic<int > nValidIntersection ;

	nValidIntersection = 0;

#pragma omp parallel for
	for (int vv = 0; vv < numVertices + 8; vv += 8)
	{
		RTCRay8 ray;

		for (int ii = 0; ii < 8; ii++)
		{
			int id;

			if ((vv + ii) >= numVertices)
			{
				id = numVertices - 1;
			}
			else
			{
				id = vv + ii;
			}

			ray.orgx[ii] = edgeEnds[id](0);
			ray.orgy[ii] = edgeEnds[id](1);
			ray.orgz[ii] = edgeEnds[id](2);
		}


		for (int ii = 0; ii < 8; ii++)
		{
			Eigen::Vector3f dir;

			int id;

			if ((vv + ii) >= numVertices)
			{
				id = numVertices - 1;
			}
			else
			{
				id = vv + ii;
			}

			//std::cout << edgeNormals[id].transpose() << std::endl;

			ray.dirx[ii] = -edgeNormals[id](0);
			ray.diry[ii] = -edgeNormals[id](1);
			ray.dirz[ii] = -edgeNormals[id](2);

			ray.geomID[ii] = RTC_INVALID_GEOMETRY_ID;
			ray.instID[ii] = RTC_INVALID_GEOMETRY_ID;
			ray.primID[ii] = RTC_INVALID_GEOMETRY_ID;

			ray.tnear[ii] = near;
			ray.tfar[ii] = maxLength;

			ray.mask[ii] = 0xFFFFFFFF;
			ray.time[ii] = 0.f;
		}

		__aligned(32) int valid8[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

		rtcIntersect8(valid8, scene, ray);

		for (int ii = 0; ii < 8; ii++)
		{
			if ((vv + ii) >= numVertices)
			{
				continue;
			}

			if (ray.primID[ii] != RTC_INVALID_GEOMETRY_ID)
			{
				oppositeEnds[vv + ii](0) = ray.orgx[ii] + ray.tfar[ii] * ray.dirx[ii];
				oppositeEnds[vv + ii](1) = ray.orgy[ii] + ray.tfar[ii] * ray.diry[ii];
				oppositeEnds[vv + ii](2) = ray.orgz[ii] + ray.tfar[ii] * ray.dirz[ii];

				oppositeEndFound[vv + ii] = true;

				nValidIntersection++;
			}
		}

	}


	std::cout << " number of valid intersections : " << nValidIntersection <<" "<<numVertices<< std::endl;
}


bool computeIntersectionWithPlane( Eigen::Vector3f& pos1, Eigen::Vector3f& pos2, Eigen::Vector3f& pos3, Eigen::Vector3f& n1, Eigen::Vector3f& n2,
	                               Eigen::Vector3f& n3 , Eigen::Vector3f& wt, Eigen::Vector3f& planeCoeffs, int planeType, Eigen::Vector3f& end1,
								   Eigen::Vector3f& end2, Eigen::Vector3f& end3, Eigen::Vector3f& end4, Eigen::Vector3f& eN1 , Eigen::Vector3f& eN2 )
{
	float diff1 = pos1(planeType) - planeCoeffs(planeType);
	float diff2 = pos2(planeType) - planeCoeffs(planeType);
	float diff3 = pos3(planeType) - planeCoeffs(planeType);

	int p = 0, n = 0;

	if (diff1 < 0)
		n++;
	else
		p++;

	if (diff2 < 0)
		n++;
	else
		p++;

	if (diff3 < 0)
		n++;
	else
		p++;

	if (p == 0 || n == 0)
		return false;

	n1(0) = 0;
	n2(0) = 0;
	n3(0) = 0;

	n1.normalize();
	n2.normalize();
	n3.normalize();

	float t1 = diff1 / (pos1(planeType) - pos2(planeType));
	float t2 = diff2 / (pos2(planeType) - pos3(planeType));
	float t3 = diff3 / (pos3(planeType) - pos1(planeType));

	Eigen::Vector3f interpN1, interpN2;
	float interpWt1, interpWt2;

	if (t1 > 0 && t1  < 1)
	{
		end1 = pos1 * t1 + pos2 * (1 - t1);

		interpN1 = n1 * t1 + n2 * (1 - t1);

		interpWt1 = wt[0] * t1 + wt[1] * (1 - t1);

		if (t2 > 0 && t2 < 1)
		{
			end2 = pos2 * t2 + pos3 * (1 - t2);

			interpN2 = n2 * t2 + n3 * (1 - t2);

			interpWt2 = wt[1] * t2 + wt[2] * (1 - t2);
		}
		else
		{
			end2 = pos3 * t3 + pos1 * (1 - t3);

			interpN2 = n3 * t3 + n1 * (1 - t3);

			interpWt2 = wt[2] * t3 + wt[0] * (1 - t3);
		}

		end3 = end1 - interpWt1 * interpN1;
		end4 = end2 - interpWt2 * interpN2; 

		eN1 = interpN1;
		eN2 = interpN2;

		return true;
	}
	else if (t2 > 0 && t2 < 1)
	{
		end1 = pos2 * t2 + pos3 * (1 - t2);
		end2 = pos3 * t3 + pos1 * (1 - t3);

		interpN1 = n2 * t2 + n3 * (1 - t2);
		interpN2 = n3 * t3 + n1 * (1 - t3);

		interpWt1 = wt[1] * t2 + wt[2] * (1 - t2);
		interpWt2 = wt[2] * t3 + wt[0] * (1 - t3);

		end3 = end1 - interpWt1 * interpN1;
		end4 = end2 - interpWt2 * interpN2;

		eN1 = interpN1;
		eN2 = interpN2;

		return true;
	}
	else
	{
		return false;
	}

}



void mergeDulicatePoints( std::vector< Eigen::Vector3f >& points )
{
	typedef std::pair< int, Eigen::Vector3f > IPoint;

	int numEdgePoints = points.size();

	std::vector< IPoint > vertexBufferWithIndices(numEdgePoints);

	for (int pp = 0; pp < numEdgePoints; pp++)
	{
		vertexBufferWithIndices[pp].first = pp;
		vertexBufferWithIndices[pp].second = points[pp];
	}

	std::sort(vertexBufferWithIndices.begin(), vertexBufferWithIndices.end(), [](const IPoint& p1, const IPoint& p2)->bool
	{

		return (p1.second(0) < p2.second(0)) || (p1.second(0) == p2.second(0) && p1.second(1) < p2.second(1) ) ||
			(p1.second(0) == p2.second(0) && p1.second(1) == p2.second(1) && p1.second(2) < p2.second(2));
	});


	std::vector< Eigen::Vector3f > vertexBuffer;
	//edgeIndices.reserve(edgePoints.size());
	vertexBuffer.reserve(points.size());

	int currentId = 0;

	vertexBuffer.push_back(vertexBufferWithIndices[0].second);

	for (int pp = 1; pp < numEdgePoints; pp++)
	{
		auto& pt = vertexBufferWithIndices[pp];
		auto& prevPt = vertexBufferWithIndices[pp - 1];

		float diff1 = std::abs(pt.second(0) - prevPt.second(0) );
		float diff2 = std::abs(pt.second(1) - prevPt.second(1) );
		float diff3 = std::abs(pt.second(2) - prevPt.second(2) );

		bool isDuplicate = diff1 < 0.00001f && diff2 < 0.00001f && diff3 < 0.00001f;

		if (!isDuplicate)
		{
			currentId++;

			vertexBuffer.push_back(pt.second);
		}
	}
}

void computeTriangulation( std::vector< Eigen::Vector3f >& points , std::vector< Eigen::Vector3f >& triangulations )
{
	mergeDulicatePoints(points);

	int numPoints = points.size();

	std::vector<  Point  > iPoints(numPoints);//std::pair< Point, int >

	for (int pp = 0; pp < numPoints; pp++)
	{
		Point pt( points[pp](1), points[pp](2) );

		

		iPoints[pp] = pt;//.first 
		//iPoints[pp].second = pp;
	}

	Triangulation dt;

	dt.insert( iPoints.begin(), iPoints.end() );

	int counter = 0;

	for (Triangulation::Finite_vertices_iterator vit = dt.finite_vertices_begin(); vit != dt.finite_vertices_end(); vit++)
	{
		Triangulation::Vertex_handle vh = vit;

		vh->info() = counter;

		counter++;
	}

	

	std::vector< unsigned int > indices;

	std::vector< Eigen::Vector3f > newPoints(points.size());

	//initialize s edges with smoothness costs
	for (Triangulation::Finite_faces_iterator fit = dt.finite_faces_begin(); fit != dt.finite_faces_end(); fit++)
	{
		Triangulation::Face_handle ch = fit;

		auto vh1 = ch->vertex(0);
		auto vh2 = ch->vertex(1);
		auto vh3 = ch->vertex(2);

		int vId1 = vh1->info();
		int vId2 = vh2->info();
		int vId3 = vh3->info();

		indices.push_back(vId1);
		indices.push_back(vId2);
		indices.push_back(vId3);

		auto& pt1 = vh1->point();
		auto& pt2 = vh1->point();
		auto& pt3 = vh1->point();

		newPoints[ vId1 ] = Eigen::Vector3f(pt1[0], pt1[1], pt1[2]);
		newPoints[vId2] = Eigen::Vector3f(pt2[0], pt2[1], pt2[2]);
		newPoints[vId3] = Eigen::Vector3f(pt3[0], pt3[1], pt3[2]);
	}

	std::vector< Eigen::Vector3f > colors(points.size(), Eigen::Vector3f(1, 0, 0));

	tr::Display3DRoutines::displayPointSet(newPoints, colors);

	tr::Display3DRoutines::displayMesh(newPoints, indices);


}


void constrainedTriangulation(std::vector< Eigen::Vector3f >& points)
{


}