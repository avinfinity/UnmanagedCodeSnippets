
#include "iostream"
#include "QString"
#include "volumeinfo.h"
#include "volumeutility.h"
#include "QDebug"

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
//#include <LDM/tiles/>
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

#include "QDomDocument"

#include "volumeconverter.h"
#include "opencvincludes.h"
#include "volumetopooctree.h"

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


void viewVolumeSkin(SoVolumeData* pVolumeData, ushort minVal, ushort maxVal);

void viewIsoSurface(imt::volume::VolumeInfo& volume, int isoVal);

void fragmentShaderDemo();




void computeOpecityMask( std::vector< imt::volume::Material >& materials )
{
	const int GRAY_VALUES = 65536;
	const int COLOR_MAP_ENTRIES = 256;

	float *colorMapWithOpacityMask = new float[4 * COLOR_MAP_ENTRIES];

	float transparencyValue = 0;

	float t = ((1.0f - (float)transparencyValue) / 10.0f);

	float colorMap[1024];
	float opacityMask[1024];

	for (int i = 0; i < 1024 / 4; i++)
	{
		colorMapWithOpacityMask[i * 4 + 0] = colorMap[i * 4 + 0] * opacityMask[i * 4 + 3];// * (0.1f + 0.9f * opacityMask[i * 4 + 3]);
		colorMapWithOpacityMask[i * 4 + 1] = colorMap[i * 4 + 1] * opacityMask[i * 4 + 3];// * (0.1f + 0.9f * opacityMask[i * 4 + 3]);
		colorMapWithOpacityMask[i * 4 + 2] = colorMap[i * 4 + 2] * opacityMask[i * 4 + 3];// * (0.1f + 0.9f * opacityMask[i * 4 + 3]);
		colorMapWithOpacityMask[i * 4 + 3] = opacityMask[i * 4 + 3];
	}

	int _ColorMapThreshold = 88;

	if (transparencyValue > 0.0)
	{
		for ( int i = _ColorMapThreshold; i < 1024 / 4; i++)
		{
			colorMapWithOpacityMask[i * 4 + 3] = t;
		}
	}

	for( auto material : materials )
	{
		float alphaValue = (1.0f - material.mTransparency) / 10.0f;
		float alphaValueTotal = material.mIsVisible ? alphaValue : 0.0f;

		int lowerBounds = (int)((double)material.mLowerBound / (GRAY_VALUES - 1) * (COLOR_MAP_ENTRIES - 1));
		int upperBounds = (int)((double)material.mUpperBound / (GRAY_VALUES - 1) * (COLOR_MAP_ENTRIES - 1));

		float transparencyFactor = material.mIsHighlighted ? 1.0f : 0.5f; // foreground / background //IsHighlighted

		for (int i = lowerBounds; i < upperBounds; i++)// 
		{
			colorMapWithOpacityMask[i * 4 + 0] = transparencyFactor * colorMapWithOpacityMask[i * 4 + 0];
			colorMapWithOpacityMask[i * 4 + 1] = transparencyFactor * colorMapWithOpacityMask[i * 4 + 1];
			colorMapWithOpacityMask[i * 4 + 2] = transparencyFactor * colorMapWithOpacityMask[i * 4 + 2];

			colorMapWithOpacityMask[i * 4 + 3] = transparencyFactor * alphaValueTotal;
		}
	}


}

