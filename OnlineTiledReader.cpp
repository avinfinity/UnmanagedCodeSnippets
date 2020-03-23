
#include "stdafx.h"
#include "OnlineTiledReader.h"
#include "OnlineTilesGenerator.h"
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
#include "volumetopooctree.h"
#include "VolumeCachedDataReader.h"


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
#define strcasecmp _stricmp

						SO_FIELDCONTAINER_SOURCE(OnlineTiledReader);


						void OnlineTiledReader::initClass()
						{
							SO_FIELDCONTAINER_INIT_CLASS(OnlineTiledReader, "OnlineTiledReader", SoVolumeReader);
						}

						void OnlineTiledReader::exitClass()
						{
							SO__FIELDCONTAINER_EXIT_CLASS(OnlineTiledReader);
						}


						OnlineTiledReader::OnlineTiledReader()
						{
							SO_FIELDCONTAINER_CONSTRUCTOR(OnlineTiledReader);


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

							m_topo = NULL;

							_RawVolumeBuffer = 0;

							_TilesGenerator = new OnlineTilesGenerator();

						}

						OnlineTiledReader::~OnlineTiledReader()
						{
							delete m_topo;
						}


						int
							OnlineTiledReader::setFilename(const SbString& filename)
						{

							SoVolumeReader::setFilename(filename);
							SbString expandPath = SbFileHelper::expandString(filename);

							m_size.setBounds(0, 0, 0, (float)_VoxelSizeX * _VolumeSizeX, (float)_VoxelSizeY * _VolumeSizeY,
								(float)_VoxelSizeZ * _VolumeSizeZ);

							m_dim[0] = _VolumeSizeX;
							m_dim[1] = _VolumeSizeY;
							m_dim[2] = _VolumeSizeZ;

							m_tileDim.setValue(_TileSize, _TileSize, _TileSize);

							int nLevels = _TopoTree->getNumLevels();

							int nFileIds = _TopoTree->getNumFiles();

							m_type = SoDataSet::UNSIGNED_SHORT;
							m_bytesPerVoxel = 2;
							m_overlap = 0;
							m_min = 0.f;
							m_max = 255.f;

							m_tileBytes = m_tileDim[0] * m_tileDim[1] * m_tileDim[2] * m_bytesPerVoxel;

							m_tileDim.setValue(_TileSize, _TileSize, _TileSize);

							m_type = SoDataSet::UNSIGNED_SHORT;
							m_bytesPerVoxel = 2;
							m_overlap = 0;
							m_min = 0.f;
							m_max = 255.f;

							m_tileBytes = m_tileDim[0] * m_tileDim[1] * m_tileDim[2] * m_bytesPerVoxel;

							std::vector<int64_t> histogram;

							_TilesGenerator->getHistogram(histogram);

							memcpy(_Histogram, histogram.data(), histogram.size() * sizeof(int64_t));

							return TRUE;
						}


						void OnlineTiledReader::setVolumeInfo(int w, int h, int d, float voxelSizeX, float voxelSizeY, float voxelSizeZ,
							int bitDepth, int rawHeaderOffset, int dataType)
						{
							_TilesGenerator->setVolumeInfo(w, h, d, voxelSizeX, voxelSizeY, voxelSizeZ, bitDepth, rawHeaderOffset, dataType);

							_VolumeSizeX = w;
							_VolumeSizeY = h;
							_VolumeSizeZ = d;

							_VoxelSizeX = voxelSizeX;
							_VoxelSizeY = voxelSizeY;
							_VoxelSizeZ = voxelSizeZ;

							_TopoTree = _TilesGenerator->topoTree();
							_TileSize = (int)_TilesGenerator->tileSize();

						}



						void OnlineTiledReader::setProgressCallback(OnlineTilesGenerator::ProgressFunc& func, void *progressDataStructure)
						{
							_ProgressDataStructure = progressDataStructure;
							_UpdateProgress = func;
						}

						void OnlineTiledReader::setImageProviderCallback(OnlineTilesGenerator::ImageProviderFunc& func, void *imageProviderData)
						{
							_ImageProviderFunc = func;

							_ImageProviderDataStruct = imageProviderData;
						}


						int
							OnlineTiledReader::getBorderFlag()
						{
							return m_overlap;
						}

						SbBool
							OnlineTiledReader::getMinMax(int & min, int & max)
						{

							min = (int)m_min;
							max = (int)m_max;
							return TRUE;
						}

						SbBool
							OnlineTiledReader::getMinMax(double & min, double & max)
						{
							min = m_min;
							max = m_max;
							return TRUE;
						}

						SbBool
							OnlineTiledReader::getMinMax(int64_t & min, int64_t & max)
						{
							min = (int64_t)m_min;
							max = (int64_t)m_max;
							return TRUE;
						}


						SoVolumeReader::ReadError
							OnlineTiledReader::getDataChar(SbBox3f &size, SoVolumeData::DataType &type, SbVec3i32 &dim)
						{
							size = m_size;
							type = m_type;
							dim = m_dim;
							return RD_NO_ERROR;
						}

						SbBool
							OnlineTiledReader::getTileSize(SbVec3i32 &size)
						{
							size = m_tileDim;
							return true;
						}

						SbBool OnlineTiledReader::getHistogram(std::vector<int64_t>& histogram)
						{
							histogram.resize((USHRT_MAX + 1));

							_TilesGenerator->getHistogram(histogram);

							//memcpy(histogram.data(), _Histogram, sizeof(__int64) * (USHRT_MAX + 1));

							return true;
						}


						SbVec3i32 OnlineTiledReader::getVolumeDimensions()
						{
							return m_dim;
						}


						void OnlineTiledReader::initializeTiles(OnlineTilesGenerator::ImageProviderFunc& imageFunc, void* dataStruct)
						{
							int numFiles = _TopoTree->getNumFiles();

							_Tiles.resize(numFiles);

							size_t tileBufferSize = _TileSize * _TileSize * _TileSize * sizeof(unsigned short);

							for (int ff = 0; ff < numFiles; ff++)
							{
								_Tiles[ff] = new SoCpuBufferObject();

								_Tiles[ff]->setSize(tileBufferSize);

								_Tiles[ff]->ref();
							}

							_TilesGenerator->generate(_Tiles, imageFunc, dataStruct);

							std::cout << " tiles generation finished " << std::endl;

						}



						const imt::volume::VolumeTopoOctree* OnlineTiledReader::topoTree() const
						{
							return _TopoTree;
						}



						//Note that this method is not thread safe
						SoBufferObject*	OnlineTiledReader::readTile(int index, const SbBox3i32&)
						{

							//std::cout << _Tiles[index]->getSize() << std::endl;

							return  _Tiles[index];
						}

						void OnlineTiledReader::getSubSlice(const SbBox2i32&, int, void *)
						{
							fprintf(stderr, "OnlineTiledReader::getSubSlice : Not Implemented\n");
							return;
						}


						SbBool  OnlineTiledReader::isThreadSafe()
						{
							return SbBool(TRUE);
						}
					}
				}
			}
		}
	}
}

