#ifndef OCTREE_H
#define OCTREE_H

#include "vector"
#include "Eigen/Dense"



struct OctreeNode
{
	int mPointId;

	int mParent;
  
	int mChildren[ 8 ];

	unsigned char mIsLeaf;

	OctreeNode();
};

class Octree
{
    Eigen::Matrix< double , 3 , 1 > mMax , mMin , mLimit;

    std::vector< OctreeNode > mNodes;

    int mTopUsedNode;


protected:

	OctreeNode& allocateNode();

public:

	Octree();

	void insertPoint( int nodeId , Eigen::Matrix< double , 3 , 1 > point , int pointId  );

	void insertPoints( std::vector< Eigen::Matrix< double , 3 , 1 > > &points );

	void insertPoints( std::vector< Eigen::Matrix< float , 3 , 1 > > &points );

	void removeNode( int nodeId );


};

#endif