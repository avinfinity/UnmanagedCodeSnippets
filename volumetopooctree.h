#ifndef __VOLUMETOPOOCTREE_H__
#define __VOLUMETOPOOCTREE_H__

#include "vector"

namespace imt{

	namespace volume{

	

class VolumeTopoOctree
{

public:

	struct Vec3i
	{
		int _X, _Y, _Z;

	};


	struct TopoOctreeNode{


		TopoOctreeNode();

		unsigned int _MinVal, _MaxVal;
		Vec3i _Position;
		int _TileId , _Level;
		int _NodeId; //A.K.A. file id
		TopoOctreeNode *_Chidren[8];
		unsigned short *_Data;

	};


public:

	VolumeTopoOctree();

	void init( Vec3i volumeDim , int tileDim , int overlap );

	int getFileId(int tileId);

	int getTileId(int fileId);

	int getNumFiles() const;
	int getNumTiles() const;

	//std::vector< VolumeTopoOctree::TopoOctreeNode* > getFileNodes( int level );

	std::vector< VolumeTopoOctree::TopoOctreeNode* > getFileNodes(int level) const;

	VolumeTopoOctree::TopoOctreeNode* getFileNode(int fileId);

	int getNumLevels();

	~VolumeTopoOctree();

protected:


	void destroyTree();

	void buildTree();

	void buildNode( TopoOctreeNode *node , int resolutionLevel , Vec3i pos );

	int nodeCount( TopoOctreeNode *node );

	void assignFileIds();

	void stackLevelNodes(int levelId, std::vector< TopoOctreeNode* >& stackedNodes, bool assignTileId = false) const;

	void stackLevelNodes( int levelId , int currentLevelId , TopoOctreeNode* , std::vector< TopoOctreeNode* >& stackedNodes , bool assignTileId ) const;

	void assignTileIds(int levelId, int currentLevelId , TopoOctreeNode* node);

	

	TopoOctreeNode *findNode(int tileId , int level );
	

protected:

	Vec3i _VolumeDim;
	int _TileDim;
	int _Overlap;

	TopoOctreeNode *_RootNode;

	std::vector< TopoOctreeNode* > _FileIdNodes;
	std::vector< int > _FileIdToTileIdMap;
	std::vector< int > _TileidToFileidMap;


	int _NumLevels;

	int _NodeCount;

	int _TileCount;

};

	}

}

#endif