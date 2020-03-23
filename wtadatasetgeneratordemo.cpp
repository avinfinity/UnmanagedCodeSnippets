
#include "iostream"
#include "QString"
#include "QDir"
#include "volumeinfo.h"
#include "rawvolumedataio.h"
#include <vtkSmartPointer.h>
#include "vtkImageData.h"
#include "vtkMarchingCubes.h"
#include "vtkImageMarchingCubes.h"
#include "vtkPointData.h"
#include "vtkIdList.h"
#include "vtkPolyDataNormals.h"
#include "display3droutines.h"
#include "vtkPLYWriter.h"
#include "vtkCylinderSource.h"
#include "display3droutines.h"
#include <vtkBooleanOperationPolyDataFilter.h>
#include <vtkTriangleFilter.h>
#include "vtkPLYWriter.h"
#include "vtkPLYReader.h"
#include "vtkPolyDataNormals.h"
#include "vtkSphereSource.h"
#include "QFile"
#include "QDataStream"
#include "QTextStream"
#include "QDebug"
#include "volumeutility.h"
#include "rawvolumedataio.h"

void generateIdealDataSet(QString dataSetPath);
void generateIdealDataSet2(QString dataSetPath);
void surfaceFromLevelSet();


void readUint16_SCV(QString filePath, int w, int h, int d, imt::volume::VolumeInfo& volume)
{
	QFile file(filePath);

	file.open(QIODevice::ReadOnly);

	QDataStream reader(&file);

	reader.skipRawData(1024);

	volume.mWidth = w;
	volume.mHeight = h;
	volume.mDepth = d;

	volume.mVolumeData = (unsigned char*)(new unsigned short[w * h * d]);

	reader.readRawData((char*)volume.mVolumeData, w * h * d * 2);

	file.close();



}


void readVGI(QString filePath, int& w, int& h, int& d, float& voxelStep)
{
	QFile file(filePath);

	file.open(QIODevice::ReadOnly);

	QTextStream reader(&file);

	qDebug() << reader.readLine() << endl;
	qDebug() << reader.readLine() << endl;
	//qDebug() << reader.readLine() << endl;

	QString title, c1, c2, c3, c4;


	reader >> title >> c1 >> c2 >> c3 >> c4;

	w = c2.toInt();
	h = c3.toInt();
	d = c4.toInt();

	qDebug() << title << " " << c2 << " " << c3 << " " << c4 << endl;

	for (int ii = 0; ii < 16; ii++)
	{
		reader.readLine();
	}

	reader >> title >> c1 >> c1 >> c2 >> c3 >> c4;

	qDebug() << title << " " << c2 << " " << c3 << " " << c4 << endl;

	voxelStep = c2.toDouble();

	file.close();
}



unsigned int levelSet(int x, int y, int z);

/*			QFile file(filePath);

			if ( !file.open(QIODevice::WriteOnly) )
			{
				qDebug() << " could not open file " << filePath << " for writing :  " << file.errorString() << endl;
			}

			QDataStream writer(&file);

			//write volume resolution
			writer << mWidth << mHeight << mDepth << mVolumeElementSize << mVolumeElementComponents;
			writer << mVolumeOrigin(0) << mVolumeOrigin(1) << mVolumeOrigin(2) << mVoxelStep(0) << mVoxelStep(1) << mVoxelStep(2);
			writer << mAirVoxelFilterVal;

			writer.writeRawData((char*)mVolumeData, mWidth * mHeight * mDepth * mVolumeElementSize * mVolumeElementComponents);

			qlonglong numVertices = mVertices.size();
			qlonglong numTriangles = mFaceIndices.size() / 3;

			writer << numVertices << numTriangles;

			writer.writeRawData((char*)mVertices.data(), numVertices * 3 * sizeof(float));
			writer.writeRawData((char*)mVertexNormals.data(), numVertices * 3 * sizeof(float));
			writer.writeRawData((char*)mFaceIndices.data(), numTriangles * 3 * sizeof(unsigned int));

			file.close();*/

