#include "volumedata.h"


namespace imt{
	
	namespace volume{



		template < typename DataType >
		CpuVolumeBuffer< DataType >::CpuVolumeBuffer()
		{
			_IsInitialized = false;
			_Width = 0;
			_Height = 0;
			_Depth = 0;

		}

		template < typename DataType >
		DataType* CpuVolumeBuffer< DataType >::data()
		{
			return _Data;
		}

		template < typename DataType >
		void CpuVolumeBuffer< DataType >::init(int width, int height, int depth)
		{

			if (_IsInitialized)
			{
				delete[] _Data;

				_Width = 0;
				_Height = 0;
				_Depth = 0;

				_IsInitialized = false;
			}

			_Width = width;
			_Height = height;
			_Depth = depth;

			_Data = new DataType[_Width * _Height * _Depth];

			_IsInitialized = true;
		}


		template < typename DataType >
		CpuVolumeBuffer< DataType >::~CpuVolumeBuffer()
		{
			delete[] _Data;
		}
		
		
		template < typename DataType >
		VolumeData< DataType >::VolumeData()
		{
			_TileManager = 0 ; 
		}

		template < typename DataType >
		void VolumeData< DataType >::setZxoFilePath(std::string filePath)
		{
			_FilePath = filePath;
		}
		

		template < typename DataType >
		void VolumeData< DataType >::init()
		{
			_File = fopen(_FilePath.c_str(), "rb");
		}
		

		template < typename DataType >
		void VolumeData< DataType >::getXSlice(int resolution, int xVal)
		{

		}

		template < typename DataType >
		void VolumeData< DataType >::getYSlice(int resolution, int yVal)
		{


		}

		template < typename DataType >
		void VolumeData< DataType >::getZSlice(int resolution, int zVal)
		{

		}

		template < typename DataType >
		void VolumeData< DataType >::getSubvolume(const VolumeRoi& roi, const int resolution, CpuVolumeBuffer< DataType >& volumeBuffer)
		{

		}

		template < typename DataType >
		void VolumeData< DataType >::setCustomTileManager(CustomTileManager *tileManager)
		{
			_TileManager = tileManager;
		}


		template < typename DataType >
		const imt::volume::VolumeTopoOctree* VolumeData< DataType >::topoTree() const
		{
			return mTopoTree;
		}


		template class CpuVolumeBuffer< unsigned char >;
		template class CpuVolumeBuffer< unsigned short >;
		template class CpuVolumeBuffer< unsigned int >;
		template class CpuVolumeBuffer< float >;


		template class VolumeData< unsigned char >;
		template class VolumeData< unsigned short >;
		template class VolumeData< unsigned int >;
		template class VolumeData< float >;

	}
	
}