

#include "CustomTiledReader.h"

#include <string.h>

#include <Inventor/fields/SoSubFieldContainer.h>
#include <Inventor/threads/SbThreadMutex.h>
#include <Inventor/threads/SbThreadRWMutex.h>
#include <Inventor/SbElapsedTime.h>
#include <Inventor/SbMathHelper.h>
#include <Inventor/helpers/SbDataTypeMacros.h>
#include <Inventor/helpers/SbFileHelper.h>
#include <Inventor/devices/SoCpuBufferObject.h>

#include <LDM/SoLDMTopoOctree.h>
//#include "mutex"

#define strcasecmp _stricmp


SO_FIELDCONTAINER_SOURCE(CustomTiledReader);


void CustomTiledReader::initClass()
{
	SO_FIELDCONTAINER_INIT_CLASS(CustomTiledReader, "CustomTiledReader", SoVolumeReader);
}

void CustomTiledReader::exitClass()
{
	SO__FIELDCONTAINER_EXIT_CLASS(CustomTiledReader);
}


CustomTiledReader::CustomTiledReader()
{
	SO_FIELDCONTAINER_CONSTRUCTOR(CustomTiledReader);


	m_size.setBounds(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f);
	m_dim.setValue(0, 0, 0);
	m_type = SoVolumeData::UNSIGNED_SHORT;
	m_bytesPerVoxel = 2;
	m_overlap = 0;
	m_tileDim.setValue(256, 256, 256);

	m_tileBytes = m_tileDim[0] * m_tileDim[1] * m_tileDim[2] * m_bytesPerVoxel;

	// This member variable is inherited from SoVolumeReader
	// and tells VolumeViz that we are an LDM volume
	m_dataConverted = TRUE;

	m_fp = NULL;
	m_topo = NULL;

	//logger = LoXX::LXManager::Manager().GetLogger("Zeiss.IMT.NG.Metrotom.VSGWrapper.CustomTilesLog", LoXX::LXPriority::lpFatal);

}

CustomTiledReader::~CustomTiledReader()
{
	fclose(m_fp);
	delete m_topo;
}


int
CustomTiledReader::setFilename(const SbString& filename)
{
	SoVolumeReader::setFilename(filename);

	if (m_fp == NULL)
	{
		SbString expandPath = SbFileHelper::expandString(filename);

		fopen_s( &m_fp, expandPath.getString(), "rb");

		if (!m_fp)
			return FALSE;

		char rawHeaderBytes[1025];

		int minVal, maxVal;

		//fseek(m_fp, 0, SEEK_SET);

		fread(rawHeaderBytes, 1, 1024, m_fp);
		fread(&minVal, sizeof(int), 1, m_fp);
		fread(&maxVal, sizeof(int), 1, m_fp);


		minVal = 1008;
		maxVal = 56057;

		fread( _Histogram , sizeof(int64_t), USHRT_MAX + 1, m_fp);

		fread(&_Header, sizeof(TilesHeader), 1, m_fp);

		_HeaderPos = sizeof(TilesHeader) + 1024 + 2 * sizeof(int) + sizeof(int64_t) * (USHRT_MAX + 1);

		m_size.setBounds( 0, 0, 0, _Header._VoxelSizeX * _Header._DimX, _Header._VoxelSizeY * _Header._DimY,
			              _Header._VoxelSizeY * _Header._DimZ);

		m_dim[0] = _Header._DimX;
		m_dim[1] = _Header._DimY;
		m_dim[2] = _Header._DimZ;

		//std::cout << " volume dimensions : " << m_dim << std::endl;
		//std::cout << " size : " << m_size << std::endl;

		m_tileDim.setValue(_Header._TileDim, _Header._TileDim, _Header._TileDim);

		m_type = SoDataSet::UNSIGNED_SHORT;
		m_bytesPerVoxel = 2;
		m_overlap = 0;
		m_min = 0.f;
		m_max = 255.f;

		m_tileBytes = m_tileDim[0] * m_tileDim[1] * m_tileDim[2] * m_bytesPerVoxel;

		_TopoTree = new imt::volume::VolumeTopoOctree();

		imt::volume::VolumeTopoOctree::Vec3i vDim;

		vDim._X = _Header._DimX;
		vDim._Y = _Header._DimY;
		vDim._Z = _Header._DimZ;

		_TopoTree->init(vDim, _Header._TileDim, 0);

		int nLevels = _TopoTree->getNumLevels();

		int nIds = 0;

		for (int ll = 0; ll < nLevels; ll++)
		{
			auto nodes = _TopoTree->getFileNodes(ll);

			for (auto node : nodes)
			{
				fread((char*)&(node->_MinVal), sizeof(int), 1, m_fp);
				fread((char*)&(node->_MaxVal), sizeof(int), 1, m_fp);

				nIds++;
			}
		}

		int nFileIds = _TopoTree->getNumFiles();

		_HeaderPos = sizeof(TilesHeader) + 1024 + 2 * sizeof(int) + sizeof(int64_t) * (USHRT_MAX + 1) + nFileIds * sizeof(int) * 2;
		//std::fstream debugFile;

		//debugFile.open("debugData.txt", std::ios::out);

		//debugFile << m_dim << std::endl;

		//debugFile.close();
		
		m_tileDim.setValue(_Header._TileDim, _Header._TileDim, _Header._TileDim);

		m_type = SoDataSet::UNSIGNED_SHORT;
		m_bytesPerVoxel = 2;
		m_overlap = 0;
		m_min = 0.f;
		m_max = 255.f;

		m_tileBytes = m_tileDim[0] * m_tileDim[1] * m_tileDim[2] * m_bytesPerVoxel;

	}


	//touch();


	return TRUE;
}

