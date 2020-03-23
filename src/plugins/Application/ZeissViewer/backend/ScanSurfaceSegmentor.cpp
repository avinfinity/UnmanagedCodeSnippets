//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#include "openOR/ScanSurfaceSegmentor.hpp"
#include <openOR/Plugin/create.hpp> //openOR_core

#include <openOR/Image/ROIContainer.hpp> //Image_Regions

#include <set>

#include <openOR/Log/Logger.hpp> //openOR_core
#   define OPENOR_MODULE_NAME "Zeiss.Separator"
#   include <openOR/Log/ModuleFilter.hpp>
# include<openOR/cleanUpWindowsMacros.hpp>

//----------------------------------------------------------------
// commented the unused function : liveIDs()

//----------------------------------------------------------------



#define LOG_THROW_RUNTIME(level, fmt) { boost::format fmt__ = fmt; LOG(level, OPENOR_MODULE, fmt__); throw std::runtime_error(fmt__.str()); }

namespace openOR {

	ScanSurfaceSegmentor::ScanSurfaceSegmentor() :
m_pVolumeData(),
	m_objectThreshold(),
	m_pObjectBoundingBoxes(),
	m_pVolumeObjectMap(),
	m_voxelInMMSize(Math::create<Math::Vector3d>(0.0, 0.0, 0.0)),
	m_useMMParams(false),
	m_borderWidth(0),
	m_minROIEdgeLength(4),
	m_minROIVolume(200),
	m_canceled(false),
	m_prog_currentSlice(0),
	m_prog_numSlices(1), // to prevent div by zero!
	m_prog_msg("Segmentation")
{}

ScanSurfaceSegmentor::~ScanSurfaceSegmentor() {}

// cancelable interface
void ScanSurfaceSegmentor::cancel() { m_canceled = true; }
bool ScanSurfaceSegmentor::isCanceled() const { return m_canceled; }

// progressable interface
double ScanSurfaceSegmentor::progress() const {
	return ((double) m_prog_currentSlice / (double)m_prog_numSlices);
}

std::string ScanSurfaceSegmentor::description() const {
	return m_prog_msg;
}

void ScanSurfaceSegmentor::setData(const AnyPtr& data, const std::string& tag) {
	if (std::tr1::shared_ptr<Image::Image3DDataUI16> pData = interface_cast<Image::Image3DDataUI16>(data)) {
		if (!((tag == "map") || (tag == "out"))) {
			m_pVolumeData = pData;
		} else {
			m_pVolumeObjectMap = pData;
		}
	}
	if (std::tr1::shared_ptr<ROIContainer> pData = interface_cast<ROIContainer>(data)) {
		m_pObjectBoundingBoxes = pData;
	}   
}

void ScanSurfaceSegmentor::setData(size_t objectThreshold)  {
	m_objectThreshold = static_cast<uint16>(objectThreshold);
}

void ScanSurfaceSegmentor::setBorderWidth( unsigned int borderWidth ) {
	m_borderWidth = borderWidth;
}

void ScanSurfaceSegmentor::setMinROIEdgeLength(unsigned int edgeLength) { m_minROIEdgeLength = edgeLength; }
void ScanSurfaceSegmentor::setMinROIVolume(unsigned int volume) { m_minROIVolume = volume; }

void ScanSurfaceSegmentor::operator() () {
	// non-paralellel surface segmentation:
	// - decide if we want to use mm parameters and set the corresponding members
	// - scan the volume for objects

	LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg(">>> Start separation ..."));

