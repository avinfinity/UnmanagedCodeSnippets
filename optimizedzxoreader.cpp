
#include "optimizedzxoreader.h"

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


namespace imt{
	
	namespace volume{
	
		//#include "mutex"

#define strcasecmp _stricmp


		SO_FIELDCONTAINER_SOURCE(OptimizedZXOReader);


		void OptimizedZXOReader::initClass()
		{
			SO_FIELDCONTAINER_INIT_CLASS(OptimizedZXOReader, "OptimizedZXOReader", SoVolumeReader);
		}

		void OptimizedZXOReader::exitClass()
		{
			SO__FIELDCONTAINER_EXIT_CLASS(OptimizedZXOReader);
		}


		OptimizedZXOReader::OptimizedZXOReader()
		{
			SO_FIELDCONTAINER_CONSTRUCTOR(OptimizedZXOReader);


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

		}

		OptimizedZXOReader::~OptimizedZXOReader()
		{
			fclose(m_fp);
			delete m_topo;
		}


		int
			OptimizedZXOReader::setFilename(const SbString& filename)
		{

			SoVolumeReader::setFilename(filename);

			if (m_fp == NULL)
			{
				SbString expandPath = SbFileHelper::expandString(filename);

				fopen_s(&m_fp, expandPath.getString(), "rb");

				if (!m_fp)
					return FALSE;

				unsigned char rawHeaderBytes[4097];

				int minVal, maxVal;

				fread(rawHeaderBytes, sizeof( unsigned char ), 4096, m_fp);

			//	std::cout << " sizeof header : " << sizeof(_Header) << std::endl;

				memcpy(&_Header, rawHeaderBytes, sizeof(_Header));

				_TopoTree = new imt::volume::VolumeTopoOctree();

				imt::volume::VolumeTopoOctree::Vec3i vDim;

				vDim._X = _Header.volumeSizeX;
				vDim._Y = _Header.volumeSizeY;
				vDim._Z = _Header.volumeSizeZ;

				_TopoTree->init(vDim, _Header._TileDim, 0);

				m_size.setBounds(0, 0, 0, (float)_Header.voxelSizeX * _Header.volumeSizeX, (float)_Header.voxelSizeY * _Header.volumeSizeY,
					(float)_Header.voxelSizeZ * _Header.volumeSizeZ);

				m_dim[0] = _Header.volumeSizeX;
				m_dim[1] = _Header.volumeSizeY;
				m_dim[2] = _Header.volumeSizeZ;

				m_tileDim.setValue(_Header._TileDim, _Header._TileDim, _Header._TileDim);

				int nLevels = _TopoTree->getNumLevels();

				int nFileIds = _TopoTree->getNumFiles();

				size_t footerFilePos = (size_t)nFileIds * (size_t)_Header._TileDim * (size_t)_Header._TileDim * (size_t)_Header._TileDim * sizeof(unsigned short) + 4096;

				//std::cout << " footer pos : " << footerFilePos<< " "<< sizeof( long ) << std::endl;

				//check 

				int err = _fseeki64(m_fp, footerFilePos, SEEK_SET);

				if (err)
				{
					std::cout << " _fseeki64 failed " <<err<< std::endl;

					return 0;
				}

				fread(&minVal, sizeof(int), 1, m_fp);
				fread(&maxVal, sizeof(int), 1, m_fp);


				minVal = 1008;
				maxVal = 56057;

				fread(_Histogram, sizeof(int64_t), USHRT_MAX + 1, m_fp);

				//std::cout << " zxo file path for read : " << filename << std::endl;
				//std::cout << " tile dimension : " << _Header._TileDim << std::endl;
				//std::cout << " volume dimensions : " << _Header.volumeSizeX << " " << _Header.volumeSizeY << " " << _Header.volumeSizeZ << std::endl;
				//std::cout << " voxel sizes : " << _Header.voxelSizeX << " " << _Header.voxelSizeY << " " << _Header.voxelSizeZ << std::endl;

				m_type = SoDataSet::UNSIGNED_SHORT;
				m_bytesPerVoxel = 2;
				m_overlap = 0;
				m_min = 0.f;
				m_max = 255.f;

				m_tileBytes = m_tileDim[0] * m_tileDim[1] * m_tileDim[2] * m_bytesPerVoxel;

				int nIds = 0;

				for (int ll = 0; ll < nLevels; ll++)
				{
					auto nodes = _TopoTree->getFileNodes(ll);

					for (auto node : nodes)
					{

						

						fread((char*)&(node->_MinVal), sizeof(int), 1, m_fp);
						fread((char*)&(node->_MaxVal), sizeof(int), 1, m_fp);

						nIds++;

						std::cout << node->_MinVal << " " << node->_MaxVal << std::endl;

						//std::cout << " read min and max : " << node->_MinVal << " " << node->_MaxVal << std::endl;
					}
				}

				//now read logical to disk file id map
				_LogicalToDiskMap.resize(nFileIds);

				fread(_LogicalToDiskMap.data(), sizeof(int), nFileIds, m_fp);

				_HeaderPos = 4096;
				
				m_tileDim.setValue(_Header._TileDim, _Header._TileDim, _Header._TileDim);

				m_type = SoDataSet::UNSIGNED_SHORT;
				m_bytesPerVoxel = 2;
				m_overlap = 0;
				m_min = 0.f;
				m_max = 255.f;

				m_tileBytes = m_tileDim[0] * m_tileDim[1] * m_tileDim[2] * m_bytesPerVoxel;

				//close the file as it won't be used anymore
				fclose(m_fp);

			}


			//touch();

			return TRUE;
		}

