
#include "CustomTileManager.h"
#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/devices/SoGLContext.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/nodes/SoVolumeSkin.h>
#include <LDM/LDMDefaultSliceAccessor.h>
#include <list>
#include "opencvincludes.h"

namespace imt{

	namespace volume{



		CustomTileManager::CustomTileManager()
		{
			_TopoTree = new SoLDMTopoOctree;

			_TopoTree2 = new imt::volume::VolumeTopoOctree();

			//_TileLoader = 0;

			_DataAccess = 0;

			_ZXOReader = 0;

			_CachedZXOReader = 0;
		}


		void CustomTileManager::setTileManager(SoLDMTileManager *)
		{
			//_TileManager = tileManager;
		}



		void CustomTileManager::setDataAccess(SoLDMDataAccess* pDataAccess)
		{
			_DataAccess = pDataAccess;
		}


		void CustomTileManager::setZXOReader(imt::volume::OptimizedZXOReader *zxoReader)
		{
			_ZXOReader = zxoReader;




		}

		void CustomTileManager::setMaxMainMemoryUsage(const unsigned int& memoryInMB)
		{
			mMaxMemoryCap = memoryInMB;
		}


		void CustomTileManager::loadFixedResolutionTiles()
		{
			//double tileMemory = m 

		}

		void CustomTileManager::init(SbVec3i32& volumeDim, int tileDim)
		{
			_TopoTree->init(volumeDim, tileDim);


			imt::volume::VolumeTopoOctree::Vec3i dim;

			dim._X = volumeDim[0];
			dim._Y = volumeDim[1];
			dim._Z = volumeDim[2];

			_TopoTree2->init(dim, tileDim, 0);

		}

		void CustomTileManager::init(SoLDMTileID)
		{
			//std::cout << " init num tiles to load "<< tilesToLoad << std::endl;




			//_TileManager->init(tilesToLoad);
		}


		void CustomTileManager::startNumDataNotify(int, int)
		{
			//std::cout << " start num data notify " <<id<<" "<<entry<< std::endl;

			//_TileManager->startNumDataNotify(id, entry);
		}


		/**
		* Indicates that LDM has finish to modify/upload new data data.
		*/
		void CustomTileManager::endNumDataNotify(bool, bool, bool)
		{

			//std::cout << " end num data notify " << std::endl;

			//_TileManager->endNumDataNotify( isAdded , sync , shouldInit );
		}


		SbVec3i32 CustomTileManager::getTileDimension(SoLDMTileID tileID) const
		{

			SbVec3i32 dim = _TopoTree->getTileDimAtLevel(tileID);

			std::cout << " get tile dimension  " << std::endl;

			//return _TileManager->getTileDimension(tileID);

			return dim;
		}

		void CustomTileManager::startTransaction()
		{
			//std::cout << " start transaction " << std::endl;

			//prioritizeTiles();//.Only called once for all geometry.
			//getMinMax();// .Only called once for all geometry and if
			//	*   ignoring the fully transparent tiles.
			//setAllowLoading();// if NO_USER_INTERACTION mode is On.Only called once for all geometry.
			//resetWeights//(for topology evaluation).Only called once for all geometry.
			//checkResidencyAndUpdateWeight//(for octree evaluation).Only called once for all geometry.
			//isInMemory//.Always called per geometry to calculate the texture front.
			//*   Allows a tile manager user to know what tiles should be locked during a given transaction
			//	*   until #getTile is called.
			//		*   #getTile or #getTileExt.Called per geometry if there is a texture front change or if more texture can be loaded.

			//std::cout << " starting transaction " << std::endl;

			//auto context = SoGLContext::getCurrent(true);

			//if (SoVolumeRendering::getMaxTexMemory() != 4096)
			//	SoVolumeRendering::setMaxTexMemory(4096);

			//_TileManager->startTransaction();

			//if (context)
			//	std::cout << " device name : " << context->getDeviceName() <<" "<<SoVolumeRendering::getMaxTexMemory()<< std::endl;
			//else
			//	std::cout << " no current context found " << std::endl;
			//std::cout << " transaction started " << std::endl;

			//prioritizeTiles(0);
			//getMinMax()
		}


		void CustomTileManager::endTransaction()
		{
			//_TileManager->endTransaction();

			//if (mAllowLoading)
			//{
			//	std::cout << " loading allowed " << std::endl;
			//}
			//else
			//{
			//	std::cout << " loading not allowed " << std::endl;
			//}
		}