	// calculate Voxel size in mm
	if (std::tr1::shared_ptr<Image::Image3DSize> pSize = interface_cast<Image::Image3DSize>(m_pVolumeData)) 
	{
		Math::Vector3d sizeMM = pSize->sizeMM();
		Math::Vector3ui sizeVoxel = pSize->size();
		if (!((sizeMM(0) == 0.0) || (sizeMM(1) == 0.0) || (sizeMM(2) == 0.0))) {
			m_voxelInMMSize = Math::create<Math::Vector3d>((sizeMM(0)/(double)sizeVoxel(0)),
				(sizeMM(1)/(double)sizeVoxel(1)),
				(sizeMM(2)/(double)sizeVoxel(2)));
			m_useMMParams = true;

			LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("Voxel size is (%1%, %2%, %3%)mm")
				% m_voxelInMMSize(0) % m_voxelInMMSize(1) % m_voxelInMMSize(2)
				);

		} else {
			m_voxelInMMSize = Math::create<Math::Vector3d>(0.0, 0.0, 0.0);
			m_useMMParams = false;
		}
	}
	
	LOG(Log::Level::Info, OPENOR_MODULE, Log::msg("Part Separation Parameters:\n"
		"  Blank Border Width = %1%%4%\n"
		"  Small Part Cutoff at\n"
		"    smallest edge smaller than %2%%4%\n"
		"    volume smaller than %3%%4%\n"
		)
		% m_borderWidth % m_minROIEdgeLength % m_minROIVolume
		% ((m_useMMParams)?std::string("mm"):std::string(" Voxel"))
		);
	// scan the volue for objects
	scanLine();

	LOG(Log::Level::Debug, OPENOR_MODULE,
		Log::msg("Separation Statistics:\n"
		"  object threshold density value: %1%\n"
		"  number of detected ROIs  : %2%\n")
		% m_objectThreshold
		% m_pObjectBoundingBoxes->size()
		);

	//liveIDs();

	LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("<<< separation done."));
	return;
}

bool ScanSurfaceSegmentor::isToSmall(const Image::RegionOfInterest& roi) {
	// - check the roi volume size and the roi edge length
	// - if they are below m_minROIVolume or m_minROIEdgeLength return false
	using namespace Math;
	Vector3ui fll = roi.frontLowerLeft();
	Vector3ui bur = roi.backUpperRight();

	Math::Vector3d size = create<Vector3d>(bur(0) - fll(0), bur(1) - fll(1), bur(2) - fll(2));

	if (m_useMMParams) {
		size = create<Vector3d>(size(0) * m_voxelInMMSize(0), size(1) * m_voxelInMMSize(1), size(2) * m_voxelInMMSize(2));
	}

	double volume = multipliedElements(size);
	double minWidth = std::min(size(0), std::min(size(1), size(2)));

	return ((volume < m_minROIVolume) || (minWidth < m_minROIEdgeLength));
}

//void ScanSurfaceSegmentor::liveIDs(unsigned int x, unsigned int y, unsigned int z) {
//
//	typedef std::set<ObjectIdType> IDset;
//
//	IDset mapIds;
//	Math::Vector3ui volumeSize = m_pVolumeObjectMap->size();
//	if (z == 0) {	x = volumeSize(0)-1; 
//					y = volumeSize(1)-1; 
//					z = volumeSize(2)-1; }
//
//	for (unsigned int slice = 0; slice < std::min(volumeSize(2), z+1); ++slice)   
//	{
//		for (unsigned int column = 0; column < ((slice != z)?volumeSize(1):(y+1)); ++column) //TODO: ternary operator ersetzen? eigentlich sehr elegant hier
//		{
//			for (unsigned int row = 0; row < ((slice != z)?volumeSize(0):((column !=y)?volumeSize(0):(x+1))); ++row)  
//			{
//				mapIds.insert(getObjectId(row, column, slice));
//			}
//		}
//	}
//
//	IDset roiIds;
//	LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("Final Regions"));
//	for (size_t i = 0; i < m_pObjectBoundingBoxes->size(); ++i) {
//		Image::RegionOfInterest roi = m_pObjectBoundingBoxes->operator()(i);
//		roiIds.insert(roi.index());
//	}
//
//	IDset onlyRoi;
//	std::set_difference(roiIds.begin(), roiIds.end(),
//		mapIds.begin(), mapIds.end(),
//		std::inserter(onlyRoi, onlyRoi.begin()));
//
//	IDset onlyMap;
//	std::set_difference(mapIds.begin(), mapIds.end(),
//		roiIds.begin(), roiIds.end(),
//		std::inserter(onlyMap, onlyMap.begin()));
//
//	if (!onlyRoi.empty() && !onlyMap.empty()) {
//		LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("Region Mismatch"));
//
//		std::cout << "  This regions are only in the ROI list:" << std::endl;
//		for (IDset::const_iterator it=onlyRoi.begin(), end=onlyRoi.end(); it != end; ++it) {
//			std::cout << *it << ", ";
//		}
//		std::cout << std::endl;
//
//		std::cout << "  This regions are only in the map:" << std::endl;
//		for (IDset::const_iterator it=onlyMap.begin(), end=onlyMap.end(); it != end; ++it) {
//			std::cout << *it << ", ";
//		}
//		std::cout << std::endl;
//	}
//
//	return;
//}

