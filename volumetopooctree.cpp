#include "volumetopooctree.h"
#include <algorithm>
#include <iostream>

namespace imt{

	namespace volume{


		VolumeTopoOctree::VolumeTopoOctree()
		{
			_RootNode = 0;

			_NumLevels = 0;

			_NodeCount = 0;

			_TileCount = 0;

			_TileDim = 256;

			_Overlap = 0;

		}


		VolumeTopoOctree::TopoOctreeNode::TopoOctreeNode()
		{

			for (int cc = 0; cc < 8; cc++)
         	 _Chidren[cc] = 0;

			_Data = 0;

			_MinVal = 0;
			_MaxVal = 0;
		};


		void VolumeTopoOctree::init( Vec3i volumeDim , int tileDim , int overlap )
		{
			_VolumeDim = volumeDim;
			_TileDim = tileDim;
			_Overlap = overlap;


			if (_RootNode)
			{
				destroyTree();
			}


			_RootNode = new TopoOctreeNode;

			_RootNode->_Position._X = 0;
			_RootNode->_Position._Y = 0;
			_RootNode->_Position._Z = 0;

			_RootNode->_Level = 0;

			_RootNode->_Data = 0;

			_NodeCount++;

			int maxDim = 0;

			maxDim = std::max( _VolumeDim._X, _VolumeDim._Y);
			maxDim = std::max( maxDim, _VolumeDim._Z);

			int tempDim = maxDim;

			int count = 0;

			while (tempDim > 0)
			{
				tempDim = tempDim >> 1;

				count++;
			}

			int tileExponent = 0;

			int td = _TileDim;

			while (td > 0)
			{
				td = td >> 1;

				tileExponent++;
			}

			tileExponent--;

			int extendedDim = 1 << count;

			_NumLevels = count - tileExponent + 1;

			if ( extendedDim / 2 == maxDim)
				_NumLevels--;

			buildTree();

			_TileCount = 0;

			assignFileIds();

		}


		void VolumeTopoOctree::buildTree()
		{
			int tileSize = _TileDim * (1 << (_NumLevels - 1));

			int xMin = 0;
			int yMin = 0;
			int zMin = 0;

			int childTileSize = _TileDim * (1 << (_NumLevels - 2));

			for (int zz = 0; zz < 2; zz++)
				for (int yy = 0; yy < 2; yy++)
					for (int xx = 0; xx < 2; xx++)
					{
						int xSplit = xMin + childTileSize * xx;
						int ySplit = yMin + childTileSize * yy;
						int zSplit = zMin + childTileSize * zz;

						if (xSplit < _VolumeDim._X && ySplit < _VolumeDim._Y && zSplit < _VolumeDim._Z)
						{
							_RootNode->_Chidren[zz * 4 + yy * 2 + xx] = new TopoOctreeNode;

							_NodeCount++;

							Vec3i pos;

							pos._X = xSplit;
							pos._Y = ySplit;
							pos._Z = zSplit;


							if ( _NumLevels >  2 )
							{
								buildNode(_RootNode->_Chidren[zz * 4 + yy * 2 + xx], 1, pos);
							}
							else
							{
								_RootNode->_Chidren[zz * 4 + yy * 2 + xx]->_Position = pos;
							}

						}

					}

		}


		void VolumeTopoOctree::buildNode( TopoOctreeNode *node ,  int resolutionLevel , Vec3i pos )
		{
			int tileSize = _TileDim * ( 1 << (_NumLevels - resolutionLevel - 1) );

			int xMin = pos._X, yMin = pos._Y, zMin = pos._Z;

			int childTileSize = _TileDim * (1 << (_NumLevels - resolutionLevel - 2));

			node->_Position = pos;

			for (int zz = 0; zz < 2; zz++)
				for (int yy = 0; yy < 2; yy++)
					for (int xx = 0; xx < 2; xx++)
					{
						int xSplit = xMin + childTileSize * xx;
						int ySplit = yMin + childTileSize * yy;
						int zSplit = zMin + childTileSize * zz;

						if ( xSplit < _VolumeDim._X && ySplit < _VolumeDim._Y && zSplit < _VolumeDim._Z )
						{
							node->_Chidren[zz * 4 + yy * 2 + xx] = new TopoOctreeNode;

							_NodeCount++;

							Vec3i pos;

							pos._X = xSplit;
							pos._Y = ySplit;
							pos._Z = zSplit;

							if ( resolutionLevel < _NumLevels - 2 )
							{
								buildNode(node->_Chidren[zz * 4 + yy * 2 + xx], resolutionLevel + 1, pos);
							}
							else
							{
								node->_Chidren[zz * 4 + yy * 2 + xx]->_Position = pos;
							}
							
						}

					}

		}


		void VolumeTopoOctree::destroyTree()
		{
			int nFileNodes = _FileIdNodes.size();

			for (int ff = 0; ff < nFileNodes; ff++)
			{
				if (_FileIdNodes[ff])
					delete _FileIdNodes[ff];

			}

			_FileIdNodes.clear();

			_FileIdToTileIdMap.clear();
			_TileidToFileidMap.clear();


			_NumLevels = 0;

			_NodeCount = 0;

			_TileCount = 0;

		}