		void CustomTileManager::loadFixedResidentTiles()
		{

			unsigned int cacheSize = 32;

			_CachedZXOReader = new LRUCachedZXOReader(_ZXOReader, _TopoTree2, _TopoTree2->getNumFiles(), _TopoTree2->getNumTiles(), cacheSize);

			int tr = 1;// _TopoTree->getLevelMax() - _TargetResolution; //

			//std::cout << " target resolution  " << tr << std::endl;

			//double memoryNeeded = 256 * 256 * 256 / (1024 * 1024); //numFileIds * 

			//std::cout << " memory required : " << memoryNeeded * numFileIds << std::endl;

			//std::cout << " number of file ids : " << numFileIds << std::endl;

			unsigned int numTiles = _TopoTree2->getNumTiles();

			int maxLevel = _TopoTree->getLevelMax();



			if (_ZXOReader)
			{

				auto zxoTopoTree = _ZXOReader->topoTree();

				//std::cout << " loading fixed resolution tiles : " << zxoTopoTree->getNumTiles() << " " << zxoTopoTree->getNumFiles() << std::endl;

				for (int tt = maxLevel; tt >= 0; tt--)
				{
					const auto zxoFileNodes = zxoTopoTree->getFileNodes(maxLevel);

					auto fileNodes2 = _TopoTree2->getFileNodes(maxLevel);

					size_t nFileNodes = fileNodes2.size();

					//for (auto fileNode : zxoFileNodes)
					for (size_t ff = 0; ff < nFileNodes; ff++)
					{
						auto node1 = zxoFileNodes[ff];
						auto node2 = fileNodes2[ff];

						*node2 = *node1;
					}
				}
			}


			//return;

			_TileIdToFixedResidenceMap.resize(numTiles);
			_TileIdToVariableResidenceMap.resize(numTiles);
			_IsTileLocked.resize(numTiles);

			std::fill(_TileIdToFixedResidenceMap.begin(), _TileIdToFixedResidenceMap.end(), -1);
			std::fill(_TileIdToVariableResidenceMap.begin(), _TileIdToVariableResidenceMap.end(), -1);
			std::fill(_IsTileLocked.begin(), _IsTileLocked.end(), 0);

			std::vector< imt::volume::VolumeTopoOctree::TopoOctreeNode* > fileNodes;

			_NumFixedTiles = 0;



			_TileWeights.clear();

			for (int tt = maxLevel; tt >= 0; tt--)
			{
				auto levelFileNodes = _TopoTree2->getFileNodes(maxLevel - tt);

				std::cout << " num file nodes : " << levelFileNodes.size() << std::endl;

				if (tt >= tr)
				{
					_NumFixedTiles += (unsigned int)levelFileNodes.size();
				}
				else
				{
					int init = (int)_TileWeights.size();

					_TileWeights.resize(_TileWeights.size() + levelFileNodes.size());

					for (int ll = init; ll < _TileWeights.size(); ll++)
					{
						_TileWeights[ll].first = levelFileNodes[ll - init]->_NodeId;
						_TileWeights[ll].second = -10;
					}
				}

				fileNodes.insert(fileNodes.end(), levelFileNodes.begin(), levelFileNodes.end());
			}

			std::sort(_TileWeights.begin(), _TileWeights.end(), [](const std::pair<int, double>& item1, const std::pair<int, double>& item2){

				return item1.first < item2.first;
			});

			//std::cout << " num of fixed resolution tiles : " << _NumFixedTiles << std::endl;

			//std::cout << " num of figh resolution tiles : " << _TileWeights.size() << " " << _TileWeights[0].first << " " << _TileWeights.back().first << " " 
			//	<< (_TileWeights.back().first - _TileWeights[0].first + 1) << std::endl;

			int numFileIds = (int)fileNodes.size();

			_FixedResidentTiles.resize(numFileIds);

			_IsTileLocked.resize(numFileIds);

			_IsTileInMemory.resize(numFileIds);

			_TileWeights.resize(fileNodes.size() - _NumFixedTiles);

			std::fill(_IsTileLocked.begin(), _IsTileLocked.end(), 0);
			std::fill(_IsTileInMemory.begin(), _IsTileInMemory.end(), 0);

			for (int ff = 0; ff < numFileIds; ff++)
			{
				_FixedResidentTiles[ff] = 0;
			}


			double _initT = cv::getTickCount();

			//_VariableResidentTiles.resize()

			//#pragma omp parallel for
			for (int ff = 0; ff < numFileIds; ff++) //(auto fileNode : fileNodes)
			{
				auto fileNode = fileNodes[ff];

				unsigned int tileId = fileNode->_TileId;

				_TileIdToFixedResidenceMap[tileId] = ff;

				unsigned int fileId = fileNode->_NodeId;

				auto tileSize = _TopoTree->getTileSize();

				// Call with null pointer to get size of data
				int resolution = _TopoTree->getLevelMax() - _TopoTree->level(tileId);

				SbBox3i32 box = _TopoTree->getTilePos(tileId);

				_FixedResidentTiles[ff] = (SoCpuBufferObject*)_CachedZXOReader->readTile(tileId);

				_FixedResidentTiles[ff]->ref();

				if (resolution > 0)
					_IsTileInMemory[fileId] = true;

			}

			std::cout << " time spent in data loading " << (cv::getTickCount() - _initT) / cv::getTickFrequency() << std::endl;

		}

