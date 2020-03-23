// (c) 2012 by Fraunhofer IPK

#include "ZeissSeparationInterface.hpp"

#include <openOR/ZeissBackend.hpp>
#include <openOR/Image/Image3DData.hpp> //Image_ImageData
#include <openOR/Image/VolumeCollector.hpp> //Image_Utility
#include <openOR/Image/HistogramCollector.hpp> //Image_Utility
#include <openOR/Image/Image3DSizeDescriptor.hpp> // Image_ImageData
#include <openOR/Image/ROIContainer.hpp>// Image_Regions
#include <openOR/Plugin/create.hpp>// openOR_core

#include <cstring>
#include <openOR/cleanUpWindowsMacros.hpp>

template<typename T>
struct AliasingDeleter {
	AliasingDeleter(std::tr1::shared_ptr<openOR::Image::Image3DDataUI16> i) : image(i) {}

	void operator() (T*& ptr) {
		image.reset();
	}

private:
	std::tr1::shared_ptr<openOR::Image::Image3DDataUI16> image;
};

struct ZeissSeparationInterfaceInternalState {
	ZeissSeparationInterfaceInternalState() :
pBackend(new openOR::ZeissBackend()),
	pVolume(openOR::createInstanceOf<openOR::Image::Image3DDataUI16>()),
	pHistogram(openOR::createInstanceOf<openOR::Image::Image1DDataUI64>()),
	output(boost::shared_array<uint16_t>(pVolume->mutableData(), AliasingDeleter<uint16_t>(pVolume))),
	pRoi(openOR::createInstanceOf<openOR::ROIContainer>()),
	inputFromFile(true),
	inputVolume(),
	wasCanceled(false),
	scalingFactor(openOR::Math::create<openOR::Math::Vector3ui>(1, 1, 1)),
	max_memory(std::numeric_limits<size_t>::max())
{
	pHistogram->setSize(static_cast<openOR::uint32>(std::numeric_limits<openOR::uint16>::max()) + 1);
	pBackend->setVolume(pVolume);
	pBackend->setData(pRoi);
	pBackend->setData(pHistogram);
	pBackend->setUseMultiScaleHistogram(4);
	pBackend->setAutoScaling(true);
}
~ZeissSeparationInterfaceInternalState() { delete pBackend; }

openOR::ZeissBackend* pBackend;
std::tr1::shared_ptr<openOR::Image::Image3DDataUI16> pVolume;
std::tr1::shared_ptr<openOR::Image::Image1DDataUI64> pHistogram;
boost::shared_array<uint16_t> output;
std::tr1::shared_ptr<openOR::ROIContainer> pRoi;
bool inputFromFile;
Volume inputVolume;
bool wasCanceled;
openOR::Math::Vector3ui scalingFactor;
size_t max_memory;
};

ZeissSeparationInterface::ZeissSeparationInterface() : m_pState(new ZeissSeparationInterfaceInternalState()) {}
ZeissSeparationInterface::~ZeissSeparationInterface() { delete m_pState; }

double ZeissSeparationInterface::progress() const {
	return m_pState->pBackend->progress();
}

void ZeissSeparationInterface::cancel() {
	m_pState->pBackend->cancel();
	m_pState->wasCanceled = true;
}

void ZeissSeparationInterface::setInputVolume( const Volume& v ) {
	m_pState->inputFromFile = false;

	m_pState->inputVolume = v;
	m_pState->wasCanceled = false;
}

void ZeissSeparationInterface::setInputVolume( const std::string& filename ) {
	m_pState->pBackend->setInputFilename(filename);
	m_pState->inputFromFile = true;
	m_pState->wasCanceled = false;
}

void ZeissSeparationInterface::setInputVolume( const char* filename ) {
	std::string fn(filename);
	setInputVolume(fn);
}

