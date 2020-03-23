#ifndef __IMT_VOLUMEDATA_H__
#define __IMT_VOLUMEDATA_H__

#include "string"
#include "vector"
#include "stdio.h"
#include "volumetopooctree.h"
#include "customtilemanager.h"

namespace imt{
	
	namespace volume{


     template < typename DataType >
	 class CpuVolumeBuffer
	 {
	 
	 public:

		 CpuVolumeBuffer();

		 DataType* data();

		 void init(int width, int height,int depth);

		 ~CpuVolumeBuffer();


	 protected:

		 int _Width, _Height, _Depth;

		 DataType *_Data;

		 bool _IsInitialized;

	 };

		
		

	template < typename DataType >
	class VolumeData
	{
		
		public:

		struct VolumeRoi
		{
			unsigned int _XStart, _YStart, _ZStart;
			unsigned int _XSize, _YSize, _ZSize;
		};


		VolumeData();

		void setZxoFilePath( std::string filePath );

		void init();

		void getXSlice(int resolution, int xVal);
		void getYSlice(int resolution, int yVal);
		void getZSlice(int resolution, int zVal);

		void getSubvolume( const VolumeRoi& roi, const int resolution, CpuVolumeBuffer< DataType >& volumeBuffer );

		void setCustomTileManager(CustomTileManager *tileManager);

		const imt::volume::VolumeTopoOctree* topoTree() const;
		

	protected:


		std::string _FilePath;
		FILE *_File;

		imt::volume::VolumeTopoOctree *mTopoTree;

		CustomTileManager *_TileManager;
		
	};
		
		
		
		
	}
	
	
	
	
	
}





#endif