void generateWtDataSet(QString meshFilePath)
{
	vtkSmartPointer< vtkPolyData > meshData = vtkSmartPointer< vtkPolyData >::New();

	vtkSmartPointer< vtkPLYReader > reader = vtkSmartPointer< vtkPLYReader >::New();

	reader->SetFileName( meshFilePath.toStdString().c_str() );
	reader->Update();

	meshData->DeepCopy(reader->GetOutput());

	tr::Display3DRoutines::displayPolyData(meshData);

	QString dataSetPath = "C:/projects/Wallthickness/data/idealdata.wtadat";

	imt::volume::VolumeInfo volInfo;

	volInfo.mWidth = 1;
	volInfo.mHeight = 1;
	volInfo.mDepth = 1;
	volInfo.mVolumeElementSize = 1;
	volInfo.mVolumeElementComponents = 1;

	volInfo.mVolumeOrigin(0) = -10;
	volInfo.mVolumeOrigin(1) = -10;
	volInfo.mVolumeOrigin(2) = -20;
	volInfo.mVoxelStep(0) = 20;
	volInfo.mVoxelStep(1) = 20;
	volInfo.mVoxelStep(2) = 40;
	volInfo.mAirVoxelFilterVal = 1000;
	volInfo.mVolumeData = new unsigned char[volInfo.mWidth * volInfo.mHeight * volInfo.mDepth * volInfo.mVolumeElementSize * volInfo.mVolumeElementComponents];



	vtkSmartPointer< vtkPolyDataNormals > normalEstimator = vtkSmartPointer< vtkPolyDataNormals >::New();

	normalEstimator->SetInputData(meshData);

	normalEstimator->ComputePointNormalsOn();
	normalEstimator->NonManifoldTraversalOff();
	normalEstimator->ConsistencyOff();
	//normalEstimator->SetNonManifoldTraversal();

	normalEstimator->Update();

	meshData->DeepCopy(normalEstimator->GetOutput());

	tr::Display3DRoutines::displayPolyData(normalEstimator->GetOutput());

	qlonglong numVertices = meshData->GetNumberOfPoints();
	qlonglong numTriangles = meshData->GetNumberOfCells();

	volInfo.mVertices.resize(numVertices);
	volInfo.mVertexNormals.resize(numVertices);



	volInfo.mFaceIndices.resize(meshData->GetNumberOfCells() * 3);

	vtkSmartPointer< vtkDataArray > normals = meshData->GetPointData()->GetNormals();

	for (int vv = 0; vv < numVertices; vv++)
	{
		double pt[3] , n[3];

		meshData->GetPoint(vv, pt);

		volInfo.mVertices[vv](0) = pt[0];
		volInfo.mVertices[vv](1) = pt[1];
		volInfo.mVertices[vv](2) = pt[2];

		normals->GetTuple(vv, n);

		volInfo.mVertexNormals[vv](0) = n[0];
		volInfo.mVertexNormals[vv](1) = n[1];
		volInfo.mVertexNormals[vv](2) = n[2];
	}
	
	vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();

	for (int tt = 0; tt < numTriangles; tt++)
	{

		meshData->GetCellPoints(tt, triangle);

		volInfo.mFaceIndices[3 * tt] = triangle->GetId(0);
		volInfo.mFaceIndices[3 * tt + 1] = triangle->GetId(1);
		volInfo.mFaceIndices[3 * tt + 2] = triangle->GetId(2);
	}


	volInfo.saveVolume(dataSetPath);

	tr::Display3DRoutines::displayMesh(volInfo.mVertices, volInfo.mFaceIndices);

}

void generateDataSetFromVGI(QString vgiFilePath, QString scvFilePath, imt::volume::VolumeInfo& volInfo, QString wtaFilePath);

void generateDataSetFromUint16SCV(QString scvFilePath, imt::volume::VolumeInfo& volInfo, QString wtaFilePath);

