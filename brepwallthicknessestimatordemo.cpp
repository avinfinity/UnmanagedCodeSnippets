#include "iostream"
#include "QString"
#include "volumeinfo.h"
#include "display3droutines.h"
#include "embreeincludes.h"
#include "opencvincludes.h"
#include "wallthicknessestimator.h"
#include "volumeutility.h"
#include "rawvolumedataio.h"
#include "vtkImageData.h"
#include "vtkImageMarchingCubes.h"
#include "vtkPolyDataNormals.h"
#include "vtkPLYWriter.h"
#include "vtkPointData.h"
#include "vtkIdList.h"

#undef near

struct EmbreeVertex
{
	float x, y, z, a;
};

struct EmbreeTriangle { int v0, v1, v2; };


void distanceTransformDemo();

void rayTraceDemo();

int main( int argc , char **argv )
{

	//distanceTransformDemo();

    rayTraceDemo();

	return 0;

	
	QString filePath = "C:/projects/Wallthickness/data/bottle/bottle.wtadat";//"C:/projects/Wallthickness/data/fanuc_engine_cover/engine_cover.wtadat";//	
	
	imt::volume::VolumeInfo volume;

	volume.loadVolume(filePath);

	std::cout << volume.mVertices.size() << " " << volume.mFaceIndices.size() / 3 << std::endl;

	

	tr::Display3DRoutines::displayMesh(volume.mVertices, volume.mFaceIndices);

	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
	_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

	rtcInit(NULL);

	RTCScene scene;

	unsigned int geomID;

	RTCDevice device;

	scene = 0;

	device = rtcNewDevice(NULL);

	long int numTriangles = volume.mFaceIndices.size() / 3;
	long int numVertices = volume.mVertices.size();

	scene = rtcDeviceNewScene(device, RTC_SCENE_DYNAMIC, RTC_INTERSECT8);

	geomID = rtcNewTriangleMesh(scene, RTC_GEOMETRY_DYNAMIC, numTriangles, numVertices, 1);

	EmbreeTriangle* triangles = (EmbreeTriangle*)rtcMapBuffer(scene, geomID, RTC_INDEX_BUFFER);

	rtcUnmapBuffer(scene, geomID, RTC_INDEX_BUFFER);

	EmbreeVertex* vertices = (EmbreeVertex*)rtcMapBuffer(scene, geomID, RTC_VERTEX_BUFFER);

	std::vector< unsigned int > indices;

	for (int tt = 0; tt < numTriangles; tt++)
	{
		
		triangles[tt].v0 = volume.mFaceIndices[3 * tt];
		triangles[tt].v1 = volume.mFaceIndices[3 * tt + 1];
		triangles[tt].v2 = volume.mFaceIndices[3 * tt + 2];


	}

	for (int pp = 0; pp < numVertices; pp++)
	{
		
		vertices[pp].x = volume.mVertices[pp](0);
		vertices[pp].y = volume.mVertices[pp](1);
		vertices[pp].z = volume.mVertices[pp](2);
		vertices[pp].a = 1.0;

	}

	rtcUnmapBuffer(scene, geomID, RTC_VERTEX_BUFFER);

	rtcCommit(scene);

	float maxLength = volume.mVoxelStep.norm() * volume.mWidth;

	std::vector< float > wallThickness( numVertices , -1 );

	double initT = cv::getTickCount();

	float near = volume.mVoxelStep.norm() * 0.01;

#pragma omp parallel for
	for ( int vv = 0; vv < numVertices; vv += 8 )
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

				ray.orgx[ii] = volume.mVertices[id](0);
				ray.orgy[ii] = volume.mVertices[id](1);
				ray.orgz[ii] = volume.mVertices[id](2);
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

				ray.dirx[ii] = -volume.mVertexNormals[id](0);
				ray.diry[ii] = -volume.mVertexNormals[id](1);
				ray.dirz[ii] = -volume.mVertexNormals[id](2);

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
					wallThickness[vv + ii] = ray.tfar[ii]; //ray.primID[ii];
				}
			}

		}
	
	std::cout << " time spent : " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;

	Eigen::Vector3f initColor(1, 0, 0) , middleColor( 0 , 1 , 0 ), endColor(0, 0, 1);

	Eigen::Vector3f colorStep = endColor - initColor;


	std::vector< Eigen::Vector3f > vertexColors( numVertices , Eigen::Vector3f(1 , 1 , 1) );

	double thickness = 0;
	int count = 0;

	for (int vv = 0; vv < numVertices; vv++)
	{
		if ( wallThickness[vv] < 0 )
			continue;

		if (wallThickness[vv] < 2.5)
		{
			vertexColors[vv] = initColor + (colorStep * wallThickness[vv] / 5.0);

			vertexColors[vv](0) =  1 - wallThickness[ vv ] / 2.5;
			vertexColors[vv](1) = wallThickness[vv] / 2.5;
			vertexColors[vv](2) = 0;
		}
		else if (wallThickness[vv] > 2.5 && wallThickness[vv] < 5)
		{
			vertexColors[vv](0) = 0;
			vertexColors[vv](1) = 1 - wallThickness[vv] / 2.5;
			vertexColors[vv](2) = wallThickness[vv] / 2.5;
		}

		thickness += wallThickness[vv];
		count++;
		
	}


	std::cout << " average thickness : " << thickness / count << std::endl;

	tr::Display3DRoutines::displayMesh(volume.mVertices, vertexColors, volume.mFaceIndices);

	return 0;
}


