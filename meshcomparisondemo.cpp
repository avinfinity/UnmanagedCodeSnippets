
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

#include "vtkPLYReader.h"
#include "vtkSTLReader.h"

#include "vtkPolyDataNormals.h"
#include "vtkIdList.h"
#include "vtkPolyDataMapper.h"
#include <vtkAppendPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkInteractorStyleTrackballCamera.h>

#include "rawvolumedataio.h"
#include "vtkPLYWriter.h"

void viewIsoSurface(imt::volume::VolumeInfo& volume, int isoVal);
void viewIsoSurface(imt::volume::VolumeInfo& volume, int isoVal1, int isoValue2);
void computeOppositeEnds(std::vector< Eigen::Vector3f >& edgeEnds, std::vector< Eigen::Vector3f >& edgeNormals,
	std::vector< Eigen::Vector3f >& oppositeEnds, std::vector< Eigen::Vector3f >& oppositeEndNormals,
	std::vector< bool >& oppositeEndFound, std::vector< float >& closestSurfaceDistances, RTCDevice& device, RTCScene& scene, float vstep);

void convertToVertexAndIndices( vtkSmartPointer< vtkPolyData > mesh, std::vector< Eigen::Vector3f >& vertices,std::vector< unsigned int >& indices)
{
	vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();

	vertices.resize(mesh->GetNumberOfPoints());
	indices.resize(mesh->GetNumberOfCells() * 3);

	int nPoints = mesh->GetNumberOfPoints();



	for (int pp = 0; pp < nPoints; pp++)
	{

		double pt[3];

		mesh->GetPoint(pp, pt );

		vertices[pp](0) = pt[0];
		vertices[pp](1) = pt[1];
		vertices[pp](2) = pt[2];


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


void testThresholdVariation()
{
	QString volumePath = "G:\\Data\\CAD, Object data\\cube.uint16_scv";

	imt::volume::VolumeInfo volume;

	imt::volume::RawVolumeDataIO::readUint16SCV(volumePath, volume);

	std::vector<long> histogram(USHRT_MAX + 1 , 0);
	
	unsigned short* vData = (unsigned short*)volume.mVolumeData;

	for (int64_t v = 0; v < volume.mWidth * volume.mHeight * volume.mDepth; v++)
	{
		histogram[vData[v]]++;
	}


	imt::volume::HistogramFilter hf(&volume);
	
	int th =  hf.ISO50Threshold(histogram);

	std::cout << "threshold : " << th << std::endl;

	//viewIsoSurface(volume, th);

	hf.plotHistogramVTK(histogram);

	int th2 = th + 5000;

	viewIsoSurface(volume, th, th2);

}

void viewIsoSurface(imt::volume::VolumeInfo& volume, int isoVal1 , int isoValue2)
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

	unsigned char *ptr = (unsigned char *)volumeData->GetScalarPointer();

	memcpy(ptr, volume.mVolumeData, volume.mWidth * volume.mHeight * volume.mDepth * 2);

	vtkSmartPointer< vtkMarchingCubes > marchingCubes1 = vtkSmartPointer< vtkMarchingCubes >::New();

	marchingCubes1->SetInputData(volumeData);

	marchingCubes1->SetValue(0, isoVal1);

	marchingCubes1->ComputeNormalsOn();

	marchingCubes1->Update(0);

	vtkSmartPointer< vtkPolyData > isoSurface1 = vtkSmartPointer< vtkPolyData >::New();

	isoSurface1->DeepCopy(marchingCubes1->GetOutput());

	vtkSmartPointer< vtkUnsignedCharArray > vColors = vtkSmartPointer< vtkUnsignedCharArray >::New();

	vColors->SetNumberOfComponents(3);
	vColors->SetName("Colors");

	int numPoints = isoSurface1->GetNumberOfPoints();

	for (int pp = 0; pp < numPoints; pp++)
	{
		unsigned char col[] = { 255  , 0, 0 };

		vColors->InsertNextTypedTuple(col);
	}

	isoSurface1->GetPointData()->SetScalars(vColors);

	vtkSmartPointer< vtkMarchingCubes > marchingCubes2 = vtkSmartPointer< vtkMarchingCubes >::New();

	marchingCubes2->SetInputData(volumeData);

	marchingCubes2->SetValue(0, isoValue2);

	marchingCubes2->ComputeNormalsOn();

	marchingCubes2->Update(0);

	vtkSmartPointer< vtkPolyData > isoSurface2 = vtkSmartPointer< vtkPolyData >::New();

	isoSurface2->DeepCopy(marchingCubes2->GetOutput());

	numPoints = isoSurface2->GetNumberOfPoints();

	vtkSmartPointer< vtkUnsignedCharArray > vColors2 = vtkSmartPointer< vtkUnsignedCharArray >::New();

	vColors2->SetNumberOfComponents(3);
	vColors2->SetName("Colors");

	for (int pp = 0; pp < numPoints; pp++)
	{
		unsigned char col[] = { 0  , 255, 0 };

		vColors2->InsertNextTypedTuple(col);
	}

	isoSurface2->GetPointData()->SetScalars(vColors2);

	vtkSmartPointer<vtkPLYWriter> writer1 = vtkSmartPointer<vtkPLYWriter>::New();
	vtkSmartPointer<vtkPLYWriter> writer2 = vtkSmartPointer<vtkPLYWriter>::New();
	
	writer1->SetFileName("C:\\Data\\mesh1.ply");
	writer1->SetInputData(isoSurface1);
	writer1->Update();

	writer2->SetFileName("C:\\Data\\mesh2.ply");
	writer2->SetInputData(isoSurface2);
	writer2->Update();

	//for (int mm = 0; mm < nMaterials - 1; mm++)
	//{
		//materialSurfaces[mm] = isoSurface(volume, materials.regions[mm + 1].lower_bound, colors[mm].block(0, 0, 3, 1));

		vtkSmartPointer<vtkRenderer> renderer1 = vtkSmartPointer<vtkRenderer>::New();

		vtkSmartPointer<vtkPolyDataMapper> mapper1 =
			vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper1->SetInputData(isoSurface1);

		vtkSmartPointer<vtkActor> actor1 =
			vtkSmartPointer<vtkActor>::New();
		actor1->GetProperty()->SetColor(255, 0, 0);
		actor1->GetProperty()->SetOpacity(1);
		actor1->SetMapper(mapper1);

		
		renderer1->AddActor(actor1);


		vtkSmartPointer<vtkRenderer> renderer2 = vtkSmartPointer<vtkRenderer>::New();

		vtkSmartPointer<vtkPolyDataMapper> mapper2 =
			vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper2->SetInputData(isoSurface2);

		vtkSmartPointer<vtkActor> actor2 =
			vtkSmartPointer<vtkActor>::New();
		actor2->GetProperty()->SetColor(0, 255, 0);
		actor2->GetProperty()->SetOpacity(0.5);
		actor2->SetMapper(mapper2);


		renderer1->AddActor(actor2);

		//tr::Display3DRoutines::displayPolyData(materialSurfaces[mm]);

	//}

	renderer1->SetBackground(0, 0, 0);
	renderer2->SetBackground(0, 0, 0);
	// Add renderer to renderwindow and render
	vtkSmartPointer< vtkRenderWindow > window = vtkSmartPointer< vtkRenderWindow >::New();
	vtkSmartPointer< vtkRenderWindowInteractor > interactor = vtkSmartPointer< vtkRenderWindowInteractor >::New();
	vtkSmartPointer< vtkInteractorStyleTrackballCamera > style = vtkSmartPointer< vtkInteractorStyleTrackballCamera >::New();
	window->AddRenderer(renderer1);
	//window->AddRenderer(renderer2);
	window->SetInteractor(interactor);
	interactor->SetInteractorStyle(style);

	window->Render();

	interactor->Start();

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

	unsigned char *ptr = (unsigned char *)volumeData->GetScalarPointer();

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





int main( int argc , char **argv )
{

	testThresholdVariation();

	return 0;

	QString filePath = "C:/projects/Wallthickness/data/bottle/bottle.wtadat";//"C:/projects/Wallthickness/data/fanuc_engine_cover/engine_cover.wtadat";//	

	QString fileName = "C:/projects/Wallthickness/data/CT multi/Calliper Assy 2016-6-9 10-12";//"C:/projects/Wallthickness/data/Datasets for Multi-material/MultiMaterial/MAR_Ref 2012-8-30 15-13"; //"C:/projects/Wallthickness/data/CT multi/Hemanth_ Sugar free 2016-6-6 11-33";//"C: / projects / Wallthickness / data / CT multi / Fuel_filter_0km_PR_1 2016 - 5 - 30 12 - 54";//;
	//"C:/projects/Wallthickness/data/Mobile charger/Wall Thick_RnD_1 2016-10-13 15-10_sep1"; 
	QString scvFilePath = fileName + ".uint16_scv";

	QString vgiFilePath = fileName + ".vgi";

	QString stlFilePath = "C:/projects/Wallthickness/data/Mobile _ Charger.stl";


	QString path1 = "C:/projects/Wallthickness/data/benchmarkdata/MSB_Demo Part STL_Aligned.stl";
	QString path2 = "C:/projects/Wallthickness/data/benchmarkdata/surface.ply";

	vtkSmartPointer< vtkSTLReader > reader1 = vtkSmartPointer< vtkSTLReader >::New();
	vtkSmartPointer< vtkPLYReader > reader2 = vtkSmartPointer< vtkPLYReader >::New();

	reader1->SetFileName(path1.toStdString().c_str());

	reader2->SetFileName(path2.toStdString().c_str());

	reader1->Update();
	reader2->Update();

	vtkSmartPointer< vtkPolyData > mesh1 = vtkSmartPointer< vtkPolyData >::New();
	vtkSmartPointer< vtkPolyData > mesh2 = vtkSmartPointer< vtkPolyData >::New();

	mesh1->ShallowCopy(reader1->GetOutput());
	mesh2->ShallowCopy(reader2->GetOutput());



	vtkSmartPointer< vtkPolyDataNormals > normalEstimator = vtkSmartPointer< vtkPolyDataNormals >::New();

	normalEstimator->SetInputData(mesh1);

	normalEstimator->Update();

	mesh1->ShallowCopy(normalEstimator->GetOutput());

	std::vector< Eigen::Vector3f > refPoints, refNormals, comparePoints, compareNormals;
	std::vector< unsigned int > compareIndices, refIndices;

	convertToVertexAndIndices(mesh1, comparePoints, compareNormals, compareIndices);
	convertToVertexAndIndices(mesh2, refPoints, refIndices);

	//tr::Display3DRoutines::displayMesh(refPoints, refIndices);
	//tr::Display3DRoutines::displayMesh( comparePoints, compareIndices);
	//return 0;

	imt::volume::VolumeInfo volume;

	volume.loadVolume(filePath);

	imt::volume::WallthicknessEstimator wte(volume);

	std::cout << " estimating wall thickness " << std::endl;

	wte.compareMeshes(refPoints, refIndices, comparePoints, compareNormals, compareIndices);

	return 0;

	wte.computeBrepRayTraceThickness(volume);

	std::vector< Eigen::Vector3f > queryPoints, queryNormals , queryOppPoints , queryOppNormals;

	Eigen::Vector3f shift(0, 0, 0.005);

	int nPoints = volume.mVertices.size();

	queryPoints.resize(nPoints);
	queryNormals.resize(nPoints);
	queryOppPoints.resize(nPoints);
	queryOppNormals.resize(nPoints);

	for (int ii = 0; ii < nPoints; ii++)
	{
		queryPoints[ii] = volume.mVertices[ii] + shift;
		queryNormals[ii] = volume.mVertexNormals[ii];
	}

	std::vector< bool > oppositeEndsFound(nPoints, false);

	std::vector< float > closestSurfaceDistances(nPoints, 1000);


	float vstep = volume.mVoxelStep(0);

	computeOppositeEnds(queryPoints, queryNormals, queryOppPoints, queryOppNormals, oppositeEndsFound , closestSurfaceDistances, wte.mDevice, wte.mScene, vstep);

	std::vector< Eigen::Vector3f > vertexColors( nPoints , Eigen::Vector3f( 1 , 1 , 1 ) );

	double thickness = 0;
	int count = 0;

	float coeff = 1;

	//float th1 = 0.0025 * 2;
	//float th2 = 

	for (int vv = 0; vv < nPoints ; vv++)
	{
		if (!oppositeEndsFound[vv])
			continue;

		if ( closestSurfaceDistances[vv] < 0.0025 * coeff )
		{
			vertexColors[vv](0) = 1 - closestSurfaceDistances[vv] / 0.0025;
			vertexColors[vv](1) = closestSurfaceDistances[vv] / 0.0025;
			vertexColors[vv](2) = 0;


		}
		else if (closestSurfaceDistances[vv] > 0.0025 * coeff && closestSurfaceDistances[vv] < 0.005 * coeff)
		{
			vertexColors[vv](0) = 0;
			vertexColors[vv](1) = 1 - closestSurfaceDistances[vv] / 0.0025;
			vertexColors[vv](2) = closestSurfaceDistances[vv] / 0.0025;
		}

		thickness += closestSurfaceDistances[vv];
		count++;

	}

	thickness /= count;

	std::cout << " average thickness " << thickness << std::endl;

	tr::Display3DRoutines::displayMesh(queryPoints, vertexColors, volume.mFaceIndices);


	return 0;
}



void computeOppositeEnds( std::vector< Eigen::Vector3f >& edgeEnds, std::vector< Eigen::Vector3f >& edgeNormals,
	                      std::vector< Eigen::Vector3f >& oppositeEnds, std::vector< Eigen::Vector3f >& oppositeEndNormals ,
						  std::vector< bool >& oppositeEndFound  , std::vector< float >& closestSurfaceDistances , RTCDevice& device, RTCScene& scene, float vstep )
{

	std::vector< Eigen::Vector3f > colors(edgeEnds.size(), Eigen::Vector3f(1, 0, 0));

	oppositeEndFound.resize(edgeEnds.size(), false);

	std::fill(oppositeEndFound.begin(), oppositeEndFound.end(), false);

	//tr::Display3DRoutines::displayPointSet(edgeEnds , colors);

	float maxLength = FLT_MAX;

	double initT = cv::getTickCount();

	float near = 0;

	std::cout << " vstep : " << vstep << std::endl;

	int numVertices = edgeEnds.size();

	std::atomic<int > nValidIntersection;

	nValidIntersection = 0;

	std::fill(closestSurfaceDistances.begin(), closestSurfaceDistances.end(), 1000);

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

			ray.dirx[ii] = edgeNormals[id](0);
			ray.diry[ii] = edgeNormals[id](1);
			ray.dirz[ii] = edgeNormals[id](2);

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

				closestSurfaceDistances[vv + ii] = ray.tfar[ii];

				oppositeEndFound[vv + ii] = true;

				nValidIntersection++;
			}
		}

	}


	std::cout << " number of valid intersections : " << nValidIntersection << std::endl;


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

				closestSurfaceDistances[vv + ii] = std::min( closestSurfaceDistances[vv + ii] ,  ray.tfar[ii] );

				oppositeEndFound[vv + ii] = true;

				nValidIntersection++;
			}
		}

	}
}