namespace {
	unsigned int enlargeBB(unsigned int currentPos, unsigned int border) {
		// - enlarge the border with zeros

		//changed: return (currentPos < border) ? 0 : currentPos-border;
		if (currentPos < border) { return 0;}
		else { return currentPos-border;}
	}
}

void ScanSurfaceSegmentor::scanLine() {
	// - in each slice process all the voxels 
	// - analyse the neighboring 13 voxels
	// - do post processing and clean up: enlarge the border of each region with zeros

	assert(m_pVolumeData && "scanLine needs an input Volume.");
	assert(m_pObjectBoundingBoxes && "scanline needs primary output");

	// build a object map
	if (!m_pVolumeObjectMap) {
		LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("No ObjectMap provided, creating my own!"));
		m_pVolumeObjectMap = createInstanceOf<ObjectMapType>();
	}

	// set the number of slices
	Math::Vector3ui volumeSize = m_pVolumeData->size();
	//changed: m_prog_numSlices = volumeSize(2)?volumeSize(2):1;
	if (volumeSize(2)) {m_prog_numSlices = volumeSize(2);}
	else {m_prog_numSlices = 1;}
	m_prog_currentSlice = 0;

	m_pObjectBoundingBoxes->deferUpdateSignals(); // as an optimization we stop sending any signals until we are finished

	if (m_pVolumeObjectMap != m_pVolumeData) {
		// Create a ObjectMap that has the same size as the input
		m_pVolumeObjectMap->setSize(m_pVolumeData->size());
		m_pVolumeObjectMap->setSizeMM(m_pVolumeData->sizeMM());
	}

	// process each Voxel
	for (unsigned int slice = 0; slice < volumeSize(2); ++slice) {
		for (unsigned int column = 0; column < volumeSize(1); ++column) {
			for (unsigned int row = 0; row < volumeSize(0); ++row) {

				processVoxel(row, column, slice);

			}
		}
		++m_prog_currentSlice;
		if (m_canceled) { break; } // we simply stop processing if we are canceled
	}

	// region post processing and cleanup
	LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("Final Regions"));
	for (size_t i = 0; i < m_pObjectBoundingBoxes->size(); /* increment at the end because of erase! */) {

		Image::RegionOfInterest roi = m_pObjectBoundingBoxes->operator()(i);

		if (roi.index() == 0 || isToSmall(roi)) {
			m_pObjectBoundingBoxes->erase(i);
		} else {
			// Add empty border space to each region
			using namespace Math;
			Vector3ui fll = roi.frontLowerLeft();
			Vector3ui bur = roi.backUpperRight();

			// can't use max() due to unsignedness
			//changed: roi.setFrontLowerLeft(create<Vector3ui>(
			//	enlargeBB(fll(0), ((m_useMMParams)?(unsigned int)(m_borderWidth / m_voxelInMMSize(0)):m_borderWidth)),
			//	enlargeBB(fll(1), ((m_useMMParams)?(unsigned int)(m_borderWidth / m_voxelInMMSize(1)):m_borderWidth)),
			//	enlargeBB(fll(2), ((m_useMMParams)?(unsigned int)(m_borderWidth / m_voxelInMMSize(2)):m_borderWidth))));

			if(m_useMMParams) {roi.setFrontLowerLeft(create<Vector3ui>(	
				enlargeBB(fll(0), (m_borderWidth / m_voxelInMMSize(0))), 
				enlargeBB(fll(1), (m_borderWidth / m_voxelInMMSize(1))), 
				enlargeBB(fll(2), (m_borderWidth / m_voxelInMMSize(2)))));}
			else {roi.setFrontLowerLeft(create<Vector3ui>(enlargeBB(fll(0),m_borderWidth), enlargeBB(fll(1),m_borderWidth), enlargeBB(fll(2),m_borderWidth)));}

			//changed: roi.setBackUpperRight(create<Vector3ui>(
			//	std::min(bur(0) + 1 + ((m_useMMParams)?(unsigned int)(m_borderWidth / m_voxelInMMSize(0)):m_borderWidth), volumeSize(0)),
			//	std::min(bur(1) + 1 + ((m_useMMParams)?(unsigned int)(m_borderWidth / m_voxelInMMSize(1)):m_borderWidth), volumeSize(1)),
			//	std::min(bur(2) + 1 + ((m_useMMParams)?(unsigned int)(m_borderWidth / m_voxelInMMSize(2)):m_borderWidth), volumeSize(2))));

			if(m_useMMParams) {roi.setBackUpperRight(create<Vector3ui>(	
				std::min(bur(0) + 1 + (unsigned int) (m_borderWidth / m_voxelInMMSize(0)), volumeSize(0)),
				std::min(bur(1) + 1 + (unsigned int) (m_borderWidth / m_voxelInMMSize(1)), volumeSize(1)),
				std::min(bur(2) + 1 + (unsigned int) (m_borderWidth / m_voxelInMMSize(2)), volumeSize(2))));}
			else {std::min(bur(0) + 1 + (m_borderWidth), volumeSize(0)), std::min(bur(1) + 1 + (m_borderWidth), volumeSize(1)), std::min(bur(2) + 1 + (m_borderWidth), volumeSize(2));}

			m_pObjectBoundingBoxes->set(i, roi);

			LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("  region %1% = ((%2%, %3%, %4%), (%5%, %6%, %7%))")
				% roi.index()
				% roi.frontLowerLeft()(0) % roi.frontLowerLeft()(1) % roi.frontLowerLeft()(2)
				% roi.backUpperRight()(0) % roi.backUpperRight()(1) % roi.backUpperRight()(2)
				);

			++i; // done only if we are not erasing!
		}
	}

	m_pObjectBoundingBoxes->resumeUpdateSignals(); // inform the world that we are done!

	return;
}

