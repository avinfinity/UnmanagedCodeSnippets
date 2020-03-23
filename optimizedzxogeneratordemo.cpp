#include "iostream"
#include "volumeinfo.h"
#include "volumeutility.h"

#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <VolumeViz/converters/SoVolumeConverter.h>
#include <LDM/converters/SoConverterParameters.h>
#include <Inventor/helpers/SbDataTypeMacros.h>
#include <LDM/readers/SoVRLdmFileReader.h>
#include <Inventor/devices/SoCpuBufferObject.h> 
#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <VolumeViz/nodes/SoVolumeData.h> 
#include <VolumeViz/nodes/SoTransferFunction.h>
#include <VolumeViz/readers/SoVRGenericFileReader.h> 
#include <LDM/converters/SoConverterParameters.h>
#include <LDM/readers/SoLDMReader.h>
#include <LDM/readers/SoVRLdmFileReader.h>
#include <LDM/SoLDMTopoOctree.h>

#include <LDM/fields/SoSFLDMResourceParameters.h>
#include "NeoInsightsLDMreader.h"
#include "NeoInsightsLDMWriter.h"
#include <VolumeViz/nodes/SoVolumeRender.h>

#include <Inventor/Xt/SoXt.h>
#include <Inventor/Xt/viewers/SoXtExaminerViewer.h>
#include <Inventor/nodes/SoSeparator.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/nodes/SoVolumeSkin.h>
#include <LDM/nodes/SoDataRange.h>
#include <LDM/nodes/SoTransferFunction.h>
#include <VolumeViz/nodes/SoVolumeRendering.h>


#include "volumeconverter.h"
#include "opencvincludes.h"
#include "volumetopooctree.h"

#include <vtkIdList.h>
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

#include "rawvolumedataio.h"
#include "volumeutility.h"
#include <Inventor/nodes/SoFragmentShader.h>
#include <VolumeViz/nodes/SoVolumeRenderingQuality.h>
#include "customtilemanager.h"
#include "customtilevisitor.h"
#include "customnodefrontmanager.h"
#include "CustomTiledReader.h"
#include "optimizedzxogenerator.h"
#include "rawvolumedataio.h"
#include "iostream"
#include "opencvincludes.h"
#include "ippr.h"
#include "ipp.h"

#include "NeoInsightsLDMWriter.h"
#include "optimizedzxoreader.h"
#include "QString"
#include <Inventor/lock/SoLockMgr.h>
#include <Inventor/SoInteraction.h>
#include <Inventor/nodekits/SoNodeKit.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/errors/SoGLError.h>
#include <Inventor/errors/SoMemoryError.h>
#include <Inventor/errors/SoReadError.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoComplexity.h>
#include <LDM/SoLDMDataAccess.h>
#include "customtilemanager.h"
#include "tileweightheap.h"
#include "rbtree.h"
#include "ZxoWriter.h"
#include "CustomTiledReader.h"

#include <Inventor/nodes/SoSwitch.h>

void testIppResize();
void viewIsoSurface(imt::volume::VolumeInfo& volume, int isoVal);
void generate(int depth, int srcWidth, int srcHeight, unsigned short* src, int dstDepth, int dstWidth, int dstHeight, unsigned short *dst);
void viewVolumeSkin(SoVolumeData* pVolumeData, ushort minVal, ushort maxVal);
void vsgLDMCOnversion(std::string filePath, int w, int h, int d, float xStep, float yStep, float zStep);
void viewVolume(std::string zxoFilePath, int minVal, int maxVal);

void viewTri()
{

	int nPoints, nCells;



	FILE *file = fopen("C:/Data/ZxoData/bitron.tri", "rb");

	fread(&nPoints, 1, sizeof(int), file);
	fread(&nCells, 1, sizeof(int), file);


	std::cout << " num points and faces :  "<< nPoints << " " << nCells << std::endl;

	std::vector< Eigen::Vector3f > points(nPoints), normals(nPoints);
	std::vector< unsigned int > indices(nCells * 3);
	fread(points.data(), points.size() * 3, sizeof(float), file);
	fread(normals.data(), normals.size() * 3, sizeof(float), file);
	fread(indices.data(), indices.size(), sizeof(unsigned int), file);

	fclose(file);


	tr::Display3DRoutines::displayMesh(points, indices);

}