int main(int argc, char **argv)
{
	QString filePrefix = //"C:/Data/WallthicknessData/11.1.17/Blister_Full 2016-10-20 14-56";
		//"C:/Data/WallthicknessData/11.1.17/Bizerba_2 2016-8-25 17-50-31"; 
		//"C:/Data/WallthicknessData/11.1.17/Blister_Single 2016-10-19 8-55";

		//"C:/projects/Wallthickness/data/Mobile charger/Wall Thick_RnD_1 2016-10-13 15-10_sep1"; // 

		//"C:/projects/Wallthickness/data/separated_part_7";

		//"C:/projects/Wallthickness/data/Mobile charger/Wall Thick_RnD_1 2016-10-13 15-10_sep1"; //"C:/projects/datasets/Bitron/separated_part_7";//
		// "C:/projects/Wallthickness/data/CT multi/Calliper Assy 2016-6-9 10-12"; //
		//"C:/projects/Wallthickness/data/RND/Big sample_BHC 2016-8-23 16-58";
		//"C:/projects/Wallthickness/data/Datasets for Multi-material/Testkolben/trig_foc0.4_314x354x390";//
		//"C:/projects/datasets/MultiMaterial/D116_MAR 2015-12-30 17-57";
		//"C:/projects/datasets/Bitron/separated_part_7";//
		"C:\\Projects\\Wallthickness\\data\\Datasets for Multi-material\\Beretta Result Oberkochen\\Beretta_1 2012-6-26 12-23";
	QString vgiFilePath = filePrefix + ".vgi";
	QString scvFilePath = filePrefix + ".uint16_scv";
	QString zxo2FilePath = filePrefix + ".ZXO2";
	QString zxoFilePath = filePrefix + ".ZXO";

	imt::volume::VolumeInfo volInfo;

	//imt::volume::VolumeUtility::loadVgiVolume(vgiFilePath, volInfo);

	imt::volume::RawVolumeDataIO::readUint16ScvHeader(scvFilePath, volInfo);

	std::cout << volInfo.mWidth << " " << volInfo.mHeight << " " << volInfo.mDepth << std::endl;

	FILE *file = fopen(scvFilePath.toStdString().c_str(), "rb");

	fseek(file, 1024, SEEK_SET);

	volInfo.mVolumeData = (unsigned char*)(new unsigned short[volInfo.mWidth * volInfo.mHeight * volInfo.mDepth] );

	fread(volInfo.mVolumeData, sizeof(unsigned short), volInfo.mWidth * volInfo.mHeight * volInfo.mDepth , file);

	fclose(file);

    //NeoInsightsLDMWriter writer;

	//writer.setVolumeInfo(volInfo.mWidth, volInfo.mHeight, volInfo.mDepth, volInfo.mVoxelStep(0), volInfo.mVoxelStep(1), volInfo.mVoxelStep(2), 1);

	//writer.generate();

	int isoThreshold =  26880; // 14528;

	std::vector< int64_t > histogram;// (USHRT_MAX);
	int minVal, maxVal;

	imt::volume::VolumeUtility::computeISOThreshold(volInfo, isoThreshold , histogram , minVal , maxVal);

	std::cout << " iso threshold : " << isoThreshold << std::endl;

	//tr::Display3DRoutines::displayHistogram(histogram, 0, maxVal);

	//return 0;



	//viewIsoSurface(volInfo,isoThreshold );//14528/';

	//return 0;

	SbVec3i32 tileDim;

	SbVec3i32 volumeDim;



	int dim = 128;// 256;//128;//32;//

	tileDim.setValue(dim, dim, dim);

	//volumeData->set

	std::cout << " volume dim from scv : " << volInfo.mWidth << " " << volInfo.mHeight << " " << volInfo.mDepth << std::endl;

	volumeDim.setValue(volInfo.mWidth, volInfo.mHeight, volInfo.mDepth);

	int bytesPerVoxel = 2;

	std::size_t tileBytes = tileDim[0] * tileDim[1] * tileDim[2] * bytesPerVoxel;

	//std::cout << " number of tiles : " << pReader->getNumVoxels() << std::endl;

	SoLDMTopoOctree *topoTree = new SoLDMTopoOctree;

	int overlap = 0;

	topoTree->init(volumeDim, tileDim[0], overlap);

	std::cout << " number of file ids : " << topoTree->getNumFileIDs() <<" "<<topoTree->getFileID(1012 ) << " " << topoTree->getNumTileIDs()<< std::endl;

	imt::volume::VolumeTopoOctree vto;

	imt::volume::VolumeTopoOctree::Vec3i volSize;

	volSize._X = volInfo.mWidth;
	volSize._Y = volInfo.mHeight;
	volSize._Z = volInfo.mDepth;

	vto.init(volSize, dim, 0);

	std::cout << vto.getFileId(1012) << std::endl;

	std::string filePath = "path1.ldm";

	SoVolumeRendering::init();

	NeoInsightsLDMReader::initClass();

#if 1

	//SoLDMGlobalResourceParameters::setNumIO()

	std::cout << " generate ldm file " << std::endl;

	SoConverterParameters* myConverterParameters = new SoConverterParameters();
	myConverterParameters->setHeaderFileName(SbString(filePath));
	myConverterParameters->setTileDim(dim);

	SoVRGenericFileReader* reader = nullptr;

	SbString inputString = scvFilePath.toStdString();
	myConverterParameters->setInputFileName(inputString);

	reader = new SoVRGenericFileReader();
    reader->setFilename(inputString);
	reader->setDataChar(SbBox3f(0, 0, 0, volInfo.mWidth *(float)volInfo.mVoxelStep(0), 
		                volInfo.mHeight*(float)volInfo.mVoxelStep(1), volInfo.mDepth*(float)volInfo.mVoxelStep(2)), 
						SoDataSet::UNSIGNED_SHORT, SbVec3i32(volInfo.mWidth, volInfo.mHeight, volInfo.mDepth), 1024);

	std::cout << " data read completed " << std::endl;


	double initT = cv::getTickCount();

	Zeiss::IMT::NG::Metrotom::VSGWrapper::VolumeConverter* ldmConverter = new Zeiss::IMT::NG::Metrotom::VSGWrapper::VolumeConverter( reader);
	
	int ret = ldmConverter->convert(myConverterParameters);
	reader->closeAllHandles();

	std::cout << " time spent in ldm conversion : " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;

	delete reader;
	delete myConverterParameters;

#endif

	return 0;

	//now load the ldm
	SoVRLdmFileReader *ldmReader = new SoVRLdmFileReader();

	if (!ldmReader->setFilename(filePath))
	{
		std::cout << " successfully set the ldm file  " << std::endl;
	}

	//SbBox3f ldmSize;

	//SoDataSet::DataType dataType;

	//SbVec3i32 dim;

	//ldmReader->getDataChar(ldmSize, dataType, dim);

	//std::cout<<" ldm size  : "<< ldmSize << std::endl;



	

	//SoVRLdmFileReader* pReader = dynamic_cast< SoVRLdmFileReader* >( volumeData->getReader() );

	

	//neoInsightsReader->setLDMReader(ldmReader);


	NeoInsightsLDMWriter *writer = new NeoInsightsLDMWriter();

#if 0
	writer->setRawFilePath(scvFilePath.toStdString());

	writer->setVolumeInfo( volInfo.mWidth , volInfo.mHeight , volInfo.mDepth , 
		                   volInfo.mVoxelStep(0) , volInfo.mVoxelStep(1), 
						   volInfo.mVoxelStep(2) , 2 );


	writer->setAdditionalData(histogram, 1000, 60000);
	writer->setLDMFilePath(zxoFilePath.toStdString());

	writer->setTileDim(dim);

	writer->generate();

	int nLevels = writer->getNumLevels();

	////check lower resolution volume for its validity
	imt::volume::VolumeInfo lowResVol;

	writer->getVolumeAtLevel(nLevels - 1, lowResVol);
#endif

	//writer->readFromFile();

	//return 0;

	//viewIsoSurface(lowResVol, isoThreshold); //20000

	//return 0;

	NeoInsightsLDMReader *neoInsightsReader = new NeoInsightsLDMReader();

	neoInsightsReader->setLDMReader(ldmReader);

	CustomTiledReader *reader2 = new CustomTiledReader();

	SoVolumeData *volumeData = new SoVolumeData;

	//volumeData->ref();

	//volumeData->fileName.setValue(filePath);

	//fragmentShaderDemo();


#define LDM_RAM 2048
#define LDM_GPU 2048

	int MaxMainMem = SoPreferences::getInt("LDM_MAINMEM", LDM_RAM);
	int MaxTexMem = SoPreferences::getInt("LDM_TEXMEM", LDM_GPU);

	std::cout << " max main memory " << MaxMainMem << std::endl;
	

	//size of 3D tiles cache memory in RAM (in MB)
	//SoLDMGlobalResourceParameters::setMaxMainMemory(MaxMainMem);
	//size of 3D textures cache memory in GPU (in MB)
	//SoLDMGlobalResourceParameters::setMaxTexMemory(MaxTexMem);

	//volumeData->setReader( *neoInsightsReader );

	volumeData->setReader(*reader2);

	std::cout << " zxo file path : " << zxoFilePath.toStdString() << std::endl;

	volumeData->fileName.setValue(zxoFilePath.toStdString());//filePath // //zxoFilePath.toStdString() //"backup.zxo2"

	volumeData->ldmResourceParameters.getValue()->resolution = 0;

	std::cout << " max tex memory : " << volumeData->ldmResourceParameters.getValue()->getMaxTexMemory() << std::endl;

	//imt::volume::VolumeTopoOctree *ldmTopoTree = neoInsightsReader->topoOctree();

	//std::cout << " num nodes in topotree : " << ldmTopoTree->getNumTiles() << " " << ldmTopoTree->getNumFiles() << std::endl;

	auto& accessManager = volumeData->getLdmManagerAccess();


	imt::volume::CustomTileVisitor *tv = new imt::volume::CustomTileVisitor();

	//tv->setTopoOctree(ldmTopoTree);

	//tv->setMinMaxThreshold(0 , 60000); //isoThreshold

	//accessManager.setTileVisitor(tv);

	//return 0;
	//volumeData->ldmResourceParameters.getValue()->fixedResolution = true;


	std::cout << " max tiles in main memory : "<< volumeData->ldmResourceParameters.getValue()->getMaxTilesInTexMem() << std::endl;
	//int64_t* histogramPointer;
	//int length;


	
	//volumeData->getHistogram(length, histogramPointer);

	//std::cout << " histogram size : " << length << std::endl;

	//getting data is faster in ALWAYS mode
	//volumeData->ldmResourceParameters.getValue()->loadPolicy.setValue(SoVolumeData::LDMResourceParameter::ALWAYS);

	//set 2D textures properties
	//this will force display to be updated every 10 time a 2Dtexture is loaded in GPU (only for debug purpose, in final application better to set to 50 at least)
	//volumeData->ldmResourceParameters.getValue()->tex2LoadRate.setValue(10);

	//size of 2D texture cache memory in GPU (in number of textures)
	//volumeData->ldmResourceParameters.getValue()->max2DTextures.setValue(10000);

	//return 0;

	//volumeData->setMaxTexMemory(4096);

	std::cout << " max memory for volume data : " << volumeData->getMaxMainMemory() << std::endl;
	std::cout << " max gpu memory for volume data : " << volumeData->getMaxTexMemory() << std::endl;

	viewVolumeSkin(volumeData, isoThreshold, 60000);

	std::cout << topoTree->getNumTileIDs() << std::endl;

	std::cout << " max level of tree : " << topoTree->getLevelMax() << std::endl;

	std::cout << " volume dimension " << volumeDim << std::endl;

	int size = (volumeDim[0] / tileDim[0] + 1) * (volumeDim[1] / tileDim[1] + 1) * (volumeDim[2] / tileDim[2] + 1);

	int nTiles = topoTree->getNumFileIDs();

	for (int tt = 0; tt < nTiles; tt++) //nTiles
	{
		SoLDMTileID tileId = topoTree->getTileID(tt);

		auto tilePos = topoTree->getTilePos(tileId);
	}
	
	SoVolumeRendering::finish();

	return 0;
}