int main( int argc , char **argv )
{

	vtkObject::GlobalWarningDisplayOff();

	imt::volume::VolumeInfo volInfo1;

	//generateDataSetFromVGI("C:/projects/Wallthickness/data/Mobile charger/Wall Thick_RnD_1 2016-10-13 15-10_sep1.vgi", "C:/projects/Wallthickness/data/Mobile charger/Wall Thick_RnD_1 2016-10-13 15-10_sep1.uint16_scv", volInfo1, "C:/projects/Wallthickness/data/Mobile charger/mc.wtadat");

	generateDataSetFromUint16SCV("C:/Projects/Wallthickness/data/separated_part_7.uint16_scv", volInfo1 , "C:/Projects/Wallthickness/data/separated_part_7.wtadat");

	return 0;
	//surfaceFromLevelSet();

	//return 0;

	//QString dataSetPath = "C:/projects/Wallthickness/data/idealdata_remesh.ply";

	////generateIdealDataSet2( dataSetPath );

	//generateWtDataSet( dataSetPath );
	//return 0;
	
#if 0
	QString rawFilePath = "C:/projects/Wallthickness/data/bottle/bottle.raw";
	QString wtaFilePath = "C:/projects/Wallthickness/data/bottle/bottle.wtadat";

	imt::volume::VolumeInfo volInfo;

	volInfo.mWidth = 455;
	volInfo.mHeight = 485;
	volInfo.mDepth = 942;

	volInfo.mVoxelStep = Eigen::Vector3f(0.165098, 0.165098, 0.165097);
	volInfo.mVolumeOrigin = Eigen::Vector3f(0, 0, 0);

	volInfo.mVolumeElementSize = 2;
	volInfo.mVolumeElementComponents = 1;
	volInfo.mAirVoxelFilterVal = 14470;
#else
	QString rawFilePath = "C:/projects/Wallthickness/data/fanuc_engine_cover/engine_cover.raw";
	QString wtaFilePath = "C:/projects/Wallthickness/data/fanuc_engine_cover/engine_cover.wtadat";

	imt::volume::VolumeInfo volInfo;

	volInfo.mWidth = 818;
	volInfo.mHeight = 1042;
	volInfo.mDepth = 626;

	volInfo.mVoxelStep = Eigen::Vector3f(0.165098, 0.165098, 0.165098);
	volInfo.mVolumeOrigin = Eigen::Vector3f(0, 0, 0);

	volInfo.mVolumeElementSize = 2;
	volInfo.mVolumeElementComponents = 1;
	volInfo.mAirVoxelFilterVal = 30332;
#endif

	imt::volume::RawVolumeDataIO volumeIO;

	//volumeIO.setVolume(volInfo);

	std::cout << " reading volume data " << std::endl;

	volumeIO.readData(rawFilePath.toStdString(), volInfo.mVolumeData);

	std::cout << " reading volume data completed " << std::endl;

	vtkSmartPointer< vtkImageData > volumeData = vtkSmartPointer< vtkImageData >::New();

	
	std::cout << " computing iso surface " << std::endl;

	volumeData->SetOrigin(volInfo.mVolumeOrigin(0), volInfo.mVolumeOrigin(1), volInfo.mVolumeOrigin(2));
	volumeData->SetSpacing(volInfo.mVoxelStep(0), volInfo.mVoxelStep(1), volInfo.mVoxelStep(2));
	volumeData->SetExtent( 0 , volInfo.mWidth - 1, 0 , volInfo.mHeight - 1, 0 , volInfo.mDepth - 1);
	volumeData->AllocateScalars(VTK_UNSIGNED_SHORT, 1);

	long int nPoints = volumeData->GetNumberOfPoints();


	long int zStep = volInfo.mHeight * volInfo.mWidth;
	long int yStep = volInfo.mWidth;

	unsigned short *ptr = (unsigned short *)volumeData->GetScalarPointer();

	memcpy(ptr, volInfo.mVolumeData, volInfo.mWidth * volInfo.mHeight * volInfo.mDepth * 2);

	vtkSmartPointer< vtkImageMarchingCubes > marchingCubes = vtkSmartPointer< vtkImageMarchingCubes >::New();

	marchingCubes->SetInputData(volumeData);

	marchingCubes->SetValue(0, volInfo.mAirVoxelFilterVal);

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

	vtkSmartPointer< vtkPLYWriter > writer = vtkSmartPointer< vtkPLYWriter >::New();

	writer->SetInputData(isoSurface);

	std::string meshPath = "C:/projects/Wallthickness/data/bottlemesh.ply";

	writer->SetFileName(meshPath.c_str());

	writer->Update();

	tr::Display3DRoutines::displayPolyData(isoSurface);

	long int numPoints = isoSurface->GetNumberOfPoints();
	long int numTriangles = isoSurface->GetNumberOfCells();

	volInfo.mVertices.resize(numPoints);
	volInfo.mVertexNormals.resize(numPoints);
	volInfo.mFaceIndices.resize(numTriangles * 3);

	vtkSmartPointer<vtkDataArray> normals = isoSurface->GetPointData()->GetNormals();

	Eigen::Vector3f center1(0, 0, 0), center2(-4.5, 0, 0), center3(4.5, 0, 0);

	for ( long int pp = 0; pp < numPoints; pp++ )
	{
		double point[3];

		isoSurface->GetPoint(pp, point);

		volInfo.mVertices[pp](0) = point[0];
		volInfo.mVertices[pp](1) = point[1];
		volInfo.mVertices[pp](2) = point[2];

		double n[3];

		normals->GetTuple(pp, n);

		volInfo.mVertexNormals[pp](0) = n[0];
		volInfo.mVertexNormals[pp](1) = n[1];
		volInfo.mVertexNormals[pp](2) = n[2];

	}

	tr::Display3DRoutines::displayPointSet(volInfo.mVertices, volInfo.mVertexNormals);

	//return 0;

	vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();


	for (long int tt = 0; tt < numTriangles; tt++)
	{
		isoSurface->GetCellPoints(tt, triangle);

		volInfo.mFaceIndices[3 * tt] = triangle->GetId(0); 
		volInfo.mFaceIndices[3 * tt + 1] = triangle->GetId(1);
		volInfo.mFaceIndices[3 * tt + 2] = triangle->GetId(2);
	}


	volInfo.saveVolume(wtaFilePath);


	
	
	return 0;
}