		int
			OptimizedZXOReader::getBorderFlag()
		{
			return m_overlap;
		}

		SbBool
			OptimizedZXOReader::getMinMax(int & min, int & max)
		{

			min = (int)m_min;
			max = (int)m_max;
			return TRUE;
		}

		SbBool
			OptimizedZXOReader::getMinMax(double & min, double & max)
		{
			min = m_min;
			max = m_max;
			return TRUE;
		}

		SbBool
			OptimizedZXOReader::getMinMax(int64_t & min, int64_t & max)
		{
			min = (int64_t)m_min;
			max = (int64_t)m_max;
			return TRUE;
		}


		SoVolumeReader::ReadError
			OptimizedZXOReader::getDataChar(SbBox3f &size, SoVolumeData::DataType &type, SbVec3i32 &dim)
		{
			size = m_size;
			type = m_type;
			dim = m_dim;
			return RD_NO_ERROR;
		}

		SbBool
			OptimizedZXOReader::getTileSize(SbVec3i32 &size)
		{
			size = m_tileDim;
			return true;
		}

		SbBool OptimizedZXOReader::getHistogram(std::vector<int64_t>& histogram)
		{
			histogram.resize((USHRT_MAX + 1));

			memcpy(histogram.data(), _Histogram, sizeof(__int64) * (USHRT_MAX + 1));

			return true;
		}



		//Note that this method is not thread safe
		SoBufferObject*	 OptimizedZXOReader::readTile(int index, const SbBox3i32& )
		{

			SoCpuBufferObject* tileBuffer = new SoCpuBufferObject();
			
			tileBuffer->setSize(m_tileBytes);

			//map the buffer , so that we can get its buffer for writing
			unsigned short* buffer = (unsigned short*)tileBuffer->map(SoBufferObject::SET);

			//get the corresponding id on disk ( in large files , file ids may not be written in sorted order ) 
			size_t id = (size_t)_LogicalToDiskMap[(size_t)index]; //(size_t)index;// 

			size_t tileSize = (size_t)m_tileDim[0] * m_tileDim[1] * m_tileDim[2];

			void* rbf = getBuffer(id * tileSize * sizeof(unsigned short) + _HeaderPos, m_tileDim[0] * m_tileDim[1] * m_tileDim[2] * 2);

			memcpy(buffer, rbf, m_tileDim[0] * m_tileDim[1] * m_tileDim[2] * 2);

			//unmap the buffer 
			tileBuffer->unmap();

			return  tileBuffer;
		}


		const imt::volume::VolumeTopoOctree* OptimizedZXOReader::topoTree() const
		{
		   
			return _TopoTree;
		
		}

		void OptimizedZXOReader::getSubSlice(const SbBox2i32&, int, void *)
		{
			fprintf(stderr, "OptimizedZXOReader::getSubSlice : Not Implemented\n");
			return;


		}


		SbBool			OptimizedZXOReader::isThreadSafe()
		{
			return SbBool(TRUE);
		}




		
		
		
	}
	
}