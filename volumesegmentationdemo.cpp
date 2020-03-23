
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
#include "overlappedvoxelsegmentation.h"
#include <vtkAppendPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkInteractorStyleTrackballCamera.h>
//#include "ipp.h"



#include "stdio.h"
#include "stdlib.h"
#include "ipp.h"



static void HandleIppError(IppStatus err,
	const char *file,
	int line) {
	if (err != ippStsNoErr) {
		printf("IPP Error code %d in %s at line %d %s\n", err, 
			file, line , ippGetStatusString(err) );
		exit(EXIT_FAILURE);
	}
}

#define ippSafeCall( err ) (HandleIppError( err, __FILE__, __LINE__ ))

void readUint16_SCV(QString filePath, int w, int h, int d , imt::volume::VolumeInfo& volume )
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


void readVGI( QString filePath , int& w , int& h , int& d , float& voxelStep )
{
	QFile file(filePath);

	file.open(QIODevice::ReadOnly);

	QTextStream reader(&file);

	qDebug() << reader.readLine() << endl;
	qDebug() << reader.readLine() << endl;
	//qDebug() << reader.readLine() << endl;

	QString title , c1 , c2 , c3 , c4;


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


void viewVolume( OutVolume& volume )
{


	//vtkSmartPointer<vtkRenderWindow> renWin =
	//	vtkSmartPointer<vtkRenderWindow>::New();
	//vtkSmartPointer<vtkRenderer> ren1 =
	//	vtkSmartPointer<vtkRenderer>::New();
	//ren1->SetBackground(0.1, 0.4, 0.2);

	//renWin->AddRenderer(ren1);

	//renWin->SetSize(301, 300); // intentional odd and NPOT  width/height

	//vtkSmartPointer<vtkRenderWindowInteractor> iren =
	//	vtkSmartPointer<vtkRenderWindowInteractor>::New();
	//iren->SetRenderWindow(renWin);

	//renWin->Render(); // make sure we have an OpenGL context.

	std::cout << " view volume start " << std::endl;

	int w = volume.size[0];
	int h = volume.size[1];
	int d = volume.size[2];

	std::cout << w << " " << h << " " << d << std::endl;

	std::vector< long > histogram( std::numeric_limits< unsigned short >::max() , 0 );

	std::vector< Eigen::Vector3f > points;

	points.reserve(10000000);

	int minVal = 50000 , maxVal = -1;
	long id = 0;
	for (int zz = 0; zz < d; zz++)
		for (int yy = 0; yy < h; yy++)
			for (int xx = 0; xx < w; xx++)
			{
				minVal = std::min(minVal, (int)volume.data[id]);
				maxVal = std::max(maxVal, (int)volume.data[id]);

				histogram[volume.data[id]]++;

				id++;
			}


	int ii = 0;

	std::vector< unsigned short > materialValues;

	for (auto val : histogram)
	{

		ii++;
		if (val == 0)
			continue;

		materialValues.push_back(ii - 1);

		std::cout << " iso value : " << ii - 1 << " " << val << std::endl;
	}

	int zStep = w * h;
	int yStep = w;

	std::cout << w *h * d << " " << id  << " " << materialValues.size() << std::endl;

	//return;


	for ( int zz = 1; zz < d - 1; zz++)
		for ( int yy = 1; yy < h - 1; yy++)
			for ( int xx = 1; xx < w - 1; xx++)
			{
				long id1 = zStep * zz + yStep * yy + xx;

				bool surfacePointFound = false;

				if ( volume.data[id1] != materialValues[ 0 ] )
					continue;

				for ( int x = xx - 1; x <= xx + 1; x++)
				{
					for ( int y = yy - 1; y <= yy + 1; y++)
					{
						for ( int z = zz - 1; z <= zz + 1; z++)
						{
							long id2 = zStep * z + yStep * y + x;

							if ( volume.data[id1] != volume.data[id2] )
							{
								surfacePointFound = true;
							}
						}
					}

				}

				if ( surfacePointFound )
				points.push_back(Eigen::Vector3f(xx, yy, zz));

			}


	std::vector< Eigen::Vector3f > pointColors(points.size(), Eigen::Vector3f(1, 0, 0));

	tr::Display3DRoutines::displayPointSet(points, pointColors);


	std::cout << " min and max values : " << minVal << " " << maxVal << std::endl;

	std::cout << w << " " << h << " " << d << std::endl;


#if 1
	vtkSmartPointer< vtkImageData > volumeData = vtkSmartPointer< vtkImageData >::New();


	volumeData->SetOrigin( 0, 0, 0);
	volumeData->SetSpacing(volume.voxel_size[0], volume.voxel_size[1], volume.voxel_size[2]);
	volumeData->SetDimensions(volume.size[0], volume.size[1], volume.size[2]);
	volumeData->SetExtent(0, volume.size[0] - 1, 0, volume.size[1] - 1, 0, volume.size[2] - 1);
	volumeData->AllocateScalars( VTK_UNSIGNED_SHORT , 1 );

	long int nPoints = volumeData->GetNumberOfPoints();

	unsigned char *ptr = (unsigned char *)volumeData->GetScalarPointer();

	memcpy(ptr, volume.data.get(), volume.size[0] * volume.size[1] * volume.size[2] * 2);

	vtkSmartPointer< vtkMarchingCubes > marchingCubes = vtkSmartPointer< vtkMarchingCubes >::New();

	marchingCubes->SetInputData(volumeData);

	int value = 65535 / 3;

	std::cout << value << " " << 2 * value << std::endl;

	marchingCubes->SetValue(0, value);

	marchingCubes->ComputeNormalsOn();

	marchingCubes->Update(0);

	vtkSmartPointer< vtkPolyData > isoSurface = vtkSmartPointer< vtkPolyData >::New();

	isoSurface->DeepCopy(marchingCubes->GetOutput());

	tr::Display3DRoutines::displayPolyData(isoSurface);


#else

	std::cout << " view volume end " << std::endl;

	vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper =
		vtkSmartPointer<vtkSmartVolumeMapper>::New();
	volumeMapper->SetBlendModeToComposite(); // composite first
#if VTK_MAJOR_VERSION <= 5
	volumeMapper->SetInputConnection(imageData->GetProducerPort());
#else
	volumeMapper->SetInputData(volumeData);
#endif 

	vtkSmartPointer<vtkVolumeProperty> volumeProperty =
		vtkSmartPointer<vtkVolumeProperty>::New();
	volumeProperty->ShadeOff();
	volumeProperty->SetInterpolationType(VTK_LINEAR_INTERPOLATION);

	vtkSmartPointer<vtkPiecewiseFunction> compositeOpacity =
		vtkSmartPointer<vtkPiecewiseFunction>::New();
	compositeOpacity->AddPoint(0.0, 0.0);
	compositeOpacity->AddPoint( 1 , 0.5);
	compositeOpacity->AddPoint(2, 1);
	volumeProperty->SetScalarOpacity(compositeOpacity); // composite first.


	vtkSmartPointer<vtkColorTransferFunction> color =
		vtkSmartPointer<vtkColorTransferFunction>::New();
	color->AddRGBPoint(0.0, 0.0, 0.0, 1.0);
	color->AddRGBPoint(1, 1.0, 0.0, 0.0);
	color->AddRGBPoint(2.0, 1.0, 1.0, 1.0);
	volumeProperty->SetColor(color);


	vtkSmartPointer<vtkVolume> rVolume =
		vtkSmartPointer<vtkVolume>::New();
	rVolume->SetMapper(volumeMapper);
	rVolume->SetProperty(volumeProperty);
	ren1->AddViewProp(rVolume);
	ren1->ResetCamera();

	// Render composite. In default mode. For coverage.
	renWin->Render();

	// Software mode, for coverage. It also makes sure we will get the same
	// regression image on all platforms.
	volumeMapper->SetRequestedRenderModeToRayCast();
	renWin->Render();

	iren->Start();

#endif

}

void viewSTLFile(QString stlFileName)
{
	vtkSmartPointer< vtkSTLReader > reader = vtkSmartPointer< vtkSTLReader >::New();

	std::cout << stlFileName.toStdString().c_str() << std::endl;

	reader->SetFileName(stlFileName.toStdString().c_str());

	reader->Update();

	vtkSmartPointer< vtkPolyData > meshData = vtkSmartPointer< vtkPolyData >::New();

	meshData->DeepCopy( reader->GetOutput() );

	std::cout << meshData->GetNumberOfCells() << " " << meshData->GetNumberOfPoints() << std::endl;


	tr::Display3DRoutines::displayPolyData(meshData);

	return;


	QFile file(stlFileName);

	file.open(QIODevice::ReadOnly);

	QTextStream reader2( &file );

	QString txt;

	reader2 >> txt;
	reader2 >> txt;

	std::vector< Eigen::Vector3f > vertexTriplets;

	while (1)
	{

		
		//qDebug() << txt << endl;

		reader2 >> txt;

		if (txt != "facet")
		{
			qDebug() << txt << endl;

			break;
		}
			

		//qDebug() << txt << endl;

		reader2 >> txt;

		if (txt == "normal")
		{
			Eigen::Vector3f n;

			//qDebug() << txt << endl;

			reader2 >> txt;

			n(0) = txt.toDouble();

			//qDebug() << txt << endl;

			reader2 >> txt;

			n(1) = txt.toDouble();

			//qDebug() << txt << endl;

			reader2 >> txt;

			n(2) = txt.toDouble();

			reader2 >> txt;

		}


		if (txt == "outer")
		{
			reader2 >> txt;
			reader2 >> txt;

			Eigen::Vector3f v1, v2, v3;

			if (txt == "vertex")
			{
				reader2 >> txt;

				v1(0) = txt.toDouble();

				reader2 >> txt;
				v1(1) = txt.toDouble();

				reader2 >> txt;
				v1(2) = txt.toDouble();

				reader2 >> txt;

			}
			else
			{
				break;
			}

			

			if (txt == "vertex")
			{
				reader2 >> txt;

				v2(0) = txt.toDouble();

				reader2 >> txt;
				v2(1) = txt.toDouble();

				reader2 >> txt;
				v2(2) = txt.toDouble();

				reader2 >> txt;

			}
			else
			{
				break;
			}


			if (txt == "vertex")
			{
				reader2 >> txt;

				v3(0) = txt.toDouble();

				reader2 >> txt;
				v3(1) = txt.toDouble();

				reader2 >> txt;
				v3(2) = txt.toDouble();

				reader2 >> txt;

			}
			else
			{
				break;
			}


			vertexTriplets.push_back(v1);
			vertexTriplets.push_back(v2);
			vertexTriplets.push_back(v3);

			//qDebug() << txt << endl;

			if (txt != "endloop")
			{
				break;
			}

			reader2 >> txt;

			//qDebug() << txt << endl;

			if (txt != "endfacet")
			{
				break;
			}

		}



		//qDebug() << txt << endl;

	}


	std::cout << " num triplets : " << vertexTriplets.size() / 3 << std::endl;

	std::vector< unsigned int > indices(vertexTriplets.size());

	for (int ii = 0; ii < vertexTriplets.size(); ii++)
	{
		indices[ii] = ii;
	}

	tr::Display3DRoutines::displayMesh(vertexTriplets, indices);


}


void viewIsoSurface(imt::volume::VolumeInfo& volume, int isoVal);

void engineDataIsoSurfaceDemo();

void viewCopiedImage()
{

	cv::Mat image(480, 640, CV_8UC3);

	unsigned char *texCPUBuffer = image.data;


	FILE *file = fopen("C:/DebugData/texImage.dat", "rb");

	fread(texCPUBuffer, 1, 640 * 480 * 3, file);

	fclose(file);

	cv::namedWindow("image" , 0);
	cv::imshow("image", image);
	cv::waitKey();

}

void resizeVolume_Uint16C1( unsigned short *inputVolume , int iWidth, int iHeight, int iDepth, unsigned short *outputVolume, int oWidth, int oHeight, int oDepth , double resizeRatio )
{
	IpprVolume inputVolumeSize , outputVolumeSize;

	int srcStep = iWidth * sizeof(unsigned short);
	int srcPlaneStep = iWidth * iHeight * sizeof(unsigned short);
	IpprCuboid srcVoi;

	int dstStep = oWidth * sizeof(unsigned short);
	int dstPlaneStep = oWidth * oHeight * sizeof(unsigned short);
	IpprCuboid dstVoi;

	double xFactor = resizeRatio, yFactor = resizeRatio, zFactor = resizeRatio;
	double xShift = 0, yShift = 0, zShift = 0;

	int interpolation =  IPPI_INTER_LINEAR;//IPPI_INTER_CUBIC2P_B05C03;//

	// Type of interpolation, the following values are possible :

	// IPPI_INTER_NN - nearest neighbor interpolation,

	//	IPPI_INTER_LINEAR - trilinear interpolation,

	//	IPPI_INTER_CUBIC - tricubic interpolation,

	//	IPPI_INTER_CUBIC2P_BSPLINE - B - spline,

	//	IPPI_INTER_CUBIC2P_CATMULLROM - Catmull - Rom spline,

	//	IPPI_INTER_CUBIC2P_B05C03 - special two - parameters filter(1 / 2, 3 / 10).

	inputVolumeSize.width = iWidth;
	inputVolumeSize.height = iHeight;
	inputVolumeSize.depth = iDepth;

	srcVoi.x = 0;
	srcVoi.y = 0;
	srcVoi.z = 0;

	srcVoi.width = iWidth;
	srcVoi.height = iHeight;
	srcVoi.depth = iDepth;

	dstVoi.x = 0;
	dstVoi.y = 0;
	dstVoi.z = 0;

	dstVoi.width = oWidth;
	dstVoi.height = oHeight;
	dstVoi.depth = oDepth;

	Ipp8u *computationBuffer;

	int bufferSize = 0;
	
	ippSafeCall(ipprResizeGetBufSize(srcVoi, dstVoi, 1, interpolation, &bufferSize));

	computationBuffer = new Ipp8u[bufferSize];


	ippSafeCall(ipprResize_16u_C1V(inputVolume, inputVolumeSize, srcStep, srcPlaneStep, srcVoi, outputVolume, dstStep,
		                dstPlaneStep, dstVoi, xFactor, yFactor, zFactor, xShift, yShift, 
		                zShift, interpolation, computationBuffer));

}


void resizeVolume_Uint16C1_MT(unsigned short *inputVolume, int iWidth, int iHeight, int iDepth, unsigned short *outputVolume, int oWidth,
	int oHeight, int oDepth, double resizeRatio)
{
	int bandWidth = 32;

	int numBands = oDepth % 32 == 0 ? oDepth / bandWidth : oDepth / bandWidth + 1;

	int extension = (1.0 / resizeRatio) + 1;

	if (numBands > 1)
	{
		for (int bb = 0; bb < numBands; bb++)
		{
			IpprVolume inputVolumeSize, outputVolumeSize;

			int srcStep = iWidth * sizeof(unsigned short);
			int srcPlaneStep = iWidth * iHeight * sizeof(unsigned short);
			IpprCuboid srcVoi;

			int dstStep = oWidth * sizeof(unsigned short);
			int dstPlaneStep = oWidth * oHeight * sizeof(unsigned short);
			IpprCuboid dstVoi;

			double xFactor = resizeRatio, yFactor = resizeRatio, zFactor = resizeRatio;
			double xShift = 0, yShift = 0, zShift = 0;

			int interpolation = IPPI_INTER_LINEAR;

			int sourceBand = (int)( bandWidth / resizeRatio + 1 );

			if (bb == 0 || bb == numBands - 1)
			{
				sourceBand += extension;
			}
			else
			{
				sourceBand += 2 * extension;
			}

			double destStartLine = bandWidth * bb;

			int destBandWidth = bandWidth;

			double sourceStartLine = destStartLine / resizeRatio;

			if (bb == numBands - 1)
			{
				destBandWidth = oDepth - bandWidth * bb;
			}

			dstVoi.x = 0;
			dstVoi.y = 0;
			dstVoi.z = 0;

			dstVoi.width = oWidth;
			dstVoi.height = oHeight;
			dstVoi.depth = destBandWidth;

			size_t sourceDataShift = 0;
			size_t destDataShift = 0;

			double sourceLineZ = bandWidth * bb / resizeRatio;

			int sourceStartZ = (int)(sourceLineZ - extension);

			if ( bb == 0 )
			  sourceStartZ = 0;

			if (bb == numBands - 1)
			{
				sourceBand = iDepth - sourceStartZ;
			}

			srcVoi.x = 0;
			srcVoi.y = 0;
			srcVoi.z = 0; 

			srcVoi.width = iWidth;
			srcVoi.height = iHeight;
			srcVoi.depth = sourceBand;

			inputVolumeSize.width = iWidth;
			inputVolumeSize.height = iHeight;
			inputVolumeSize.depth = sourceBand;

			sourceDataShift = (size_t)sourceStartZ * (size_t)iWidth * (size_t)iHeight;
			destDataShift = (size_t)bandWidth * (size_t)bb * (size_t)oWidth * (size_t)oHeight;

			Ipp8u *computationBuffer;

			zShift = - destStartLine + sourceStartZ * resizeRatio;

			int bufferSize = 0;

			ippSafeCall(ipprResizeGetBufSize(srcVoi, dstVoi, 1, interpolation, &bufferSize));

			computationBuffer = new Ipp8u[bufferSize];

			ippSafeCall(ipprResize_16u_C1V(inputVolume + sourceDataShift, inputVolumeSize, srcStep, srcPlaneStep, 
				                           srcVoi, outputVolume + destDataShift, dstStep,dstPlaneStep, dstVoi, 
										   xFactor, yFactor, zFactor, xShift, yShift, zShift, interpolation,computationBuffer));

			delete[] computationBuffer;
		}
	}
	else
	{
		resizeVolume_Uint16C1(inputVolume, iWidth, iHeight, iDepth, outputVolume, oWidth, oHeight, oDepth, resizeRatio);
	}

}







void pyrDown(imt::volume::VolumeInfo& inputVolume, imt::volume::VolumeInfo& outputVolume)
{
	outputVolume.mWidth = inputVolume.mWidth / 2;
	outputVolume.mHeight = inputVolume.mHeight / 2;
	outputVolume.mDepth = inputVolume.mDepth / 2;

	outputVolume.mVolumeData = ( unsigned char* )(new unsigned short[outputVolume.mWidth * outputVolume.mHeight * outputVolume.mDepth] );

	outputVolume.mVoxelStep = inputVolume.mVoxelStep;

	memset(outputVolume.mVolumeData, 0, (size_t)outputVolume.mWidth * (size_t)outputVolume.mHeight * (size_t)outputVolume.mDepth * sizeof(unsigned short));

	resizeVolume_Uint16C1_MT((unsigned short*)inputVolume.mVolumeData, inputVolume.mWidth, inputVolume.mHeight, inputVolume.mDepth,
		(unsigned short*)outputVolume.mVolumeData, outputVolume.mWidth, outputVolume.mHeight, outputVolume.mDepth , 0.5);

}


void showMultiMaterialIssue(imt::volume::VolumeInfo& volume, Materials& materials);

int main( int argc , char **argv )
{


	std::cout << (100 >> 1) << std::endl;


	//viewCopiedImage();

	//	return 0;

	//engineDataIsoSurfaceDemo();

	//return 0;


	QString filePath = "C:/projects/Wallthickness/data/bottle/bottle.wtadat";//"C:/projects/Wallthickness/data/fanuc_engine_cover/engine_cover.wtadat";//	

	QString fileName =
		"C:\\Data\\fEDER_CLIP";
		//"G:\\Data\\CAD, Object data\\connectingRod";
		//"G:/Data/CAD, Object data/Aluminium_1";
		//"C:\\Data\\VolumeNotLoading\\sphere_simulation_noScat_noBH_50kV_1ray";
		//"C:\\Projects\\Wallthickness\\data\\Datasets for Multi-material\\MultiMaterial\\MAR_Ref 2012-8-30 15-13";

		//"C:\\Projects\\Wallthickness\\data\\CT multi\\Hemanth_ Sugar free 2016-6-6 11-33"; //
		//"C:/Projects/Wallthickness/data/separated_part_7";
	//"G:/Data/Multimaterial/T2_R2_BSC_M800_BHC_02_03";
	//"C:/Data/WallthicknessData/10.03.17/Mobile cover_Static 2017-3-10 12-44";
	//"C:/Data/WallthicknessData/11.1.17/Bizerba_2 2016-8-25 17-50-31";
	//"C:/Data/WallthicknessData/11.1.17/Blister_Single 2016-10-19 8-55";
	// "C:/projects/Wallthickness/data/CT multi/Calliper Assy 2016-6-9 10-12";
	//"C:/projects/Wallthickness/data/Datasets for Multi-material/MultiMaterial/MAR_Ref 2012-8-30 15-13"; 
	//"C:/projects/Wallthickness/data/Mobile charger/Wall Thick_RnD_1 2016-10-13 15-10_sep1";
	//"C:/projects/Wallthickness/data/Mobile charger/Wall Thick_RnD_1 2016-10-13 15-10_sep1"; 
	//"G:/Data/separated_part_0";
	//"C:/projects/Wallthickness/data/CT multi/Hemanth_ Sugar free 2016-6-6 11-33";
	//"C: / projects / Wallthickness / data / CT multi / Fuel_filter_0km_PR_1 2016 - 5 - 30 12 - 54";//;
	//"G:/projects/Wallthickness/data/Mobile charger/Wall Thick_RnD_1 2016-10-13 15-10_sep1"; 
	//"C:\\Projects\\Data\\separated_part_0";
		//"G:\\Data\\Mixed material from Ecaterina\\T2_R2_BSC_M800_BHC_02_03";
	//"C:/projects/datasets/bug_38950/Sample H wo frame_1 2012-9-10 6-27";//
	QString scvFilePath = fileName + ".uint16_scv";
	QString vgiFilePath = fileName + ".vgi";
	QString stlFilePath = "C:/projects/Wallthickness/data/Mobile _ Charger.stl";

	//viewSTLFile(stlFilePath);

	//return 0;

	int w, h, d;

	float voxelStep;

	imt::volume::VolumeInfo volume;

	//readVGI(vgiFilePath , w , h , d , voxelStep );

	

	//readUint16_SCV(scvFilePath , w , h , d , volume );

	std::cout << "uint16scv file path : " << scvFilePath.toStdString() << std::endl;

	imt::volume::RawVolumeDataIO::readUint16SCV(scvFilePath, volume);

	//FILE *file = fopen("C:\\Data\\resizedVolume.dat" , "rb");

	//
	//volume.mWidth = 1021;
	//volume.mHeight = 1029;
	//volume.mDepth = 1033;

	//volume.mVolumeData = (unsigned char*)(new short[volume.mWidth * volume.mHeight * volume.mDepth]);


	//fread(volume.mVolumeData, 2, volume.mWidth * volume.mHeight * volume.mDepth , file);

	//fclose(file);


	imt::volume::VolumeInfo& resizedVolume = volume;

	std::cout << " resizing volume " << std::endl;

	//pyrDown(volume, resizedVolume);

	//resizedVolume = volume;

	std::cout << " volume resized " << std::endl;


	std::cout << volume.mWidth << " " << volume.mHeight << " " << volume.mDepth << " " << voxelStep << std::endl;

	std::cout << resizedVolume.mWidth << " " << resizedVolume.mHeight << " " << resizedVolume.mDepth << " " << voxelStep << std::endl;
    //volume.loadVolume(filePath);

	//tr::Display3DRoutines::displayMesh(volume.mVertices, volume.mFaceIndices);

	std::cout << " air voxel filter threshold : " << volume.mAirVoxelFilterVal << std::endl;


	imt::volume::HistogramFilter filter( &volume );

	std::vector< long long > histogram( std::numeric_limits< unsigned short >::max() , 0 )  ;

	unsigned short *vData = (unsigned short*)resizedVolume.mVolumeData;

	for (int zz = 0; zz < resizedVolume.mDepth; zz++)
		for (int yy = 0; yy < resizedVolume.mHeight; yy++)
			for (int xx = 0; xx < resizedVolume.mWidth; xx++)
			{
				histogram[vData[zz * resizedVolume.mWidth * resizedVolume.mHeight + yy * resizedVolume.mWidth + xx]] += 1;
			} 


	long maxHistogramVal = *( std::max_element( histogram.begin() , histogram.end() ) );

	//tr::Display3DRoutines::displayHistogram(histogram, 0, maxHistogramVal);
	//filter.plotHistogram(histogram);


	long iso50Th =  filter.ISO50Threshold(histogram);
	long otsuTh = filter.OtsuThreshold(histogram);

	std::cout << "iso 50 threshold : " << iso50Th << std::endl;

	viewIsoSurface(volume, iso50Th);
	

	std::cout << " iso 50 threshold : " << iso50Th << std::endl;
	std::cout << " otsu threshold : " << otsuTh << std::endl;
	std::cout << " volume graphics threshold : " << volume.mAirVoxelFilterVal << std::endl;

	//setup the volume

	Volume vol;

	vol.size[0] = resizedVolume.mWidth;
	vol.size[1] = resizedVolume.mHeight;
	vol.size[2] = resizedVolume.mDepth;

	vol.voxel_size[0] = resizedVolume.mVoxelStep(0);
	vol.voxel_size[1] = resizedVolume.mVoxelStep(1);
	vol.voxel_size[2] = resizedVolume.mVoxelStep(2);

	vol.data = (uint16_t*)resizedVolume.mVolumeData;

	std::cout << volume.mWidth << " " << volume.mHeight << " " << volume.mDepth << std::endl;

	std::cout << resizedVolume.mWidth << " " << resizedVolume.mHeight << " " << resizedVolume.mDepth << std::endl;


	ZeissSegmentationInterface segmenter;

	SegmentationParameter param;
	param.max_memory = static_cast<size_t>(2048) * 1024 * 1024;
	param.output_material_index = false;
	segmenter.setParameter(param);

	segmenter.setInputVolume(vol);

	Materials materials = segmenter.getMaterialRegions();


	//std::cout << " num materials : " << materials.regions.size() << std::endl;

	//std::cout << " lower bound : " << materials.regions[3].lower_bound << std::endl;


	std::cout << materials.regions[0].lower_bound << " " << materials.regions[1].lower_bound << " " << materials.regions[1].upper_bound << std::endl;

	viewIsoSurface(resizedVolume, materials.regions[materials.first_material_index].lower_bound);

	//showMultiMaterialIssue(resizedVolume, materials);

	return 0;

	//2304,4128,5024

	std::vector< int > isoThresholds(3);

	isoThresholds[0] = 2304;
	isoThresholds[1] = 4128;
	isoThresholds[2] = 5024;

	//imt::volume::OverlappedVoxelSegmentation ovs(volume);

	//ovs.compute(isoThresholds);



	//return 0;

	materials.first_material_index = 1;


	std::cout << " first material index : " << materials.first_material_index << " " << materials.regions[1].lower_bound << std::endl;

	//return 0;

	Materials materials2;

	materials2.regions.resize(2);

	materials2.first_material_index = 1;

	materials2.regions[0] = materials.regions[0];
	materials2.regions[1] = materials.regions[1];
	materials2.regions[1].lower_bound = materials.regions[1].lower_bound; //(materials.regions[0].lower_bound + materials.regions[1].lower_bound) / 2;
	materials2.regions[1].upper_bound = materials.regions[materials.regions.size() - 1].upper_bound;

	//applying method do segmentation
	segmenter.doSegmentation( materials );

	OutVolume ov = segmenter.getSegmentVolume();

	viewVolume(ov);

	//return 0;

	std::cout << " number of materials : " << materials.regions.size() << std::endl;


	std::cout << materials.first_material_index << " " << materials.regions.size() << std::endl;

	std::cout << materials.regions[1].lower_bound << " " << materials.regions[0].lower_bound << std::endl;

	std::cout << " first material lower and upper bound " << materials.regions[1].lower_bound << " " << materials.regions[1].upper_bound << std::endl;

	long airPeak = materials.regions[0].peak;
	long materialPeak = materials.regions[1].peak;

	std::cout << airPeak << " " << materialPeak << " " << (airPeak + materialPeak) / 2 << " " << (airPeak + materials.regions[1].lower_bound) / 2 << std::endl;


	// segmenter.doSegmentation(materials);


	//std::cout << materials.regions[1].lower_bound << std::endl;

	//segmenter.doBenchmark();



	 //segmenter.doSegmentation();

	//Materials m =  segmenter.getMaterialRegions();

	//std::cout << " number of materials : " << m.regions.size() << std::endl;

	//ZeissSeparationInterface separator;


	//separator.setInputVolume(vol);

	//separator.doSeparation();

	//std::cout<<" background peak value "<< separator.getBackgroundPeakValue() << std::endl;

	//std::cout << "  object threshold  : " << separator.getThreshold() * std::numeric_limits<uint16_t>::max() << std::endl;


	std::cout << materials.regions[1].lower_bound << " " << materials.regions[1].upper_bound << " " << materials.regions[2].lower_bound << std::endl;


	int objTh = materials.regions[1].lower_bound;// (airPeak + materialPeak) / 2;//materials.regions[1].upper_bound;//// iso50Th;//volume.mAirVoxelFilterVal;// separator.getThreshold() * std::numeric_limits<uint16_t>::max();separator.getBackgroundPeakValue();// 
	//volume.mAirVoxelFilterVal;// volume.mV//materials.regions[1].lower_bound;//materials.regions[2].lower_bound;//
	//materials.regions[1].lower_bound + 40;//iso50Th;//

	//imt::volume::VolumeSegmenter segmenter2( volume );

	//std::cout << " computing segmentation " << std::endl;

	//segmenter2.compute(objTh, 20, 20);

	//return 0;
	
	std::cout << " fraunhoffer threshold " << objTh << std::endl;

	//double threshold = separator.getThreshold();

	//std::cout << "  threshold : " << threshold << std::endl;

	//separator.doSeparation();


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

	memcpy( ptr, volume.mVolumeData, volume.mWidth * volume.mHeight * volume.mDepth * 2);

	vtkSmartPointer< vtkMarchingCubes > marchingCubes = vtkSmartPointer< vtkMarchingCubes >::New();

	marchingCubes->SetInputData(volumeData);

	marchingCubes->SetValue(0, objTh);

	marchingCubes->ComputeNormalsOn();

	marchingCubes->Update(0);

	vtkSmartPointer< vtkPolyData > isoSurface = vtkSmartPointer< vtkPolyData >::New();

	isoSurface->DeepCopy(marchingCubes->GetOutput());

	vtkSmartPointer< vtkPLYWriter > writer = vtkSmartPointer< vtkPLYWriter >::New();

	writer->SetFileName("C:/projects/Wallthickness/data/Mobile_Charger.ply");

	writer->SetInputData(isoSurface);

	writer->Update();

	tr::Display3DRoutines::displayPolyData(isoSurface);

	return 0;
}



void buildTransferFunction()
{



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


vtkSmartPointer<vtkPolyData> isoSurface(imt::volume::VolumeInfo& volume, int isoVal , Eigen::Vector3f color)
{
	vtkSmartPointer< vtkImageData > volumeData = vtkSmartPointer< vtkImageData >::New();


	volumeData->SetOrigin(volume.mVolumeOrigin(0), volume.mVolumeOrigin(1), volume.mVolumeOrigin(2));
	volumeData->SetSpacing(volume.mVoxelStep(0), volume.mVoxelStep(1), volume.mVoxelStep(2));
	volumeData->SetDimensions(volume.mWidth, volume.mHeight, volume.mDepth);
	volumeData->SetExtent(0, volume.mWidth - 1, 0, volume.mHeight - 1, 0, volume.mDepth - 1);
	volumeData->AllocateScalars(VTK_UNSIGNED_SHORT, 1);

	long int nPoints = volumeData->GetNumberOfPoints();

	vtkSmartPointer< vtkUnsignedCharArray > vColors = vtkSmartPointer< vtkUnsignedCharArray >::New();

	vColors->SetNumberOfComponents(3);
	vColors->SetName("Colors");


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

	isoSurface->DeepCopy( marchingCubes->GetOutput() );

	int numPoints = isoSurface->GetNumberOfPoints();

	for ( int pp = 0; pp < numPoints; pp++ )
	{
		unsigned char col[] = { 255 * color(0) , 255 * color(1), 255 * color(2) };

		vColors->InsertNextTypedTuple(col);
	}

	isoSurface->GetPointData()->SetScalars(vColors);

	return isoSurface;

}


void showMultiMaterialIssue(imt::volume::VolumeInfo& volume,  Materials& materials)
{
	int nMaterials = materials.regions.size();

	std::vector<vtkSmartPointer<vtkPolyData>> materialSurfaces(nMaterials - 1);
	std::vector<vtkSmartPointer<vtkActor>> actors(nMaterials - 1);

	Eigen::Vector4f colors[3] = { Eigen::Vector4f(1 , 1 , 1 , 0.1) , Eigen::Vector4f(0 , 1 , 0 , 0.3) , Eigen::Vector4f(1, 0 , 0 , 1) };

	// Create renderers and add actors of plane and cube
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();

	std::cout << "number of materials : " << nMaterials << std::endl;

	for (int mm = 0; mm < nMaterials - 1 ; mm++)
	{
		materialSurfaces[mm] = isoSurface(volume, materials.regions[mm + 1].lower_bound , colors[mm].block(0,0 , 3, 1));

		vtkSmartPointer<vtkPolyDataMapper> mapper =
			vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper->SetInputData(materialSurfaces[mm]);

		actors[mm] =
			vtkSmartPointer<vtkActor>::New();
		actors[mm]->GetProperty()->SetColor(colors[mm](0), colors[mm](1), colors[mm](2));
		actors[mm]->GetProperty()->SetOpacity(colors[mm](3));
		actors[mm]->SetMapper(mapper);

		renderer->AddActor(actors[mm]);

		//tr::Display3DRoutines::displayPolyData(materialSurfaces[mm]);

	}

	renderer->SetBackground(0, 0, 0);
	// Add renderer to renderwindow and render
	vtkSmartPointer< vtkRenderWindow > window = vtkSmartPointer< vtkRenderWindow >::New();
	vtkSmartPointer< vtkRenderWindowInteractor > interactor = vtkSmartPointer< vtkRenderWindowInteractor >::New();
	vtkSmartPointer< vtkInteractorStyleTrackballCamera > style = vtkSmartPointer< vtkInteractorStyleTrackballCamera >::New();
	window->AddRenderer(renderer);
	window->SetInteractor(interactor);
	interactor->SetInteractorStyle(style);

	window->Render();

	interactor->Start();

	
}


void engineDataIsoSurfaceDemo()
{
	QString volumePath = "C:/projects/Wallthickness/data/Engine.raw";

	imt::volume::VolumeInfo volInfo;

	volInfo.mWidth = 256;
	volInfo.mHeight = 256;
	volInfo.mDepth = 110;

	QFile file(volumePath);

	file.open(QIODevice::ReadOnly);

	QDataStream reader(&file);


	volInfo.mVolumeData = new unsigned char[volInfo.mWidth * volInfo.mHeight * volInfo.mDepth];

	reader.readRawData((char*)volInfo.mVolumeData, volInfo.mWidth * volInfo.mHeight * volInfo.mDepth);

	file.close();


	Volume vol;

	vol.size[0] = volInfo.mWidth;
	vol.size[1] = volInfo.mHeight;
	vol.size[2] = volInfo.mDepth;

	vol.voxel_size[0] = volInfo.mVoxelStep(0);
	vol.voxel_size[1] = volInfo.mVoxelStep(1);
	vol.voxel_size[2] = volInfo.mVoxelStep(2);

	uint16_t* rawData = new uint16_t[volInfo.mWidth * volInfo.mHeight * volInfo.mDepth];

	for (size_t ii = 0; ii < volInfo.mWidth * volInfo.mHeight * volInfo.mDepth; ii++)
	{
		rawData[ii] = volInfo.mVolumeData[ii] * 5;
	}

	vol.data = rawData;

	ZeissSegmentationInterface segmenter;

	SegmentationParameter param;
	param.max_memory = static_cast<size_t>(2048) * 1024 * 1024;
	param.output_material_index = false;
	segmenter.setParameter(param);

	segmenter.setInputVolume(vol);

	Materials materials = segmenter.getMaterialRegions();

	std::cout << " number of material regions : " << materials.regions.size() << std::endl;

	volInfo.mVolumeData = (unsigned char*)rawData;

	std::cout <<" th1 : " <<materials.regions[1].lower_bound * 0.2 << std::endl;


	viewIsoSurface(volInfo, 183 * 5);



}