void generateIdealDataSet(QString dataSetPath)
{

	vtkSmartPointer< vtkCylinderSource > cylinder = vtkSmartPointer< vtkCylinderSource >::New();
	vtkSmartPointer< vtkTriangleFilter > triangulator = vtkSmartPointer< vtkTriangleFilter >::New();

	//the dimensions are in milimeter

	float height = 40;

	cylinder->SetHeight(height);
	cylinder->SetRadius(10);
	cylinder->SetResolution(20);

	cylinder->Update();

	vtkSmartPointer< vtkPolyData > cylinderDataOuter = vtkSmartPointer< vtkPolyData >::New();

	cylinderDataOuter->DeepCopy(cylinder->GetOutput());

	triangulator->SetInputData(cylinderDataOuter);

	triangulator->Update();

	cylinderDataOuter->DeepCopy(triangulator->GetOutput());

	//tr::Display3DRoutines::displayPolyData(cylinderDataOuter);


	vtkSmartPointer< vtkPolyData > cylinderDataInner1 = vtkSmartPointer< vtkPolyData >::New();
	vtkSmartPointer< vtkPolyData > cylinderDataInner2 = vtkSmartPointer< vtkPolyData >::New();


	cylinder->SetHeight(height + 10);
	cylinder->SetRadius(4);
	cylinder->SetResolution(20);
	cylinder->SetCenter(-4.5, 0, 0);

	cylinder->Update();

	cylinderDataInner1->DeepCopy(cylinder->GetOutput());

	triangulator->SetInputData(cylinderDataInner1);

	triangulator->Update();

	cylinderDataInner1->DeepCopy(triangulator->GetOutput());


	cylinder->SetHeight(height + 10);
	cylinder->SetRadius(4);
	cylinder->SetResolution(20);
	cylinder->SetCenter(4.5, 0, 0);

	cylinder->Update();

	cylinderDataInner2->DeepCopy(cylinder->GetOutput());

	triangulator->SetInputData(cylinderDataInner2);

	triangulator->Update();

	cylinderDataInner2->DeepCopy(triangulator->GetOutput());


	vtkSmartPointer< vtkBooleanOperationPolyDataFilter > subtractor = vtkSmartPointer< vtkBooleanOperationPolyDataFilter >::New();

	subtractor->SetInputData(0, cylinderDataOuter);
	subtractor->SetInputData(1, cylinderDataInner1);

	subtractor->SetOperationToDifference();

	subtractor->Update();


	vtkSmartPointer< vtkPolyData > volData = vtkSmartPointer< vtkPolyData >::New();

	volData->DeepCopy(subtractor->GetOutput());


	subtractor->SetInputData(0, volData);
	subtractor->SetInputData(1, cylinderDataInner2);

	subtractor->SetOperationToDifference();

	subtractor->Update();

	volData->DeepCopy(subtractor->GetOutput());

	//tr::Display3DRoutines::displayPolyData(volData);


	vtkSmartPointer< vtkPLYWriter > writer = vtkSmartPointer< vtkPLYWriter >::New();

	writer->SetFileName( dataSetPath.toStdString().c_str() );

	writer->SetInputData(volData);

	writer->Update();

	QString path1 = "C:/projects/Wallthickness/data/cyl1.ply";
	QString path2 = "C:/projects/Wallthickness/data/cyl2.ply";
	QString path3 = "C:/projects/Wallthickness/data/cyl3.ply";

	writer->SetFileName(path1.toStdString().c_str());

	writer->SetInputData(cylinderDataOuter);

	writer->Update();
	writer->SetFileName(path2.toStdString().c_str());

	writer->SetInputData(cylinderDataInner1);

	writer->Update();

	writer->SetFileName(path3.toStdString().c_str());

	writer->SetInputData(cylinderDataInner2);

	writer->Update();


	//vtkSmartPointer< vtkPLYReader > reader = vtkSmartPointer< vtkPLYReader >::New();

}


