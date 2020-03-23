/*=======================================================================
*** THE CONTENT OF THIS WORK IS PROPRIETARY TO FEI S.A.S, (FEI S.A.S.),            ***
***              AND IS DISTRIBUTED UNDER A LICENSE AGREEMENT.                     ***
***                                                                                ***
***  REPRODUCTION, DISCLOSURE,  OR USE,  IN WHOLE OR IN PART,  OTHER THAN AS       ***
***  SPECIFIED  IN THE LICENSE ARE  NOT TO BE  UNDERTAKEN  EXCEPT WITH PRIOR       ***
***  WRITTEN AUTHORIZATION OF FEI S.A.S.                                           ***
***                                                                                ***
***                        RESTRICTED RIGHTS LEGEND                                ***
***  USE, DUPLICATION, OR DISCLOSURE BY THE GOVERNMENT OF THE CONTENT OF THIS      ***
***  WORK OR RELATED DOCUMENTATION IS SUBJECT TO RESTRICTIONS AS SET FORTH IN      ***
***  SUBPARAGRAPH (C)(1) OF THE COMMERCIAL COMPUTER SOFTWARE RESTRICTED RIGHT      ***
***  CLAUSE  AT FAR 52.227-19  OR SUBPARAGRAPH  (C)(1)(II)  OF  THE RIGHTS IN      ***
***  TECHNICAL DATA AND COMPUTER SOFTWARE CLAUSE AT DFARS 52.227-7013.             ***
***                                                                                ***
***                   COPYRIGHT (C) 1996-2014 BY FEI S.A.S,                        ***
***                        MERIGNAC, FRANCE                                        ***
***                      ALL RIGHTS RESERVED                                       ***
**=======================================================================*/
/*=======================================================================
** Author      : David BEILLOIN (Dec 2008)
**=======================================================================*/
#define BUILDING_DLL 1
#include "NeoInsightsLDMReader.h"

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

#if defined(WIN32)
#define strcasecmp _stricmp
#endif

SO_FIELDCONTAINER_SOURCE(NeoInsightsLDMReader);

void NeoInsightsLDMReader::initClass()
{
	SO_FIELDCONTAINER_INIT_CLASS(NeoInsightsLDMReader, "NeoInsightsLDMReader", SoVolumeReader);
}

void NeoInsightsLDMReader::exitClass()
{
	SO__FIELDCONTAINER_EXIT_CLASS(NeoInsightsLDMReader);
}


NeoInsightsLDMReader::NeoInsightsLDMReader()
{
	SO_FIELDCONTAINER_CONSTRUCTOR(NeoInsightsLDMReader);


	m_size.setBounds(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f);
	m_dim.setValue(0, 0, 0);
	m_type = SoVolumeData::UNSIGNED_SHORT;
	m_bytesPerVoxel = 2;
	m_overlap = 0;
	m_tileDim.setValue(64, 64, 64);

	m_tileBytes = m_tileDim[0] * m_tileDim[1] * m_tileDim[2] * m_bytesPerVoxel;

	// This member variable is inherited from SoVolumeReader
	// and tells VolumeViz that we are an LDM volume
	m_dataConverted = TRUE;

	m_fp = NULL;
	m_topo = NULL;
}

NeoInsightsLDMReader::~NeoInsightsLDMReader()
{
	fclose(m_fp);
	delete m_topo;
}


void NeoInsightsLDMReader::setLDMReader(SoVRLdmFileReader *ldmReader)
{
	mLDMReader = ldmReader;
}


SbBool NeoInsightsLDMReader::getHistogram(std::vector<int64_t>& numVox)
{

	return mLDMReader->getHistogram(numVox);

	_Histogram = numVox;

	return true;
}


imt::volume::VolumeTopoOctree* NeoInsightsLDMReader::topoOctree()
{
	return mTopoTree;
}


