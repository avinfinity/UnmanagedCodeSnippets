#include "LRUCachedZXOReader.h"


namespace imt{

	namespace volume{



		template<typename Key, typename Value>	
		LRUCache<Key , Value>::LRUCache(size_t max_size) : _max_size(max_size)
		{
		}


		template<typename Key, typename Value>
		void LRUCache<Key, Value>::put(const Key& key,  Value& value)
			{
				auto it = _CacheItemsMap.find(key);
				_CacheItemsList.push_front(KeyValuePair(key, value));

				if (it != _CacheItemsMap.end())
				{
					_CacheItemsList.erase(it->second);
					_CacheItemsMap.erase(it);
				}

				_CacheItemsMap[key] = _CacheItemsList.begin();

				if (_CacheItemsMap.size() > _max_size)
				{
					auto last = _CacheItemsList.end();
					last--;
					_CacheItemsMap.erase(last->first);
					_CacheItemsList.pop_back();
				}
			}


		template<typename Key, typename Value>
		Value& LRUCache<Key, Value>::get(const Key& key)
			{
				auto it = _CacheItemsMap.find(key);

				if (it == _CacheItemsMap.end())
				{
					throw std::range_error("There is no such key in cache");
				}
				else
				{
					_CacheItemsList.splice(_CacheItemsList.begin(), _CacheItemsList, it->second);

					return it->second->second;
				}
			}

		template<typename Key, typename Value>
		bool LRUCache<Key, Value>::exists(const Key& key) const
			{
				return _CacheItemsMap.find(key) != _CacheItemsMap.end();
			}

		template<typename Key, typename Value>
		size_t LRUCache<Key, Value>::size() const
			{
				return _CacheItemsMap.size();
			}




			LRUCachedZXOReader::BufferObjectData::BufferObjectData(SoBufferObject* buffer) : _Buffer(buffer)
			{
				_Buffer->ref();
			}

			SoBufferObject* LRUCachedZXOReader::BufferObjectData::buffer()
			{
				return _Buffer;			
			}

			LRUCachedZXOReader::BufferObjectData::~BufferObjectData()
			{
				//std::cout << " calling unref " << std::endl;

				//_Buffer->unref();
			}

			LRUCachedZXOReader::LRUCachedZXOReader(OptimizedZXOReader *reader, VolumeTopoOctree *topoTree, unsigned int numFileIds,
				unsigned int numTileIds, unsigned int cacheSize) : _Reader(reader), _TopoTree(topoTree), _CacheSize(cacheSize)
			{
				_NumFileIds = numFileIds;
				_NumTileIds = numTileIds;

				_UsedHistory.resize(cacheSize);
				_TileCacheId.resize(numTileIds);

				_Cache = new LRUCache< unsigned int, BufferObjectData >(cacheSize);

			}

			SoBufferObject* LRUCachedZXOReader::readTile(unsigned int tileId)
			{
				//first check if the tile exist in the cache
				if (_Cache->exists(tileId))
				{
					return _Cache->get(tileId).buffer();
				}

				auto fileId = _TopoTree->getFileId(tileId);

				auto fileNode = _TopoTree->getFileNode(fileId);

				SbBox3i32 box;

				auto tile = _Reader->readTile(fileId, box);

				_Cache->put(tileId, BufferObjectData(tile));

				return _Cache->get(tileId).buffer();
			}

			LRUCachedZXOReader::~LRUCachedZXOReader()
			{

			}



			template class LRUCache< unsigned int, LRUCachedZXOReader::BufferObjectData >;


	}

}