void ZeissSeparationInterface::setInputVolume( const std::string& uint16_scv_filename, const VolumeHeader headerinfos) {
	using namespace openOR;

	std::tr1::shared_ptr<Image::Image3DSizeDescriptor> pImageSize = createInstanceOf<Image::Image3DSizeDescriptor>();
	pImageSize->setSize(Math::create<Math::Vector3ui>(headerinfos.size[0], headerinfos.size[1], headerinfos.size[2]));
	pImageSize->setSizeMM(Math::create<Math::Vector3d>(headerinfos.voxel_size[0] * headerinfos.size[0], headerinfos.voxel_size[1] * headerinfos.size[1], headerinfos.voxel_size[2] * headerinfos.size[2]));

	m_pState->pBackend->setInputDataDesciptor(uint16_scv_filename, pImageSize, headerinfos.header_size);
	m_pState->inputFromFile = true;
	m_pState->wasCanceled = false;
}

void ZeissSeparationInterface::setInputVolume( const char* uint16_scv_filename, const VolumeHeader headerinfos) {
	std::string fn(uint16_scv_filename);
	setInputVolume(fn, headerinfos);
}

void ZeissSeparationInterface::setParameter( const Parameter& p ) {
	m_pState->pBackend->setMaxMemorySize(p.max_memory);
	m_pState->pBackend->setBorderSize(p.border_size);
	m_pState->pBackend->setMinRegionSize(p.min_part_size, p.min_part_volume);
	m_pState->pBackend->setExpandMaskTimes(p.expand_mask);
	m_pState->pBackend->setNumberSeparationThreads(p.num_threads);
	if(p.histogram_analysis)
	{
		// histgram steps 1, 2 and 3
		m_pState->pBackend->setAutoScaling(false);
		m_pState->pBackend->setUseMultiScaleHistogram(3);
	}else{
		// build no histogram (threshold needed from extern!!!!)
		m_pState->pBackend->setAutoScaling(false);
		m_pState->pBackend->setUseMultiScaleHistogram(0);
	}
	m_pState->pBackend->setSearchForHighDensityCarrierMaterial(true);

	m_pState->max_memory = p.max_memory;
}

void ZeissSeparationInterface::doSeparation() {
	// - do both of the steps below
	doSeparation(getThreshold()); 
}

double ZeissSeparationInterface::getThreshold() {
	// - when input file is given, load input volume and get scalingfactor
	// - otherwise create own copy of m_pState->inputVolume and get the scaling factor
	// - create Histogramn Regions for the different matrials


	//load input file
	if (m_pState->inputFromFile) 
	{
		m_pState->pBackend->load();
		m_pState->scalingFactor = m_pState->pBackend->getScalingFactor();
	} 
	else //make a copy of inputVolume 
	{
		using namespace openOR;
		Volume v = m_pState->inputVolume;
		Math::Vector3ui size = Math::create<Math::Vector3ui>(v.size[0], v.size[1], v.size[2]);
		Math::Vector3d  sizeMM = Math::create<Math::Vector3d>( ((double)v.size[0]) * v.voxel_size[0],
			((double)v.size[1]) * v.voxel_size[1],
			((double)v.size[2]) * v.voxel_size[2]
		);

		std::tr1::shared_ptr<Image::Image3DSizeDescriptor> sizeDesc = createInstanceOf<Image::Image3DSizeDescriptor>();
		sizeDesc->setSize(size);
		sizeDesc->setSizeMM(sizeMM);

		std::tr1::shared_ptr<Image::DataCollector<uint16> > pVColl = m_pState->pBackend->createVolumeCollector(size);
		pVColl->setData(sizeDesc, "inSize");

		// transfer the Volume slicewise to prevent excessive memory consumption
		for (size_t i = 0; i < v.size[2]; ++i) {
			pVColl->setData(v.data + (i * (v.size[0] * v.size[1])), v.size[0] * v.size[1], "in");
			pVColl->operator()();

			if (m_pState->wasCanceled) { break; }
		}
		//get the scaling to obtain the correct number of multiresolution histograms
		if (std::tr1::shared_ptr<Image::VolCollUI16> pCol = interface_cast<Image::VolCollUI16>(pVColl)) {
			m_pState->scalingFactor = pCol->getScaling();
		}
	}
	//get material regions from multi resolution histograms
	if (!m_pState->wasCanceled) {
		m_pState->pBackend->createHistogramRegions();
	}

	return ((double)m_pState->pBackend->objectThreshold() / (double)std::numeric_limits<uint16_t>::max());
}