int
NeoInsightsLDMReader::setFilename(const SbString& filename)
{

	if (m_fp == NULL)
	{
		SbString expandPath = SbFileHelper::expandString(filename);

		m_fp = fopen(expandPath.getString(), "rb");

		if (!m_fp)
		return FALSE;

		char rawHeaderBytes[1025];

		int minVal, maxVal;

		fread(rawHeaderBytes, 1, 1024, m_fp);
		fread(&minVal, sizeof(int), 1, m_fp);
		fread(&maxVal, sizeof(int), 1, m_fp);

		minVal = 1008;
		maxVal = 56057;

		_Histogram.resize(USHRT_MAX + 1);

		fread(_Histogram.data(), sizeof(int64_t), USHRT_MAX + 1, m_fp);

		fread( &_Header , sizeof(TilesHeader) , 1 , m_fp );//_HeaderPos = 

		m_size.setBounds( 0, 0, 0, _Header._VoxelSizeX * _Header._DimX, _Header._VoxelSizeY * _Header._DimY,
			              _Header._VoxelSizeY * _Header._DimZ );

		m_dim[0] = _Header._DimX;
		m_dim[1] = _Header._DimY;
		m_dim[2] = _Header._DimZ;
		
		m_tileDim.setValue( _Header._TileDim , _Header._TileDim , _Header._TileDim );

		m_type = SoDataSet::UNSIGNED_SHORT;
		m_bytesPerVoxel = 2;
		m_overlap = 0;
		m_min = 0.f;
		m_max = 255.f;

		m_tileBytes = m_tileDim[0] * m_tileDim[1] * m_tileDim[2] * m_bytesPerVoxel;

		mTopoTree = new imt::volume::VolumeTopoOctree();

		imt::volume::VolumeTopoOctree::Vec3i vDim;

		vDim._X = _Header._DimX;
		vDim._Y = _Header._DimY;
		vDim._Z = _Header._DimZ;

		std::cout << " tile dim : " << _Header._TileDim << std::endl;

		mTopoTree->init( vDim , _Header._TileDim , 0 );

		int nLevels = mTopoTree->getNumLevels();

		int nIds = 0;

		for (int ll = 0; ll < nLevels; ll++)
		{
			auto nodes = mTopoTree->getFileNodes(ll);

			for (auto node : nodes)
			{
				fread( (char*)&(node->_MinVal) , sizeof(int) , 1 , m_fp );
				fread( (char*)&(node->_MaxVal) , sizeof(int) , 1 , m_fp );

				nIds++;
			}
		}

		int nFileIds = mTopoTree->getNumFiles();

		_HeaderPos = sizeof(TilesHeader) + 1024 + 2 * sizeof( int ) + sizeof( int64_t ) * ( USHRT_MAX + 1 ) + nFileIds * sizeof( int ) * 2;

		std::cout<<" matching file id sizes : "<<nFileIds<<" "<<nIds<<std::endl;

#if 0

		FILE *fp2 = fopen("backup.zxo2", "wb");

		fwrite(rawHeaderBytes, 1, 1024, fp2);
		fwrite(&minVal, sizeof(int), 1, fp2);
		fwrite(&maxVal, sizeof(int), 1, fp2);
		fwrite(_Histogram.data(), sizeof(int64_t), ( USHRT_MAX + 1 ), fp2);
		fwrite(&_Header, sizeof(TilesHeader), 1, fp2);


		int nFileIds = mTopoTree->getNumFiles();

		for (int ff = 0; ff < nFileIds; ff++)
		{

			auto node = mTopoTree->getFileNode(ff);

			SbBox3i32 tilePos;

			//mLDMReader->getFil
			
			SoCpuBufferObject* buffer2 = (SoCpuBufferObject*)mLDMReader->readTile(ff, tilePos );

			//delete tileBuffer;

			unsigned short* matchBuffer = (unsigned short*)buffer2->map(SoBufferObject::READ_ONLY);

			//bool matchNotFound = false;

			//for (size_t ii = 0; ii < m_tileDim[0] * m_tileDim[1] * m_tileDim[2]; ii++)
			//{
			//	if (buffer[ii] != matchBuffer[ii])
			//	{
			//		matchNotFound = true;
			//	}
			//}

			fwrite(matchBuffer, sizeof(unsigned short), m_tileDim[0] * m_tileDim[1] * m_tileDim[2], fp2);

			buffer2->unmap();

			buffer2->clearInstance();
		}


		fclose(fp2);
#endif

	}

	return TRUE;
}


