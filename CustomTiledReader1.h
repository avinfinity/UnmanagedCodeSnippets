#pragma once

#include <Inventor/SbLinear.h>
#include <Inventor/STL/map>
#include <Inventor/STL/list>

#include <Inventor/nodes/SoNode.h>
#include <Inventor/fields/SoSubField.h>

#include <Inventor/fields/SoSFNode.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoSFUInt32.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoSFFilePathString.h>
#include <Inventor/threads/SbThread.h>
#include <Inventor/threads/SbThreadMutex.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/readers/SoVolumeReader.h>
#include "volumetopooctree.h"


class CustomTiledReader : public SoVolumeReader
{
	SO_FIELDCONTAINER_HEADER(CustomTiledReader);

	struct TilesHeader
	{
		int _DimX, _DimY, _DimZ;
		float _VoxelSizeX, _VoxelSizeY, _VoxelSizeZ;
		int _TileDim;
	};



public:

	/** Default Constructor. */
	CustomTiledReader();

	/** set use file name */
	virtual int setFilename(const SbString& filename);

	
	/** Standard reader stuf redefine */
	virtual SoVolumeReader::ReadError getDataChar(SbBox3f &size, SoVolumeData::DataType &type, SbVec3i32 &dim);
	virtual int getBorderFlag();
	virtual SbBool getTileSize(SbVec3i32 &size);
	virtual SbBool getMinMax(int & min, int & max);
	virtual SbBool getMinMax(double & min, double & max);
	virtual SbBool getMinMax(int64_t & min, int64_t & max);
	virtual SoBufferObject* readTile(int index, const SbBox3i32& tilePosition);
	virtual void getSubSlice(const SbBox2i32& subSlice, int sliceNumber, void * data);
	virtual SbBool isThreadSafe();
	virtual SbBool isDataConverted();
	virtual SbBool getHistogram(std::vector<int64_t>& histogram);


	~CustomTiledReader();

protected:

	SoLDMTopoOctree* m_topo;
	imt::volume::VolumeTopoOctree *_TopoTree;
	TilesHeader _Header;


	size_t _HeaderPos;

	SbBox3f m_size;
	SoVolumeData::DataType m_type;
	SbVec3i32 m_dim;
	SbVec3i32 m_tileDim;
	double m_min;
	double m_max;
	int m_overlap;
	size_t m_tileBytes;

	int64_t _Histogram[USHRT_MAX + 1];

	FILE* m_fp;

	//static LoXX::LXLogger* logger;
	//Zeiss::IMT::Core::CriticalSection _Criticalsection;

	SbThreadMutex _Mutex;


};