void generateIdealDataSet2(QString dataSetPath)
{

	vtkSmartPointer< vtkSphereSource > sphereSource = vtkSmartPointer< vtkSphereSource >::New();
	vtkSmartPointer< vtkTriangleFilter > triangulator = vtkSmartPointer< vtkTriangleFilter >::New();

	//the dimensions are in milimeter

	float height = 40;

	//sphereSource->SetHeight(height);
	sphereSource->SetRadius(10);
	//sphereSource->SetResolution(20);

	sphereSource->SetThetaResolution(20);
	sphereSource->SetPhiResolution(20);

	sphereSource->Update();

	vtkSmartPointer< vtkPolyData > cylinderDataOuter = vtkSmartPointer< vtkPolyData >::New();

	cylinderDataOuter->DeepCopy(sphereSource->GetOutput());

	triangulator->SetInputData(cylinderDataOuter);

	triangulator->Update();

	cylinderDataOuter->DeepCopy(triangulator->GetOutput());

	//tr::Display3DRoutines::displayPolyData(cylinderDataOuter);


	vtkSmartPointer< vtkPolyData > cylinderDataInner1 = vtkSmartPointer< vtkPolyData >::New();
	vtkSmartPointer< vtkPolyData > cylinderDataInner2 = vtkSmartPointer< vtkPolyData >::New();


	//cylinder->SetHeight(height + 10);
	sphereSource->SetRadius(4);
	//cylinder->SetResolution(20);
	sphereSource->SetCenter(-4.5, 0, 0);

	sphereSource->Update();

	cylinderDataInner1->DeepCopy(sphereSource->GetOutput());

	triangulator->SetInputData(cylinderDataInner1);

	triangulator->Update();

	cylinderDataInner1->DeepCopy(triangulator->GetOutput());


	//cylinder->SetHeight(height + 10);
	sphereSource->SetRadius(4);
	//cylinder->SetResolution(20);
	sphereSource->SetCenter(4.5, 0, 0);

	sphereSource->Update();

	cylinderDataInner2->DeepCopy(sphereSource->GetOutput());

	triangulator->SetInputData(cylinderDataInner2);

	triangulator->Update();

	cylinderDataInner2->DeepCopy(triangulator->GetOutput());


	vtkSmartPointer< vtkBooleanOperationPolyDataFilter > subtractor = vtkSmartPointer< vtkBooleanOperationPolyDataFilter >::New();

	subtractor->SetInputData(0, cylinderDataOuter);
	subtractor->SetInputData(1, cylinderDataInner1);

	subtractor->SetOperationToDifference();

	subtractor->Update();


	vtkSmartPointer< vtkPolyData > volData = vtkSmartPointer< vtkPolyData >::New();

	volData->DeepCopy(subtractor->GetOutput());


	subtractor->SetInputData(0, volData);
	subtractor->SetInputData(1, cylinderDataInner2);

	subtractor->SetOperationToDifference();

	subtractor->Update();

	volData->DeepCopy(subtractor->GetOutput());

	//tr::Display3DRoutines::displayPolyData(volData);


	vtkSmartPointer< vtkPLYWriter > writer = vtkSmartPointer< vtkPLYWriter >::New();

	writer->SetFileName(dataSetPath.toStdString().c_str());

	writer->SetInputData(volData);

	writer->Update();

	QString path1 = "C:/projects/Wallthickness/data/cyl1.ply";
	QString path2 = "C:/projects/Wallthickness/data/cyl2.ply";
	QString path3 = "C:/projects/Wallthickness/data/cyl3.ply";

	writer->SetFileName(path1.toStdString().c_str());

	writer->SetInputData(cylinderDataOuter);

	writer->Update();
	writer->SetFileName(path2.toStdString().c_str());

	writer->SetInputData(cylinderDataInner1);

	writer->Update();

	writer->SetFileName(path3.toStdString().c_str());

	writer->SetInputData(cylinderDataInner2);

	writer->Update();


	//vtkSmartPointer< vtkPLYReader > reader = vtkSmartPointer< vtkPLYReader >::New();

}



