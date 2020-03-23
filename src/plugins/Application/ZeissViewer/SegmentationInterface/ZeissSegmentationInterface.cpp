// (c) 2012 by Fraunhofer IPK

#include "ZeissSegmentationInterface.hpp"

#include <openOR/ZeissBackend.hpp>
#include <openOR/Image/Image3DData.hpp> //Image_ImageData
#include <openOR/Image/VolumeCollector.hpp> // Image_Utility
#include <openOR/Image/HistogramCollector.hpp> // Image_ Utility
#include <openOR/Image/Image3DSizeDescriptor.hpp> //Image_ImageData
#include <openOR/Plugin/create.hpp> // openOR_core

#include <cstring>
#include <openOR/cleanUpWindowsMacros.hpp>

//#include <boost/date_time/posix_time/posix_time.hpp>

template<typename T>
struct AliasingDeleter {
	AliasingDeleter(std::tr1::shared_ptr<openOR::Image::Image3DDataUI16> i) : image(i) {}

	void operator() (T*& ptr) {
		image.reset();
	}

private:
	std::tr1::shared_ptr<openOR::Image::Image3DDataUI16> image;
};

struct ZeissSegmentationInterfaceInternalState {
	ZeissSegmentationInterfaceInternalState() :
pBackend(new openOR::ZeissBackend()),
	wasCanceled(false),
	pVolume(openOR::createInstanceOf<openOR::Image::Image3DDataUI16>()),
	pHistogram(openOR::createInstanceOf<openOR::Image::Image1DDataUI64>()),
	pHistogram2(openOR::createInstanceOf<openOR::Image::Image1DDataUI64>()),
	pHistogram4(openOR::createInstanceOf<openOR::Image::Image1DDataUI64>()),
	pRegions(openOR::createInstanceOf<openOR::Image::Image1DData<openOR::Triple<size_t> > >()),
	cachedRegionsValid(false),
	cachedRegions(),
	cachedResult(false),
	output(boost::shared_array<uint16_t>(pVolume->mutableData(), AliasingDeleter<uint16_t>(pVolume))),
	inputVolume(),
	scalingFactor(openOR::Math::create<openOR::Math::Vector3ui>(1, 1, 1)),
	max_memory(std::numeric_limits<size_t>::max())
{
	pHistogram->setSize(static_cast<openOR::uint32>(std::numeric_limits<openOR::uint16>::max()) + 1);
	pBackend->setVolume(pVolume);
	pBackend->setOutputVolume(pVolume);
	pBackend->setData(pHistogram);
	pBackend->setData(pHistogram2);
	pBackend->setData(pHistogram4);
	pBackend->setData(pRegions);
	pBackend->setUseMultiScaleHistogram(4);
	pBackend->setAutoScaling(true);
}
~ZeissSegmentationInterfaceInternalState() { 
	delete pBackend; 
}

openOR::ZeissBackend* pBackend;
bool wasCanceled;
std::tr1::shared_ptr<openOR::Image::Image3DDataUI16> pVolume;
std::tr1::shared_ptr<openOR::Image::Image1DDataUI64> pHistogram;
std::tr1::shared_ptr<openOR::Image::Image1DDataUI64> pHistogram2;
std::tr1::shared_ptr<openOR::Image::Image1DDataUI64> pHistogram4;
std::tr1::shared_ptr<openOR::Image::Image1DData<openOR::Triple<size_t> > > pRegions;
bool cachedRegionsValid;
Materials cachedRegions;
bool cachedResult;
boost::shared_array<uint16_t> output;
Volume inputVolume;
openOR::Math::Vector3ui scalingFactor;

size_t max_memory;
};

ZeissSegmentationInterface::ZeissSegmentationInterface() : m_pState(new ZeissSegmentationInterfaceInternalState()) {}
ZeissSegmentationInterface::~ZeissSegmentationInterface() { delete m_pState; }

double ZeissSegmentationInterface::progress() const {
	return m_pState->pBackend->progress();
}

