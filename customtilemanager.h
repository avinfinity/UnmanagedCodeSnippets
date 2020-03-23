#ifndef __IMT_CUSTOMTILEMANAGER_H__
#define __IMT_CUSTOMTILEMANAGER_H__

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
#include <LDM/readers/SoLDMReader.h>
#include <LDM/SoLDMNodeFrontManager.h>
#include <LDM/SoLDMTileManager.h>
#include <VolumeViz/readers/SoVolumeReader.h>
#include <VolumeViz/readers/SoVRLdmFileReader.h>

#include <unordered_map>
#include <queue>
#include <unordered_set>
#include "volumetopooctree.h"
#include "optimizedzxoreader.h"
#include "QThread"
#include "LRUCachedZXOReader.h"


namespace imt{

	namespace volume{





		class CustomTileManager : public SoLDMTileManager
		{


		public:

			CustomTileManager();


			void setTileManager(SoLDMTileManager *tileManager);

			void setDataAccess( SoLDMDataAccess* pDataAccess );

			void setZXOReader(imt::volume::OptimizedZXOReader *zxoReader);

			void init(SbVec3i32& volumeDim, int tileDim);

			void setMaxMainMemoryUsage( const unsigned int& memoryInMB );

			void loadFixedResolutionTiles();


			/**
			* Function of initialization. This function is called only once after the SoLDMNodeFrontManager
			* initialization (so the tile manager can query the number of data tiles to manage by calling
			* getNodeFrontManager()->getNumFileIDs()).
			* The input parameter indicates which tiles should be in memory after initialization.
			* All tileIds between 0 and up to tilesToLoad must be in memory.
			*/
			virtual void init(SoLDMTileID tilesToLoad);

			/**
			* Indicates that LDM starts to modify/upload new data data.
			*/
			virtual void startNumDataNotify(int id = -1, int entry = -1);

			/**
			* Indicates that LDM has finish to modify/upload new data data.
			*/
			virtual void endNumDataNotify(bool isAdded, bool sync, bool shouldInit = false);

			/**
			* Get Tile dimension. For the default topology (octree) this function should
			* always return the same size.
			*/
			virtual SbVec3i32 getTileDimension(SoLDMTileID tileID) const;

			/**
			* Functions to keep memory state stable when querying state.
			* Indicates a memory transaction is starting.
			* Memory state should not change until #endTransaction is called.
			* A transaction is defined as follow:
			* startTransaction() indicates the following set of functions will/might be called to analyze how
			* texture front should evolve and what set of tiles to use to render a particular geometry.
			* Note that the data front managed by the tile manager should evolve based on the weights passed by
			* #checkResidencyAndUpdateWeight.
			* For a given transaction, functions will be called in the following order:
			* #startTransaction ()
			* {
			*   #prioritizeTiles (for topology evaluation). Only called once for all geometry.
			*   #getMinMax (for topology evaluation). Only called once for all geometry and if
			*   ignoring the fully transparent tiles.
			*   #setAllowLoading if NO_USER_INTERACTION mode is On. Only called once for all geometry.
			*   #resetWeights (for topology evaluation). Only called once for all geometry.
			*   #checkResidencyAndUpdateWeight (for octree evaluation). Only called once for all geometry.
			*   #isInMemory. Always called per geometry to calculate the texture front.
			*   Allows a tile manager user to know what tiles should be locked during a given transaction
			*   until #getTile is called.
			*   #getTile or #getTileExt. Called per geometry if there is a texture front change or if more texture can be loaded.
			* }
			* #endTransaction()
			*/
			virtual void startTransaction();

			/**
			* Notify a transaction has ended.
			*/
			virtual void endTransaction();

			/**
			* Returns whether the input tile resides in memory and update its weight.
			* Called when the tile visitor evaluates the topology.
			* The input weight is an indication of how LDM prioritize data for the current
			* scene state. Tiles of higher weight should arrive in memory first.
			* Note that the root tile must always resides in memory.
			*/
			virtual bool checkResidencyAndUpdateWeight(SoLDMTileID tileID, float weight);

			/**
			* Indicates whether a tile is in main memory.
			* Called when the node front manager evaluates the texture front.
			*/
			virtual bool isInMemory(SoLDMTileID tileID) const;

			/**
			* Indicates whether the data attached to a tile is in main memory.
			* Called when the node front manager evaluates the texture front.
			*
			* This allows to manage data that implements SoBufferAsyncInterface.
			* If the specified tile does not implement SoBufferAsyncInterface then
			* this method always returns true.  So effectively it always returns
			* true for non-LDM volumes.
			*/
			virtual bool isDataInMemory(const SoLDMTileID& tileID) const;