		void CustomTileManager::loadTiles()
		{
			//if (!_TileLoader)
			//{
			//	_TileLoader = new TileLoaderThread( _TilesToLoad  , _TilePositions, _VariableResidentTiles , _VariableResidentTileIds ,
			//		_AllowLoading, _CachedZXOReader, _TopoTree);
			//}

			//if (_TileLoader->isRunning())
			//{
			//	_TileLoader->wait();
			//}

#if 0
			auto tileWeights = _TileWeights;

			//double initT = cv::getTickCount();

			//sort the tiles with weights
			std::sort(tileWeights.begin(), tileWeights.end(), [](const std::pair<int, double>& item1, const std::pair<int, double>& item2)->bool
			{
				return item1.second > item2.second;
			});

			int nTiles = (int)tileWeights.size();

			std::vector< unsigned int >  tilePositions;

			_CandidateTiles.clear();

			//collect the candidate tiles
			for (int tt = 0; tt < nTiles; tt++)
			{
				if (!_IsTileLocked[tileWeights[tt].first] && _IsTileInMemory[tileWeights[tt].first] && tileWeights[tt].second > DBL_EPSILON) //&& !isInMemory(tileWeights[tt].first)
				{
					_CandidateTiles.push_back(tileWeights[tt].first);
				}
			}

			int nSlots = (int)_CandidateTiles.size();

			unsigned int numLoadedTiles = _NumLoadedTiles;

			unsigned int backShift = 0;

			while (numLoadedTiles < _VariableResidentTiles.size())
			{
				tilePositions.push_back(numLoadedTiles);

				numLoadedTiles++;

				nSlots--;

				backShift++;

				if (nSlots == 0)
					break;
			}

			if (nSlots > 0)
			{
				for (int tt = nTiles - 1 - backShift; tt >= 0; tt--)
				{
					int tileId = tileWeights[tt].first;

					if (_IsTileLocked[tileId])
						continue;

					int variableResidenceId = _TileIdToVariableResidenceMap[tileId];

					tilePositions.push_back(variableResidenceId);
				}
			}


			_TilesToLoad = _CandidateTiles;
			_TilePositions = tilePositions;

			_TilesToLoad.resize(_TilePositions.size());

			//std::cout << " num candidate tiles " << _TilesToLoad.size() << std::endl;

			//if (_TilesToLoad.size() > 0)
			//{
			//	_TileLoader->start();
			//}
#endif

		}


		unsigned int CustomTileManager::numVariableResidentTiles()
		{
			unsigned int numVariableResidentTiles = 0;


			return numVariableResidentTiles;
		}

		void CustomTileManager::setTargetResolution(unsigned int resolution)
		{
			_TargetResolution = resolution;
		}


