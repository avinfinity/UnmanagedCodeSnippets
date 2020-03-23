#include "octree.h"


OctreeNode::OctreeNode()
{
  mPointId = -1;

  mParent = -1;
  
  for( int ii = 0; ii < 8; ii++ )
  {
     mChildren[ ii ] = -1;
  }

  mIsLeaf = 1;

}

Octree::Octree()
{

}


void Octree::insertPoint( int nodeId , Eigen::Matrix< double , 3 , 1 > point , int pointId )
{
	Eigen::Vector3d val = point - mMin;	

	if( mNodes[ nodeId ].mIsLeaf )
	{
	    int id = 4 * ( val.x() * 2 > mLimit( 0 ) ) + 
		         2 * ( val.y() * 2 > mLimit( 1 ) ) + 
			     ( val.z() * 2 > mLimit( 2 ) );


		OctreeNode& node = allocateNode();

		node.mPointId = pointId;
		node.mParent = nodeId;
		node.mIsLeaf = 1;

		mNodes[ nodeId ].mChildren[ id ] = mTopUsedNode;
	}
	else
	{
		int id = 4 * ( val.x() * 2 > mLimit( 0 ) ) + 
		         2 * ( val.y() * 2 > mLimit( 1 ) ) + 
			     ( val.z() * 2 > mLimit( 2 ) );

		insertPoint( mNodes[ nodeId ].mChildren[ id ] , point , pointId );
	}

}


OctreeNode& Octree::allocateNode()
{
   mTopUsedNode++;

   return mNodes[ mTopUsedNode ];
}

void Octree::insertPoints( std::vector< Eigen::Matrix< double , 3 , 1 > > &points )
{
  
}

void Octree::insertPoints( std::vector< Eigen::Matrix< float , 3 , 1 > > &points )
{

}