			/**
			* Launch an asynchronous request for the data attached to the specified tile.
			*
			* This allows to manage data that implements SoBufferAsyncInterface.
			*/
			virtual bool requestRefetch(const SoLDMTileID& tileID, const double weight) const;


			/**
			* Gets the data buffer associated with a tile for a given data set. LDM expects the data to be stored
			* as a stack of slices along the Z Inventor axis.
			* Called after texture front evaluation. Only called on tiles that the tile manager declared as
			* resident in memory. The input tileID has been called previously by the #isInMemory function
			* and the answer was true.
			*/
			virtual const SoBufferObject* getTile(SoLDMTileID tileID, unsigned short dataSetId);

			/**
			* This function must return the data associated with the given tileID and dataSetId
			* but it must be stored as a stack of slices along the X Inventor axis.
			* Only called if the useExtendedData flag from SoDataSet is TRUE. Allows equalizing performance
			* of axis aligned slices roaming through a data set.
			*/
			virtual const SoBufferObject* getTileExt(SoLDMTileID tileID, unsigned short dataSetId);

			/**
			* Allows accessing a 2D buffer for slice and volumeSkin rendering. @BR
			* When needing to render an SoOrthoSlice or SoVolumeSkin, LDM accesses the 2D buffer needed
			* by calling the functions of the tile manager inner slice accessor object.
			*/
			//class LDM_API LDMSliceAccessor {

			//public:
			//	/**
			//	* Default Destructor.
			//	*/
			//	virtual ~LDMSliceAccessor() {}

			//	/**
			//	* Access the 2D buffer of the given slice along the sliceAxis axis at sliceNumber position
			//	* within the tile. @BR See #releaseOrthoSliceBuffer.
			//	*/
			//	virtual SoBufferObject* getOrthoSliceBuffer(const SoLDMTileID& tile, int sliceAxis, int slice, SoState * state = NULL, bool useExtendedData = false);

			//	/**
			//	* Release the buffer returned by #getOrthoSliceBuffer.
			//	*/
			//	virtual void releaseOrthoSliceBuffer(SoBufferObject*);

			//	/**
			//	* Specifies the kind of slice the accessor is managing.
			//	*/
			//	virtual void setLargeSliceSupport(SbBool flag);
			//};

			/**
			* Return an instance of slice accessor handling the data corresponding to the given data set/id pair.
			* Called when rendering an SoOrthoSlice.
			* When subclassing from the tile manager there is no immediate need to redefine the functions of the
			* slice accessor. LDM already handles the extraction of the buffer from the 3D tile.
			* One can simply use the LDMDefaultSliceAccessor.
			*/
			virtual LDMSliceAccessor* getLdmSliceAccessor(const SoLDM::DataSetIdPair& p);

			/**
			* Gets the minimum and maximum values in a tile of data. @BR
			* The returned value indicates whether it has been calculated yet or in other words, if the tile
			* was ever loaded.
			* Only called if ignoring fully transparent tiles (see SoVolumeRendering).
			*/
			virtual bool getMinMax(SoLDMTileID tile, double& min, double& max) const;

			/**
			* This function must ensure that all the data associated with the input set of tiles is loaded in
			* main memory when returning. The data associated to a given tileID must be locked in memory
			* until #releaseTileData is called.
			* Called when using the data access functions (getData) from the SoLDMDataAccess.
			*/
			virtual void getTileData(const std::vector<SoLDMTileID>& tileIDs, int& errorIndicator, unsigned short volumeDataId, SoLDMDataAccess* pDataAccess);

			/**
			* Initiates an asynchronous request and returns immediately.
			* It must use dataAccess->getRequestedTile() to get the requested tile list.
			* Returns true if the request is in fact synchronous (all tiles were already loaded).
			* [OIVJAVA-WRAPPER PACK{AsyncRequestInfo}]
			*/
			virtual bool requestTileData(int requestId, int& memError, SoLDMDataAccess& dataAccess);

			/**
			* Releases previously requested data associated with a given tileID from memory.
			* If supported, dataSetId allows you to specify which data set should be released.
			* -1 means all data sets in the MultiDataVolumeGroup.
			*/
			virtual void releaseTileData(SoLDMTileID tileID, unsigned short dataSetId = -1);

			/**
			* Notification of a memory resource change.
			* If the application changes the main memory resource using the SoLDMResourceManager,
			* resourceChangeNotify is called to notify the SoLDMTileManager that more tiles can be
			* loaded or some must be unloaded.
			*/
			virtual void resourceChangeNotify();

			/**
			* This function is called when using the NO_USER_INTERACTION mode.
			* This mode should essentially allow loading only when the user does not interact with the scene.
			*/
			virtual void setAllowLoading(bool allowLoad);