void NeoInsightsLDMReader::setRawData(int w, int h, int d, float voxelStep, unsigned short *volData)
{
	m_tileDim.setValue(64, 64, 64);

	m_type = SoDataSet::UNSIGNED_SHORT;
	m_bytesPerVoxel = 2;
	m_overlap = 0;
	m_min = 0.f;
	m_max = std::numeric_limits<unsigned short>::max();

	m_tileBytes = m_tileDim[0] * m_tileDim[1] * m_tileDim[2] * m_bytesPerVoxel;

	m_dim.setValue(w, h, d);

	m_size.setBounds(0, 0, 0, w * voxelStep, h * voxelStep, d * voxelStep);


	//if (mTopoTree)
	//{
	mTopoTree = new imt::volume::VolumeTopoOctree;

	imt::volume::VolumeTopoOctree::Vec3i dim;

	dim._X = m_dim[0];
	dim._Y = m_dim[1];
	dim._Z = m_dim[2];

	mTopoTree->init(dim, 64, 0);
	//}

	mTiles.resize(mTopoTree->getNumFiles(), 0);

	//
	
}


int
NeoInsightsLDMReader::getBorderFlag()
{
	// return the overlap on each direction between tiles.
	return m_overlap;
}

SbBool
NeoInsightsLDMReader::getMinMax(int & min, int & max)
{

	std::cout << " getting min max " << std::endl;

	return mLDMReader->getMinMax(min, max);

	min = (int)m_min;
	max = (int)m_max;
	return TRUE;
}

SbBool
NeoInsightsLDMReader::getMinMax(double & min, double & max)
{
	std::cout << " getting min max " << std::endl;

	return mLDMReader->getMinMax(min, max);

	min = m_min;
	max = m_max;
	return TRUE;
}

SbBool
NeoInsightsLDMReader::getMinMax(int64_t & min, int64_t & max)
{

	std::cout << " getting min max " << std::endl;

	//return mLDMReader->getMinMax(min, max);

	min = (int64_t)m_min;
	max = (int64_t)m_max;
	return TRUE;
}


SoVolumeReader::ReadError
NeoInsightsLDMReader::getDataChar(SbBox3f &size, SoVolumeData::DataType &type, SbVec3i32 &dim)
{
	//SoVolumeReader::ReadError success = mLDMReader->getDataChar(size , type, dim);

	size = m_size;
	type = m_type;
	dim = m_dim;

	

	return RD_NO_ERROR;
}

SbBool
NeoInsightsLDMReader::getTileSize(SbVec3i32 &size)
{
	mLDMReader->getTileSize(size);

	//std::cout << "ldm tile size and custom tile size : " << size<< " " << m_tileDim;

	//return true;



	size = m_tileDim;
	return true;
}


void NeoInsightsLDMReader::buildTiles()
{


}


SbBool
NeoInsightsLDMReader::getTileInfo(const int index, SbBox3i32& cellBox, int &resolution)
{

	//return mLDMReader->getTileIn
	

	if (m_topo == NULL) //create an LDMTopoOctree just to understand where each tile is in 3D space
	{
		m_topo = new SoLDMTopoOctree;
		m_topo->init(m_dim, m_tileDim[0], m_overlap);
	}



	//given a tile index, retrieve a tileID, the bounding box and resolution
	SoLDMTileID tileID = m_topo->getTileID(index);
	cellBox = m_topo->getTilePos(tileID);
	resolution = m_topo->getLevelMax() - m_topo->level(tileID);

	return TRUE;
}

SoBufferObject*
NeoInsightsLDMReader::readTile(int index, const SbBox3i32& tilePosition)
{
	auto node = mTopoTree->getFileNode(index);
	
	SoCpuBufferObject* tileBuffer = new SoCpuBufferObject();
	tileBuffer->setSize(m_tileBytes);
	
	unsigned short* buffer = (unsigned short*)tileBuffer->map(SoBufferObject::SET);

	fseek(m_fp, index * m_tileDim[0] * m_tileDim[1] * m_tileDim[2] * sizeof( unsigned short ) + _HeaderPos, SEEK_SET);

	fread(buffer, sizeof(unsigned short), m_tileDim[0] * m_tileDim[1] * m_tileDim[2], m_fp);

    tileBuffer->unmap();
	
	return tileBuffer;
}

void NeoInsightsLDMReader::getSubSlice(const SbBox2i32&, int, void *)
{
	fprintf(stderr, "NeoInsightsLDMReader::getSubSlice : Not Implemented\n");
	return;
}

SbBool
NeoInsightsLDMReader::isThreadSafe()
{
	return SbBool(TRUE);
}