		void CustomTileManager::setVariableResidenceMemorySize(unsigned int memorySizeInMB)
		{

#if 0
			_VariableResidenceMemory = memorySizeInMB;

			auto tileSize = _TopoTree->getTileSize();

			unsigned int memorySize = tileSize[0] * tileSize[1] * tileSize[2] * sizeof(unsigned short);

			unsigned int tileSizeInMB = memorySize / (1024 * 1024);

			auto multiplier = 1;

			if (memorySize < 1024 * 1024 * sizeof(unsigned short))
			{
				multiplier = (1024 * 1024 * sizeof(unsigned short)) / memorySize;
			}

			if (multiplier > 1)
				_MaxVariableTiles = memorySizeInMB * multiplier;
			else
				_MaxVariableTiles = memorySizeInMB / tileSizeInMB;

			std::cout << " max variable num tiles : " << _MaxVariableTiles << std::endl;

			_VariableResidentTileIds.resize(_MaxVariableTiles);

			_TileWeights.resize(_MaxVariableTiles);
#endif
		}



		bool CustomTileManager::checkResidencyAndUpdateWeight(SoLDMTileID tileID, float weight)
		{
			unsigned int resolution = _TopoTree->getLevelMax() - _TopoTree->level(tileID.getID());

			if (resolution == 0)
			{
				auto fileId = _TopoTree->getFileID(tileID);

				int vId = fileId - _NumFixedTiles;

				_TileWeights[vId].first = (unsigned int)tileID.getID();

				_TileWeights[vId].second = weight;
			}

			if (_TileIdToFixedResidenceMap[tileID.getID()] >= 0)
			{
				return true;
			}

			bool updateSuccessfull = false;

			return updateSuccessfull;
		}

		bool CustomTileManager::isInMemory(SoLDMTileID tileID) const
		{
			if (tileID.getID() >= (int64_t)_TileIdToFixedResidenceMap.size())
			{
				return false;
			}

			if (_TileIdToFixedResidenceMap[tileID.getID()] >= 0)
			{
				return true;
			}

			return false;
		}


		bool CustomTileManager::isDataInMemory(const SoLDMTileID& tileID) const
		{
			if (_TileIdToFixedResidenceMap[tileID.getID()] >= 0)
			{
				return true;
			}

			return false;

		}


		bool CustomTileManager::requestRefetch(const SoLDMTileID&, const double) const
		{
			//std::cout << " request prefetch tile Id : " << tileID << " " << weight << std::endl;
			//return _TileManager->requestRefetch( tileID , weight );

			bool requestrefetchSuccessfull = false;

			return requestrefetchSuccessfull;
		}


		const SoBufferObject* CustomTileManager::getTile(SoLDMTileID tileID, unsigned short)
		{
			std::cout << " get tile : " << tileID << std::endl;

			if (_TileIdToFixedResidenceMap[tileID.getID()] >= 0)
			{
				return _FixedResidentTiles[_TileIdToFixedResidenceMap[tileID.getID()]];
			}

			//if (_TileIdToVariableResidenceMap[tileID.getID()] >= 0)
			//{
			//	return _VariableResidentTiles[_TileIdToVariableResidenceMap[tileID.getID()]];
			//}

			return 0;
		}


		/**
		* This function must return the data associated with the given tileID and dataSetId
		* but it must be stored as a stack of slices along the X Inventor axis.
		* Only called if the useExtendedData flag from SoDataSet is TRUE. Allows equalizing performance
		* of axis aligned slices roaming through a data set.
		*/
		const SoBufferObject* CustomTileManager::getTileExt(SoLDMTileID tileID, unsigned short dataSetId)
		{
			//std::cout << " get tile ext " << tileID << " " << dataSetId << std::endl;

			//return _TileManager->getTileExt(tileID, dataSetId);

			return 0;
		}


		SoLDMTileManager::LDMSliceAccessor* CustomTileManager::getLdmSliceAccessor(const SoLDM::DataSetIdPair& p)
		{
			//std::cout << " get slice accessor " << std::endl;

			return new LDMDefaultSliceAccessor(p); //_TileManager->getLdmSliceAccessor(p);
		}


		bool CustomTileManager::getMinMax(SoLDMTileID tile, double& min, double& max) const
		{
			bool hasMinMax = true;

			int fileId = _TopoTree2->getFileId((int)tile.getID());

			auto fileNode = _TopoTree2->getFileNode(fileId);

			min = fileNode->_MinVal;
			max = fileNode->_MaxVal;

			return hasMinMax;
		}


