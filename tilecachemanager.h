#ifndef __IMT_TILECACHEMANAGER_H__
#define __IMT_TILECACHEMANAGER_H__

#include "vector"
#include "volumetopooctree.h"
#include "unordered_map"

namespace imt{
	
	namespace volume{

   template < typename DataType > class CpuVolumeBuffer;
		

   template < typename DataType >
	class TileCacheManager
	{
		
	  public:
		
		TileCacheManager();

		bool isTileInCache( int fileId );

	  protected:


		std::vector< CpuVolumeBuffer< DataType >* > _CachedTiles;

		std::unordered_map< int, VolumeTopoOctree::TopoOctreeNode* > _TileCacheMap;

		VolumeTopoOctree *_TopoTree;
		
	};	
		
		
		
		
	}
	
	
	
}




#endif