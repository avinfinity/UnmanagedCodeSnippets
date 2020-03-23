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
#include "optimizedzxogenerator.h"
#include "rawvolumedataio.h"
#include "iostream"
#include "opencvincludes.h"
#include "ippr.h"
#include "ipp.h"

#include "NeoInsightsLDMWriter.h"
#include "optimizedzxoreader.h"

#include "openOR/ZeissBackend.hpp"

#include <openOR/Image/ZeissRawImporter.hpp>// Zeiss_ImageIO
#include <openOR/Image/ZeissRawExporter.hpp>// Zeiss_ImagIO
#include <openOR/Image/ZeissRawExtendedExporter.hpp>// Zeiss_ImagIO

#include <openOR/Image/HistogramCollector.hpp>// Image_Utility
#include <openOR/Image/SubvolumeHistogramCollector.hpp>// Image_Utility
#include <openOR/Image/VolumeCollector.hpp>// Image_Utility
#include <openOR/Image/HistogramAnalyser.hpp>// Image_Utility
#include <openOR/Image/HistogramMerger.hpp>// Image_Utility
#include <openOR/Image/HistogramProbabilityAnalyser.hpp>// Image_Utility
#include <openOR/ScanSurfaceSegmentor.hpp>// ZeissBackend
#include <openOR/MTScanSurfaceSegmentor.hpp>// ZeissBackend
#include <openOR/ProbabilitySegmentation.hpp>// ZeissBackend
#include <openOR/Image/ROIContainer.hpp> // Image_regions

#include <boost/function.hpp>
#include <functional>
#include <algorithm>

#include <openOR/Plugin/create.hpp> //openOR_core
#include <openOR/Image/Image3DData.hpp> //Image_ImageData

#include <openOR/Log/Logger.hpp> //openOR_core
#   define OPENOR_MODULE_NAME "Application.ZeissBackend"
#   include <openOR/Log/ModuleFilter.hpp>

//#include <openOR/cleanUpWindowsMacros.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

void viewVolumeSkin(SoVolumeData* pVolumeData, ushort minVal, ushort maxVal);
void viewVolume(std::string zxoFilePath, int minVal, int maxVal);