void UnlockLicense();

void redBlackTreeDemo();

int main( int argc , char **argv )
{

	//viewTri();

	//return 0;

	//std::cout << " red black tree demo " << std::endl;

	//redBlackTreeDemo();

	//return 0;

	UnlockLicense();
	std::string uint16SCVPath = "C:/Data/VolumeNotLoading/sphere_simulation_noScat_noBH_50kV_1ray.uint16_scv"; //"C:/Data/ZxoData/VV_Test_Kupplung_7GB_Vx31 2015-6-19 9-16.uint16_scv";//// "C:/Data/07.06.17/Test Pice_BHC 2017-9-7 12-40.uint16_scv"; //"C:\\Data\\ZxoData\\07.06.17\\Test Pice 2017-9-7 10-16.uint16_scv"; //"C:/projects/Wallthickness/data/separated_part_7.uint16_scv";
		//"C:/Data/WallthicknessData/11.1.17/Blister_Full 2016-10-20 14-56.uint16_scv"; 
		//"C:/projects/Wallthickness/data/Mobile charger/Wall Thick_RnD_1 2016-10-13 15-10_sep1.uint16_scv";
		//"C:/projects/Wallthickness/data/CT multi/Calliper Assy 2016-6-9 10-12.uint16_scv"; //
		//"C:/Data/WallthicknessData/11.1.17/Bizerba_2 2016-8-25 17-50-31.uint16_scv"; // 4GB data
		//"C:/Data/ZxoData/ZXO From PCI - Not to Share/sample uint16 INTERNAL USE ONLY/VAST CT-Messung 2016-9-6 10-51-38.uint16_scv"; // 1 GB Data
		//"C:/Data/ZxoData/ZXO From PCI - Not to Share/sample uint16 INTERNAL USE ONLY/Sitz grau 1x1 2016-9-6 12-15-32.uint16_scv"; // 33 GB data
		
	"C:/Data/ZxoData/ZXO From PCI - Not to Share/sample uint16 INTERNAL USE ONLY/FEM_ForceFrame 2 2016-8-22 12-39.uint16_scv";//16 GB Data

	std::string filePath = "C:/Data/ZxoData/Test Pice_BHC 2017-9-7 12-40_minmax.zxo";//VV_Test_Kupplung_7GB_Vx31 2015-6-19 9-16.ZXO";// ""C:/Data/ZxoData/separated_part_7.ZXO";//// "C:/Data/ZxoData/separated_part_7.ZXO";
	"C:/Data/ZxoData/Bizerba_2 2016-8-25 17-50-31.ZXO";//"C:/Data/ZxoData/tcg_teil_bh 2017-2-1 14-46-5.ZXO";//FEM_ForceFrame 2 2016-8-22 12-39.ZXO//"C:/Data/ZxoData/ZXO From PCI - Not to Share/sample uint16 INTERNAL USE ONLY/FEM_ForceFrame 2 2016-8-22 12-39.zxo";
	viewVolume(filePath, 0, 65535);

	//return 0;
		
	
	//testIppResize();

	//return 0;

	std::string zxoFilePath = "C:/Data/ZxoData/Test Pice_BHC 2017-9-7 12-40_MinMax.ZXO";

	//imt::volume::OptimizedZXOGenerator zxoGen;

	ZxoWriter zxoGen;

	zxoGen.setRawFilePath(uint16SCVPath);

	imt::volume::VolumeInfo vol;

	//imt::volume::RawVolumeDataIO::readUint16SCV(QString(uint16SCVPath.c_str()), vol);

	std::vector< int64_t > histogram;// (USHRT_MAX);
	int minVal, maxVal;

	int isoThreshold = 0;

	//imt::volume::VolumeUtility::computeISOThreshold(vol, isoThreshold, histogram, minVal, maxVal);

	//delete[] vol.mVolumeData;

	std::cout << " volume dimensions : " << vol.mWidth << " " << vol.mHeight << " " << vol.mDepth << std::endl;

	std::cout << vol.mVoxelStep.transpose() << std::endl;

	//viewIsoSurface(vol, isoThreshold);

	//return 0;

	//vsgLDMCOnversion(uint16SCVPath, vol.mWidth, vol.mHeight, vol.mDepth, vol.mVoxelStep(0), vol.mVoxelStep(1), vol.mVoxelStep(2));

	zxoGen.setVolumeInfo(vol.mWidth, vol.mHeight, vol.mDepth, vol.mVoxelStep(0), vol.mVoxelStep(1), vol.mVoxelStep(2), 16, 1024, 1);

	double initT = cv::getTickCount();

	zxoGen.setZXOFilePath(zxoFilePath);

	initT = cv::getTickCount();

	//zxoGen.generate();

	//return 0;

	std::cout << " time spent in generation : " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;


	std::cout << " time spent in tiles generation " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;

	//minVal = 36880;// 14528, maxVal = 65000; //

	viewVolume(zxoFilePath, minVal, maxVal);

	return 0;
}


