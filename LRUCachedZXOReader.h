#ifndef __IMT_LRUCACHEDZXOREADER_H__
#define __IMT_LRUCACHEDZXOREADER_H__

#include <VolumeViz/nodes/SoVolumeData.h>
#include "optimizedzxoreader.h"
#include <unordered_map>

namespace imt{
	
	namespace volume{
		

		template<typename Key, typename Value>
		class LRUCache {
		public:
			typedef typename std::pair<Key, Value> KeyValuePair;
			typedef typename std::list<KeyValuePair>::iterator ListIterator;

			LRUCache(size_t max_size);

			void put(const Key& key, Value& value);

		    Value& get(const Key& key);

			bool exists(const Key& key) const;

			size_t size() const;

		private:


			std::list<KeyValuePair> _CacheItemsList;
			std::unordered_map<Key, ListIterator> _CacheItemsMap;
			size_t _max_size;
		};


		class LRUCachedZXOReader
		{

		public:

			class BufferObjectData
			{
				SoBufferObject* _Buffer;

			public:

				BufferObjectData(SoBufferObject* buffer);

				SoBufferObject* buffer();

				~BufferObjectData();

			};

		protected:
			OptimizedZXOReader *_Reader;

			std::vector< unsigned int > _UsedHistory;

			std::list< unsigned int > _TileCacheId;

			unsigned int _NumFileIds, _NumTileIds, _CacheSize;

			VolumeTopoOctree *_TopoTree;

			LRUCache< unsigned int, BufferObjectData >* _Cache;


		public:

			LRUCachedZXOReader( OptimizedZXOReader *reader, VolumeTopoOctree *topoTree, unsigned int numFileIds,
				unsigned int numTileIds, unsigned int cacheSize);

			SoBufferObject* readTile(unsigned int tileId);

			~LRUCachedZXOReader();

		};
		
		
		
	}
	
}


#endif