void fragmentShaderDemo()
{

	// Property node which allows SoVolumeRender to draw High Quality volumes.  
	SoVolumeRenderingQuality *pVolShader = new SoVolumeRenderingQuality;
	pVolShader->lighting = TRUE;
	pVolShader->preIntegrated = TRUE;
	pVolShader->edgeColoring = TRUE;
	pVolShader->jittering = TRUE;
	pVolShader->gradientQuality.setValue(SoVolumeRenderingQuality::MEDIUM);


	SoFragmentShader* fragmentShader = new SoFragmentShader;
	fragmentShader->sourceProgram.setValue("C:/projects/OpenInventor_Temp/src/VolumeViz/contrib/2DTransferFunction/src/VolumeViz/contrib/2DTransferFunction/customShader.glsl");

	// Set the shader parameters.
	fragmentShader->addShaderParameter1i("tex2D", SoPreferences::getInt("IVVR_TF2D_TEX_UNIT", 15)); // for now force to 15
	fragmentShader->addShaderParameter1i("data1", 1);

	SbString source;



	fragmentShader->invalidateSource();

	fragmentShader->buildSourceToLoad(source);

	//std::cout << " **************************************fragment shader source : " << fragmentShader->getSourceProgram() << std::endl;

	pVolShader->shaderObject.set1Value(SoVolumeShader::FRAGMENT_COMPUTE_COLOR, fragmentShader);
	pVolShader->lighting = TRUE;

	auto sh = pVolShader->getFragmentShader(0);

	std::cout << sh->getSourceProgram() << std::endl;

	//pVolShader->get

}