int
CustomTiledReader::getBorderFlag()
{
	return m_overlap;
}

SbBool
CustomTiledReader::getMinMax(int & min, int & max)
{

	min = (int)m_min;
	max = (int)m_max;
	return TRUE;
}

SbBool
CustomTiledReader::getMinMax(double & min, double & max)
{
	min = m_min;
	max = m_max;
	return TRUE;
}

SbBool
CustomTiledReader::getMinMax(int64_t & min, int64_t & max)
{
	min = (int64_t)m_min;
	max = (int64_t)m_max;
	return TRUE;
}


SoVolumeReader::ReadError
CustomTiledReader::getDataChar(SbBox3f &size, SoVolumeData::DataType &type, SbVec3i32 &dim)
{
	size = m_size;
	type = m_type;
	dim = m_dim;
	return RD_NO_ERROR;
}

SbBool
CustomTiledReader::getTileSize(SbVec3i32 &size)
{
	size = m_tileDim;
	return true;
}

SbBool CustomTiledReader::getHistogram(std::vector<int64_t>& histogram)
{
	histogram.resize(( USHRT_MAX + 1 ));

	//std::fill(histogram.begin(), histogram.end(), 0);

	memcpy(histogram.data(), _Histogram, sizeof(__int64) * ( USHRT_MAX + 1 ));

	return true;
}



//Note that this method is not thread safe
SoBufferObject*
CustomTiledReader::readTile(int index, const SbBox3i32& tilePosition)
{

	//lock();

	//_Criticalsection.Lock();

	//Zeiss::IMT::NG::Metrotom::Data::Entities::Logger::ErrorAsync("CustomTileReading", "Begin Tile: " + index);



	/*std::stringstream msg;
	msg << "Begin Tile: " << index;

	logger->LogDebug(msg.str());*/

	SoCpuBufferObject* tileBuffer = new SoCpuBufferObject();
	tileBuffer->setSize(m_tileBytes);

	SbVec3f center = tilePosition.getCenter();//warning treated as error fix ( if we don't use tilePosition , it will be treated as warning )

	//map the buffer , so that we can get its data for copying
	unsigned short* buffer = (unsigned short*)tileBuffer->map(SoBufferObject::SET);

	size_t id = (size_t)index;

	size_t loc = (size_t)m_tileDim[0] * m_tileDim[1] * m_tileDim[2];


	_Mutex.lock();
	//std::cout << " tile  " << index << " begin " << std::endl;
	//reach file id of the tile in the file ( random access may make it slow )
	//fseek(m_fp, (long)(id * loc * sizeof(unsigned short) + _HeaderPos), SEEK_SET);

	//read the actual tile from disk to mapped buffer 
	//fread(buffer, sizeof(unsigned short), m_tileDim[0] * m_tileDim[1] * m_tileDim[2], m_fp);

	void* rbf = getBuffer(id * loc * sizeof(unsigned short) + _HeaderPos, m_tileDim[0] * m_tileDim[1] * m_tileDim[2] * 2);

	memcpy(buffer, rbf, m_tileDim[0] * m_tileDim[1] * m_tileDim[2] * 2);
	
	//std::cout << " tile  " << index << " end " << std::endl;
	_Mutex.unlock();
	//unmap the buffer 
	tileBuffer->unmap();

	//Do note that here we are allocating memory with new and we are not deallocating it. So it is the responsiblity of 
	// class consuming this method to deallocate the data after use.

	/*std::stringstream msg2;
	msg2 << "End Tile: " << index;

	logger->LogDebug(msg.str());*/

	//Zeiss::IMT::NG::Metrotom::Data::Entities::Logger::ErrorAsync("CustomTileReading", "End Tile: " + index);

	//unlock();

	//_Criticalsection.Unlock();

	return  tileBuffer;
}

void CustomTiledReader::getSubSlice(const SbBox2i32&, int, void *)
{
	fprintf(stderr, "CustomTiledReader::getSubSlice : Not Implemented\n");
	return;

	
}

SbBool
CustomTiledReader::isThreadSafe()
{
	return SbBool(FALSE);
}


SbBool
CustomTiledReader::isDataConverted()
{
	return SbBool(TRUE);
}