int main(int argc, char **argv)
{
	std::string zxoFilePath = "C:/Data/ZxoData/CZXRM_Sandbox.ZXO";

	int minVal = 0 , maxVal = 0;

	SoVolumeRendering::init();

	viewVolume( zxoFilePath , minVal , maxVal );

	return 0;
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

void viewVolume(std::string zxoFilePath, int minVal, int maxVal)
{
	SoVolumeRendering::init();
	imt::volume::OptimizedZXOReader::initClass();

	SoVolumeData *volumeData = new SoVolumeData;

	imt::volume::OptimizedZXOReader *reader = new imt::volume::OptimizedZXOReader();

	volumeData->setReader(*reader, true);

	volumeData->fileName.setValue(zxoFilePath);//filePath // //zxoFilePath.toStdString() //"backup.zxo2"

	std::vector< imt::volume::Material > materials;

	imt::volume::VolumeUtility::computeMaterials( volumeData , materials );

	//volumeData->ldmResourceParameters.getValue()->resolution = 0;

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
	root->addChild(volumeData);
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


#if 0

void createHistogramRegions() {

	std::tr1::shared_ptr<openOR::Image::Image1DData<openOR::Triple<size_t> > > m_pRegionsOrg, m_pRegionsHalf, m_pRegionsThird, m_pRegionsFourth, m_pRegions;
	std::tr1::shared_ptr<openOR::Image::Image1DData<openOR::Quad<double> > > m_pProbabilities;

	std::tr1::shared_ptr<openOR::Image::Image1DDataUI64>         m_pHistogramFirst;
	std::tr1::shared_ptr<openOR::Image::Image1DDataUI64>         m_pHistogramSecond;
	std::tr1::shared_ptr<openOR::Image::Image1DDataUI64>         m_pHistogramThird;
	std::tr1::shared_ptr<openOR::Image::Image1DDataUI64>         m_pHistogramFourth;

	size_t                                               m_maxMemorySize;
	int                                                  m_downSampler;
	size_t                                               m_ROIBorderSize;
	size_t                                               m_ROIMinEdgeLength;
	size_t                                               m_ROIMinVolume;
	bool                                                 m_outputMaterialIndex;
	unsigned int                                         m_nFirstMaterialIndex;
	unsigned int                                         m_nSegmentationRadius;
	int                                                  m_expandMaskTimes;
	bool                                                 m_bGPU;
	float                                                m_fProbabilityThreshold;
	size_t                                               m_nFilterRadius;
	size_t                                               m_nOpenMPCurrentThreads;
	size_t                                               m_nOpenMPMaxThreads;
	float                                                m_fCPUPerformanceValue;
	bool                                                 m_bAutoScaling;
	bool                                                 m_bMTSurfSegment;
	unsigned int                                         m_numberSeparationThreads;
	bool                                                 m_bSearchForHighDensityCarrierMaterial;
	int                                                  m_useMultiScaleHistogram;
	size_t                                               m_objectThreshold;
	size_t                                               m_background;
	size_t                                               m_usedHistogramResolutions;

	// - initialize Region- and Probablility-members if they are uninitialized
	// - depending on the number of histograms to be used create instance of a Histogramanalyser, caluculate the regions and store them to the matching member
	// - merge all histograms 
	// - correct the shifts among the histograms (because the peaks in the histograms appear in different places due to the different resolutions)
	// - create instance of HistogramProbabilityAnalyser and and fit normal distributions on the histogram peaks

	//assert(m_pHistogramFirst && "Need filled histogram to calculate material regions.");
	if (!m_pRegions) 
	{
		m_pRegions = openOR::createInstanceOf< openOR::Image::Image1DData< openOR::Triple<size_t> > >();
	}
	if (!m_pProbabilities) 
	{
		m_pProbabilities = openOR::createInstanceOf<openOR::Image::Image1DData<openOR::Quad<double> > >();
	}
	else
	{

		m_pProbabilities = openOR::createInstanceOf<openOR::Image::Image1DData<openOR::Quad<double> > >();
		LOG(openOR::Log::Level::Info, OPENOR_MODULE, openOR::Log::msg("Probabilities were already set. Resetting them."));

	}

	uint nObjectThresholdFromHistogramResolustionStep = 0;
	bool bSearchforhighdensitycarriermaterial = m_bSearchForHighDensityCarrierMaterial;
	// create second and third histogram
	if (m_useMultiScaleHistogram >= 2)
	{
		assert(m_pHistogramSecond && m_pHistogramThird && "Need reduced histograms to use multi-scale histograms.");

		if (!m_pRegionsOrg) 
		{ 
			m_pRegionsOrg = openOR::createInstanceOf<openOR::Image::Image1DData<openOR::Triple<size_t> > >(); 
		}
		
		if (!m_pRegionsHalf) 
		{
			m_pRegionsHalf = openOR::createInstanceOf<openOR::Image::Image1DData<openOR::Triple<size_t> > >(); 
		}
		
		if (!m_pRegionsThird) 
		{ 
			m_pRegionsThird = openOR::createInstanceOf<openOR::Image::Image1DData<openOR::Triple<size_t> > >(); 
		}
		
		if (!m_pRegionsFourth) 
		{ 
			m_pRegionsFourth = openOR::createInstanceOf<openOR::Image::Image1DData<openOR::Triple<size_t> > >(); 
		}
		
		if (!m_pRegions) 
		{ 
			m_pRegions = openOR::createInstanceOf<openOR::Image::Image1DData<openOR::Triple<size_t> > >(); 
		}

		//initialize first histogram analyser and compute regions
		std::tr1::shared_ptr< openOR::Image::HistogramAnalyser> pCalc = openOR::createInstanceOf<openOR::Image::HistogramAnalyser>();
		pCalc->setData(m_pHistogramFirst, "in");
		pCalc->setData(m_pRegionsOrg, "out");
		pCalc->setSearchforhighdensitycarriermaterial(bSearchforhighdensitycarriermaterial);
		(*pCalc)();

		m_objectThreshold = pCalc->objectThreshold();
		m_background = pCalc->backgroundPeak();

		//initialize second histogram analyser and compute regions on half the resolution
		pCalc = openOR::createInstanceOf<openOR::Image::HistogramAnalyser>();
		pCalc->setData(m_pHistogramSecond, "in");
		pCalc->setData(m_pRegionsHalf, "out");
		pCalc->setSearchforhighdensitycarriermaterial(bSearchforhighdensitycarriermaterial);
		(*pCalc)();
		if (m_pRegionsHalf->size() > 1 && m_objectThreshold >= m_pHistogramFirst->size() - (64 + 1))
		{
			nObjectThresholdFromHistogramResolustionStep = 1;
			m_objectThreshold = pCalc->objectThreshold();
		}

		//initialize third histogram analyser and compute regions on a third of the resolution
		if (m_usedHistogramResolutions >= 3)
		{
			pCalc = openOR::createInstanceOf<openOR::Image::HistogramAnalyser>();
			pCalc->setData(m_pHistogramThird, "in");
			pCalc->setData(m_pRegionsThird, "out");
			pCalc->setSearchforhighdensitycarriermaterial(bSearchforhighdensitycarriermaterial);
			(*pCalc)();
			if (m_pRegionsThird->size() > 1 && m_objectThreshold >= m_pHistogramFirst->size() - (64 + 1)){
				nObjectThresholdFromHistogramResolustionStep = 2;
				m_objectThreshold = pCalc->objectThreshold();
			}

		}

		//initialize fourth histogram analyser and compute regions on a quarter of the resolution
		if (m_usedHistogramResolutions >= 4)
		{
			pCalc = openOR::createInstanceOf<openOR::Image::HistogramAnalyser>();
			pCalc->setData(m_pHistogramFourth, "in");
			pCalc->setData(m_pRegionsFourth, "out");
			pCalc->setSearchforhighdensitycarriermaterial(bSearchforhighdensitycarriermaterial);
			(*pCalc)();
			if (m_pRegionsFourth->size() > 1 && m_objectThreshold >= m_pHistogramFirst->size() - (64 + 1)){
				nObjectThresholdFromHistogramResolustionStep = 3;
				m_objectThreshold = pCalc->objectThreshold();
			}
		}

		//merge all above histograms
		std::tr1::shared_ptr<openOR::Image::HistogramMerger> pMerger = openOR::createInstanceOf<openOR::Image::HistogramMerger>();
		pMerger->setData(m_pRegionsOrg, "in");
		pMerger->setData(m_pRegionsHalf, "in");
		if (m_usedHistogramResolutions >= 3) pMerger->setData(m_pRegionsThird, "in");
		if (m_usedHistogramResolutions >= 4) pMerger->setData(m_pRegionsFourth, "in");
		pMerger->setData(m_pRegions, "out");
		pMerger->setData(m_pHistogramFirst);

		//openOR::setCurrentStep(pMerger);

		//m_pCurrentStep = interface_cast<Progressable>(step);        // can be null, that is fine!
		//m_pCanCancelCurrentStep = interface_cast<Cancelable>(step); // dito
		//if (m_inOneGoFlag) {
		//	assert(m_currentStepNum < m_numSteps && "one step to far!");
		//	++m_currentStepNum;
		//}


		try {
			(*pMerger)();
		}
		catch (...) {
			m_pRegions = m_pRegionsOrg;
		}

		//correct the shift in peaks: (the peaks in the histograms appear in different places due to the different resolutions)
		if (1)//(m_correctRegionShift)
		{

			std::vector<std::tr1::shared_ptr<openOR::Image::Image1DData<openOR::Triple<size_t> > > > vecMultiResolutionDensityIntervals;
			vecMultiResolutionDensityIntervals.push_back(m_pRegionsOrg);
			vecMultiResolutionDensityIntervals.push_back(m_pRegionsHalf);
			if (m_usedHistogramResolutions >= 3) vecMultiResolutionDensityIntervals.push_back(m_pRegionsThird);
			if (m_usedHistogramResolutions >= 4)  vecMultiResolutionDensityIntervals.push_back(m_pRegionsFourth);

			//get the correction for shifting the peaks to match the peaks in the other histograms
			std::vector<int> vecShifts = pMerger->getShiftCorrections();

			//shift the peaks
			correctRegionShifts(vecMultiResolutionDensityIntervals, vecShifts);
			
			if (nObjectThresholdFromHistogramResolustionStep > 0 && nObjectThresholdFromHistogramResolustionStep < vecShifts.size())
			{
				m_objectThreshold = m_objectThreshold - vecShifts.at(nObjectThresholdFromHistogramResolustionStep);
			}

			size_t vecSize = vecMultiResolutionDensityIntervals.size();

			m_pRegionsOrg = vecMultiResolutionDensityIntervals[0];
			if (vecSize > 2) m_pRegionsHalf = vecMultiResolutionDensityIntervals[1];
			if (vecSize > 3) m_pRegionsThird = vecMultiResolutionDensityIntervals[2];
			if (vecSize > 4) m_pRegionsFourth = vecMultiResolutionDensityIntervals[3];
		}
	}
	else if (m_useMultiScaleHistogram == 1)
	{
		// if there is only one Histogram there is no merging to do and the result is the one from the original scale
		std::tr1::shared_ptr<openOR::Image::HistogramAnalyser> pCalc = openOR::createInstanceOf<openOR::Image::HistogramAnalyser>();
		pCalc->setData(m_pHistogramFirst, "in");
		pCalc->setData(m_pRegions, "out");

		setCurrentStep(pCalc);
		(*pCalc)();

		m_objectThreshold = pCalc->objectThreshold();
		m_background = pCalc->backgroundPeak();
	}
	else {

		// object threshold should be set from outside
		m_objectThreshold = 0x0fff;
		m_background = 0;
		//FH: added a note for the user to know that we use empirical values for the object threshold and the background peak

	}

	// set first material
	for (size_t i = 0; i < m_pRegions->size(); i++) {
		const openOR::Triple<size_t>& region = m_pRegions->data()[i];
		if (region.second > m_objectThreshold) {
			setFirstMaterialIndex(static_cast<unsigned int>(i));
			break;
		}
	}


	//calc probabilities and fit nomal distributions on the histogram
	std::tr1::shared_ptr<openOR::Image::HistogramProbabilityAnalyser> pProbAnalyser = openOR::createInstanceOf<openOR::Image::HistogramProbabilityAnalyser>();
	pProbAnalyser->setData(m_pHistogramSecond);
	pProbAnalyser->setData(m_pRegions);
	pProbAnalyser->setData(m_pProbabilities);
	pProbAnalyser->setHighDensityMaterialSearch(true);
	(*pProbAnalyser)();


}
#endif