unsigned int levelSet(int x, int y, int z)
{
	float h = 400;
	float r = 100;

	float w = 2 * r;
	float l = 2 * r;
	

	float step = 2;

	float xc = x * step;
	float yc = y * step;
	float zc = z * step;

	Eigen::Vector2f center( r + 50, r) , center1( r / 2 + 45 , r  ) , center2( 3 * r / 2 + 55 , r  ) , pc( xc , yc );

	float r1 = 40, r2 = 40;

	if (zc < 25 || zc > 375)
		return 0;

	float d1 = (pc - center1).norm();
	float d2 = (pc - center2).norm();

	float d = std::min(d1, d2);

	if ( d1 < r1 || d2 < r2 )
	{
		return 0;//( r - r1 ) + d;
	}
	else if ((pc - center).norm() < r)
	{
		return 100;// (pc - center).norm();
	}


	return 0;
}


void surfaceFromLevelSet()
{
	imt::volume::VolumeInfo volinfo;

	int s = 0;

	volinfo.mWidth = 200 + 2 * s;
	volinfo.mHeight = 200 + 2 * s;
	volinfo.mDepth = 400;

	volinfo.mVoxelStep = Eigen::Vector3f(2, 2, 2);
	volinfo.mVolumeOrigin = Eigen::Vector3f(0, 0, 0);
	volinfo.mVolumeData = (unsigned char*)( new unsigned short[ volinfo.mWidth * volinfo.mHeight * volinfo.mDepth ] );

	long int m1 = 0, m2 = 0;

	std::vector< Eigen::Vector3f > pts;

	memset(volinfo.mVolumeData, 0, volinfo.mWidth * volinfo.mHeight * volinfo.mDepth * 2);

	

	for (int zz = 0; zz < volinfo.mDepth ; zz++)
		for (int yy = s; yy < volinfo.mHeight; yy++)
			for (int xx = s; xx < volinfo.mWidth ; xx++)
			{
				long int id = zz * volinfo.mWidth * volinfo.mHeight + yy * volinfo.mWidth + xx;
				int l = levelSet(xx - s, yy - s, zz);

				

				if ( l == 0 )
				{
					m1++;
				}
					

				if ( l ==  100 )
				{
					m2++;

					l = 100;

					pts.push_back( Eigen::Vector3f( xx, yy, zz ) );
				}

				((unsigned short*)volinfo.mVolumeData)[id] = l;
					
 			}

	std::vector< Eigen::Vector3f > colors(pts.size(), Eigen::Vector3f(1, 0, 0));

	tr::Display3DRoutines::displayPointSet(pts, colors);


	std::cout << m1 << "  " << m2 << std::endl;

	vtkSmartPointer< vtkImageData > volumeData = vtkSmartPointer< vtkImageData >::New();


	volumeData->SetOrigin(volinfo.mVolumeOrigin(0), volinfo.mVolumeOrigin(1), volinfo.mVolumeOrigin(2));
	volumeData->SetSpacing(volinfo.mVoxelStep(0), volinfo.mVoxelStep(1), volinfo.mVoxelStep(2));
	volumeData->SetExtent(0, volinfo.mWidth - 1, 0, volinfo.mHeight - 1, 0, volinfo.mDepth - 1);
	volumeData->AllocateScalars(VTK_UNSIGNED_SHORT, 1);

	long int nPoints = volumeData->GetNumberOfPoints();


	long int zStep = volinfo.mHeight * volinfo.mWidth;
	long int yStep = volinfo.mWidth;

	unsigned short *ptr = (unsigned short *)volumeData->GetScalarPointer();

	memcpy( ptr , volinfo.mVolumeData, volinfo.mWidth * volinfo.mHeight * volinfo.mDepth * 2 );
	

	vtkSmartPointer< vtkImageMarchingCubes > marchingCubes = vtkSmartPointer< vtkImageMarchingCubes >::New();

	marchingCubes->SetInputData(volumeData);

	marchingCubes->SetValue(0, 99);

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

	vtkSmartPointer< vtkPLYWriter > writer = vtkSmartPointer< vtkPLYWriter >::New();

	writer->SetInputData(isoSurface);

	std::string meshPath = "C:/projects/Wallthickness/data/bottlemesh.ply";

	writer->SetFileName(meshPath.c_str());

	writer->Update();

	tr::Display3DRoutines::displayPolyData(isoSurface);

}