void distanceTransformDemo()
{
	QString filePath = "C:/projects/Wallthickness/data/bottle/bottle.wtadat";//"C:/projects/Wallthickness/data/fanuc_engine_cover/engine_cover.wtadat";//	

	imt::volume::VolumeInfo volume;

	volume.loadVolume(filePath);

	

	std::cout << volume.mVertices.size() << " " << volume.mFaceIndices.size() / 3 << std::endl;


//	tr::Display3DRoutines::displayMesh(volume.mVertices, volume.mFaceIndices);

	imt::volume::WallthicknessEstimator est(volume);


	std::cout << " computing distance transform cuda : " << std::endl;

	double initT = cv::getTickCount();

	//est.computeDistanceTransformCUDA(volume);

	std::cout << " distance transform cuda finished in : " <<( cv::getTickCount() - initT ) / cv::getTickFrequency() << " seconds "<< std::endl;

	est.computeBrepSphereThickness(volume);

	tr::Display3DRoutines::displayMesh(volume.mVertices, volume.mSphereVertexColors, volume.mFaceIndices);


}


void rayTraceDemo()
{
	//QString filePath = "C:/projects/Wallthickness/data/bottle/bottle.wtadat";//"C:/projects/Wallthickness/data/fanuc_engine_cover/engine_cover.wtadat";//	

	imt::volume::VolumeInfo volume;

	//volume.loadVolume(filePath);

	QString filePath = "C:/Data/WallthicknessData/11.1.17/Blister_Single 2016-10-19 8-55.uint16_scv";

	imt::volume::RawVolumeDataIO::readUint16SCV( filePath , volume );

	std::vector< int64_t > histogram;// (USHRT_MAX);
	int minVal, maxVal;

	int isoThreshold = 0;


	std::cout << volume.mWidth << " " << volume.mHeight << " " << volume.mDepth << std::endl;

	return ;

	imt::volume::VolumeUtility::computeISOThreshold(volume, isoThreshold, histogram, minVal, maxVal);

	volume.mAirVoxelFilterVal = isoThreshold;


	std::cout << volume.mVertices.size() << " " << volume.mFaceIndices.size() / 3 << std::endl;

	vtkSmartPointer< vtkImageData > volumeData = vtkSmartPointer< vtkImageData >::New();

	volumeData->SetOrigin(volume.mVolumeOrigin(0), volume.mVolumeOrigin(1), volume.mVolumeOrigin(2));
	volumeData->SetSpacing(volume.mVoxelStep(0), volume.mVoxelStep(1), volume.mVoxelStep(2));
	volumeData->SetExtent(0, volume.mWidth - 1, 0, volume.mHeight - 1, 0, volume.mDepth - 1);
	volumeData->AllocateScalars(VTK_UNSIGNED_SHORT, 1);

	long int nPoints = volumeData->GetNumberOfPoints();


	long int zStep = volume.mHeight * volume.mWidth;
	long int yStep = volume.mWidth;

	unsigned short *ptr = (unsigned short *)volumeData->GetScalarPointer();

	memcpy(ptr, volume.mVolumeData, volume.mWidth * volume.mHeight * volume.mDepth * 2);

	vtkSmartPointer< vtkImageMarchingCubes > marchingCubes = vtkSmartPointer< vtkImageMarchingCubes >::New();

	marchingCubes->SetInputData(volumeData);

	marchingCubes->SetValue(0, volume.mAirVoxelFilterVal);

	//marchingCubes->ComputeNormalsOn();

	marchingCubes->Update();



	std::cout << " computing iso surface finished " << std::endl;

	vtkSmartPointer< vtkPolyData > isoSurface = vtkSmartPointer< vtkPolyData >::New();

	isoSurface->DeepCopy(marchingCubes->GetOutput());



	vtkSmartPointer< vtkPolyDataNormals > normalEstimator = vtkSmartPointer< vtkPolyDataNormals >::New();

	normalEstimator->SetInputData(isoSurface);

	normalEstimator->ComputePointNormalsOn();

	normalEstimator->ComputeCellNormalsOn();

	normalEstimator->SplittingOff();

	//normalEstimator->NonManifoldTraversalOn();

	normalEstimator->ConsistencyOn();

	normalEstimator->Update();

	isoSurface->DeepCopy(normalEstimator->GetOutput());

	//vtkSmartPointer< vtkPLYWriter > writer = vtkSmartPointer< vtkPLYWriter >::New();

	//writer->SetInputData(isoSurface);

	//std::string meshPath = "C:/projects/Wallthickness/data/bottlemesh.ply";

	//writer->SetFileName(meshPath.c_str());

	//writer->Update();

	//tr::Display3DRoutines::displayPolyData(isoSurface);

	long int numPoints = isoSurface->GetNumberOfPoints();
	long int numTriangles = isoSurface->GetNumberOfCells();

	volume.mVertices.resize(numPoints);
	volume.mVertexNormals.resize(numPoints);
	volume.mFaceIndices.resize(numTriangles * 3);

	vtkSmartPointer<vtkDataArray> normals = isoSurface->GetPointData()->GetNormals();

	Eigen::Vector3f center1(0, 0, 0), center2(-4.5, 0, 0), center3(4.5, 0, 0);

	for (long int pp = 0; pp < numPoints; pp++)
	{
		double point[3];

		isoSurface->GetPoint(pp, point);

		volume.mVertices[pp](0) = point[0];
		volume.mVertices[pp](1) = point[1];
		volume.mVertices[pp](2) = point[2];

		double n[3];

		normals->GetTuple(pp, n);

		volume.mVertexNormals[pp](0) = n[0];
		volume.mVertexNormals[pp](1) = n[1];
		volume.mVertexNormals[pp](2) = n[2];

	}

	vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();


	for (long int tt = 0; tt < numTriangles; tt++)
	{
		isoSurface->GetCellPoints(tt, triangle);

		volume.mFaceIndices[3 * tt] = triangle->GetId(0);
		volume.mFaceIndices[3 * tt + 1] = triangle->GetId(1);
		volume.mFaceIndices[3 * tt + 2] = triangle->GetId(2);
	}

	//tr::Display3DRoutines::displayPointSet(volume.mVertices, volume.mVertexNormals);


	//tr::Display3DRoutines::displayMesh(volume.mVertices, volume.mFaceIndices);

	imt::volume::WallthicknessEstimator est(volume);

	//est.computeDistanceTransform(volume);

	est.computeBrepRayTraceThicknessMin(volume);

	tr::Display3DRoutines::displayMesh(volume.mVertices, volume.mRayVertexColors, volume.mFaceIndices);
}