void vsgLDMCOnversion( std::string filePath , int w , int h , int d , float xStep ,  float yStep , float zStep )
{
	std::cout << " generate ldm file " << std::endl;

	int dim = 256;

	std::string ldmFilePath = "path1.ldm";

	SoConverterParameters* myConverterParameters = new SoConverterParameters();
	myConverterParameters->setHeaderFileName(SbString( ldmFilePath));
	myConverterParameters->setTileDim(dim);

	SoVRGenericFileReader* reader = nullptr;

	SbString inputString = filePath;// scvFilePath.toStdString();
	myConverterParameters->setInputFileName(inputString);

	reader = new SoVRGenericFileReader();
	reader->setFilename(inputString);
	reader->setDataChar( SbBox3f(0, 0, 0, w *(float)xStep , h * yStep , d * zStep ),
		                 SoDataSet::UNSIGNED_SHORT, SbVec3i32( w , h, d), 1024);

	std::cout << " data read completed " << std::endl;


	double initT = cv::getTickCount();

	Zeiss::IMT::NG::Metrotom::VSGWrapper::VolumeConverter* ldmConverter = new Zeiss::IMT::NG::Metrotom::VSGWrapper::VolumeConverter(reader);

	int ret = ldmConverter->convert(myConverterParameters);
	reader->closeAllHandles();

	std::cout << " time spent in ldm conversion : " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;

	delete reader;
	delete myConverterParameters;
}