void ScanSurfaceSegmentor::processVoxel(unsigned int x, unsigned int y, unsigned int z) {
	// This is the most heartpiece of the algorithm.
	// the scan moves along through each voxel, and checks if a it hits an object.
	// if so it creates a new region or
	// if any neighbor already was a region this region is grown.
	// if more than one region is adjacent to this voxel these regions are merged.

	if (pixelIsObject(x,y,z)) {
		std::vector<ObjectIdType> neighborObjects = neighborIsObject(x, y, z);

		if (neighborObjects.empty()) {
			if ( std::numeric_limits<ObjectIdType>::max() <= m_pObjectBoundingBoxes->size() ) {
				// the only real downside of this algorithm is that we can run out of object ids
				// if there are too many region simultaneously alive.
				// This can happen in large grids, for example.
				// to make matters work we do not clean up dead regions imedeately, for efficiency.
				// so in case we do run out we need to clean those up!

				compactRegions(x, y, z);

				if ( std::numeric_limits<ObjectIdType>::max() <= m_pObjectBoundingBoxes->size() ) {
					// if we still have no available id we are out of luck!

					LOG_THROW_RUNTIME(Log::Level::Error, Log::msg("[ScanSurfaceSegmentor] Out of object ids"));
				}
			}

			// create a new object
			ObjectIdType id = m_pObjectBoundingBoxes->size()+1;

			Image::RegionOfInterest roi;
			roi.setFrontLowerLeft(Math::create<Math::Vector3ui>(x,y,z));
			roi.setBackUpperRight(Math::create<Math::Vector3ui>(x,y,z));
			roi.setIndex(id);

			m_pObjectBoundingBoxes->add(roi);

			writeOut(id, x, y, z);
		} else {
			// grow an existing object
			ObjectIdType id = neighborObjects.back();
			neighborObjects.pop_back();

			growROI(id, x, y, z);
			writeOut(id, x, y, z);

			// merge with other adjacent regions
			for ( ; !neighborObjects.empty(); neighborObjects.pop_back()) {
				id = connectROI(id, neighborObjects.back());
			}
		}
	} else {
		writeOut(0, x, y, z);
	}

	return;
}

bool ScanSurfaceSegmentor::pixelIsObject(unsigned int x, unsigned int y, unsigned int z) {
	const Math::Vector3ui size = m_pVolumeData->size();
	return (m_pVolumeData->data()[static_cast<size_t>(z) * size(1) * size(0) + y * size(0) + x] > m_objectThreshold);
}

namespace {
	void pushIfNew(std::vector<uint16>& c, uint16 id) {  
		if (std::find(c.begin(), c.end(), id) == c.end()) { c.push_back(id); }
	}
}

