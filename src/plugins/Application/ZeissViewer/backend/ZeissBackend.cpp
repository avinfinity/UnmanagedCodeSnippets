//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR commercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
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

#include <openOR/cleanUpWindowsMacros.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>


// ***** Empirical values *****
#define EMPIRICAL_FACTOR 1.35
#define EMPIRICAL_HIGHRESOLUTION 768.0 // needed to estimate the scaling factor for multi histogram calculation 
#define EMPIRICAL_ULTRAHIGHRESOLUTION 1536.0 // needed to estimate the scaling factor for multi histogram calculation 

#ifdef _OPENMP
#include <omp.h>
#endif

namespace openOR {

	//---------------------------------------------------------------------------------------------------------------------------
	// ZeissBackend impl
	ZeissBackend::ZeissBackend() : 
m_inputFN("input.vgi"),
	m_outputBN("separated.vgi"),
	m_isRawInput(false),
	m_pCurrentStep(),
	m_currentStepNum(0),
	m_numSteps(4),
	m_inOneGoFlag(false),
	m_pCanCancelCurrentStep(),
	m_canceled(false),
	m_maxMemorySize(std::numeric_limits<size_t>::max()),
	m_downSampler(0),
	m_ROIBorderSize(5),
	m_ROIMinEdgeLength(5),
	m_ROIMinVolume(150),
	m_outputMaterialIndex(false),
	m_nFirstMaterialIndex(0),
	m_nSegmentationRadius(2),
	m_expandMaskTimes(0),
	m_useMultiScaleHistogram(1),
	m_timeFilterRadius(0),
	m_timeNormalDistribution(0),
	m_timePostprocessing(0),
	m_bGPU(false),
	m_fCPUPerformanceValue(0.75),
	m_bAutoScaling(true),
	m_usedHistogramResolutions(0),
	m_nOpenMPMaxThreads(1),
	m_correctRegionShift(true),
	m_bMTSurfSegment(true),
	m_bSearchForHighDensityCarrierMaterial(true),
	m_numberSeparationThreads(1)
{
}

ZeissBackend::~ZeissBackend() {}

//---------------------------------------------------------------------------------------------------------------------------
// Parameter and Data setup
void ZeissBackend::setInputFilename(const std::string& filename) { m_isRawInput = false; m_inputFN = filename; }
void ZeissBackend::setOutputFilename(const std::string& basename) { m_outputBN = basename; } 

void ZeissBackend::setData(const AnyPtr& data) {
	//depending on the type of data set the corresponding members:
	//if data is Volume reset volumeData and outputMap and set new data
	if (VolumeType pData = interface_cast<Image::Image3DDataUI16>(data)) {
		if (!m_pVolumeData) { m_pVolumeData = pData; }
		else if (!m_pOutputMap) { m_pOutputMap = pData; }
		else {
			m_pVolumeData.reset();
			m_pOutputMap.reset();
			setData(pData);
		}
	}
	//if data is 1DData set the member histogram that is not set yet
	if (std::tr1::shared_ptr<Image::Image1DDataUI64> pData = interface_cast<Image::Image1DDataUI64>(data)) {
		if (!m_pHistogramFirst) m_pHistogramFirst = pData;
		else if (!m_pHistogramSecond) m_pHistogramSecond = pData;
		else if (!m_pHistogramThird) m_pHistogramThird = pData;
		else if (!m_pHistogramFourth) m_pHistogramFourth = pData;
		else {
			m_pHistogramFirst.reset();
			m_pHistogramSecond.reset();
			m_pHistogramThird.reset();
			m_pHistogramFourth.reset();
			setData(pData);
		}
	}
	//if data is 1DData triple set member pRegions that is not set yet
	if (std::tr1::shared_ptr<Image::Image1DData<Triple<size_t> > > pData = interface_cast<Image::Image1DData<Triple<size_t> > >(data)) {
		if (!m_pRegions) { m_pRegions = pData; }
		else if (!m_pRegionsOrg) { m_pRegionsOrg = pData; }
		else if (!m_pRegionsHalf) { m_pRegionsHalf = pData; }
		else if (!m_pRegionsThird) { m_pRegionsThird = pData; }
		else if (!m_pRegionsFourth) { m_pRegionsFourth = pData; }
		else {
			m_pRegions.reset();
			m_pRegionsOrg.reset();
			m_pRegionsHalf.reset();
			m_pRegionsThird.reset();
			m_pRegionsFourth.reset();
			setData(pData);
		}
	}
	//if data is ROiCOntainer set member m_pVROIs
	if (std::tr1::shared_ptr<ROIContainer> pData = interface_cast<ROIContainer>(data)) {
		m_pVROIs = pData;
	}
	//if data is 1DData quad set member probabilities
	if (std::tr1::shared_ptr<Image::Image1DData<Quad<double> > > pData = interface_cast<Image::Image1DData<Quad<double> > >(data)) {
		m_pProbabilities = pData;
	}
	return;
}

void ZeissBackend::setVolume(VolumeType pVolume) {
	m_pVolumeData = pVolume;
}

void ZeissBackend::setOutputVolume(VolumeType pVolume) {
	m_pOutputMap = pVolume;
}

void ZeissBackend::setMaxMemorySize(const size_t& maxMemorySize) {
	m_maxMemorySize = maxMemorySize;
}

void ZeissBackend::setDownSampler(const int& downSampler) {
	m_downSampler = downSampler;
}

void ZeissBackend::setBorderSize(size_t borderSize) {
	m_ROIBorderSize = borderSize;
}

void ZeissBackend::setMinRegionSize(size_t minEdgeLength, size_t minRegionVolume) {
	m_ROIMinEdgeLength = minEdgeLength;
	m_ROIMinVolume = minRegionVolume;
}

void ZeissBackend::setUseMultiScaleHistogram(int newUseMultiScaleHistogram) {
	m_useMultiScaleHistogram = newUseMultiScaleHistogram;
}

void ZeissBackend::setCorrectRegionShift(bool newCorrectRegionShift) {
	m_correctRegionShift = newCorrectRegionShift;
}

void ZeissBackend::setOutputMaterialIndex(bool materialIndex){
	m_outputMaterialIndex = materialIndex;
}

unsigned int ZeissBackend::firstMaterialIndex(){
	return m_nFirstMaterialIndex;
}
void ZeissBackend::setFirstMaterialIndex(unsigned int nFirstMaterialIndex){
	m_nFirstMaterialIndex = nFirstMaterialIndex;
}

unsigned int ZeissBackend::segmentationRadius(){
	return m_nSegmentationRadius;
}
void ZeissBackend::setSegmentationRadius(unsigned int nSegmentationRadius){
	m_nSegmentationRadius = nSegmentationRadius;
}

size_t ZeissBackend::objectThreshold() const {
	return m_objectThreshold;
}

void ZeissBackend::overrideObjectThreshold(size_t t) {
	m_objectThreshold = t;
}

size_t ZeissBackend::background() {
	return m_background;
}

Math::Vector3ui ZeissBackend::getScalingFactor() {
	return m_scalingFactor;
}

std::tr1::shared_ptr<Image::Image3DDataUI16> ZeissBackend::objectMap() const {
	return m_pOutputMap;
}

std::tr1::shared_ptr<Image::Image3DDataUI16> ZeissBackend::materialMap() const {
	return m_pOutputMap;
}

std::tr1::shared_ptr<Image::Image3DDataUI16> ZeissBackend::volume() const {
	return m_pVolumeData;
}

// Workflow operations
void ZeissBackend::load() {
	loadFile(m_inputFN);
}

typedef std::tr1::shared_ptr<Image::DataCollectorMultiplexerUI16> DataCollectMUXUI16Ptr;
typedef std::tr1::shared_ptr<Image::Image1DDataUI64> HistogramPtr;

namespace {
	std::tr1::shared_ptr<Image::HistogramCollectorUI16> createHistogramCollector(HistogramPtr& pHist) {
		if (!pHist) {
			pHist = createInstanceOf<Image::Image1DDataUI64>();
		}
		pHist->setSize(static_cast<uint32>(std::numeric_limits<uint16>::max()) + 1);

		std::tr1::shared_ptr<Image::HistogramCollectorUI16> pHColl =  createInstanceOf<Image::HistogramCollectorUI16>();
		pHColl->setData(pHist, "out");

		return pHColl;
	}
}

std::tr1::shared_ptr<Image::DataCollector<uint16> > ZeissBackend::createVolumeCollector(Math::Vector3ui& volumeSize) {
	// - create Volume (pVColl) of the given size
	// - depending on the size set a resolution (bHighResVolume or bUltraHighResVolume) and a maximum scaling factor
	// - depending on the number of histograms to be used (m_useMultiScaleHistogram), create the same number of additional volumes and histograms
	// - feed all volumes and histograms as well as the output volume (m_pVolumeData) to a multiplexer
	// - return the multiplexer

	//create volume of the given size
	std::tr1::shared_ptr<Image::Image3DSizeDescriptor> pVolSize = createInstanceOf<Image::Image3DSizeDescriptor>();
	pVolSize->setSize(volumeSize);
	std::tr1::shared_ptr<Image::VolCollUI16> pVColl = createInstanceOf<Image::VolCollUI16>();
	pVColl->setMaxMemorySize(m_maxMemorySize);
	pVColl->setNumThreads(m_numberSeparationThreads);
	pVColl->setData(pVolSize, "inSize");

	//get scaling of the volume
	m_isScaling = interface_cast<Image::VolCollUI16>(pVColl)->isScaling();
	m_scalingFactor = interface_cast<Image::VolCollUI16>(pVColl)->getScaling();

	Math::Vector3ui scaling = pVColl->getScaling();

	double scalingFactor = ((double) scaling(0)) * scaling(1) * scaling(2);

	m_usedHistogramResolutions =  0;


	//decide if volume is HighResolution or UltraHighResolution
	bool bHighResVolume = false;
	bool bUltraHighResVolume = false;

	//TODO: set and describe empirical value 
	if (EMPIRICAL_HIGHRESOLUTION * EMPIRICAL_HIGHRESOLUTION *EMPIRICAL_HIGHRESOLUTION < ((double) volumeSize(0)) * volumeSize(1) * volumeSize(2) / scalingFactor)    bHighResVolume = true;
	if (EMPIRICAL_ULTRAHIGHRESOLUTION * EMPIRICAL_ULTRAHIGHRESOLUTION * EMPIRICAL_ULTRAHIGHRESOLUTION < ((double) volumeSize(0)) * volumeSize(1) * volumeSize(2) / scalingFactor) bUltraHighResVolume = true;

	//depending on the resolution of the volume set maximum scaling factor
	size_t maxScalingfactor = 4;

	if (m_bAutoScaling) {
		if (bHighResVolume && bUltraHighResVolume) {
			maxScalingfactor = 8;
#if DEBUGINFOS == 1
			std::cout << "UltraHighResVolume: 1,";
#endif
		} else if(bHighResVolume && !bUltraHighResVolume) {
			maxScalingfactor = 6;
#if DEBUGINFOS == 1
			std::cout << "HighResVolume: 1,";
#endif
		}
	} else {
#if DEBUGINFOS == 1
		std::cout << "LowResVolume: 1,";
#endif
	}

	//depending on whether autoscaling is set and the number of Multiscale Histograms create the same number of Histograms.
	//feed them to the multiplexer
	std::tr1::shared_ptr<Image::DataCollectorMultiplexerUI16> pMultiplexer = createInstanceOf<Image::DataCollectorMultiplexerUI16>();

	//use one histogram
	if (m_useMultiScaleHistogram >= 1 || m_bAutoScaling) {
		std::tr1::shared_ptr<Image::HistogramCollectorUI16> pHCollOrg = createHistogramCollector(m_pHistogramFirst);
		pMultiplexer->addCollector(pHCollOrg);
		m_usedHistogramResolutions++;
	}

	std::tr1::shared_ptr<Image::SimpleVolumeCollectorUI16> pVollOut = createInstanceOf<Image::SimpleVolumeCollectorUI16>();
	pVollOut->setData(m_pVolumeData, "out");
	pMultiplexer->addCollector(pVollOut);

	//use two histograms
	if (m_useMultiScaleHistogram >= 2 || m_bAutoScaling) {
		std::tr1::shared_ptr<Image::HistogramCollectorUI16> pHCollSecond = createHistogramCollector(m_pHistogramSecond);
		std::tr1::shared_ptr<Image::VolCollUI16> pVColSecond = createInstanceOf<Image::VolCollUI16>();
#if DEBUGINFOS == 1
		std::cout << " 2,";
#endif
		pVColSecond->setScalingFactor(2);
		pVColSecond->setData(pHCollSecond, "out");
		pVColSecond->setNumThreads(m_numberSeparationThreads);
		pMultiplexer->addCollector(pVColSecond);
		m_usedHistogramResolutions++;
	}

	//use three histograms
	if (m_useMultiScaleHistogram == 3 && !m_bAutoScaling) {
		std::tr1::shared_ptr<Image::HistogramCollectorUI16> pHCollThird = createHistogramCollector(m_pHistogramThird);
		std::tr1::shared_ptr<Image::VolCollUI16> pVColThird = createInstanceOf<Image::VolCollUI16>();
		pVColThird->setScalingFactor(3); // see above
		pVColThird->setData(pHCollThird, "out");
		pVColThird->setNumThreads(m_numberSeparationThreads);
		pMultiplexer->addCollector(pVColThird);
		m_usedHistogramResolutions++;
#if DEBUGINFOS == 1
		std::cout << " 3,";
#endif
	}

	//still use three histograms
	if (m_useMultiScaleHistogram == 4 || m_bAutoScaling) {
		std::tr1::shared_ptr<Image::HistogramCollectorUI16> pHCollThird = createHistogramCollector(m_pHistogramThird);
		std::tr1::shared_ptr<Image::VolCollUI16> pVColThird = createInstanceOf<Image::VolCollUI16>();
#if DEBUGINFOS == 1
		std::cout << " 4,";
#endif
		pVColThird->setScalingFactor(4); // see above
		pVColThird->setData(pHCollThird, "out");
		pVColThird->setNumThreads(m_numberSeparationThreads);
		pMultiplexer->addCollector(pVColThird);
		m_usedHistogramResolutions++;
	}

	//if volume is high or ultra high resolution use four histograms
	if (m_bAutoScaling && (bHighResVolume || bUltraHighResVolume) ) {
		std::tr1::shared_ptr<Image::HistogramCollectorUI16> pHCollFourth = createHistogramCollector(m_pHistogramFourth);
		std::tr1::shared_ptr<Image::VolCollUI16> pVColFourth = createInstanceOf<Image::VolCollUI16>();
#if DEBUGINFOS == 1
		std::cout << " " << maxScalingfactor << std::endl;
#endif
		pVColFourth->setScalingFactor(maxScalingfactor);
		pVColFourth->setData(pHCollFourth, "out");
		pVColFourth->setNumThreads(m_numberSeparationThreads);
		pMultiplexer->addCollector(pVColFourth);
		m_usedHistogramResolutions++;
	}

		pVColl->setData(pMultiplexer, "out");
		return pVColl;
}

void ZeissBackend::loadFile(const std::string& filename) {
	// - load a file and import volume and meta information
	// - write them to volume collector
	if (!m_pVolumeData) {
		m_pVolumeData = createInstanceOf<Image::Image3DDataUI16>();
	} 
	// create importer and set volume filename
	std::tr1::shared_ptr<Image::ZeissRawImporter<uint16> > pLoader = createInstanceOf<Image::ZeissRawImporter<uint16> >();
	pLoader->setFilename(filename);
	pLoader->setIsRawData(m_isRawInput, m_inputSkipHeader);

	if (m_isRawInput) {
		pLoader->setData(m_pInputSizeDescriptor, "inputSize");
	}

	// create meta information class (now public) and get meta information from importer
	Image::ZeissRawImporter<uint16>::MetaInformation meta;
	pLoader->getMetaInfo(meta);

	// get volume size as vector
	Math::Vector3ui size = meta.size;

	// create volume collector and set volume size as argument
	std::tr1::shared_ptr<Image::DataCollector<uint16> > pVColl = createVolumeCollector(size);

	// set volume collector
	pLoader->setData(pVColl, std::string());
	setCurrentStep(pLoader);
	pLoader->operator()();

#if DEBUGINFOS == 1
	std::cout << "IN: " << m_pVolumeData->size()(0) << " " << m_pVolumeData->size()(1) << " " << m_pVolumeData->size()(2) << std::endl;
#endif
	return;
}

void ZeissBackend::setInputDataDesciptor(const std::string& filename, const std::tr1::shared_ptr<Image::Image3DSizeDescriptor>& pSize, size_t skipHeader) {
	m_isRawInput = true;
	m_inputFN = filename;
	m_pInputSizeDescriptor = pSize;
	m_inputSkipHeader = skipHeader;
}

void ZeissBackend::createHistogramRegions() {
	// - initialize Region- and Probability-members if they are uninitialized
	// - depending on the number of histograms to be used create instance of a Histogramanalyser, calculate the regions and store them to the matching member
	// - merge all histograms 
	// - correct the shifts among the histograms (because the peaks in the histograms appear in different places due to the different resolutions)
	// - create instance of HistogramProbabilityAnalyser and and fit normal distributions on the histogram peaks

   // Todo: error handling
	assert(m_pHistogramFirst && "Need filled histogram to calculate material regions.");
	if (!m_pRegions) { 
		m_pRegions = createInstanceOf<Image::Image1DData<Triple<size_t> > >(); 
	}
	if (!m_pProbabilities) { 
      // Todo: error handling
		m_pProbabilities = createInstanceOf<Image::Image1DData<Quad<double> > >(); 
	}
	else
	{
		m_pProbabilities = createInstanceOf<Image::Image1DData<Quad<double> > >(); 
	}

	uint nObjectThresholdFromHistogramResolustionStep = 0;
	bool bSearchforhighdensitycarriermaterial = m_bSearchForHighDensityCarrierMaterial;

	// create second and third histogram
	if (m_useMultiScaleHistogram >= 2) 
	{
		std::cout << " creating multiresolution histogram " << std::endl;

		assert(m_pHistogramSecond && m_pHistogramThird && "Need reduced histograms to use multi-scale histograms.");
		if (!m_pRegionsOrg) { m_pRegionsOrg = createInstanceOf<Image::Image1DData<Triple<size_t> > >(); }
		if (!m_pRegionsHalf) { m_pRegionsHalf = createInstanceOf<Image::Image1DData<Triple<size_t> > >(); }
		if (!m_pRegionsThird) { m_pRegionsThird = createInstanceOf<Image::Image1DData<Triple<size_t> > >(); }
		if (!m_pRegionsFourth) { m_pRegionsFourth = createInstanceOf<Image::Image1DData<Triple<size_t> > >(); }
		if (!m_pRegions) { m_pRegions = createInstanceOf<Image::Image1DData<Triple<size_t> > >(); }

		//initialize first histogram analyser and compute regions
		std::tr1::shared_ptr<Image::HistogramAnalyser> pCalc = createInstanceOf<Image::HistogramAnalyser>();
		pCalc->setData(m_pHistogramFirst, "in");
		pCalc->setData(m_pRegionsOrg, "out");
		pCalc->setSearchforhighdensitycarriermaterial(bSearchforhighdensitycarriermaterial);
		(*pCalc)();

		m_objectThreshold = pCalc->objectThreshold();
		m_background = pCalc->backgroundPeak();

		//initialize second histogram analyzer and compute regions on half the resolution
		pCalc = createInstanceOf<Image::HistogramAnalyser>();
		pCalc->setData(m_pHistogramSecond, "in");
		pCalc->setData(m_pRegionsHalf, "out");
		pCalc->setSearchforhighdensitycarriermaterial(bSearchforhighdensitycarriermaterial);
		(*pCalc)();
		if (m_pRegionsHalf->size() > 1 && m_objectThreshold >= m_pHistogramFirst->size() - (64 + 1) )
		{
			nObjectThresholdFromHistogramResolustionStep = 1;
			m_objectThreshold = pCalc->objectThreshold();
		}

		//initialize third histogram analyzer and compute regions on a third of the resolution
		if (m_usedHistogramResolutions >= 3) 
		{
			pCalc = createInstanceOf<Image::HistogramAnalyser>();
			pCalc->setData(m_pHistogramThird, "in");
			pCalc->setData(m_pRegionsThird, "out");
			pCalc->setSearchforhighdensitycarriermaterial(bSearchforhighdensitycarriermaterial);
			(*pCalc)();
			if (m_pRegionsThird->size() > 1 && m_objectThreshold >= m_pHistogramFirst->size() - (64 + 1) ){
				nObjectThresholdFromHistogramResolustionStep = 2;
				m_objectThreshold = pCalc->objectThreshold();
			}
		}

		//initialize fourth histogram analyzer and compute regions on a quarter of the resolution
		if (m_usedHistogramResolutions >= 4) 
		{
			pCalc = createInstanceOf<Image::HistogramAnalyser>();
			pCalc->setData(m_pHistogramFourth, "in");
			pCalc->setData(m_pRegionsFourth, "out");
			pCalc->setSearchforhighdensitycarriermaterial(bSearchforhighdensitycarriermaterial);
			(*pCalc)();
			if (m_pRegionsFourth->size() > 1 && m_objectThreshold >= m_pHistogramFirst->size() - (64 + 1) ){
				nObjectThresholdFromHistogramResolustionStep = 3;
				m_objectThreshold = pCalc->objectThreshold();
			}
		}

		//merge all above histograms
		std::tr1::shared_ptr<Image::HistogramMerger> pMerger = createInstanceOf<Image::HistogramMerger>();
		
		pMerger->setData(m_pRegionsOrg, "in");
		pMerger->setData(m_pRegionsHalf, "in");
		
		if (m_usedHistogramResolutions >= 3) 
			pMerger->setData(m_pRegionsThird, "in");
		
		if (m_usedHistogramResolutions >= 4) 
			pMerger->setData(m_pRegionsFourth, "in");
		
		pMerger->setData(m_pRegions, "out");
		pMerger->setData(m_pHistogramFirst);

		setCurrentStep(pMerger);
		try {
			(*pMerger)();
		} catch(...) {
			m_pRegions = m_pRegionsOrg;
		}

		//correct the shift in peaks: (the peaks in the histograms appear in different places due to the different resolutions)
		if (m_correctRegionShift) 
		{
			std::vector<std::tr1::shared_ptr<Image::Image1DData<openOR::Triple<size_t> > > > vecMultiResolutionDensityIntervals;
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
		std::tr1::shared_ptr<Image::HistogramAnalyser> pCalc = createInstanceOf<Image::HistogramAnalyser>();
		pCalc->setData(m_pHistogramFirst, "in");
		pCalc->setData(m_pRegions, "out");

		setCurrentStep(pCalc);
		(*pCalc)();

		m_objectThreshold = pCalc->objectThreshold();
		m_background = pCalc->backgroundPeak();
	} else {
		m_objectThreshold = 0x0fff;
		m_background = 0;
	}

	// set first material
	for (size_t i = 0; i < m_pRegions->size(); i++) {
		const openOR::Triple<size_t>& region = m_pRegions->data()[i];
		if (region.second > m_objectThreshold) {
			setFirstMaterialIndex(static_cast<unsigned int>(i));
			break;
		}
	}

#if DEBUGINFOS == 1
	std::cout << "Initial Regions Guess: " << m_pRegions->size() << std::endl << std::flush;      
#endif

	//calc probabilities and fit normal distributions on the histogram
	std::tr1::shared_ptr<Image::HistogramProbabilityAnalyser> pProbAnalyser = createInstanceOf<Image::HistogramProbabilityAnalyser>();
	pProbAnalyser->setData(m_pHistogramSecond);
	pProbAnalyser->setData(m_pRegions);
	pProbAnalyser->setData(m_pProbabilities);
	pProbAnalyser->setHighDensityMaterialSearch(true);
	(*pProbAnalyser)();

#if DEBUGINFOS == 1
	std::cout << "Enhanced Regions Guess (High density search):" << m_pRegions->size() << std::endl;
#endif

}

void ZeissBackend::correctRegionShifts(std::vector<std::tr1::shared_ptr<Image::Image1DData<openOR::Triple<size_t> > > >& vecMultiResolutionDensityIntervals, const std::vector<int>& vecShifts) const {// - shift the individual histograms according to the sift given in the vector vecShifts

	int nCurrentIndex;
	openOR::uint64* pCurrentData;
	int rawDataSize = 0xffff;

	//shift the histograms according to the values given in vecshifts
	for(uint i = 0; i < (vecMultiResolutionDensityIntervals.size()); ++i) {
		if (i == 0) pCurrentData = m_pHistogramFirst->mutableData();
		else if (i == 1) pCurrentData = m_pHistogramSecond->mutableData();
		else if (i == 2) pCurrentData = m_pHistogramThird->mutableData();
		else pCurrentData = m_pHistogramFourth->mutableData();

		//left shift (vecShifts[i]>0) or right shift vecShifts[i]<0
		if (vecShifts[i] > 0) {
			for (int index = 0; index < rawDataSize; ++index) {
				nCurrentIndex = static_cast<int>(index) + vecShifts[i];
				if(nCurrentIndex > 0 && nCurrentIndex < rawDataSize) pCurrentData[index] = pCurrentData[nCurrentIndex];
				else pCurrentData[index] = 0;
			}
		} else {
			for (int index = rawDataSize - 1; index >= 0; --index) {
				nCurrentIndex = static_cast<int>(index) + vecShifts[i];
				if(nCurrentIndex > 0 && nCurrentIndex < rawDataSize) pCurrentData[index] = pCurrentData[nCurrentIndex];
				else pCurrentData[index] = 0;
			}
		}

		for (uint r=0; r < vecMultiResolutionDensityIntervals.at(i)->size(); ++r) {
			vecMultiResolutionDensityIntervals.at(i)->mutableData()[r].first = std::max<float>(0.0, static_cast<float>(vecMultiResolutionDensityIntervals.at(i)->data()[r].first) - vecShifts[i]);
			vecMultiResolutionDensityIntervals.at(i)->mutableData()[r].second = std::max<float>(0.0, static_cast<float>(vecMultiResolutionDensityIntervals.at(i)->data()[r].second) - vecShifts[i]);
			vecMultiResolutionDensityIntervals.at(i)->mutableData()[r].third = std::max<float>(0.0, static_cast<float>(vecMultiResolutionDensityIntervals.at(i)->data()[r].third) - vecShifts[i]);
		}              
	}
}

void ZeissBackend::segmentMaterials() {
	// - create instance of HistogramProbabilityAnalyser, fit normal distributions on the histogram and store it to m_pProbabilities
	// - create instance of ProbabilitySegmentation and segment the volume according to Probabilities and the Meterialindex

	boost::posix_time::ptime segmentmaterials_start(boost::posix_time::microsec_clock::local_time());

	assert(m_pVolumeData && "Need Volume Data to segment.");
	assert(m_pProbabilities && "Need probabilities to segment.");

	// compute new probabilities, fir normal distributions on histogram and store it to m_pProbabilities
	std::tr1::shared_ptr<Image::HistogramProbabilityAnalyser> pProbAnalyser = createInstanceOf<Image::HistogramProbabilityAnalyser>();
	pProbAnalyser->setData(m_pHistogramFirst);
	pProbAnalyser->setData(m_pRegions);
	pProbAnalyser->setData(m_pProbabilities);
	pProbAnalyser->setHighDensityMaterialSearch(false);

	(*pProbAnalyser)();

	//work in-place if not requested otherwise
	if (!m_pOutputMap) {m_pOutputMap = m_pVolumeData;}

#if DEBUGINFOS == 1
	std::cout << "Final Probabilities: " << m_pProbabilities->size() << std::endl;
#endif

	//segment materials according to calculated probabilities
	std::tr1::shared_ptr<ProbabilitySegmentation> pMaterialSegmentor = createInstanceOf<ProbabilitySegmentation>();
	pMaterialSegmentor->setData(m_pVolumeData, "in");
	pMaterialSegmentor->setData(m_pProbabilities, "in");
	pMaterialSegmentor->setData(m_pOutputMap, "out");
	pMaterialSegmentor->setFirstMaterialIndex(m_nFirstMaterialIndex);
	pMaterialSegmentor->setOutputMaterialIndex(m_outputMaterialIndex);
	pMaterialSegmentor->setRadius(m_nSegmentationRadius);
	pMaterialSegmentor->setGPU(m_bGPU);
	pMaterialSegmentor->setCPUPerformanceValue(m_fCPUPerformanceValue);
	setCurrentStep(pMaterialSegmentor);

	pMaterialSegmentor->operator ()();

	// get timings
	m_timeNormalDistribution = pMaterialSegmentor->getTimeNormalDistribution();
	m_timeFilterRadius = pMaterialSegmentor->getTimeFilterRadius();
	m_timePostprocessing = pMaterialSegmentor->getTimePostprocessing();
	m_nOpenMPCurrentThreads = pMaterialSegmentor->getOpenMPCurrentThreads();
	m_fProbabilityThreshold= pMaterialSegmentor->getProbabilityThreshold();
	m_nFilterRadius = pMaterialSegmentor->getFilterRadius();
}

void ZeissBackend::setNumberSeparationThreads(const unsigned int& numberSeparationThreads) {
	m_numberSeparationThreads = numberSeparationThreads;
}

void ZeissBackend::calclulateRegionsOfInterest() { //TODO: Typo
	// - depending on the on the number of separation threads:
	// - calculate the ROIs either with the MTScanSurfaceSegmentor (multi threaded)
	// - or with ScanSurfaceSegmentor (single threaded)
	// - clean up after segemtation (with purgeMask()) and expand the region masks


	assert(m_pVolumeData && "Need Volume Data to separate.");
	assert(m_pHistogramFirst && "Need Parameters to separate.");  // otherwise the m_objectThreshold is invalid

	if (!m_pVROIs) { m_pVROIs = createInstanceOf<ROIContainer>(); }
	else        { m_pVROIs->clear(); }

	// Work in-place if not requested otherwise
	if (!m_pOutputMap) { m_pOutputMap = m_pVolumeData; }

	boost::posix_time::ptime start(boost::posix_time::microsec_clock::local_time());

	if (m_numberSeparationThreads > 0) {
		std::tr1::shared_ptr<MTScanSurfaceSegmentor> pScanSurfaceSegmentor = createInstanceOf<MTScanSurfaceSegmentor>();
		pScanSurfaceSegmentor->setData(m_pVolumeData, "in");
		pScanSurfaceSegmentor->setData(m_objectThreshold);
		pScanSurfaceSegmentor->setData(m_pVROIs, "out");
		pScanSurfaceSegmentor->setData(m_pOutputMap, "out");

		pScanSurfaceSegmentor->setBorderWidth(m_ROIBorderSize);
		pScanSurfaceSegmentor->setMinROIEdgeLength(m_ROIMinEdgeLength);
		pScanSurfaceSegmentor->setMinROIVolume(m_ROIMinVolume);
		pScanSurfaceSegmentor->setNumberSubvolumes(m_numberSeparationThreads);

		setCurrentStep(pScanSurfaceSegmentor);
		pScanSurfaceSegmentor->operator ()();
	} else {
		std::tr1::shared_ptr<ScanSurfaceSegmentor> pScanSurfaceSegmentor = createInstanceOf<ScanSurfaceSegmentor>();
		pScanSurfaceSegmentor->setData(m_pVolumeData, "in");
		pScanSurfaceSegmentor->setData(m_objectThreshold);
		pScanSurfaceSegmentor->setData(m_pVROIs, "out");
		pScanSurfaceSegmentor->setData(m_pOutputMap, "out");

		pScanSurfaceSegmentor->setBorderWidth(m_ROIBorderSize);
		pScanSurfaceSegmentor->setMinROIEdgeLength(m_ROIMinEdgeLength);
		pScanSurfaceSegmentor->setMinROIVolume(m_ROIMinVolume);

		setCurrentStep(pScanSurfaceSegmentor);
		pScanSurfaceSegmentor->operator ()();
	}

	boost::posix_time::ptime end(boost::posix_time::microsec_clock::local_time());

	//clean up the ROI indices
	purgeMask();

	for (int i = 0; i < m_expandMaskTimes; i++) {
		expandRegionMasks();
	}
	return;
}

void ZeissBackend::purgeMask() {
	// - find the maximum ROI (maxRoi) index in m_pVROIs
	// - set all indices in the outpotMap that are higher than maxRoi to zero 

	using openOR::Image::Image3DDataUI16;
	using openOR::Math::Vector3ui;

	//find maximum ROI index (ROI = Region of Interest)
	unsigned int maxRoi = 0;
	for (uint i = 0; i < m_pVROIs->size(); i++) {
		unsigned int roiIndex = (*m_pVROIs)(i).index();
		if (roiIndex > maxRoi) maxRoi = roiIndex;
	}

	if (maxRoi >= std::numeric_limits<uint16>::max()) maxRoi = std::numeric_limits<uint16>::max() - 1;

	//initialize a map and write the ROI indices to it
	uint16* map = new uint16[maxRoi + 1];
	for (int i = 0; i < (maxRoi + 1); i++) map[i] = 0;
	for (uint i = 0; i < m_pVROIs->size(); i++) {
		unsigned int roiIndex = (*m_pVROIs)(i).index();
		if (roiIndex > std::numeric_limits<uint16>::max()) {
			// TODO: error handling; should actually never happen
		} else {
			map[roiIndex] = roiIndex;
		}
	}

	std::tr1::shared_ptr<Image3DDataUI16> pMask = m_pOutputMap;
	Vector3ui size = pMask->size();

	size_t ind;
	openOR::uint16* pData = pMask->mutableData();

	unsigned long long rowLenght = size[0];
	unsigned long long sliceLenght = size[0] * size[1];

	//set all ROI Indices to zero that are higher than the maximum ROI index
	for (unsigned long long z = 0; z < (size[2] - 1); z++) 
	{
		for (unsigned long long y = 0; y < (size[1] - 1); y++) 
		{
			ind = z * sliceLenght + y * rowLenght;
			for (unsigned long long x = 0; x < (size[0] - 1); x++) 
			{
				if (pData[ind] <= maxRoi) pData[ind] = map[pData[ind]];
				else pData[ind] = 0;
				ind++;
			}
		}
	}

	delete map;
}

void ZeissBackend::separateAndSave() {
	separateAndSave(m_outputBN);
}

namespace {
	struct BBSumReducer { // function object used as a binary operation for std::accumalate below
		Math::Vector3ui operator()(const Math::Vector3ui& acc, const Image::RegionOfInterest& r) const {
			uint32 newX = acc(0) + r.backUpperRight()(0) - r.frontLowerLeft()(0);
			uint32 newY = acc(1) + r.backUpperRight()(1) - r.frontLowerLeft()(1);
			uint32 newZ = acc(2) + r.backUpperRight()(2) - r.frontLowerLeft()(2);
			return Math::create<Math::Vector3ui>(newX, newY, newZ);
		}
	};

	struct RegionsOfInterestOrder { // function object: used as a binary function for std::sort below 
		Math::Vector3i m_tolerance;
		RegionsOfInterestOrder(const Math::Vector3ui& tolerance) {
			m_tolerance = Math::create<Math::Vector3i>(tolerance(0), tolerance(1), tolerance(2));
		}

		bool operator()(const Image::RegionOfInterest& r1, const Image::RegionOfInterest& r2) const {
			Math::Vector3i c1 = Math::create<Math::Vector3i>((r1.backUpperRight()(0) + r1.frontLowerLeft()(0)) / 2, (r1.backUpperRight()(1) + r1.frontLowerLeft()(1)) / 2, (r1.backUpperRight()(2) + r1.frontLowerLeft()(2)) / 2);
			Math::Vector3i c2 = Math::create<Math::Vector3i>((r2.backUpperRight()(0) + r2.frontLowerLeft()(0)) / 2, (r2.backUpperRight()(1) + r2.frontLowerLeft()(1)) / 2, (r2.backUpperRight()(2) + r2.frontLowerLeft()(2)) / 2);

			if (c1(2) < (c2(2) - m_tolerance(2))) return true;
			if ((c1(2) - m_tolerance(2)) > c2(2)) return false;
			if (c1(1) < (c2(1) - m_tolerance(1))) return true;
			if ((c1(1) - m_tolerance(1)) > c2(1)) return false;
			if (c1(0) < (c2(0) - m_tolerance(0))) return true;
			if ((c1(0) - m_tolerance(0)) > c2(0)) return false;

			return false;
		}
	};
}


void ZeissBackend::separateAndSave(const std::string& basename) {
	// - Separate ROIs into separate Volumes and save them

	assert(m_pVolumeData && "Need Volume Data to separate.");

	std::tr1::shared_ptr<openOR::Image::ZeissRawExtendedExporterUI16MaskUI16> pExporter = createInstanceOf<openOR::Image::ZeissRawExtendedExporterUI16MaskUI16>();
	std::tr1::shared_ptr<openOR::Image::ZeissRawImporterUI16> pImporter = createInstanceOf<openOR::Image::ZeissRawImporterUI16>();

	setCurrentStep(pImporter);

	if (m_pVROIs->empty()) {
		LOG(Log::Level::Info, OPENOR_MODULE, Log::msg("No Objects found!"));
		return;
	}

	std::vector<Image::RegionOfInterest> vrois;
	for (unsigned int i = 0; i < m_pVROIs->size(); i++) {
		vrois.push_back((*m_pVROIs)(i));
	}

	Math::Vector3ui sum = std::accumulate(vrois.begin(), vrois.end(), Math::create<Math::Vector3ui>(0, 0, 0), BBSumReducer());
	double fToleranceFactor = 4.0; //TODO: set and describe empirical value 
	Math::Vector3ui tolerance = Math::create<Math::Vector3ui>(sum(0) / (fToleranceFactor * vrois.size()), sum(1) / (fToleranceFactor * vrois.size()), sum(2) / (fToleranceFactor * vrois.size()));

	std::sort(vrois.begin(), vrois.end(), RegionsOfInterestOrder(tolerance));

	pExporter->setFilename(basename);
	pExporter->setData(vrois, "RegionsOfInterest");
	pExporter->setData(m_background, "fill");
	pExporter->setData(m_pOutputMap, "mask");

	// scan surface segmentator changes the volume, so we always need to re-read it from the disk
	if (true) {
		pImporter->setFilename(m_inputFN);
		pImporter->setData(pExporter, std::string());
		pImporter->setIsRawData(m_isRawInput, m_inputSkipHeader);
		if (m_isRawInput) {
			pImporter->setData(m_pInputSizeDescriptor, "inputSize");
		}

		(*pImporter)();
	} else {
		pExporter->setData(m_pVolumeData, "inSize");
		pExporter->setData(m_pVolumeData, "in");

		(*pExporter)();
	}

	return;
}

void ZeissBackend::setExpandMaskTimes(int expandMask) {
	m_expandMaskTimes = expandMask;
}

void ZeissBackend::expandRegionMasks() {
	// - expand the Region Masks to neigbouring voxels that have the value zero

	using openOR::Image::Image3DDataUI16;
	using openOR::Math::Vector3ui;

	std::tr1::shared_ptr<Image3DDataUI16> pMask = m_pOutputMap;
	Vector3ui size = pMask->size();

	openOR::uint16* pData = pMask->mutableData();

	size_t rowLenght = size[0];
	size_t sliceLenght = size[0] * size[1];

	// expand Regions to neighbour regions that have index zero
	if (m_numberSeparationThreads == 0) {
		size_t ind;
		// expand forwards
		for (size_t z = 0; z < (size[2] - 1); z++) {
			for (size_t y = 0; y < (size[1] - 1); y++) {
				ind = z * sliceLenght + y * rowLenght;

				for (size_t x = 0; x < (size[0] - 1); x++) {
					if (pData[ind] == 0) pData[ind] = pData[ind + 1];
					if (pData[ind] == 0) pData[ind] = pData[ind + rowLenght];
					if (pData[ind] == 0) pData[ind] = pData[ind + sliceLenght];
					ind++;
				}
			}
		}

		// expand backwards
		for (size_t z = (size[2] - 1); z > 0 ; z--) {
			for (size_t y = (size[1] - 1); y > 0; y--) {
				ind = z * sliceLenght + y * rowLenght + rowLenght - 1;
				for (size_t x = (size[0] - 1); x > 0; x--) {
					if (pData[ind] == 0) pData[ind] = pData[ind - 1];
					if (pData[ind] == 0) pData[ind] = pData[ind - rowLenght];
					if (pData[ind] == 0) pData[ind] = pData[ind - sliceLenght];
					ind--;
				}
			}
		}
	} 

	else 
	{ // do the process from above in parallel
		size_t depth = size(2);
		size_t sliceProThread = depth / m_numberSeparationThreads;

		uint16* tmpSlices = new uint16[sliceLenght * (m_numberSeparationThreads - 1)];

#pragma  omp parallel num_threads(m_numberSeparationThreads)
		{

			// copy first slice of the next subvolume
#pragma     omp master
			{
				for (int p = 0; p < (m_numberSeparationThreads - 1); p++) {
					size_t zEnd = (p + 1) * sliceProThread;

					std::copy(pData + (zEnd * sliceLenght), pData + (zEnd * sliceLenght + sliceLenght), tmpSlices + (p * sliceLenght));
				}
			}
#pragma     omp barrier

			// expand forward (skip last slice)
#pragma     omp for
			for (int p = 0; p < m_numberSeparationThreads; p++) {
				size_t zStart = p * sliceProThread;
				size_t zEnd = (p + 1) * sliceProThread;
				if (p == (m_numberSeparationThreads - 1)) zEnd = depth;

				// expand forwards
				for (size_t z = zStart; z < (zEnd - 1); z++) {
					for (size_t y = 0; y < (size[1] - 1); y++) {
						size_t ind = z * sliceLenght + y * rowLenght;

						for (size_t x = 0; x < (size[0] - 1); x++) {
							register uint16 val = pData[ind];
							if (val == 0) val = pData[ind + 1];
							if (val == 0) val = pData[ind + rowLenght];
							if (val == 0) val = pData[ind + sliceLenght];
							pData[ind] = val;
							ind++;
						}
					}
				}
			}

			// expand forward (last slice)
#pragma     omp for
			for (int p = 0; p < (m_numberSeparationThreads - 1); p++) {
				size_t zEnd = (p + 1) * sliceProThread;
				size_t z = zEnd - 1;

				for (size_t y = 0; y < (size[1] - 1); y++) {
					size_t ind = z * sliceLenght + y * rowLenght;
					size_t sliceInd = p * sliceLenght + y * rowLenght;

					for (size_t x = 0; x < (size[0] - 1); x++) {
						register uint16 val = pData[ind];
						if (val == 0) val = pData[ind + 1];
						if (val == 0) val = pData[ind + rowLenght];
						if (val == 0) val = tmpSlices[sliceInd];
						pData[ind] = val;
						ind++;
						sliceInd++;
					}
				}
			}

			// copy last slice of the previous subvolume
#pragma     omp master
			{
				for (int p = 1; p < m_numberSeparationThreads; p++) {
					size_t zStart = p * sliceProThread;
					size_t zEnd = zStart - 1;

					std::copy(pData + (zEnd * sliceLenght), pData + (zEnd * sliceLenght + sliceLenght), tmpSlices + ((p - 1) * sliceLenght));
				}
			}
#pragma     omp barrier

			// expand backwards (skip first slice)
#pragma     omp for
			for (int p = 0; p < m_numberSeparationThreads; p++) {
				size_t zStart = p * sliceProThread;
				size_t zEnd = (p + 1) * sliceProThread;
				if (p == (m_numberSeparationThreads - 1)) zEnd = depth;

				// expand backwards
				for (size_t z = (zEnd - 1); z > zStart; z--) {
					for (size_t y = (size[1] - 1); y > 0; y--) {
						size_t ind = z * sliceLenght + y * rowLenght + rowLenght - 1;
						for (size_t x = (size[0] - 1); x > 0; x--) {
							register uint16 val = pData[ind];
							if (val == 0) val = pData[ind - 1];
							if (val == 0) val = pData[ind - rowLenght];
							if (val == 0) val = pData[ind - sliceLenght];
							pData[ind] = val;
							ind--;
						}
					}
				}
			}

			// expand backwards (first slice)
#pragma     omp for
			for (int p = 1; p < m_numberSeparationThreads; p++) {
				size_t zStart = p * sliceProThread;
				size_t z = zStart;

				for (size_t y = (size[1] - 1); y > 0; y--) {
					size_t ind = z * sliceLenght + y * rowLenght + rowLenght - 1;
					size_t sliceInd = (p - 1) * sliceLenght + y * rowLenght + rowLenght - 1;
					for (size_t x = (size[0] - 1); x > 0; x--) {
						register uint16 val = pData[ind];
						if (val == 0) val = pData[ind - 1];
						if (val == 0) val = pData[ind - rowLenght];
						if (val == 0) val = tmpSlices[sliceInd];
						pData[ind] = val;
						ind--;
						sliceInd--;
					}
				}
			}
		}

		delete[] tmpSlices;
	}
}

namespace {
	template<bool set_to_state = true>
	struct raii_flag {
		raii_flag(bool& flag) : flag_(flag) { flag_ = set_to_state; }
		~raii_flag() { flag_ = !set_to_state; }
	private:
		bool& flag_;
	};
}

void ZeissBackend::operator()() {

	// - basic workflow of a separation task
	raii_flag<> guard(m_inOneGoFlag);
	m_currentStepNum=0;

	loadFile(m_inputFN);

	createHistogramRegions();

	calclulateRegionsOfInterest();

	separateAndSave(m_outputBN);
}

//---------------------------------------------------------------------------------------------------------------------------
// Progressable interface
// This is a laymans statemachine with setCurrentStep as the external event raiser
void ZeissBackend::setCurrentStep(const AnyPtr& step) {
	m_pCurrentStep = interface_cast<Progressable>(step);        // can be null, that is fine!
	m_pCanCancelCurrentStep = interface_cast<Cancelable>(step); // dito
	if (m_inOneGoFlag) {
		assert(m_currentStepNum < m_numSteps && "one step to far!");
		++m_currentStepNum;
	}
}

double ZeissBackend::progress() const {

	double prog = 0.0;

	//changed: prog = (m_pCurrentStep) ? m_pCurrentStep->progress() : 0.0;
	if (m_pCurrentStep){ prog = m_pCurrentStep->progress();}
	else {prog = 0.0;}
	if (m_inOneGoFlag) { prog = ((double)(m_currentStepNum - 1)/(double) m_numSteps) + (prog / (double) m_numSteps); }

	return prog;
}

std::string ZeissBackend::description() const {
	//changed: return (m_pCurrentStep)? m_pCurrentStep->description() : std::string("Zeiss Volume Separation");
	if (m_pCurrentStep) {
      return m_pCurrentStep->description();
   }
	else {
      return std::string("Zeiss Volume Separation");
   }
}

void ZeissBackend::cancel() {
	m_canceled = true;
	if (m_pCanCancelCurrentStep) { m_pCanCancelCurrentStep->cancel(); }
}

bool ZeissBackend::isCanceled() const {
	return m_canceled;
}

double ZeissBackend::doBenchmark(char* path, size_t size, bool reset){
	// - benchmark the segmentation process by calling createHistograms() and segmentMaterials() for multiple data 
	// - time everything and write a benchmark file

	std::cout << "[info] start benchmark!" << std::endl;

   std::string strpath(path, path + size); //convert to string
	boost::filesystem::path somedir(strpath);  //set path
	boost::filesystem::directory_iterator end_iter; // create itertor for directories

	boost::property_tree::ptree ptree;  // create property tree
	std::string strlabel;   
	size_t nCount = 1;

	if(!reset && boost::filesystem::exists(m_strPathBenchmark)){   // if no reset and file exist read property tree from xml (that way the next benchmark results can be added to the xml)
		std::cout << "[info] read xml-file" << std::endl;
		read_xml(m_strPathBenchmark, ptree);
		nCount = ptree.get<int>("benchmark.entries");
	}
	else if (reset){
		std::cout << "[info] reset xml-file" << std::endl;
	}
	else{
		std::cout << "[error] xml-file does not exist" << std::endl;
		return -2.0;
	}

	if ( boost::filesystem::exists(somedir) && boost::filesystem::is_directory(somedir)){   // ckeck if directory exists and is a directory
		for( boost::filesystem::directory_iterator dir_iter(somedir) ; dir_iter != end_iter ; ++dir_iter){  // iterate through directories
			if (boost::filesystem::is_regular_file(dir_iter->status()) && boost::filesystem::extension(dir_iter->path()) == ".vgi"){ // check if file is regular and extension is ".vgi"
				std::cout << "[info] load " << nCount << " file" << std::endl;
				loadFile(dir_iter->path().string());   // load volume

				std::string filename = dir_iter->path().filename().string();

				boost::posix_time::ptime starttime(boost::posix_time::microsec_clock::local_time());
				
				// create histogram
				createHistogramRegions();  

				boost::posix_time::ptime nhistotime(boost::posix_time::microsec_clock::local_time());
				std::cout << "hist time" << (nhistotime-starttime).total_milliseconds() << std::endl;

				size_t nwidth = m_pVolumeData->size()(0);
				size_t nheight = m_pVolumeData->size()(1);
				size_t ndepth = m_pVolumeData->size()(2);

				// segment volume
				segmentMaterials();

				boost::posix_time::ptime nsegtime(boost::posix_time::microsec_clock::local_time());

				strlabel = "benchmark.measurements.dataset";
				strlabel = strlabel + boost::lexical_cast<std::string>(nCount+1);

				ptree.put(strlabel+".filename",filename);

				ptree.put(strlabel+".volumesize.x",nwidth);
				ptree.put(strlabel+".volumesize.y",nheight);
				ptree.put(strlabel+".volumesize.z",ndepth);

				size_t nsum = 0;
				if(m_nFirstMaterialIndex > 1)
				{
					for(int i = m_pRegions->data()[0].second; i < m_pRegions->data()[m_nFirstMaterialIndex-1].second;i++)
					{
						nsum += m_pHistogramFirst->data()[i];
					}
				}
				size_t ntmp = (nwidth * nheight * ndepth)+1.3*nsum;
				size_t m_nfilterradiusnew = m_nFilterRadius*ntmp/(nwidth * nheight * ndepth);

				ptree.put(strlabel+".gpu",m_bGPU);
				ptree.put(strlabel+".numthreads",m_nOpenMPCurrentThreads);
				ptree.put(strlabel+".cpuperformancevalue",m_fCPUPerformanceValue);
				ptree.put(strlabel+".filterradius",m_nfilterradiusnew);
				ptree.put(strlabel+".threshold",m_fProbabilityThreshold);
				ptree.put(strlabel+".materials",m_pProbabilities->size());
				ptree.put(strlabel+".timehistogram", (nhistotime-starttime).total_seconds());
				ptree.put(strlabel+".timenormaldistribution", m_timeNormalDistribution);
				ptree.put(strlabel+".timefilterradius", m_timeFilterRadius);
				ptree.put(strlabel+".timepostprocessing", m_timePostprocessing);
				nCount++;
			}
		}
	}
	else{
		std::cout << "[error] volume directory does not exist" << std::endl;
		return -3.0;
	}
	if(nCount<=0){
		std::cout << "[error] no volumes found in volume directory" << std::endl;
		return -4.0;
	}
	ptree.put("benchmark.entries", nCount);
	write_xml(m_strPathBenchmark.data(), ptree);
	return 1.0;
}

double ZeissBackend::saveBenchmark(bool reset){
#if DEBUGINFOS == 1
	std::cout << "Start Benchmark!" << std::endl;
#endif

	boost::property_tree::ptree ptree;  // create property tree
	std::string strLabel;   
	size_t nCount = 0;


	 // if no reset and file exist read property tree from xml (that way the next benchmark results can be ADDED to the xml)
	if(!reset && boost::filesystem::exists(m_strPathBenchmark) && boost::filesystem::extension(m_strPathBenchmark) == ".xml"){  
#if DEBUGINFOS == 1
		std::cout << "[Info] Read XML-file" << std::endl;
#endif
		read_xml(m_strPathBenchmark, ptree);
		nCount = ptree.get<int>("Benchmark.Entries");

	}
	else if (reset || !boost::filesystem::exists(m_strPathBenchmark) && boost::filesystem::extension(m_strPathBenchmark) == ".xml"){
#if DEBUGINFOS == 1
		std::cout << "[Info] Reset/Create XML-file" << std::endl;
#endif
	}
	else{
#if DEBUGINFOS == 1
		std::cout << "[Error] XML-file does not exist" << std::endl;
		std::cout << m_strPathBenchmark << std::endl;
#endif
		return -5.0;
	}

	size_t nWidth = m_pVolumeData->size()(0);
	size_t nHeight = m_pVolumeData->size()(1);
	size_t nDepth = m_pVolumeData->size()(2);

	strLabel = "Benchmark.Measurements.Dataset";
	strLabel = strLabel + boost::lexical_cast<std::string>(nCount+1);

	ptree.put(strLabel+".Volumesize.x",nWidth);
	ptree.put(strLabel+".Volumesize.y",nHeight);
	ptree.put(strLabel+".Volumesize.z",nDepth);



	size_t nSum = 0;
	if(m_nFirstMaterialIndex > 1)
	{
#if DEBUGINFOS == 1
		std::cout << "first mat: " << m_nFirstMaterialIndex << std::endl; 
#endif
		for(int i = m_pRegions->data()[0].second; i < m_pRegions->data()[m_nFirstMaterialIndex-1].second;i++)
		{
			nSum += m_pHistogramFirst->data()[i];
		}
	}

	size_t nNumOfVoxel = nWidth * nHeight * nDepth;

	double nTimeNormalized = m_timeFilterRadius / static_cast<double>((nNumOfVoxel + EMPIRICAL_FACTOR * nSum));

	size_t m_ntimeFilterRadiusNew = static_cast<size_t>(nTimeNormalized * nNumOfVoxel);

#if DEBUGINFOS == 1
	std::cout << "save m_timeFilterRadius: " << m_timeFilterRadius << "new filter radius: " << m_ntimeFilterRadiusNew << std::endl;
#endif
	// write everythin to the xml file
	ptree.put(strLabel+".GPU",m_bGPU);
	ptree.put(strLabel+".NumThreads",m_nOpenMPCurrentThreads);
	ptree.put(strLabel+".CPUPerformanceValue",m_fCPUPerformanceValue);
	ptree.put(strLabel+".FilterRadius",m_nFilterRadius);
	ptree.put(strLabel+".Threshold",m_fProbabilityThreshold);
	ptree.put(strLabel+".Materials",m_pProbabilities->size());
	ptree.put(strLabel+".TimeHistogram", m_timeHist);
	ptree.put(strLabel+".TimeNormalDistribution", m_timeNormalDistribution);
	ptree.put(strLabel+".TimeFilterRadius", m_ntimeFilterRadiusNew);
	ptree.put(strLabel+".TimePostProcessing", m_timePostprocessing);
	ptree.put("Benchmark.Entries", nCount+1);

	write_xml(m_strPathBenchmark.data(), ptree);
#if DEBUGINFOS == 1
	std::cout << "Done write file" << std::endl;
#endif
	return 1.0;
}

void ZeissBackend::setBenchmarkFile(char* path, size_t size)
{
	std::string strPathBenchmark(path, path + size);
	m_strPathBenchmark = strPathBenchmark;
}

double ZeissBackend::getSegmentationTimeEstimation(int nRegions){
	// - based on the precalculated benchmark file and the number of Regions (nRegions) estimate the time that is needed for the Segmentation

	size_t nCurrentWidth = m_pVolumeData->size()(0);
	size_t nCurrentHeight = m_pVolumeData->size()(1);
	size_t nCurrentDepth = m_pVolumeData->size()(2);

	size_t nCurrentNumOfRegions = m_pProbabilities->size();
	//std::cout << "Materials for SegmentationTimeEstimation: " << nCurrentNumOfRegions << std::endl;
	if(nRegions>0)
		nCurrentNumOfRegions = nRegions;

	if(nCurrentWidth <= 0 || nCurrentHeight <= 0|| nCurrentDepth <= 0){
#if DEBUGINFOS == 1
		std::cout << "[Error] No volume loaded OR volume corrupt" << std::endl;
#endif
		return -2.0;
	}

	if(nCurrentNumOfRegions <= 0){
#if DEBUGINFOS == 1
		std::cout << "[Error] No regions detected (Create histogram first)" << std::endl;
#endif
		return -3.0;
	}

	boost::property_tree::ptree ptree;  // create property tree
	double fFilterRadiusMeanTimeNormalized = 0;
	double fPostprocessingMeanTimeNormalzed = 0;
	double fNormalDistributionMeanTimeNormalized = 0;

	size_t nCounter = 0;
	// if no reset and file exist read property tree from xml (that way the next benchmark results can be ADDED to the xml)
	if(boost::filesystem::exists(m_strPathBenchmark) && boost::filesystem::extension(m_strPathBenchmark) == ".xml"){   
		read_xml(m_strPathBenchmark, ptree);
		BOOST_FOREACH(boost::property_tree::ptree::value_type &pTreeValue, ptree.get_child("Benchmark.Measurements")){ // changed the variable name of the iterator from v to pTreeValue
			if(m_bGPU == pTreeValue.second.get<bool>("GPU") && m_nSegmentationRadius == pTreeValue.second.get<size_t>("FilterRadius"))
			{
				size_t nTimeNormalDistribution = pTreeValue.second.get<size_t>("TimeNormalDistribution");
				size_t nTimeFilterRadius = pTreeValue.second.get<size_t>("TimeFilterRadius");
				size_t nTimePostprocessing = pTreeValue.second.get<size_t>("TimePostProcessing");
#if DEBUGINFOS == 1
				std::cout << "Read xml: Radius: " << m_nSegmentationRadius << ", filter rad. time: " << nTimeFilterRadius << ", post pro.: "<< nTimePostprocessing << std::endl;
#endif

				size_t nNumMaterials = pTreeValue.second.get<size_t>("Materials");;

				size_t nVolSizeX = pTreeValue.second.get<size_t>("Volumesize.x");
				size_t nVolSizeY = pTreeValue.second.get<size_t>("Volumesize.y");
				size_t nVolSizeZ = pTreeValue.second.get<size_t>("Volumesize.z");
				
            size_t nThreads = pTreeValue.second.get<size_t>("NumThreads");

				fNormalDistributionMeanTimeNormalized += (nTimeNormalDistribution / static_cast<double>(nNumMaterials));
				fFilterRadiusMeanTimeNormalized += (nTimeFilterRadius * nThreads / static_cast<double>(nVolSizeX * nVolSizeY * nVolSizeZ * nNumMaterials));
				fPostprocessingMeanTimeNormalzed += (nTimePostprocessing / static_cast<double>(nVolSizeX * nVolSizeY * nVolSizeZ));

				nCounter++;
			}
		}

		if(nCounter == 0){
#if DEBUGINFOS == 1
			std::cout << "[Error] XML-File corrupt"<< std::endl;
#endif
			return -4.0;
		}
		else{
			fNormalDistributionMeanTimeNormalized /= nCounter;
			fFilterRadiusMeanTimeNormalized /= nCounter;
			fPostprocessingMeanTimeNormalzed /= nCounter;

			size_t nSum = 0;
			if(m_nFirstMaterialIndex > 1)
			{
#if DEBUGINFOS == 1
				std::cout << "First mat:" << m_nFirstMaterialIndex << std::endl;
#endif
				for(int i = m_pRegions->data()[0].second; i < m_pRegions->data()[m_nFirstMaterialIndex-1].second;i++)
				{
					nSum += m_pHistogramFirst->data()[i];
				}
			}

			size_t nNumOfVoxel = nCurrentWidth * nCurrentHeight * nCurrentDepth;

#ifdef _OPENMP
			m_nOpenMPMaxThreads = omp_get_max_threads();
#endif

			double fThreads = static_cast<int>(m_nOpenMPMaxThreads*m_fCPUPerformanceValue);
#if DEBUGINFOS == 1
			std::cout << "filter radius estimation: " << fFilterRadiusMeanTimeNormalized * (nNumOfVoxel+ EMPIRICAL_FACTOR * nSum) * nCurrentNumOfRegions / fThreads << std::endl;
			std::cout << "post processing estimation: " << fPostprocessingMeanTimeNormalzed * nNumOfVoxel << std::endl;
#endif

			double fTimeEstimation = fNormalDistributionMeanTimeNormalized * nCurrentNumOfRegions  + 
				fFilterRadiusMeanTimeNormalized * (nNumOfVoxel+ EMPIRICAL_FACTOR * nSum) * nCurrentNumOfRegions / fThreads +
				fPostprocessingMeanTimeNormalzed * nNumOfVoxel;

			return fTimeEstimation;
		}
	}
	else{
#if DEBUGINFOS == 1
		std::cout << "[Error] Benchmark.xml does not exist (Run benchmark first AND check path)" << std::endl;
#endif
		return -5.0;
	}

#if DEBUGINFOS == 1
	std::cout << "[Error] Unexpected error" << std::endl; 
#endif
	return -1.0;
}

double ZeissBackend::getMaterialRegionsTimeEstimation(size_t nCurrentWidth, size_t nCurrentHeight, size_t nCurrentDepth){
	// - based on the precalculated benchmark file and current volume dimensions (nCurrentWidth, nCurremtHeight, nCurrentDepth) estimate the time that is needed for the Segmentation

	if(nCurrentWidth <= 0 || nCurrentHeight <= 0|| nCurrentDepth <= 0){
#if DEBUGINFOS == 1
		std::cout << "[Error] No volume loaded OR volume corrupt" << std::endl;
#endif
		return -2.0;
	}

	boost::property_tree::ptree ptree;  // create property tree
	double fHistogramTimeNormalized = 0;

	size_t nCounter = 0;

	// if no reset and file exist read property tree from xml (that way the next benchmark results can be ADDED to the xml)
	if(boost::filesystem::exists(m_strPathBenchmark) && boost::filesystem::extension(m_strPathBenchmark) == ".xml"){   
		read_xml(m_strPathBenchmark, ptree);
		BOOST_FOREACH(boost::property_tree::ptree::value_type &pTreeValue, ptree.get_child("Benchmark.Measurements")){ // changed the variable name of the iterator from v to pTreeValue

			size_t nTimeHistogram = pTreeValue.second.get<size_t>("TimeHistogram");

			size_t nVolSizeX = pTreeValue.second.get<size_t>("Volumesize.x");
			size_t nVolSizeY = pTreeValue.second.get<size_t>("Volumesize.y");
			size_t nVolSizeZ = pTreeValue.second.get<size_t>("Volumesize.z");

			fHistogramTimeNormalized += (nTimeHistogram / static_cast<double>(nVolSizeX * nVolSizeY * nVolSizeZ));

			nCounter++;
		}

		if(nCounter == 0){
#if DEBUGINFOS == 1
			std::cout << "[Error] XML-File corrupt"<< std::endl;
#endif
			return -4.0;
		}
		else{
			fHistogramTimeNormalized /= static_cast<double>(nCounter);

			size_t nNumOfVoxel = nCurrentWidth * nCurrentHeight * nCurrentDepth;
			double fTimeEstimation = fHistogramTimeNormalized * nNumOfVoxel;

			//m_pHistogramOrg
			return fTimeEstimation;
		}
	}
	else{
#if DEBUGINFOS == 1
		std::cout << "[Error] xml file does not exist (Run benchmark first AND check path):" << m_strPathBenchmark << std::endl;
#endif
		return -5.0;
	}
#if DEBUGINFOS == 1
	std::cout << "[Error] Unexpected error" << std::endl; 
#endif
	return -1.0;
}

void ZeissBackend::setHistTime(size_t time)
{
	m_timeHist = time;
}

void ZeissBackend::setAutoScaling(bool autosclaing)
{
	m_bAutoScaling = autosclaing;
}

size_t ZeissBackend::usedHistogramResolutions() {
	return m_usedHistogramResolutions;
}

void ZeissBackend::setSearchForHighDensityCarrierMaterial(bool bSearch){
	m_bSearchForHighDensityCarrierMaterial = bSearch;
}

//end namespace
}