//void testIppResize()
//{
//	std::string path = "C:/projects/Wallthickness/data/separated_part_7.uint16_scv";
//
//	imt::volume::VolumeInfo vol;
//
//	imt::volume::RawVolumeDataIO::readUint16SCV(QString(path.c_str()), vol);
//
//	int isoThreshold = 0;
//
//	std::vector< int64_t > histogram;
//
//	int minVal, maxVal;
//
//	imt::volume::VolumeUtility::computeISOThreshold(vol, isoThreshold, histogram, minVal, maxVal);
//
//	std::cout << " iso threshold : " << isoThreshold << std::endl;
//
//	imt::volume::VolumeUtility::extractIsoSurface(vol, isoThreshold);
//
//	viewIsoSurface(vol, isoThreshold);
//
//
//	imt::volume::VolumeInfo resizedVol;
//
//	resizedVol.mWidth = vol.mWidth / 2;
//	resizedVol.mHeight = vol.mHeight / 2;
//	resizedVol.mDepth = vol.mDepth / 2;
//
//	resizedVol.mVolumeData = (unsigned char *)( new unsigned short[resizedVol.mWidth * resizedVol.mHeight * resizedVol.mDepth] );
//
//	generate(vol.mWidth, vol.mHeight, vol.mDepth, (unsigned short*)vol.mVolumeData, resizedVol.mWidth, resizedVol.mHeight, resizedVol.mDepth, (unsigned short*)resizedVol.mVolumeData);
//
//
//}


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


	int nPoints = isoSurface->GetNumberOfPoints();
	int nCells = isoSurface->GetNumberOfCells();

	std::vector< float > points( nPoints * 3 ), normals( nPoints * 3 );
	std::vector< unsigned int > indices( nCells * 3 );

	vtkSmartPointer< vtkDataArray > isoSurfaceNormals = isoSurface->GetPointData()->GetNormals();

	std::cout << " normals size : " << isoSurfaceNormals->GetNumberOfTuples() << " " << isoSurfaceNormals->GetNumberOfComponents() << std::endl;

	for (int pp = 0; pp < nPoints; pp++)
	{
		double pt[3] , n[3];

		isoSurface->GetPoint(pp, pt);

		isoSurfaceNormals->GetTuple(pp, n);

		points[3 * pp] = pt[0];
		points[3 * pp + 1] = pt[1];
		points[3 * pp + 2] = pt[2];

		normals[3 * pp] = n[0];
		normals[3 * pp + 1] = n[1];
		normals[3 * pp + 2] = n[2];
	}

	vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();

	for (int tt = 0; tt < nCells; tt++)
	{
		isoSurface->GetCellPoints(tt, triangle);

		indices[3 * tt] = triangle->GetId(0);
		indices[3 * tt + 1] = triangle->GetId(1);
		indices[3 * tt + 2] = triangle->GetId(2);
	}


	FILE *file = fopen("C:/Data/ZxoData/bitron.tri", "wb");

	fwrite(&nPoints, 1, sizeof(int), file);
	fwrite(&nCells, 1, sizeof(int), file);
	fwrite(points.data(), points.size(), sizeof(float), file);
	fwrite(normals.data(), normals.size(), sizeof(float), file);
	fwrite(indices.data(), indices.size(), sizeof(unsigned int), file);

	fclose(file);

	tr::Display3DRoutines::displayPolyData(isoSurface);

}


void generate(int depth, int srcWidth, int srcHeight, unsigned short* src, int dstDepth, int dstWidth, int dstHeight, unsigned short *dst)
{
	int srcStep = srcWidth * sizeof(Ipp16u);
	int srcPlaneStep = srcWidth * srcHeight * sizeof(Ipp16u);
	int dstStep = srcStep / 2;
	int dstPlaneStep = srcPlaneStep / 4;

	IpprCuboid srcVoi, dstVoi;

	srcVoi.x = 0;
	srcVoi.y = 0;
	srcVoi.z = 0;

	srcVoi.width = srcWidth;
	srcVoi.height = srcHeight;
	srcVoi.depth = depth;

	dstVoi.x = 0;
	dstVoi.y = 0;
	dstVoi.z = 0;

	dstVoi.width = dstWidth;
	dstVoi.height = dstHeight;
	dstVoi.depth = dstDepth;

	IpprVolume volume = { srcWidth, srcHeight, depth };

	float xFactor = 0.5;
	float yFactor = 0.5;
	float zFactor = 0.5;

	int resizeBufferSize;
	Ipp8u *resizeBuffer;

	ipprResizeGetBufSize(srcVoi, dstVoi, 1, IPPI_INTER_LINEAR, &resizeBufferSize);

	std::cout << " resized buffer size : " << resizeBufferSize << std::endl;

	resizeBuffer = new Ipp8u[resizeBufferSize];

	//ipprResize_16u_C1V(src, volume, srcStep, srcPlaneStep, srcVoi, dst, dstStep, dstPlaneStep, dstVoi, xFactor, yFactor, zFactor, 0, 0, 0, IPPI_INTER_LINEAR, resizeBuffer);


}