void ZeissSegmentationInterface::cancel() {
	m_pState->pBackend->cancel();
	m_pState->wasCanceled = true;
	m_pState->cachedRegionsValid = false;
	m_pState->cachedResult = false;
}


void ZeissSegmentationInterface::setInputVolume( const Volume& v ) {
	m_pState->inputVolume = v;
	m_pState->wasCanceled = false;
	m_pState->cachedRegionsValid = false;
	m_pState->cachedResult = false;
}

void ZeissSegmentationInterface::setParameter( const SegmentationParameter& p ) {
	m_pState->pBackend->setMaxMemorySize(p.max_memory);
	m_pState->pBackend->setOutputMaterialIndex(p.output_material_index);
	m_pState->pBackend->setSegmentationRadius(p.segmentation_radius);


	m_pState->max_memory = p.max_memory;
	m_pState->wasCanceled = false;
	m_pState->cachedRegionsValid = false;
	m_pState->cachedResult = false;
}

void ZeissSegmentationInterface::doSegmentation() {
	// - do segmentation (both steps below)
	doSegmentation(getMaterialRegions());
}

Materials ZeissSegmentationInterface::getMaterialRegions() {
	// - create a copy of the volume to work with
	// - obtain the scaling factor and calculate the material regions
	// - set for each region the peaks position and upper/lower bound and store them to cachedRegions

	if (!m_pState->cachedRegionsValid) {

		using namespace openOR;
		//get Volume and Voxelsize
		Volume v = m_pState->inputVolume;
		Math::Vector3ui size = Math::create<Math::Vector3ui>(v.size[0], v.size[1], v.size[2]);
		Math::Vector3d  sizeMM = Math::create<Math::Vector3d>( ((double)v.size[0]) * v.voxel_size[0],
			((double)v.size[1]) * v.voxel_size[1],
			((double)v.size[2]) * v.voxel_size[2]
		);

		//create a copy of the volume for further processing
		std::tr1::shared_ptr<Image::Image3DSizeDescriptor> sizeDesc = createInstanceOf<Image::Image3DSizeDescriptor>();
		sizeDesc->setSize(size);
		sizeDesc->setSizeMM(sizeMM);

		std::tr1::shared_ptr<Image::DataCollector<uint16> > pVColl = m_pState->pBackend->createVolumeCollector(size);
		pVColl->setData(sizeDesc, "inSize");

		//transfer the Volume slicewise to prevent excessive memory consumption
		for (size_t i = 0; i < v.size[2]; ++i) 
		{
			pVColl->setData(v.data + (i * (v.size[0] * v.size[1])), v.size[0] * v.size[1], "in");
			pVColl->operator()();

			if ( m_pState->wasCanceled ) 
			{ 
				break; 
			}
		}
		//get the scaling to obtain the correct number of multi resolution histograms
		std::tr1::shared_ptr<Image::VolCollUI16> pCol = interface_cast<Image::VolCollUI16>(pVColl);
		m_pState->scalingFactor = pCol->getScaling();


		if ( !m_pState->wasCanceled ) 
		{

			//get material regions from the multi resolution histograms
			m_pState->pBackend->createHistogramRegions();

			size_t regionCount = getWidth(m_pState->pRegions);

			m_pState->cachedRegions.first_material_index = m_pState->pBackend->firstMaterialIndex();
			m_pState->cachedRegions.regions.clear();

			//set for each region the peak postion and upper/lower bound and store them to cachedRegions
			for (size_t i = 0; i < regionCount; i++) 
			{
				openOR::Triple<size_t> element = m_pState->pRegions->data()[i];
				
				MaterialRegion r;

				r.lower_bound = element.first;
				r.peak = element.second;
				r.upper_bound = element.third;

				m_pState->cachedRegions.regions.push_back(r);
			}

			m_pState->cachedRegionsValid = true;
		}
	}
	return m_pState->cachedRegions;
}

