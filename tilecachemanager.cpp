#include "tilecachemanager.h"


namespace imt{
	
	namespace volume{


		template < typename DataType >
		TileCacheManager< DataType >::TileCacheManager()
		{

		}


		template < typename DataType >
		bool TileCacheManager< DataType >::isTileInCache(int fileId)
		{
			return (_TileCacheMap.find(fileId) != _TileCacheMap.end());
		}

		
		
	}
	
	
	
}