void ZeissSeparationInterface::doSeparation(double threshold) {
	// - calcultates Regions of interest for a given Threshold (overrides the Threshold)

	if (!m_pState->wasCanceled) {
		assert(threshold >= 0.0 && "Separation threshold cannot be below 0.0");
		assert(threshold <= 1.0 && "Separation threshold cannot exceed 1.0");

		m_pState->pBackend->overrideObjectThreshold((size_t)(threshold * (double)std::numeric_limits<uint16_t>::max()));
		m_pState->pBackend->calclulateRegionsOfInterest();

		//m_pState->scalingFactor = m_pState->pBackend->getScalingFactor();
	}
}

OutVolume ZeissSeparationInterface::getMaskVolume() const {
	// - returns the Volume from m_pState

	OutVolume out;

	out.size[0] = m_pState->pVolume->size()[0];
	out.size[1] = m_pState->pVolume->size()[1];
	out.size[2] = m_pState->pVolume->size()[2];

	out.voxel_size[0] = m_pState->pVolume->sizeMM()[0] / (double) out.size[0];
	out.voxel_size[1] = m_pState->pVolume->sizeMM()[1] / (double) out.size[1];
	out.voxel_size[2] = m_pState->pVolume->sizeMM()[2] / (double) out.size[2];

	m_pState->output = boost::shared_array<uint16_t>(m_pState->pVolume->mutableData(), AliasingDeleter<uint16_t>(m_pState->pVolume));
	out.data = m_pState->output;

	return out;
}

std::vector<ROI> ZeissSeparationInterface::getROIs(bool useOriginalVolumeSize /*= false*/) const {
	// - returns the regions of interest from m_pState

	std::vector<ROI> rois;

	Scaling s;
	if (useOriginalVolumeSize) {
		s.factor[0] = m_pState->scalingFactor[0];
		s.factor[1] = m_pState->scalingFactor[1];
		s.factor[2] = m_pState->scalingFactor[2];
	} else {
		s.factor[0] = 1;
		s.factor[1] = 1;
		s.factor[2] = 1;
	}

	for (unsigned int i = 0; i < m_pState->pRoi->size(); ++i) {
		openOR::Image::RegionOfInterest roi_in = m_pState->pRoi->operator()(i);

		ROI roi;
		roi.frontLowerLeft[0] = roi_in.frontLowerLeft()[0] * s.factor[0];
		roi.frontLowerLeft[1] = roi_in.frontLowerLeft()[1] * s.factor[1];
		roi.frontLowerLeft[2] = roi_in.frontLowerLeft()[2] * s.factor[2];

		roi.backUpperRight[0] = roi_in.backUpperRight()[0] * s.factor[0];
		roi.backUpperRight[1] = roi_in.backUpperRight()[1] * s.factor[1];
		roi.backUpperRight[2] = roi_in.backUpperRight()[2] * s.factor[2];

		roi.index = roi_in.index();

		rois.push_back(roi);
	}

	return rois;
}

uint16_t ZeissSeparationInterface::getBackgroundPeakValue() const {
	return m_pState->pBackend->background();
}

Scaling ZeissSeparationInterface::getMaskScaling() const {

	Scaling s;
	s.factor[0] = m_pState->scalingFactor[0];
	s.factor[1] = m_pState->scalingFactor[1];
	s.factor[2] = m_pState->scalingFactor[2];

	return s;
}

void ZeissSeparationInterface::saveParts(const std::string& filename /*= ""*/) {
	if (!filename.empty()) {
		m_pState->pBackend->setOutputFilename(filename);
	}
	m_pState->pBackend->separateAndSave();
}

void ZeissSeparationInterface::saveParts(const char* filename /*= ""*/) {
	std::string fn(filename);
	saveParts(fn);
}