void viewVolumeSkin(SoVolumeData* pVolumeData, ushort minVal, ushort maxVal)
{

	// If necessary, specify the actual range of the data values.
	//    By default VolumeViz maps the entire range of the voxel data type
	//    (e.g. 0..65535 for unsigned short) into the colormap.  This works
	//    great for byte (8 bit) voxels, but not so well for 16 bit voxels
	//    and not at all for floating point voxels. So it's not actually
	//    necessary for this data set, but shown here for completeness.
	//    NOTE: Min/max values are stored in the header for LDM format
	//    files, but for other formats the getMinMax query can take a
	//    long time because VolumeViz has to examine every voxel.
	SoDataRange *pRange = new SoDataRange();

	int voxelSize = pVolumeData->getDataSize();


	//std::cout << " voxel size : " << voxelSize << " " << pVolumeData->getDataType() << std::endl;

	//if (voxelSize > 1) 
	{
		//double minval, maxval;
		//pVolumeData->getMinMax(minval, maxval);
		pRange->min = minVal;
		pRange->max = maxVal;
	}

	// Use a predefined colorMap with the SoTransferFunction
	SoTransferFunction* pTransFunc = new SoTransferFunction;
	//pTransFunc->predefColorMap = SoTransferFunction::STANDARD;//AIRWAY //NONE; //STANDARD;//TEMPERATURE;//

	//pTransFunc->minValue = minVal;
	//pTransFunc->maxValue = maxVal;

	pTransFunc->minValue.setValue(0);
	pTransFunc->maxValue.setValue(65535);

	std::fstream colorMapWriter;

	std::vector<float> interpolatedMap(65536 * 4);

	colorMapWriter.open("C:/Data/ZxoData/separated_part_7_colormap.dat", std::ios::out | std::ios::binary);

	colorMapWriter.write((char*)interpolatedMap.data(), 65536 * 4 * sizeof(float));

	colorMapWriter.close();

	pTransFunc->colorMap.setValues(0, 65536 * 4, interpolatedMap.data());
	pTransFunc->enableNotify(TRUE);
	pTransFunc->touch();


	//NONE,
	//	/** Grey (Default). This is constant intensity (white) with a 0..1 alpha ramp.
	//	*                  A good initial color map for volume rendering. */
	//	GREY,
	//	/** Gray (Synonym of grey) */
	//	GRAY = GREY,
	//	/** Temperature (opaque). */
	//	TEMPERATURE,
	//	/** Physics (opaque). */
	//	PHYSICS,
	//	/** Standard (opaque). */
	//	STANDARD,
	//	/** Glow (opaque). This is similar to "hot iron" type color maps. */
	//	GLOW,
	//	/** Blue red (opaque). */
	//	BLUE_RED,
	//	/** Seismic */
	//	SEISMIC,
	//	/** Blue white red (opaque). */
	//	BLUE_WHITE_RED,
	//	/** Intensity (opaque). This is an intensity ramp (gray scale) with constant alpha (1).
	//	*                      A good initial color map for slice rendering. */
	//	INTENSITY,
	//	/** 256 labels (opaque). A good color map for rendering label volumes. */
	//	LABEL_256,
	//	/** VolRenRed (opacity ramp). Suitable for volume rendering. **/
	//	VOLREN_RED,
	//	/** VolRenGreen (opacity ramp). Suitable for volume rendering. **/
	//	VOLREN_GREEN,
	//	/** Airway.
	//	* This colormap is especially adapted to display airways of CT Scan datasets.
	//	* Ranges of data may have to be adapted depending on your acquisition device's calibration.
	//	* Please refer to SoDataRange node for details. */
	//	AIRWAY,
	//	/** Airway surfaces.
	//	* This colormap is especially adapted to display airways and soft tissues of CT Scan datasets.
	//	* Ranges of data may have to be adapted depending on your acquisition device's calibration.
	//	* Please refer to SoDataRange node for details. */
	//	AIRWAY_SURFACES

	// Node in charge of drawing the volume
	SoVolumeSkin *pVolRender = new SoVolumeSkin;

	// Node in charge of drawing the volume
	SoVolumeRender* pVolRender2 = new SoVolumeRender;
	pVolRender2->samplingAlignment = SoVolumeRender::DATA_ALIGNED;

	// Assemble the scene graph
	// Note: SoVolumeSkin must appear after the SoVolumeData node.
	SoSeparator *root = new SoSeparator;
	root->ref();
	root->addChild(pVolumeData);
	//root->addChild(pRange);
	root->addChild(pTransFunc);
	root->addChild(pVolRender2);

	Widget myWindow = SoXt::init("");

	// Set up viewer:
	SoXtExaminerViewer *myViewer = new SoXtExaminerViewer(myWindow);
	myViewer->setSceneGraph(root);
	myViewer->setTitle("Volume Skin");
	myViewer->show();


	SoXt::show(myWindow);
	SoXt::mainLoop();
	SoVolumeRendering::finish();
	SoXt::finish();


}

