//****************************************************************************
// (c) 2014 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR commercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#include "openOR/MTScanSurfaceSegmentor.hpp"
#include <openOR/Plugin/create.hpp> //openOr_core

#include <openOR/Image/ROIContainer.hpp> //Image_Regions

#include <set>

#include <boost/shared_array.hpp>

#ifdef _OPENMP
#  include <omp.h>
#endif

#include <openOR/Log/Logger.hpp> //openOR_core
#   define OPENOR_MODULE_NAME "Zeiss.MTSeparator"
#   include <openOR/Log/ModuleFilter.hpp>
# include<openOR/cleanUpWindowsMacros.hpp>

#define LOG_THROW_RUNTIME(level, fmt) { boost::format fmt__ = fmt; LOG(level, OPENOR_MODULE, fmt__); throw std::runtime_error(fmt__.str()); }

namespace openOR {

	MTScanSurfaceSegmentor::MTScanSurfaceSegmentor() :
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
	m_prog_msg("Segmentation"),
	m_numSubVolumes(1),
	m_zero(0),
	m_maxObjectId(std::numeric_limits<ObjectIdType>::max())
{}

MTScanSurfaceSegmentor::~MTScanSurfaceSegmentor() {}

// cancelable interface
void MTScanSurfaceSegmentor::cancel() { m_canceled = true; }
bool MTScanSurfaceSegmentor::isCanceled() const { return m_canceled; }

// progressable interface
double MTScanSurfaceSegmentor::progress() const {
	return ((double) m_prog_currentSlice / (double) m_prog_numSlices);
}

std::string MTScanSurfaceSegmentor::description() const {
	return m_prog_msg;
}

void MTScanSurfaceSegmentor::setData(const AnyPtr& data, const std::string& tag) {
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

void MTScanSurfaceSegmentor::setData(size_t objectThreshold)  {
	m_objectThreshold = static_cast<uint16>(objectThreshold);
}

void MTScanSurfaceSegmentor::setBorderWidth(unsigned int borderWidth) {
	m_borderWidth = borderWidth;
}

void MTScanSurfaceSegmentor::setMinROIEdgeLength(unsigned int edgeLength) { m_minROIEdgeLength = edgeLength; }
void MTScanSurfaceSegmentor::setMinROIVolume(unsigned int volume) { m_minROIVolume = volume; }
void MTScanSurfaceSegmentor::setNumberSubvolumes(unsigned int numSubvolumes) { m_numSubVolumes = numSubvolumes; }

namespace {
	unsigned int enlargeBB2(unsigned int currentPos, unsigned int border) {
		// - Enlarge the border with zeros

		// changed: return (currentPos < border) ? 0 : currentPos - border;
		if (currentPos < border){return 0;}
		else {return currentPos - border;}
	}
}

void MTScanSurfaceSegmentor::operator() () { //TODO: vielleicht Namen ändern: MTScanSurfaceSeparator, da hier die Separierung in Teilvolumen stattfindet
	
	// Parallel Surface Segmentation: 
	// - decide if we want to use mm parameters and set the corresponding members
	// - create object map and subvolume parts for parallel computation
	// - iterate over the parts and scan for objects
	// - merge the regions in the neighbouring parts if they are connected
	// - do region post processing and clean up: delete small regions, add empty border spaces to the regions

	std::cout << "separation" << std::endl;

	LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg(">>> Start separation ..."));

	// calculate Voxel size in mm, decide if we want to use paremeter in mm
	if (std::tr1::shared_ptr<Image::Image3DSize> pSize = interface_cast<Image::Image3DSize>(m_pVolumeData)) {
		Math::Vector3d sizeMM = pSize->sizeMM();
		Math::Vector3ui sizeVoxel = pSize->size();
		if (!((sizeMM(0) == 0.0) || (sizeMM(1) == 0.0) || (sizeMM(2) == 0.0))) {
			m_voxelInMMSize = Math::create<Math::Vector3d>((sizeMM(0)/(double)sizeVoxel(0)), (sizeMM(1)/(double)sizeVoxel(1)), (sizeMM(2)/(double)sizeVoxel(2)));
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

	assert(m_pVolumeData && "scanLine needs an input Volume.");
	assert(m_pObjectBoundingBoxes && "scanline needs primary output");

	// create object map member
	if (!m_pVolumeObjectMap) {
		LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("No ObjectMap provided, creating my own!"));
		m_pVolumeObjectMap = createInstanceOf<ObjectMapType>();
	}
	// set number of sclices
	Math::Vector3ui volumeSize = m_pVolumeData->size();
	//changed: m_prog_numSlices = volumeSize(2) ? volumeSize(2) : 1;
	if (volumeSize(2)) { 
		m_prog_numSlices = volumeSize(2);}
	else { 
		m_prog_numSlices = 1;}
	m_prog_currentSlice = 0;

	if (m_pVolumeObjectMap != m_pVolumeData) {
		// Create a ObjectMap that has the same size as the input
		m_pVolumeObjectMap->setSize(m_pVolumeData->size());
		m_pVolumeObjectMap->setSizeMM(m_pVolumeData->sizeMM());
	}

	m_width = volumeSize(0);
	m_height = volumeSize(1);
	m_depth = volumeSize(2);

	m_pVolumeObjectDataPtr = m_pVolumeObjectMap->mutableData();
	m_pVolumeDataPtr = m_pVolumeData->mutableData();

	//part.rois.deferUpdateSignals(); // as an optimization we stop sending any signals until we are finished

	//create subvolume parts 
	std::vector<SubVolumePartitioning> parts;
	size_t partSize = m_depth / m_numSubVolumes;
	for (int i = 0; i < m_numSubVolumes; i++) {
		SubVolumePartitioning part;
		if (i == 0) part.start = 0;
		else part.start = parts.back().end;

		if (i < (m_numSubVolumes - 1)) part.end = part.start + partSize;
		else part.end = m_depth;

		parts.push_back(part);
	}

	bool success = true;
	std::runtime_error err("void");

#ifdef _OPENMP
#  pragma omp parallel for num_threads(m_numSubVolumes) reduction(&:success) shared(err)
#endif
	// iterate over the subvolume parts and scan each one
	for (int i = 0; i < m_numSubVolumes; i++) {
		try {
			scanLine(parts[i]);
			success &= true;
		} catch (const std::runtime_error& e) {
			success &= false;
			err = e;
		}
	}

	if (!success)
		throw err;

	m_prog_currentSlice = m_depth;

	std::vector<Image::RegionOfInterest> rois;
	// merge the regions in neighbouring parts
	mergeRegions(parts, rois);

	// region post processing and cleanup
	LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("Final Regions"));
	for (size_t i = 0; i < rois.size(); /* increment at the end because of erase! */) {
		Image::RegionOfInterest roi = rois[i];

		if (roi.index() == 0 || isToSmall(roi)) {
			LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("  removing %1% = ((%2%, %3%, %4%), (%5%, %6%, %7%))")
				% roi.index()
				% roi.frontLowerLeft()(0) % roi.frontLowerLeft()(1) % roi.frontLowerLeft()(2)
				% roi.backUpperRight()(0) % roi.backUpperRight()(1) % roi.backUpperRight()(2)
				);

			rois.erase(rois.begin() + i);
		} else {
			// Add empty border space to each region
			using namespace Math;
			Vector3ui fll = roi.frontLowerLeft();
			Vector3ui bur = roi.backUpperRight();

			// can't use max() due to unsignedness
			//changed: roi.setFrontLowerLeft(create<Vector3ui>(
			//enlargeBB2(fll(0), ((m_useMMParams) ? (unsigned int) (m_borderWidth / m_voxelInMMSize(0)) : m_borderWidth)),
			//enlargeBB2(fll(1), ((m_useMMParams) ? (unsigned int) (m_borderWidth / m_voxelInMMSize(1)) : m_borderWidth)),
			//enlargeBB2(fll(2), ((m_useMMParams) ? (unsigned int) (m_borderWidth / m_voxelInMMSize(2)) : m_borderWidth))));

			if(m_useMMParams) {roi.setFrontLowerLeft(create<Vector3ui>(	
				enlargeBB2(fll(0), (m_borderWidth / m_voxelInMMSize(0))), 
				enlargeBB2(fll(1), (m_borderWidth / m_voxelInMMSize(1))), 
				enlargeBB2(fll(2), (m_borderWidth / m_voxelInMMSize(2)))));}
			else {roi.setFrontLowerLeft(create<Vector3ui>(enlargeBB2(fll(0),m_borderWidth), enlargeBB2(fll(1),m_borderWidth), enlargeBB2(fll(2),m_borderWidth)));}


			//changed: roi.setBackUpperRight(create<Vector3ui>(
			//std::min(bur(0) + 1 + ((m_useMMParams) ? (unsigned int) (m_borderWidth / m_voxelInMMSize(0)) : m_borderWidth), volumeSize(0)),
			//std::min(bur(1) + 1 + ((m_useMMParams) ? (unsigned int) (m_borderWidth / m_voxelInMMSize(1)) : m_borderWidth), volumeSize(1)),
			//std::min(bur(2) + 1 + ((m_useMMParams) ? (unsigned int) (m_borderWidth / m_voxelInMMSize(2)) : m_borderWidth), volumeSize(2))));

			if(m_useMMParams) {roi.setBackUpperRight(create<Vector3ui>(	
				std::min(bur(0) + 1 + (unsigned int) (m_borderWidth / m_voxelInMMSize(0)), volumeSize(0)),
				std::min(bur(1) + 1 + (unsigned int) (m_borderWidth / m_voxelInMMSize(1)), volumeSize(1)),
				std::min(bur(2) + 1 + (unsigned int) (m_borderWidth / m_voxelInMMSize(2)), volumeSize(2))));}
			else {std::min(bur(0) + 1 + (m_borderWidth), volumeSize(0)), std::min(bur(1) + 1 + (m_borderWidth), volumeSize(1)), std::min(bur(2) + 1 + (m_borderWidth), volumeSize(2));}

			rois[i] = roi;
			m_pObjectBoundingBoxes->add(roi);

			LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("  region %1% = ((%2%, %3%, %4%), (%5%, %6%, %7%))")
				% roi.index()
				% roi.frontLowerLeft()(0) % roi.frontLowerLeft()(1) % roi.frontLowerLeft()(2)
				% roi.backUpperRight()(0) % roi.backUpperRight()(1) % roi.backUpperRight()(2)
				);

			++i; // done only if we are not erasing!
		}
	}

	//part.rois.resumeUpdateSignals(); // inform the world that we are done!

	LOG(Log::Level::Debug, OPENOR_MODULE,
		Log::msg("Separation Statistics:\n"
		"  object threshold density value: %1%\n"
		"  number of detected ROIs  : %2%\n")
		% m_objectThreshold
		% rois.size()
		);

	LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("<<< separation done."));
	return;
}