			/**
			* Reset weights to init the topology before evaluating it. This function is called
			* before a topology evaluation starting with checkResidencyAndUpdateWeight(0, weight).
			*/
			virtual void resetWeights();

			/**
			* Allows to let the user know what tiles are prioritary and should not be reset
			* in the resetWeight functions.
			* All tileIds between 0 and up to maxTileId are prioritary.
			* This function is called right before the resetWeight function.
			*/
			virtual void prioritizeTiles(SoLDMTileID maxTileId);

			/**
			* Update a region of data in memory. All the tiles intersecting the regions must
			* be refetched.
			* Called if the application uses the SoDataSet::updateRegion function.
			*/
			virtual void updateRegions(const SbBox3i32* region, int numRegions);

			/**
			* This function is only used if in multiple data mode.
			* There was a scene graph change in multiple data mode and only one tile manager must remain
			* for all data sets involved. LDM will keep the one that has loaded the more data.
			*/
			virtual int getMemoryUsed() const;

			/**
			* Flush tiles of a given resolution from memory pool.
			* Called only if the fixedResolution LDM mode is ON or if the maximum resolution threshold changes
			* (See LDMResourceParameter).
			*/
			virtual void flushTilesOfRes(int resolution);

			/**
			* Debug purpose only. Used for visual feedback to highlight tiles in main memory.
			* Only called if the SoVolumeRendering::DRAW_TOPOLOGY flag is true.
			*/
			virtual void getTileIDInMemory(std::vector<LDM_TILE_ID_TYPE>& tilesInMemory) const;

			/**
			* Indicates that the manager should be initialized.
			*/
			virtual void shouldInit() {};

			/**
			* Set the LOCKED state to FALSE for all tiles loaded in memory
			*/
			virtual void unLockTiles();

			/**
			* Set the LOCKED state to true for all tile loaded in memory
			*/
			virtual void lockTiles();

			/**
			* Set the LOCKED state to true for the given SoLDMTileID
			*/
			virtual void lockTile(const SoLDMTileID& tileId);

			/**
			* Set the LOCKED state to false for the given SoLDMTileID
			*/
			virtual void unLockTile(const SoLDMTileID& tileId, bool forceUnload = false);

			void loadTiles();

			void loadFixedResidentTiles();

			unsigned int numVariableResidentTiles();

			void setTargetResolution(unsigned int resolution);

			void setVariableResidenceMemorySize(unsigned int memorySizeInMB);



			protected:


			SoLDMTileManager *_TileManager;

			SoLDMDataAccess* _DataAccess;

			SoLDMTopoOctree *_TopoTree;
			imt::volume::VolumeTopoOctree *_TopoTree2;



			std::vector< SoCpuBufferObject* > _FixedResidentTiles;
			std::vector< SoCpuBufferObject* > _VariableResidentTiles;

			unsigned int _NumFixedTiles;

			std::vector< int > _VariableResidentTileIds;
			std::vector< unsigned char > _IsTileLocked , _IsTileInMemory;


			std::unordered_set< unsigned int > _TilesToFlush;

			std::vector< int > _TileIdToVariableResidenceMap , _TileIdToFixedResidenceMap;

			std::vector< std::pair< unsigned int, double > > _TileWeights;

			imt::volume::OptimizedZXOReader *_ZXOReader;

			LRUCachedZXOReader *_CachedZXOReader;

			std::vector< unsigned int > _TilesToLoad, _TilePositions, _CandidateTiles;

			unsigned int _NumLoadedTiles;

			bool _AllowLoading;

			unsigned int mMaxMemoryCap, _VariableResidenceMemory , _MaxVariableTiles;

			unsigned int _TargetResolution;

			protected:

				class TileLoaderThread : public QThread
				{

				public:

					TileLoaderThread( std::vector<unsigned int>& tileIds, std::vector<unsigned int>& tilePositions, std::vector< SoCpuBufferObject* >& residentTiles, 
						std::vector<int>& residentTileIds, bool& loadFlag, LRUCachedZXOReader* zxoReader, SoLDMTopoOctree *topoTree);


					virtual void run();


				protected:

					std::vector< SoCpuBufferObject* >& _ResidentTiles;

					bool& _LoadFlag;

					//SoLDMDataAccess* _DataAccess;

					LRUCachedZXOReader* _TileReader;

					SoLDMTopoOctree *_TopoTree;

					std::vector<unsigned int>& _TileIds, &_TilePositions;

					std::vector<int>& _VariableResidentTileIds;


				};



				TileLoaderThread *_TileLoader;

		};


	}


}




#endif