void viewVolume( std::string zxoFilePath , int minVal , int maxVal )
{
	SoDB::init();

	SoVolumeRendering::init();

	SoVolumeRendering::setMaxMainMemory(8192);

	std::cout << " max main memory :  "<< SoVolumeRendering::getMaxTexMemory() << std::endl;

	SoVolumeRendering::setIgnoreFullyTransparentTiles(true);
	
	imt::volume::OptimizedZXOReader::initClass();

	SoVolumeData *volumeData = new SoVolumeData;

	SoLDMDataAccess& ldmAccess = volumeData->getLdmDataAccess(); //;//

	imt::volume::CustomTileManager *customTileManager = new imt::volume::CustomTileManager();

	SoLDMTopoOctree *topoTree = new SoLDMTopoOctree;



	auto prevTileManager = volumeData->getTileManager();

	customTileManager->setTileManager(prevTileManager);

	imt::volume::OptimizedZXOReader *reader2 = new imt::volume::OptimizedZXOReader();

	CustomTiledReader *reader = new CustomTiledReader;

	volumeData->setReader(*reader2, true);

	

	volumeData->fileName.setValue(zxoFilePath);//filePath // //zxoFilePath.toStdString() //"backup.zxo2"

	topoTree->init(volumeData->getDimension(), volumeData->getTileDimension()[0]);

	//ldmAccess.setDataSet( volumeData );//

	//customTileManager->init(volumeData->getDimension(), volumeData->getTileDimension()[0]);

	//customTileManager->setDataAccess(&ldmAccess);
	//customTileManager->setTargetResolution(0);
	//customTileManager->setVariableResidenceMemorySize(4096);
	//customTileManager->setZXOReader(reader);

	//std::cout << " get fixed resolution tiles " << std::endl;
	//customTileManager->loadFixedResidentTiles();

	//volumeData->getLdmManagerAccess().setTileManager(customTileManager);
	std::cout << " max tex memory : "<< volumeData->ldmResourceParameters.getValue()->getMaxTexMemory() << std::endl;

	std::cout << " volume dimensions : " << volumeData->getDimension() << std::endl;

	volumeData->ldmResourceParameters.getValue()->resolution = 0;

	volumeData->ldmResourceParameters.getValue()->fixedResolution = TRUE;

	volumeData->ldmResourceParameters.getValue()->setMaxTexMemory(4096);

	volumeData->ldmResourceParameters.getValue()->subTileDimension.setValue(64, 64, 64);

    //return;

	std::cout << volumeData->getDimension() << " " << volumeData->getTileDimension()  << std::endl;

	std::cout << topoTree->getNumFileIDs() << " " << topoTree->getNumTileIDs() << std::endl;
	

	//ldmAccess.isTileToPrefecth();
	//ldmAccess.setGetDataMode(  )

	

	// If necessary, specify the actual range of the data values.
	// By default VolumeViz maps the entire range of the voxel data type
	// (e.g. 0..65535 for unsigned short) into the colormap.  This works
	// great for byte (8 bit) voxels, but not so well for 16 bit voxels
	// and not at all for floating point voxels. So it's not actually
	// necessary for this data set, but shown here for completeness.
	// NOTE: Min/max values are stored in the header for LDM format
	// files, but for other formats the getMinMax query can take a
	// long time because VolumeViz has to examine every voxel.
	SoDataRange *pRange = new SoDataRange();

	int voxelSize = volumeData->getDataSize();


	//std::cout << " voxel size : " << voxelSize << " " << pVolumeData->getDataType() << std::endl;

	//if (voxelSize > 1) 
	{
		//double minval, maxval;
		//pVolumeData->getMinMax(minval, maxval);
		pRange->min = minVal;
		pRange->max = maxVal;
	}

	// Use a predefined colorMap with the SoTransferFunction
	SoTransferFunction* pTransFunc = new SoTransferFunction;
	//pTransFunc->predefColorMap = SoTransferFunction::NONE;//NONE;//NONE; //STANDARD;//

	//pTransFunc->minValue = minVal;
	//pTransFunc->maxValue = maxVal;

	pTransFunc->minValue.setValue(0);
	pTransFunc->maxValue.setValue(65535);//

	std::fstream colorMapReader;

	std::vector<float> interpolatedMap(65536 * 4);

	colorMapReader.open("C:/Data/ZxoData/Test_Kupplung_2GB_Vx52µm 2015-6-2 10-29.dat", std::ios::in | std::ios::binary); //separated_part_7_colormap.dat

	colorMapReader.read((char*)interpolatedMap.data(), 65536 * 4 * sizeof(float));

	colorMapReader.close();

	//for (int ii = 0; ii < 22800 ; ii++)
	//{
	//	interpolatedMap[4 * ii + 3] = 0;
	//}

	pTransFunc->colorMap.setValues(0, 65536 * 4, interpolatedMap.data());
	pTransFunc->enableNotify(TRUE);
	pTransFunc->touch();


	//NONE,
	//	/** Grey (Default). This is constant intensity (white) with a 0..1 alpha ramp.
	//	*                  A good initial color map for volume rendering. */
	//	GREY,
	//	/** Gray (Synonym of grey) */
	//	GRAY = GREY,
	//	/** Temperature (opaque). */
	//	TEMPERATURE,
	//	/** Physics (opaque). */
	//	PHYSICS,
	//	/** Standard (opaque). */
	//	STANDARD,
	//	/** Glow (opaque). This is similar to "hot iron" type color maps. */
	//	GLOW,
	//	/** Blue red (opaque). */
	//	BLUE_RED,
	//	/** Seismic */
	//	SEISMIC,
	//	/** Blue white red (opaque). */
	//	BLUE_WHITE_RED,
	//	/** Intensity (opaque). This is an intensity ramp (gray scale) with constant alpha (1).
	//	*                      A good initial color map for slice rendering. */
	//	INTENSITY,
	//	/** 256 labels (opaque). A good color map for rendering label volumes. */
	//	LABEL_256,
	//	/** VolRenRed (opacity ramp). Suitable for volume rendering. **/
	//	VOLREN_RED,
	//	/** VolRenGreen (opacity ramp). Suitable for volume rendering. **/
	//	VOLREN_GREEN,
	//	/** Airway.
	//	* This colormap is especially adapted to display airways of CT Scan datasets.
	//	* Ranges of data may have to be adapted depending on your acquisition device's calibration.
	//	* Please refer to SoDataRange node for details. */
	//	AIRWAY,
	//	/** Airway surfaces.
	//	* This colormap is especially adapted to display airways and soft tissues of CT Scan datasets.
	//	* Ranges of data may have to be adapted depending on your acquisition device's calibration.
	//	* Please refer to SoDataRange node for details. */
	//	AIRWAY_SURFACES

	SoMaterial* material = new SoMaterial;

	material->setName("MainMaterial");//NoResourceText
	material->ambientColor.setValue(0.2f, 0.2f, 0.2f);
	material->diffuseColor.set1Value(0, 0.8f, 0.8f, 0.8f);
	material->specularColor.setValue(0.2f, 0.2f, 0.2f);
	material->emissiveColor.setValue(1.0f, 1.0f, 1.0f);
	material->shininess.setValue(0.2f);
	
	// Node in charge of drawing the volume
	SoVolumeSkin *pVolRender = new SoVolumeSkin;

	// Node in charge of drawing the volume
	SoVolumeRender* pVolRender2 = new SoVolumeRender;
	pVolRender2->samplingAlignment = SoVolumeRender::DATA_ALIGNED;//BOUNDARY_ALIGNED;//

	pVolRender2->subdivideTile = TRUE;


	SoDirectionalLight* directionalLight = new SoDirectionalLight;
	SoOrthographicCamera* camera = new SoOrthographicCamera;
	// Assemble the scene graph
	// Note: SoVolumeSkin must appear after the SoVolumeData node.
	//SoSceneManager *root = new SoSceneManager;
	SoSeparator *root = new SoSeparator;
	root->ref();


	SoSwitch *_RenderQualitySwitch = new SoSwitch();
	_RenderQualitySwitch->setName("RenderQualitySwitch");//NoResourceText
	_RenderQualitySwitch->whichChild = 0;
	root->addChild(_RenderQualitySwitch);
	SoVolumeRenderingQuality *_RenderingQuality = new SoVolumeRenderingQuality();
	_RenderingQuality->setName("RenderQuality");//NoResourceText
	//_RenderingQuality->lightingModel = SoVolumeRenderingQuality::OPENGL;
	_RenderingQuality->deferredLighting = TRUE;
	_RenderingQuality->interpolateOnMove = TRUE;
	_RenderQualitySwitch->addChild(_RenderingQuality);

	root->insertChild(directionalLight, 0);
	root->addChild(material);
	root->addChild(volumeData);
	//root->addChild(pRange);
	root->addChild(pTransFunc);
	
	//root->addChild(camera);
	//root->addChild(material);

	SoComplexity* complexity = new SoComplexity();
	complexity->value = 0.1f;
	complexity->textureQuality = 0.1f;
	root->addChild(complexity);

	root->addChild(pVolRender2);

	Widget myWindow = SoXt::init("");


	// Set up viewer:
	SoXtExaminerViewer *myViewer = new SoXtExaminerViewer(myWindow);

	//myViewer->setTransparencyType(  )
	myViewer->setSceneGraph(root);
	myViewer->setTitle("Volume Skin");
	myViewer->show();



	SoXt::show(myWindow);
	SoXt::mainLoop();
	SoVolumeRendering::finish();
	SoXt::finish();

}


