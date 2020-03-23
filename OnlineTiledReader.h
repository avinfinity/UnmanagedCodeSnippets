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
#include <VolumeViz/readers/SoVRLdmFileReader.h>
#include "OnlineTilesGenerator.h"

namespace imt {

	namespace volume {

		class VolumeTopoOctree;
	}
}
namespace Zeiss
{
	namespace IMT
	{
		namespace NG
		{
			namespace NeoInsights
			{
				namespace Volume
				{
					namespace DataProcessing
					{

						class OnlineTiledReader : public SoVolumeReader 
						{
							SO_FIELDCONTAINER_HEADER(OnlineTiledReader);

						public:


							OnlineTiledReader();


							void setVolumeInfo(int w, int h, int d, float voxelSizeX, float voxelSizeY, float voxelSizeZ,
								int bitDepth, int rawHeaderOffset, int dataType);

	
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
							//virtual void getSubSlice(const SbBox2i32& subSlice, int sliceNumber, SoBufferObject * data);
							virtual SbBool isThreadSafe();
							virtual SbBool getHistogram(std::vector<int64_t>& histogram);

							SbVec3i32 getVolumeDimensions();

							void initializeTiles(OnlineTilesGenerator::ImageProviderFunc& imageFunc, void* dataStruct);

							const imt::volume::VolumeTopoOctree* topoTree() const;

							void setProgressCallback( OnlineTilesGenerator::ProgressFunc& func, void *progressDataStructure );

							~OnlineTiledReader();



						protected:

							void setImageProviderCallback( OnlineTilesGenerator::ImageProviderFunc& func, void *imageProviderData);



						public:

							//this buffer would get populated during memory init
							SoCpuBufferObject *_RawVolumeBuffer;
							SbVec3i32 m_dim;
						protected:

							SoLDMTopoOctree* m_topo;
							imt::volume::VolumeTopoOctree *_TopoTree;

							size_t _HeaderPos;

							SbBox3f m_size;
							SoVolumeData::DataType m_type;

							SbVec3i32 m_tileDim;
							double m_min;
							double m_max;
							int m_overlap;
							size_t m_tileBytes;

							int64_t _Histogram[USHRT_MAX + 1];

							std::vector< int > _LogicalToDiskMap;
							std::vector< SoCpuBufferObject* > _Tiles;
							

							int _VolumeSizeX, _VolumeSizeY, _VolumeSizeZ;
							double _VoxelSizeX, _VoxelSizeY, _VoxelSizeZ;
							int _TileSize;


							OnlineTilesGenerator *_TilesGenerator;
							OnlineTilesGenerator::ProgressFunc _UpdateProgress;
							OnlineTilesGenerator::ImageProviderFunc _ImageProviderFunc;
							void *_ProgressDataStructure, *_ImageProviderDataStruct;

							std::fstream logger;

						};
					}
				}
			}
		}
	}
}