		void CustomTileManager::getTileData(const std::vector<SoLDMTileID>& tileIDs, int&, unsigned short, SoLDMDataAccess*)
		{
			//std::cout << " get tile data multiple " << tileIDs.size() << " " << volumeDataId << std::endl;

			std::vector< SoLDMTileID > filteredTileIds;

			for (auto tileId : tileIDs)
			{
				if (_TileIdToFixedResidenceMap[tileId.getID()] >= 0)
					continue;

				filteredTileIds.push_back(tileId);
			}

			//_TileManager->getTileData(tileIDs, errorIndicator, volumeDataId, pDataAccess);

			//_TileManager->getTileData(filteredTileIds, errorIndicator, volumeDataId, pDataAccess);
		}


		bool CustomTileManager::requestTileData(int requestId, int& memError, SoLDMDataAccess& dataAccess)
		{
			std::cout << " request tile data " << std::endl;

			return _TileManager->requestTileData(requestId, memError, dataAccess);
		}


		void CustomTileManager::releaseTileData(SoLDMTileID tileID, unsigned short dataSetId)
		{
			std::cout << " release tile data " << std::endl;
			_TileManager->releaseTileData(tileID, dataSetId);
		}


		void CustomTileManager::resourceChangeNotify()
		{
			//_TileManager->resourceChangeNotify();
		}


		void CustomTileManager::setAllowLoading(bool allowLoad)
		{
			//std::cout << " set allow loading " << allowLoad << std::endl;

			_AllowLoading = allowLoad;

			//if (mAllowLoading)
			//{
			//	std::cout << " allow loading " << std::endl;
			//}
			//else
			//{
			//	std::cout << " disallow loading " << std::endl;
			//}

			//if (_AllowLoading)
			//	loadTiles();

			//_TileManager->setAllowLoading(allowLoad);
		}

		void CustomTileManager::resetWeights() //impo
		{
			//std::cout << " reset wights " << std::endl;

			//_TileManager->resetWeights();
		}


		void CustomTileManager::prioritizeTiles(SoLDMTileID maxTileId)
		{

			std::cout << " prioritize tiles " << maxTileId << std::endl;

			//_TileManager->prioritizeTiles( maxTileId );
		}

		void CustomTileManager::updateRegions(const SbBox3i32*, int)
		{

			//std::cout << " update regions " << std::endl;

			//_TileManager->updateRegions(region, numRegions);
		}

		int CustomTileManager::getMemoryUsed() const
		{
			std::cout << " get memory used " << std::endl;

			return 4096;//_TileManager->getMemoryUsed();
		}


		void CustomTileManager::flushTilesOfRes(int resolution)
		{
			std::cout << " flush tiles of Res " << resolution << std::endl;

			//return _TileManager->flushTilesOfRes(resolution);
		}

		/**
		* Debug purpose only. Used for visual feedback to highlight tiles in main memory.
		* Only called if the SoVolumeRendering::DRAW_TOPOLOGY flag is true.
		*/
		void CustomTileManager::getTileIDInMemory(std::vector<LDM_TILE_ID_TYPE>&) const
		{
			//_TileManager->getTileIDInMemory(tilesInMemory);
		}

		/**
		* Indicates that the manager should be initialized.
		*/
		//virtual void shouldInit() {};

		/**
		* Set the LOCKED state to FALSE for all tiles loaded in memory
		*/
		void CustomTileManager::unLockTiles()
		{
			//std::cout << " unlock tiles " << std::endl; 

			//_TileManager->unLockTiles();
		}

		/**
		* Set the LOCKED state to true for all tile loaded in memory
		*/
		void CustomTileManager::lockTiles()
		{
			//std::cout << " lock tiles " << std::endl;

			//_TileManager->lockTiles();
		}

		/**
		* Set the LOCKED state to true for the given SoLDMTileID
		*/
		void CustomTileManager::lockTile(const SoLDMTileID& tileId)
		{
			//std::cout << " lock tile : " << tileId.getID() << std::endl;

			_IsTileLocked[tileId.getID()] = true;

			//_TileManager->lockTile(tileId);
		}

		/**
		* Set the LOCKED state to false for the given SoLDMTileID
		*/
		void CustomTileManager::unLockTile(const SoLDMTileID& tileId, bool)
		{
			//std::cout << " unlock tile " << std::endl;

			_IsTileLocked[tileId.getID()] = false;

			//_TileManager->unLockTile(tileId, forceUnload);
		}


	}

}