void UnlockLicense()
{
	const char* pssw_OpenIV_beg = "License OpenInventor 9.6 1-Jan-0 0 ";
	const char* pssw_OpenIV_end = " \"APP-CALIGO\"";
	const int pssw_OpenIV_main[] =
	{
		'6' + 0, '5' + 1, '6' + 2, '8' + 3, '3' + 4,
		'c' + 5, 'm' + 6, 'u' + 7, 'k' + 8, 'i' + 9,
		'3' + 10, 'a' + 11, 0
	};

	const char* pssw_VolumeVizLDM_beg = "License VolumeVizLDM 9.6 1-Jan-0 0 ";
	const char* pssw_VolumeVizLDM_end = " \"APP-CALIGO\"";
	const int pssw_VolumeVizLDM_main[] =
	{
		'm' + 0, 't' + 1, '8' + 2, 'p' + 3, '5' + 4,
		't' + 5, 'x' + 6, 'c' + 7, 'w' + 8, '5' + 9,
		'h' + 10, 'v' + 11, 0
	};

	char letter[2];
	letter[1] = '\0';
	int i;

	static SbString string_OpenIV;
	string_OpenIV.makeEmpty();
	string_OpenIV += pssw_OpenIV_beg;
	for (i = 0; pssw_OpenIV_main[i] != 0; i++)
	{
		letter[0] = (char)(pssw_OpenIV_main[i] - i);
		string_OpenIV += letter;
	}
	string_OpenIV += pssw_OpenIV_end;

	string_OpenIV += ":";

	string_OpenIV += pssw_VolumeVizLDM_beg;
	for (i = 0; pssw_VolumeVizLDM_main[i] != 0; i++)
	{
		letter[0] = (char)(pssw_VolumeVizLDM_main[i] - i);
		string_OpenIV += letter;
	}
	string_OpenIV += pssw_VolumeVizLDM_end;

	SoLockManager::SetUnlockString((char*)string_OpenIV.getString());
}



void redBlackTreeDemo()
{
	std::vector< std::pair< unsigned int, double > > tileWeights(5);

	tileWeights[0].first = 3;
	tileWeights[0].second = 0.75;

	tileWeights[1].first = 6;
	tileWeights[1].second = 0.45;

	tileWeights[2].first = 2;
	tileWeights[2].second = 0.25;

	tileWeights[3].first = 7;
	tileWeights[3].second = 0.85;

	tileWeights[4].first = 0;
	tileWeights[4].second = 0.35;

	imt::volume::TileWeightHeap twh(73);

	twh.initHeap( tileWeights  );

}