bool MTScanSurfaceSegmentor::isToSmall(const Image::RegionOfInterest& roi) {

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

void MTScanSurfaceSegmentor::scanLine(SubVolumePartitioning& part) {
	// - in each slice process all voxels
	// - analyse the neighbouring 13 voxels (explanation: see processVoxel())

	std::vector<ObjectIdType> neighborObjects;
	// there are 13 neighboring voxels: explanation see processVoxel()!
	neighborObjects.resize(13); 

	for (unsigned int slice = part.start; slice < part.end; ++slice) {
		for (unsigned int column = 0; column < m_height; ++column) {
			for (unsigned int row = 0; row < m_width; ++row) {

				processVoxel(row, column, slice, neighborObjects, part);

			}
		}
#ifdef _OPENMP
#  pragma omp atomic
#endif
		++m_prog_currentSlice;
		if (m_canceled) { break; } // we simply stop processing if we are canceled
	}

	return;
}

void MTScanSurfaceSegmentor::processVoxel(unsigned int x, unsigned int y, unsigned int z, std::vector<ObjectIdType>& neighborObjects, SubVolumePartitioning& part) {
	// This is the most heartpiece of the algorithm.
	// the scan moves along through each voxel, and checks if a it hits an object.
	// if so it creates a new region or
	// if any neighbor already was a region this region is grown.
	// if more than one region is adjacent to this voxel these regions are merged.

	if (pixelIsObject(x, y, z)) {
		size_t numNbs = neighborIsObject(neighborObjects, x, y, z, part);

		if (numNbs == 0) {
			if (m_maxObjectId <= part.rois_size) {
				// the only real downside of this algorithm is that we can run out of object ids
				// if there are too many region simultaneously alive.
				// This can happen in large grids, for example.
				// to make matters work we do not clean up dead regions imedeately, for efficiency.
				// so in case we do run out we need to need to clean those up!

				compactRegions(x, y, z, part);

				if (m_maxObjectId <= part.rois_size) {
					// if we still have no available id we are out of luck!
					LOG_THROW_RUNTIME(Log::Level::Error, Log::msg("[MTScanSurfaceSegmentor] Out of object ids"));
				}
			}

			// create a new object
			ObjectIdType id = part.rois_size + 1;

			Image::RegionOfInterest roi;
			roi.setFrontLowerLeft(Math::create<Math::Vector3ui>(x, y, z));
			roi.setBackUpperRight(Math::create<Math::Vector3ui>(x, y, z));
			roi.setIndex(id);

			part.rois[part.rois_size++] = roi;

			writeOut(id, x, y, z);
		} else {
			// grow an existing object
			ObjectIdType id = neighborObjects[numNbs - 1];
			//neighborObjects.pop_back();

			growROI(id, x, y, z, part);
			writeOut(id, x, y, z);

			// merge with other adjacent regions
			for (int i = (numNbs - 2); i >= 0; i--) {
				ObjectIdType nbId = neighborObjects[i];
				if (nbId == id) continue;
				if (part.rois[nbId - 1].index() == 0) continue;
				id = connectROI(id, nbId, part);
			}
		}
	} else {
		writeOut(0, x, y, z);
	}

	return;
}

bool MTScanSurfaceSegmentor::pixelIsObject(unsigned int x, unsigned int y, unsigned int z) {
	return (m_pVolumeDataPtr[static_cast<size_t>(z) * m_height * m_width + y * m_width + x] > m_objectThreshold);
}

size_t MTScanSurfaceSegmentor::neighborIsObject(std::vector<MTScanSurfaceSegmentor::ObjectIdType>& neighborIds, unsigned int x, unsigned int y, unsigned int z, SubVolumePartitioning& part) {
	
	// - add all 13 neighbour ids if they are objects

	int num = 0;
	if (ObjectIdType id = getObjectId(((int) x) - 1, y, z, part))                           /*{ pushIfNew(result, id); }*/ 
		neighborIds[num++] = id;
	for (int dx = -1; dx <= 1; ++dx) {
		if (ObjectIdType id = getObjectId(((int) x) + dx, ((int) y) - 1, z, part))              /*{ pushIfNew(result, id); }*/ 
			neighborIds[num++] = id;
	}
	for (int dy = -1; dy <= 1; ++dy) {
		for (int dx = -1; dx <= 1; ++dx) {
			if (ObjectIdType id = getObjectId(((int) x) + dx, ((int) y) + dy, ((int) z) - 1, part)) /*{ pushIfNew(result, id); }*/ 
				neighborIds[num++] = id;
		}
	}
	return num;
}

bool MTScanSurfaceSegmentor::cannotConnect(const Image::RegionOfInterest& roi, unsigned int x, unsigned int y, unsigned int z, const SubVolumePartitioning& part) const {
	// a region is closed (will not be connected or grown any more)
	// if it is too far behind the currently processed voxel.
	// NOTE: if the neighborIsObject() is changed the value of what is
	// considered too far (connectRadius) needs to be changed!
	const unsigned int connectRadius = 2;

	// don't remove regions at the beginning of the subvolume - might get merged with the previous subvolume-regions
	// except for the first subvolume, which does not have a predecessor
	if (z != 0 && roi.frontLowerLeft()(2) == part.start) return false;

	// TODO: make sure that changig from '>' to '>=' didn't break anything
	if ((z - part.start) >= connectRadius && (z - part.start - connectRadius) >= roi.backUpperRight()(2)) return true;

	return false;
}

void MTScanSurfaceSegmentor::writeOut(ObjectIdType id, unsigned int x, unsigned int y, unsigned int z) {
	m_pVolumeObjectDataPtr[static_cast<size_t>(z) * m_height * m_width + y * m_width + x] = id;
}

MTScanSurfaceSegmentor::ObjectIdType& MTScanSurfaceSegmentor::getObjectId(unsigned int x, unsigned int y, unsigned int z, const SubVolumePartitioning& part) {
	// we do not need to check lower bounds here because of the unsigned data types
	// (and yes, that is well defined)
	// make sure that we are in the correct part
	if (x >= m_width || y >= m_height || z >= m_depth || z >= part.end || z < part.start) { return m_zero; }
	return m_pVolumeObjectDataPtr[static_cast<size_t>(z) * m_height * m_width + y * m_width + x];
}

namespace {
	Math::Vector3ui vecMin(Math::Vector3ui lhs, Math::Vector3ui rhs) {
		return Math::create<Math::Vector3ui>(std::min(lhs(0), rhs(0)), std::min(lhs(1), rhs(1)), std::min(lhs(2), rhs(2)));
	}

	Math::Vector3ui vecMax(Math::Vector3ui lhs, Math::Vector3ui rhs) {
		return Math::create<Math::Vector3ui>(std::max(lhs(0), rhs(0)), std::max(lhs(1), rhs(1)), std::max(lhs(2), rhs(2)));
	}
}

void MTScanSurfaceSegmentor::growROI(ObjectIdType id, unsigned int x, unsigned int y, unsigned int z, SubVolumePartitioning& part) {

	// - grow the region if the indices x,y,z lie outside given region of interest

	Image::RegionOfInterest& roi = part.rois[id - 1];

	if (roi.frontLowerLeft()(0) > x) roi.frontLowerLeft()(0) = x;
	if (roi.frontLowerLeft()(1) > y) roi.frontLowerLeft()(1) = y;
	if (roi.frontLowerLeft()(2) > z) roi.frontLowerLeft()(2) = z;
	if (roi.backUpperRight()(0) < x) roi.backUpperRight()(0) = x;
	if (roi.backUpperRight()(1) < y) roi.backUpperRight()(1) = y;
	if (roi.backUpperRight()(2) < z) roi.backUpperRight()(2) = z;

	//part.rois[id - 1] = roi;
}

MTScanSurfaceSegmentor::ObjectIdType MTScanSurfaceSegmentor::connectROI(ObjectIdType c1, ObjectIdType c2, SubVolumePartitioning& part) {

	// - connect two regions and take the lower id for the merged regions
	// - merge the two regions sizes
	// - set the other region (region with the higher id) to zero ("dead")

	assert((c1 != 0) && (c2 != 0) && (c1 != c2) && "wrong connection");
	// always swallow the region with the higher id
	ObjectIdType original = std::min(c1, c2);
	ObjectIdType other    = std::max(c1, c2);

	Image::RegionOfInterest& roi = part.rois[original - 1];
	Image::RegionOfInterest& otherRoi = part.rois[other - 1];

	// merge region sizes
	roi.setFrontLowerLeft(vecMin(roi.frontLowerLeft(), otherRoi.frontLowerLeft()));
	roi.setBackUpperRight(vecMax(roi.backUpperRight(), otherRoi.backUpperRight()));

	assert((otherRoi.index() == 0) || (otherRoi.index() == other) && "Index mismatch 1!");

	rewriteRegionId(otherRoi, original, part);
	assert(otherRoi.index() == original && "Index mismatch 2!");

	// this region is dead, i.e. it is not refererd in the map any more, so we mark it as a gooner
	otherRoi.setIndex(0);

	assert(otherRoi.index() == 0 && "Index mismatch 3!");

	return original;
}

void MTScanSurfaceSegmentor::rewriteRegionId(Image::RegionOfInterest& roi, ObjectIdType newId, SubVolumePartitioning& part) {
	assert(roi.index() != 0 && "rewriting background not allowed!");

	// - write a new id to the the given region of interest

	const size_t oldRoiIndex = roi.index();

	for (unsigned int z = roi.frontLowerLeft()(2); z <= roi.backUpperRight()(2); ++z) {
		for (unsigned int y = roi.frontLowerLeft()(1); y <= roi.backUpperRight()(1); ++y) {
			for (unsigned int x = roi.frontLowerLeft()(0); x <= roi.backUpperRight()(0); ++x) {
				ObjectIdType& id = getObjectId(x, y, z, part);
				if (id == oldRoiIndex) { id = newId; }
			}
		}
	}

	roi.setIndex(newId);
}

void MTScanSurfaceSegmentor::compactRegions(unsigned int x, unsigned int y, unsigned int z, SubVolumePartitioning& part) {
	
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

	size_t dbgCountBefore = part.rois_size;

	std::vector<Image::RegionOfInterest> new_rois;
	new_rois.resize(0xffff);
	size_t new_rois_size = 0;

	for (size_t i = 0; i < part.rois_size; i++) {
		m_prog_currentSlice = savedCurrentSlice + (tmpMaxSlice * ((double) i / (double) part.rois_size));

		Image::RegionOfInterest roi = part.rois[i];
		if (roi.index() == 0) {
			// remove "dead" regions from the list
			//part.rois.erase(part.rois.begin() + i);

		} else if (isToSmall(roi) && cannotConnect(roi, x, y, z, part)) {
			// we want to remove small regions, that are too small to be relevant
			// and cannot connect to active regions any more (because they are too
			// far away from the action).
			rewriteRegionId(roi, 0, part);
			//part.rois.erase(part.rois.begin() + i);
		} else {
			// these are the regions which are alive and interesting, we want to keep
			// them, but we rewrite the id to have the lowest available id.
			if (roi.index() != (new_rois_size + 1)) rewriteRegionId(roi, new_rois_size + 1, part);
			new_rois[new_rois_size++] = roi;
		}
	}

	part.rois = new_rois;
	part.rois_size = new_rois_size;

	m_prog_currentSlice = savedCurrentSlice; // set progress to where we were before.
	m_prog_msg = "Segmentation";

	LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("    kept %1% of %2% regions (removed %3%%% dead regions)")
		% part.rois_size % dbgCountBefore
		% (int)((1.0 - ((double) part.rois_size / (double) dbgCountBefore)) * 100)
		);

	LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("  compacting done go on separating ..."));

	return;
}