		int VolumeTopoOctree::getNumLevels()
		{
			return _NumLevels;
		}


		int VolumeTopoOctree::nodeCount(TopoOctreeNode *node)
		{
			if (!node)
				return 0;

			int count = 0;
			
			for (int cc = 0; cc < 8; cc++)
			{
				count += nodeCount(node->_Chidren[cc]);

				if (node->_Chidren[cc])
				{
					count++;
				}
			}

			return count;
		}


		void VolumeTopoOctree::stackLevelNodes(int levelId, std::vector< TopoOctreeNode* >& stackedNodes, bool assignTileId) const
		{
			int currentLevelId = 0;

			stackLevelNodes(levelId, currentLevelId, _RootNode, stackedNodes , assignTileId );
		}

		void VolumeTopoOctree::stackLevelNodes( int levelId , int currentLevelId , TopoOctreeNode* node, std::vector< TopoOctreeNode* >& stackedNodes , bool assignTileId ) const
		{
			if (currentLevelId < levelId - 1)
			{
				for (int cc = 0; cc < 8; cc++)
				{
					if (node->_Chidren[cc])
					{
						stackLevelNodes(levelId, currentLevelId + 1, node->_Chidren[cc], stackedNodes, assignTileId);
					}
				}
				
			}
			else 
			{
				if (levelId == 0)
				{
					stackedNodes.push_back(node);
					return;
				}

				for (int cc = 0; cc < 8; cc++)
				{
					if (node->_Chidren[cc])
					{
						stackedNodes.push_back(node->_Chidren[cc]);
					}
				}
			}
		}


		void VolumeTopoOctree::assignTileIds(int levelId, int currentLevelId , TopoOctreeNode* node)
		{
			if (currentLevelId < levelId - 1)
			{
				for (int cc = 0; cc < 8; cc++)
				{
					if (node->_Chidren[cc])
					{
						assignTileIds(levelId, currentLevelId + 1, node->_Chidren[cc]);
					}
					else
					{
					   _TileCount += (1 << 3 * (levelId - currentLevelId - 1));
					}
				}

			}
			else
			{
				if (levelId == 0)
				{
					node->_TileId = _TileCount++;

					return;
				}

				for (int cc = 0; cc < 8; cc++)
				{
					if (node->_Chidren[cc])
					{
						node->_Chidren[cc]->_TileId = _TileCount;
					}
					
					_TileCount++;
				}
			}
		}

		int VolumeTopoOctree::getFileId(int tileId )
		{
			if (tileId >= _TileidToFileidMap.size())
				return -1;

			return _TileidToFileidMap[tileId];
		}


		int VolumeTopoOctree::getTileId(int fileId)
		{
			if ( fileId >= _FileIdToTileIdMap.size())
				return -1;

			return _FileIdToTileIdMap[fileId];
		}

		

		void VolumeTopoOctree::assignFileIds()
		{
			int fileId = 0;

			int numNodes = nodeCount(_RootNode) + 1;

			int numTiles = 0;

			for (int ll = 0; ll < _NumLevels; ll++)
			{
				numTiles += 1 << ll * 3;
			}


			_FileIdNodes.resize(numNodes);
			_FileIdToTileIdMap.resize(numNodes);
			_TileidToFileidMap.resize(numTiles);

			std::fill(_FileIdToTileIdMap.begin(), _FileIdToTileIdMap.end(), -1);
			std::fill(_TileidToFileidMap.begin(), _TileidToFileidMap.end(), -1);

			_TileCount = 0;

			for ( int ll = 0; ll < _NumLevels; ll++ )
			{
				std::vector< TopoOctreeNode* > levelNodes;

				stackLevelNodes(ll, levelNodes);

				assignTileIds(ll, 0, _RootNode);

				std::cout << _TileCount << " " << numTiles << std::endl;

				for (auto node : levelNodes)
				{
					node->_NodeId = fileId++;

					_FileIdNodes[node->_NodeId] = node;

					_FileIdNodes[node->_NodeId]->_Level = ll;

					if ( node->_NodeId >= _FileIdToTileIdMap.size() )
						std::cout << " bad file id " << std::endl;

					_FileIdToTileIdMap[node->_NodeId] = node->_TileId;

					if (_TileidToFileidMap.size() <= node->_TileId)
						std::cout << " bad tile id " << std::endl;

					_TileidToFileidMap[node->_TileId] = node->_NodeId;
				}
			}

			std::cout << " tile id assignment finished " << std::endl;
		}


		int VolumeTopoOctree::getNumFiles() const
		{
			return _FileIdToTileIdMap.size();
		}

		int VolumeTopoOctree::getNumTiles() const
		{
			return _TileidToFileidMap.size();
		}


		std::vector< VolumeTopoOctree::TopoOctreeNode* > VolumeTopoOctree::getFileNodes(int level) const
		{
			std::vector< VolumeTopoOctree::TopoOctreeNode* > collectedNodes;

			stackLevelNodes(level, collectedNodes);

			return collectedNodes;
		}


		VolumeTopoOctree::TopoOctreeNode* VolumeTopoOctree::getFileNode(int fileId)
		{
			return _FileIdNodes[fileId];
		}


		VolumeTopoOctree::~VolumeTopoOctree()
		{
			destroyTree();
		}


	}





}