void generateDataSetFromVGI( QString vgiFilePath , QString scvFilePath , imt::volume::VolumeInfo& volInfo , QString wtaFilePath )
{

	int w = 0, h = 0, d = 0;
	float voxelStep = 0;

	readVGI(vgiFilePath, w, h, d, voxelStep);

	readUint16_SCV(scvFilePath, w, h, d, volInfo);

	//imt::volume::

	volInfo.mAirVoxelFilterVal = 14976;


	vtkSmartPointer< vtkImageData > volumeData = vtkSmartPointer< vtkImageData >::New();


	std::cout << " computing iso surface " << std::endl;

	volumeData->SetOrigin(volInfo.mVolumeOrigin(0), volInfo.mVolumeOrigin(1), volInfo.mVolumeOrigin(2));
	volumeData->SetSpacing(volInfo.mVoxelStep(0), volInfo.mVoxelStep(1), volInfo.mVoxelStep(2));
	volumeData->SetExtent(0, volInfo.mWidth - 1, 0, volInfo.mHeight - 1, 0, volInfo.mDepth - 1);
	volumeData->AllocateScalars(VTK_UNSIGNED_SHORT, 1);

	long int nPoints = volumeData->GetNumberOfPoints();


	long int zStep = volInfo.mHeight * volInfo.mWidth;
	long int yStep = volInfo.mWidth;

	unsigned short *ptr = (unsigned short *)volumeData->GetScalarPointer();

	memcpy(ptr, volInfo.mVolumeData, volInfo.mWidth * volInfo.mHeight * volInfo.mDepth * 2);

	vtkSmartPointer< vtkImageMarchingCubes > marchingCubes = vtkSmartPointer< vtkImageMarchingCubes >::New();

	marchingCubes->SetInputData(volumeData);

	marchingCubes->SetValue(0, volInfo.mAirVoxelFilterVal);

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

	vtkSmartPointer< vtkPLYWriter > writer = vtkSmartPointer< vtkPLYWriter >::New();

	writer->SetInputData(isoSurface);

	std::string meshPath = "C:/projects/Wallthickness/data/bottlemesh.ply";

	writer->SetFileName(meshPath.c_str());

	writer->Update();

	tr::Display3DRoutines::displayPolyData(isoSurface);

	long int numPoints = isoSurface->GetNumberOfPoints();
	long int numTriangles = isoSurface->GetNumberOfCells();

	volInfo.mVertices.resize(numPoints);
	volInfo.mVertexNormals.resize(numPoints);
	volInfo.mFaceIndices.resize(numTriangles * 3);

	vtkSmartPointer<vtkDataArray> normals = isoSurface->GetPointData()->GetNormals();

	Eigen::Vector3f center1(0, 0, 0), center2(-4.5, 0, 0), center3(4.5, 0, 0);

	for (long int pp = 0; pp < numPoints; pp++)
	{
		double point[3];

		isoSurface->GetPoint(pp, point);

		volInfo.mVertices[pp](0) = point[0];
		volInfo.mVertices[pp](1) = point[1];
		volInfo.mVertices[pp](2) = point[2];

		double n[3];

		normals->GetTuple(pp, n);

		volInfo.mVertexNormals[pp](0) = n[0];
		volInfo.mVertexNormals[pp](1) = n[1];
		volInfo.mVertexNormals[pp](2) = n[2];

	}

	tr::Display3DRoutines::displayPointSet(volInfo.mVertices, volInfo.mVertexNormals);

	//return 0;

	vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();


	for (long int tt = 0; tt < numTriangles; tt++)
	{
		isoSurface->GetCellPoints(tt, triangle);

		volInfo.mFaceIndices[3 * tt] = triangle->GetId(0);
		volInfo.mFaceIndices[3 * tt + 1] = triangle->GetId(1);
		volInfo.mFaceIndices[3 * tt + 2] = triangle->GetId(2);
	}


	volInfo.saveVolume(wtaFilePath);

}

