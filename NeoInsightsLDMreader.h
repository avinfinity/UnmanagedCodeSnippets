#ifndef __NEOINSIGHTSLDMREADER_H__
#define __NEOINSIGHTSLDMREADER_H__

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
#include <LDM/readers/SoLDMReader.h>
#include <VolumeViz/readers/SoVolumeReader.h>
#include <VolumeViz/readers/SoVRLdmFileReader.h>
#include "volumetopooctree.h"


// When building the node as a DLL under Win32 we must explicitly
// declare this entry point as visible outside the DLL.
// The macro BUILDING_DLL is defined in the node's source file.
#ifdef BUILDING_DLL
#define DLL_EXPORT VC_DLL_EXPORT
#else
#define DLL_EXPORT VC_DLL_IMPORT
#endif

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning(disable:4251)
#endif

class SoLDMTopoOctree;
class SoBufferObject;


class NeoInsightsLDMReader : public SoVolumeReader //SoLDMReader //
{
	SO_FIELDCONTAINER_HEADER(NeoInsightsLDMReader);

	struct TilesHeader
	{
		int _DimX, _DimY, _DimZ;
		float _VoxelSizeX, _VoxelSizeY, _VoxelSizeZ;
		int _TileDim;
	};

public:

	/** Default Constructor. */
	NeoInsightsLDMReader();

	/** set use file name */
	virtual int setFilename(const SbString& filename);

	void setRawData(int w, int h, int d , float voxelStep , unsigned short *volData);

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

	void buildTileHierarchy();

	void setLDMReader(SoVRLdmFileReader *ldmReader);

	virtual SbBool getHistogram(std::vector<int64_t>& numVox);

	imt::volume::VolumeTopoOctree *topoOctree();



protected:
	/** Destructor */
	~NeoInsightsLDMReader();

	/** return position from LDM index */
	SbBool getTileInfo(const int index, SbBox3i32& cellBox, int& resolution);

	void buildTiles();

private:
	SoLDMTopoOctree* m_topo;

	imt::volume::VolumeTopoOctree *mTopoTree;

	TilesHeader _Header;

	int _HeaderPos;

	SbBox3f m_size;
	SoVolumeData::DataType m_type;
	SbVec3i32 m_dim;
	SbVec3i32 m_tileDim;
	double m_min;
	double m_max;
	int m_overlap;
	size_t m_tileBytes;

	FILE* m_fp;

	std::vector< unsigned short* > mTiles;

	SoVRLdmFileReader *mLDMReader;

	std::vector< __int64 > _Histogram;
};



#endif