QString CreateUserDataXml()
{


	QDomDocument volumeStartPointXmlDoc;

	QDomElement volumeStartPointNode = volumeStartPointXmlDoc.createElement("VolumeStartPosition");
//	volumeStartPointNode.appendChild(volumeStartPointXmlDoc.createTextNode(InputDescription.VolumeStartPoint.ToString()));
	//volumeStartPointXmlDoc.AppendChild(volumeStartPointNode);

#if 0
	XmlNode volumeStartPointNode = volumeStartPointXmlDoc.CreateElement("VolumeStartPosition");//NoResourceText
	volumeStartPointNode.AppendChild(volumeStartPointXmlDoc.CreateTextNode(InputDescription.VolumeStartPoint.ToString()));
	volumeStartPointXmlDoc.AppendChild(volumeStartPointNode);

	XmlDocument volumeDefiningAngleXmlDoc = new XmlDocument();

	XmlNode volumeDefiningAngleNode = volumeDefiningAngleXmlDoc.CreateElement("VolumeDefiningAngle");//NoResourceText
	volumeDefiningAngleNode.AppendChild(volumeDefiningAngleXmlDoc.CreateTextNode(InputDescription.VolumeDefiningAngle.ToString(System.Globalization.CultureInfo.InvariantCulture)));
	volumeDefiningAngleXmlDoc.AppendChild(volumeDefiningAngleNode);

	// Original file name saved as base64 encoded string to prevent characters outside of ASCII table mess up during saving
	XmlDocument rawFileByteXmlDoc = new XmlDocument();
	XmlNode rawFileByteNode = rawFileByteXmlDoc.CreateElement("OriginalFileBase64");
	rawFileByteNode.AppendChild(rawFileByteXmlDoc.CreateTextNode(System.Convert.ToBase64String(Encoding.Unicode.GetBytes(InputDescription.RAWFile))));
	rawFileByteXmlDoc.AppendChild(rawFileByteNode);


	InputDescription.PublishedVersion = AESEncryptionService.EncryptString(ConfigurationManager.AppSettings["PublishedVersion"], AESEncryptionService.Password);
	XmlDocument PublishedVersionXmlDoc = new XmlDocument();
	XmlNode PublishedVersionNode = PublishedVersionXmlDoc.CreateElement("PublishedVersion");
	PublishedVersionNode.AppendChild(PublishedVersionXmlDoc.CreateTextNode(InputDescription.PublishedVersion.ToString(System.Globalization.CultureInfo.InvariantCulture)));
	PublishedVersionXmlDoc.AppendChild(PublishedVersionNode);

	//Signs LDM Files with LDMSecureKey and the text which get encrypted is 
	// combination of InputDescription.Name, InputDescription.VolumeDefiningAngle, InputDescription.VolumeDefiningAngle
	//So Whatevere lDM Comes for Rndering should have File Hashed in this format
	InputDescription.FileIdentifier = AESEncryptionService.EncryptString(string.Format("{0}{1}", Path.GetFileNameWithoutExtension(InputDescription.Name), ConfigurationManager.AppSettings["PublishedVersion"]), AESEncryptionService.Password);
	XmlDocument volumeFileByteXmlDoc = new XmlDocument();
	XmlNode volumeFileByteNode = volumeFileByteXmlDoc.CreateElement("FileIdentifier");
	volumeFileByteNode.AppendChild(volumeFileByteXmlDoc.CreateTextNode(InputDescription.FileIdentifier.ToString(System.Globalization.CultureInfo.InvariantCulture)));
	volumeFileByteXmlDoc.AppendChild(volumeFileByteNode);

#endif

	return "";

	//return volumeStartPointXmlDoc.InnerXml + "\n  " + volumeDefiningAngleXmlDoc.InnerXml + "\n  " + rawFileByteXmlDoc.InnerXml + "\n" + volumeFileByteXmlDoc.InnerXml + "\n" + PublishedVersionXmlDoc.InnerXml;
}


void viewVolumeSkin( SoVolumeData* pVolumeData , ushort minVal , ushort maxVal )
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
	pTransFunc->predefColorMap = SoTransferFunction::STANDARD;//AIRWAY //NONE; //STANDARD;//TEMPERATURE;//

	pTransFunc->minValue = minVal;
	pTransFunc->maxValue = maxVal;
	

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

	tr::Display3DRoutines::displayPolyData(isoSurface);

}