void generateDataSetFromUint16SCV(QString scvFilePath, imt::volume::VolumeInfo& volInfo, QString wtaFilePath)
{
	int w = 0, h = 0, d = 0;
	float voxelStep = 0;

	//readVGI(vgiFilePath, w, h, d, voxelStep);

	imt::volume::RawVolumeDataIO::readUint16SCV(scvFilePath,volInfo);

	volInfo.mAirVoxelFilterVal = 14976;

	std::vector< __int64 > histogram;
	int minVal, maxVal;

	int isoThreshold = 30880;

	//imt::volume::VolumeUtility::computeISOThreshold( volInfo , isoThreshold , histogram , minVal , maxVal );

	//return;

	volInfo.mAirVoxelFilterVal = isoThreshold;

	vtkSmartPointer< vtkImageData > volumeData = vtkSmartPointer< vtkImageData >::New();


	std::cout << " computing iso surface " << std::endl;

	volumeData->SetOrigin(volInfo.mVolumeOrigin(0), volInfo.mVolumeOrigin(1), volInfo.mVolumeOrigin(2));
	volumeData->SetSpacing(volInfo.mVoxelStep(0), volInfo.mVoxelStep(1), volInfo.mVoxelStep(2));
	volumeData->SetExtent(0, volInfo.mWidth - 1, 0, volInfo.mHeight - 1, 0, volInfo.mDepth - 1);
	volumeData->AllocateScalars(VTK_UNSIGNED_SHORT, 1);

	long int nPoints = volumeData->GetNumberOfPoints();


	long int zStep = volInfo.mHeight * volInfo.mWidth;
	long int yStep = volInfo.mWidth;

	unsigned short *ptr = (unsigned short *)volumeData->GetScalarPointer();

	memcpy(ptr, volInfo.mVolumeData, volInfo.mWidth * volInfo.mHeight * volInfo.mDepth * 2);

	vtkSmartPointer< vtkImageMarchingCubes > marchingCubes = vtkSmartPointer< vtkImageMarchingCubes >::New();

	marchingCubes->SetInputData(volumeData);

	marchingCubes->SetValue(0, volInfo.mAirVoxelFilterVal);

	marchingCubes->ComputeNormalsOn();

	marchingCubes->Update();



	std::cout << " computing iso surface finished " << std::endl;

	vtkSmartPointer< vtkPolyData > isoSurface = vtkSmartPointer< vtkPolyData >::New();

	isoSurface->DeepCopy(marchingCubes->GetOutput());

	//vtkSmartPointer< vtkPolyDataNormals > normalEstimator = vtkSmartPointer< vtkPolyDataNormals >::New();

	//normalEstimator->SetInputData(isoSurface);

	//normalEstimator->ComputePointNormalsOn();

	//normalEstimator->ComputeCellNormalsOn();

	//normalEstimator->SplittingOff();

	////normalEstimator->NonManifoldTraversalOn();

	//normalEstimator->ConsistencyOn();

	//normalEstimator->Update();

	//isoSurface->DeepCopy(normalEstimator->GetOutput());

	/*vtkSmartPointer< vtkPLYWriter > writer = vtkSmartPointer< vtkPLYWriter >::New();

	writer->SetInputData(isoSurface);

	std::string meshPath = "C:/projects/Wallthickness/data/bottlemesh.ply";

	writer->SetFileName(meshPath.c_str());

	writer->Update();*/

	tr::Display3DRoutines::displayPolyData(isoSurface);

	long int numPoints = isoSurface->GetNumberOfPoints();
	long int numTriangles = isoSurface->GetNumberOfCells();

	volInfo.mVertices.resize(numPoints);
	volInfo.mVertexNormals.resize(numPoints);
	volInfo.mFaceIndices.resize(numTriangles * 3);

	vtkSmartPointer<vtkDataArray> normals = isoSurface->GetPointData()->GetNormals();

	Eigen::Vector3f center1(0, 0, 0), center2(-4.5, 0, 0), center3(4.5, 0, 0);

	for (long int pp = 0; pp < numPoints; pp++)
	{
		double point[3];

		isoSurface->GetPoint(pp, point);

		volInfo.mVertices[pp](0) = point[0];
		volInfo.mVertices[pp](1) = point[1];
		volInfo.mVertices[pp](2) = point[2];

		double n[3];

		normals->GetTuple(pp, n);

		volInfo.mVertexNormals[pp](0) = n[0];
		volInfo.mVertexNormals[pp](1) = n[1];
		volInfo.mVertexNormals[pp](2) = n[2];

	}

	tr::Display3DRoutines::displayPointSet(volInfo.mVertices, volInfo.mVertexNormals);

	//return 0;

	vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();


	for (long int tt = 0; tt < numTriangles; tt++)
	{
		isoSurface->GetCellPoints(tt, triangle);

		volInfo.mFaceIndices[3 * tt] = triangle->GetId(0);
		volInfo.mFaceIndices[3 * tt + 1] = triangle->GetId(1);
		volInfo.mFaceIndices[3 * tt + 2] = triangle->GetId(2);
	}


	volInfo.saveVolume(wtaFilePath);

}