std::vector<ScanSurfaceSegmentor::ObjectIdType> ScanSurfaceSegmentor::neighborIsObject(unsigned int x, unsigned int y, unsigned int z) {
		std::vector<ObjectIdType> result;
		// - if neighbouring voxels are objects add them to the result

		// There can never be more than 4 neighbours, so we
		// reserve them up front.
		result.reserve(4); // TODO Falsch? Es werden 13 nachbarn angeschaut!

		if (ObjectIdType id = getObjectId(((int)x)-1, y, z))                           { pushIfNew(result, id); }
		for (int dx = -1; dx <= 1; ++dx) {
			if (ObjectIdType id = getObjectId(((int)x)+dx, ((int)y)-1, z))              { pushIfNew(result, id); }
		}
		for (int dy = -1; dy <= 1; ++dy) {
			for (int dx = -1; dx <= 1; ++dx) {
				if (ObjectIdType id = getObjectId(((int)x)+dx, ((int)y)+dy, ((int)z)-1)) { pushIfNew(result, id); }
			}
		}

		return result;
}

bool ScanSurfaceSegmentor::cannotConnect(const Image::RegionOfInterest& roi, unsigned int x, unsigned int y, unsigned int z) {
	// a region is closed (will not be connected or grown any more)
	// if it is too far behind the currently processed voxel.
	// NOTE: if the neighborIsObject() is changed, the value of what is
	// considered too far (connectRadius) needs to be changed!
	const unsigned int connectRadius = 2;

	if (connectRadius < z && roi.backUpperRight()(2) < (z - connectRadius)) { return true; }

	return false;
}

void ScanSurfaceSegmentor::writeOut(ObjectIdType id, unsigned int x, unsigned int y, unsigned int z) {
	const Math::Vector3ui size = m_pVolumeObjectMap->size();
	m_pVolumeObjectMap->mutableData()[static_cast<size_t>(z) * size(1) * size(0) + y * size(0) + x] = id;
}

ScanSurfaceSegmentor::ObjectIdType ScanSurfaceSegmentor::getObjectId(unsigned int x, unsigned int y, unsigned int z) {
		const Math::Vector3ui size = m_pVolumeObjectMap->size();

		// we do not need to check lower bounds here because of the unsigned data types
		// (and yes, that is well defined)
		// make sure that we are in the correct part
		if (static_cast<unsigned int>(size(0)) <= x || static_cast<unsigned int>(size(1)) <= y || static_cast<unsigned int>(size(2)) <= z) { return 0; }
		return m_pVolumeObjectMap->data()[static_cast<size_t>(z) * size(1) * size(0) + y * size(0) + x];
}

namespace {
	Math::Vector3ui vecMin(Math::Vector3ui lhs, Math::Vector3ui rhs) {
		return Math::create<Math::Vector3ui>(std::min(lhs(0), rhs(0)), std::min(lhs(1), rhs(1)), std::min(lhs(2), rhs(2)));
	}

	Math::Vector3ui vecMax(Math::Vector3ui lhs, Math::Vector3ui rhs) {
		return Math::create<Math::Vector3ui>(std::max(lhs(0), rhs(0)), std::max(lhs(1), rhs(1)), std::max(lhs(2), rhs(2)));
	}
}

void ScanSurfaceSegmentor::growROI(ObjectIdType id, unsigned int x, unsigned int y, unsigned int z) {
	// - grow the region if the indices x,y,z lie outside given region of interest
	Image::RegionOfInterest roi = m_pObjectBoundingBoxes->operator()(id-1);

	roi.setFrontLowerLeft(vecMin(roi.frontLowerLeft(), Math::create<Math::Vector3ui>(x, y, z)));
	roi.setBackUpperRight(vecMax(roi.backUpperRight(), Math::create<Math::Vector3ui>(x, y, z)));

	m_pObjectBoundingBoxes->set(id-1, roi);
}