void MTScanSurfaceSegmentor::mergeRegions(const std::vector<SubVolumePartitioning>& parts, std::vector<Image::RegionOfInterest>& rois) {

	// - initialize the first region
	// - iterate over the parts and merge the regions in the current part with the regions from the previous part if the are connected
	// - discard to small regions that can not be connected anymore

	size_t regionMapSize = 0xffff * parts.size();
	ObjectIdType* regionMap = new ObjectIdType[regionMapSize];
	boost::shared_array<uint16> sharedRegionMap(regionMap);
	for (size_t i = 0; i < regionMapSize; i++) regionMap[i] = 0;

	size_t roiIndex = 0;

	// initialize with the first part
	for (int r = 0; r < parts[0].rois_size; r++) {
		Image::RegionOfInterest roi = parts[0].rois[r];

		if ((roi.backUpperRight()(2) + 1) != parts[0].end) {
			if (isToSmall(roi)) {
				continue;
			}
		}

		if (roi.index() == 0) {
			continue;
		}

		size_t oldIndex = roi.index();
		roi.setIndex(++roiIndex);
		rois.push_back(roi);
		regionMap[oldIndex] = roiIndex;
	}

	for (int i = 1; i < parts.size(); i++) {
		size_t offset = i * 0xffff;
		size_t prevOffset = (i - 1) * 0xffff;

		const SubVolumePartitioning& part = parts[i]; // TODO Namen inkonsequent!
		const SubVolumePartitioning& prevPart = parts[i - 1];
		// merge regions in part i-1 and i

		const std::vector<Image::RegionOfInterest>& rois1 = prevPart.rois;
		const std::vector<Image::RegionOfInterest>& rois2 = part.rois;

		size_t p2start = part.start;
		size_t p1end = p2start - 1;

		for (int r1 = 0; r1 < rois1.size(); r1++) {
			const Image::RegionOfInterest& roi1 = rois1[r1];

			if (roi1.index() == 0) continue;

			const Math::Vector3ui r1bur = roi1.backUpperRight();
			const Math::Vector3ui r1fll = roi1.frontLowerLeft();

			if (r1bur(2) < p1end) continue;

			for (int r2 = 0; r2 < rois2.size(); r2++) {
				// check if there even is a potential for connection
				const Image::RegionOfInterest& roi2 = rois2[r2];

				if (roi2.index() == 0) continue;

				const Math::Vector3ui r2bur = roi2.backUpperRight();
				const Math::Vector3ui r2fll = roi2.frontLowerLeft();

				// discard if the regions are not connected at the part intersection
				//if ((r1bur(2) + 1) != r2fll(2)) continue;
				if (r2fll(2) > p2start) continue;

				// discard if the regions are offset in the x-axis
				if ((r1bur(0) + 1) < r2fll(0) || (r2bur(0) + 1) < r1fll(0)) continue;

				// discard if the regions are offset in the y-axis
				if ((r1bur(1) + 1) < r2fll(1) || (r2bur(1) + 1) < r1fll(1)) continue;


				// TODO: mins/maxs
				size_t xStart = r2fll(0);//std::min(r1fll(0), r2fll(0));
				size_t xEnd = r2bur(0);//std::max(r1bur(0), r2bur(0));
				size_t yStart = r2fll(1);//std::min(r1fll(1), r2fll(1));
				size_t yEnd = r2bur(1);//std::max(r1bur(1), r2bur(1));

				bool doConnect = false;

				for (size_t y = yStart; y <= yEnd; y++) {
					for (size_t x = xStart; x <= xEnd; x++) {
						ObjectIdType objectId2 = getObjectId(x, y, p2start, part);
						bool isR2 = (objectId2 == roi2.index());
						if (!isR2) continue;
						bool isR1 = false;
						// make sure the search is the same as in neighborIsObject
						for (int dx = -1; dx <= 1; dx++) {
							for (int dy = -1; dy <= 1; dy++) {
								ObjectIdType objectId1 = getObjectId((int) x + dx, (int) y + dy, p1end, prevPart);
								isR1 = (objectId1 == roi1.index());
								if (isR1) break;
							}
							if (isR1) break;
						}
						doConnect = (isR1 && isR2);
						if (doConnect) break;
					}
					if (doConnect) break;
				}

				if (!doConnect) continue;

				// connect the rois r1 from i-1 and r2 from part i
				if (regionMap[offset + roi2.index()] != 0) {
					// if r2 is already connected to some roi, connect r1 to it (reverse split)
					// remove r1
					size_t oldRoi1Mapping = regionMap[prevOffset + roi1.index()];
					size_t newRoiMapping = regionMap[offset + roi2.index()];

					if (newRoiMapping != oldRoi1Mapping) rois[oldRoi1Mapping - 1].setIndex(0);

					// make r1 point to the roi r2 is pointing to
					regionMap[prevOffset + roi1.index()] = newRoiMapping;

					// update the roi r1 is pointing to to incorporate r1
					Image::RegionOfInterest& roi = rois[newRoiMapping - 1];
					roi.setFrontLowerLeft(vecMin(roi.frontLowerLeft(), roi1.frontLowerLeft()));
					roi.setBackUpperRight(vecMax(roi.backUpperRight(), roi1.backUpperRight()));
				} else {
					// just expand r1 by r2
					size_t roiMapping = regionMap[prevOffset + roi1.index()];
					regionMap[offset + roi2.index()] = roiMapping;

					// update r1 to incorporate r2
					Image::RegionOfInterest& roi = rois[roiMapping - 1];
					roi.setFrontLowerLeft(vecMin(roi.frontLowerLeft(), roi2.frontLowerLeft()));
					roi.setBackUpperRight(vecMax(roi.backUpperRight(), roi2.backUpperRight()));
				}
			}
		}

		// just add the rest of the rois to the new rois, provided that they are big enough
		for (int r = 0; r < part.rois_size; r++) {
			Image::RegionOfInterest roi = part.rois[r];
			size_t oldIndex = roi.index();

			// discard already connected rois
			if (regionMap[offset + oldIndex] != 0) {
				continue;
			}

			if (roi.index() == 0) {
				continue;
			}

			// discard too small regions that cannot be further connected
			if ((roi.backUpperRight()(2) + 1) != part.end) {
				if (isToSmall(roi)) {
					continue;
				}
			}

			roi.setIndex(++roiIndex);
			rois.push_back(roi);
			regionMap[offset + oldIndex] = roiIndex;
		}
	}

#ifdef _OPENMP
#  pragma omp parallel for num_threads(m_numSubVolumes)
#endif
	for (int i = 0; i < parts.size(); i++) {
		size_t offset = 0xffff * i;
		// remap regions
		for (size_t z = parts[i].start; z < parts[i].end; z++) {
			for (size_t y = 0; y < m_height; y++) {
				for (size_t x = 0; x < m_width; x++) {
					ObjectIdType& region = m_pVolumeObjectDataPtr[z * m_height * m_width + y * m_width + x];
					region = regionMap[offset + (size_t) region];
				}
			}
		}
	}
}

}