void ZeissSegmentationInterface::doSegmentation(const Materials& materials) {
	// - for the given Histogramregions segment the materials
	if ( m_pState->wasCanceled ) { return; }

	if (!m_pState->cachedResult) {

		//store materials to m_pState
		m_pState->pRegions->setSize(materials.regions.size());

		for (size_t i = 0; i < materials.regions.size(); i++) 
		{
			m_pState->pRegions->mutableData()[i] = openOR::Triple<size_t>(
				materials.regions[i].lower_bound,
				materials.regions[i].peak,
				materials.regions[i].upper_bound );
		}
		//segment materials
		m_pState->pBackend->setFirstMaterialIndex(materials.first_material_index);
		m_pState->pBackend->segmentMaterials();
		m_pState->cachedResult = true;
	}

	return;
}

//namespace {
//   Histogram copyHistogram(std::tr1::shared_ptr<openOR::Image::Image1DDataUI64> pHistogram) {
//      Histogram hist;
//      memcpy(hist.data, pHistogram->dataPtr(), pHistogram->size() * sizeof(uint64_t));
//      return hist;
//   }
//}

//std::vector<Histogram> ZeissSegmentationInterface::getHistograms() {
//   std::vector<Histogram> hist;
//
//   hist.push_back(copyHistogram(m_pState->pHistogram));
//   hist.push_back(copyHistogram(m_pState->pHistogram2));
//   hist.push_back(copyHistogram(m_pState->pHistogram4));
//
//   return hist;
//}


MultiResolutionHistograms ZeissSegmentationInterface::getMultiResolutionHistograms() {
	// get the three histograms
	MultiResolutionHistograms hist;

	memcpy(hist.first_histogram_data, m_pState->pHistogram->dataPtr(), m_pState->pHistogram->size() * sizeof(uint64_t));
	memcpy(hist.second_histogram_data, m_pState->pHistogram2->dataPtr(), m_pState->pHistogram2->size() * sizeof(uint64_t));
	memcpy(hist.third_histogram_data, m_pState->pHistogram4->dataPtr(), m_pState->pHistogram4->size() * sizeof(uint64_t));

	return hist;
}


OutVolume ZeissSegmentationInterface::getSegmentVolume() const {

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

Scaling ZeissSegmentationInterface::getScaling() const {

	Scaling s;
	s.factor[0] = m_pState->scalingFactor[0];
	s.factor[1] = m_pState->scalingFactor[1];
	s.factor[2] = m_pState->scalingFactor[2];

	return s;
}

double ZeissSegmentationInterface::doBenchmark(bool reset) {
	// - same code as in doSegmentation() only with timings 

	//boost::posix_time::ptime timeStart(boost::posix_time::microsec_clock::local_time());
	Materials mat = getMaterialRegions();
	//boost::posix_time::ptime timeHist(boost::posix_time::microsec_clock::local_time());

	//m_pState->pBackend->setHistTime(static_cast<int>((timeHist-timeStart).total_seconds()));

	doSegmentation(mat);
	if (!m_pState->wasCanceled)
		return m_pState->pBackend->saveBenchmark(reset); // save measured time to SegmentationBenchmark.xml file

	return -6;
}

double ZeissSegmentationInterface::getSegmentationTimeEstimation(){
	return m_pState->pBackend->getSegmentationTimeEstimation(-1);
}

double ZeissSegmentationInterface::getSegmentationTimeEstimation(const Materials& materials){
	return m_pState->pBackend->getSegmentationTimeEstimation(materials.regions.size());
}

double ZeissSegmentationInterface::getHistogramTimeEstimation(){
	return m_pState->pBackend->getMaterialRegionsTimeEstimation(m_pState->inputVolume.size[0], m_pState->inputVolume.size[1], m_pState->inputVolume.size[2]);
}

void ZeissSegmentationInterface::setBenchmarkPath(char* path, size_t size)
{
	m_pState->pBackend->setBenchmarkFile(path, size);
}