ScanSurfaceSegmentor::ObjectIdType ScanSurfaceSegmentor::connectROI(ObjectIdType c1, ObjectIdType c2) {

	// - connect two regions and take the lower id for the merged regions
	// - merge the two regions sizes
	// - set the other region (region with the higher id) to zero ("dead")

	assert((c1 != 0) && (c2 != 0) && (c1 != c2) && "wrong connection");
	// always swallow the region with the higher id
	ObjectIdType original = std::min(c1, c2);
	ObjectIdType other    = std::max(c1, c2);

	Image::RegionOfInterest roi = m_pObjectBoundingBoxes->operator()(original-1);
	Image::RegionOfInterest otherRoi = m_pObjectBoundingBoxes->operator()(other-1);

	// merge region sizes
	roi.setFrontLowerLeft(vecMin(roi.frontLowerLeft(), otherRoi.frontLowerLeft()));
	roi.setBackUpperRight(vecMax(roi.backUpperRight(), otherRoi.backUpperRight()));

	m_pObjectBoundingBoxes->set(original-1, roi);

	assert((otherRoi.index() == 0) || (otherRoi.index() == other) && "Index mismatch 1!");

	rewriteRegionId(otherRoi, original);
	assert(otherRoi.index() == original && "Index mismatch 2!");

	// this region is dead, i.e. it is not refererd in the map any more, so we mark it as a gooner
	otherRoi.setIndex(0);

	assert(otherRoi.index() == 0 && "Index mismatch 3!");

	m_pObjectBoundingBoxes->set(other-1, otherRoi);

	return original;
}

void ScanSurfaceSegmentor::rewriteRegionId(Image::RegionOfInterest& roi, ObjectIdType newId) {
	// - write a new id to the the given region of interest

	assert(roi.index() != 0 && "rewriting background not allowed!");

	for (unsigned int z = roi.frontLowerLeft()(2); z <= roi.backUpperRight()(2); ++z) {
		for (unsigned int y = roi.frontLowerLeft()(1); y <= roi.backUpperRight()(1); ++y) {
			for (unsigned int x = roi.frontLowerLeft()(0); x <= roi.backUpperRight()(0); ++x) {
				if (getObjectId(x, y, z) == roi.index()) { writeOut(newId, x, y, z); }
			}
		}
	}

	roi.setIndex(newId);
}

void ScanSurfaceSegmentor::compactRegions(unsigned int x, unsigned int y, unsigned int z) {
	// - remove regions that are not needed anymore or regions that are to small
	// - re write the region ids, so that we always have the lowest possible id

	LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("  Compacting regions"));

	// keep showing progress
	// Todo: make it possible to cancel from here, should probably be done by using exceptions
	//       needs carefull testing.
	m_prog_msg = "Compacting Regions";
	unsigned int savedCurrentSlice = m_prog_currentSlice;
	unsigned int tmpMaxSlice = m_prog_numSlices - savedCurrentSlice;
	if (tmpMaxSlice > 8) { tmpMaxSlice -= 5; }

	size_t dbgCountBefore = m_pObjectBoundingBoxes->size();

	for (size_t i = 0; i < m_pObjectBoundingBoxes->size(); /* increment at the end because of erase! */) {
		m_prog_currentSlice = savedCurrentSlice + (tmpMaxSlice * ((double) i / (double) m_pObjectBoundingBoxes->size()));

		Image::RegionOfInterest roi = m_pObjectBoundingBoxes->operator()(i);
		if (roi.index() == 0) {
			// remove "dead" regions from the list
			m_pObjectBoundingBoxes->erase(i);

		} else if (isToSmall(roi) && cannotConnect(roi, x, y, z)) {
			// we want to remove small regions, that are too small to be relevant
			// and cannot connect to active regions any more (because they are too
			// far away from the action).
			rewriteRegionId(roi, 0);
			m_pObjectBoundingBoxes->erase(i);

		} else {
			// these are the regions which are alive and interesting, we want to keep
			// them, but we rewrite the id to have the lowest available id.
			rewriteRegionId(roi, i+1);

			m_pObjectBoundingBoxes->set(i, roi);

			++i; // done only if we are not erasing!
		}
	}

	m_prog_currentSlice = savedCurrentSlice; // set progress to where we were before.
	m_prog_msg = "Segmentation";

	LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("    kept %1% of %2% regions (removed %3%%% dead regions)")
		% m_pObjectBoundingBoxes->size() % dbgCountBefore
		% (int)((1.0-((double) m_pObjectBoundingBoxes->size() / (double) dbgCountBefore)) * 100)
		);

	//liveIDs(x,y,z);

	LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("  compacting done go on separating ..."));
	return;
}

}


