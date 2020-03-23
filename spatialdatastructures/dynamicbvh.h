/*
 * Copyright 2014 <copyright holder> <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef DYNAMICBVH_H
#define DYNAMICBVH_H

#include "openglincludes.h"
#include "Eigen/Dense"
#include <Eigen/Core>
#include "spatialdatastructures/ray.h"
#include "vector"
#include "spatialdatastructures/defintions.h"



namespace vc{
  
  
class HalfEdgeMeshVBOManager; 

/***
 * Axis aligned bounding box for BVH.  
 ***/
struct BVHaabb
{
   Eigen::Array3f mMin , mMax;  
  
   void operator + ( BVHaabb &other );
   
   void addPoint( Eigen::Array3f& point ); 

   void include( BVHaabb &aabb );
   
   bool includeAndCompare( BVHaabb &aabb );
   
   float intersect( const Ray& ray ) const ;
   
   void reset();
   
    Eigen::Vector3f center();
   
   unsigned int largestAxis();
};

/***
 * A BVH Node. kind tells the sort axis for median order arrangement of triangles underlying this node (1 , 2 , 3..respectively for each axis x , y , z ).
 * parent denotes the id of parent node for the current node. Root node has no parents , so its value for root node is -1.
 * mFirst and mLast indicates id of triangle data sets in sorted order list . ( note that they have positive values only for leaf node ( i.e. the node for which kind = 0. ) ).
 * mLeft and mRight are axis aligned bounding boxes for left and right children of the node. ( the AABB's would be non empty only for non-leaf node , i.e for the nodes for which kind > 0 . )  
 ***/

struct BVHNode
{  
   unsigned kind, parent;
  
   int mFirst , mLast , mExtraSpace;
   
   bool mNeedsUpdate;
   
   int mPolygonId;
   
   BVHaabb mLeft , mRight;
   
   
};

/**
 *
 **/

struct SortDataSet
{
  int mId;

  int mNodeId;
};



typedef std::vector< BVHNode >::iterator NodeIterator;

typedef std::vector< SortDataSet >::iterator DataSetIterator;

class DynamicBVH
{
  std::vector< HbrVertexData > &mVertexData2;
  
  int mNumVertices;

  EditableHbrMesh *mHbrMesh;
  
  std::vector< BVHNode > mNodes;
  
  int mNumNodes;

  std::vector< SortDataSet > mDataSet;
  
  std::vector< Eigen::Array3f >  mTCenter ; 
  std::vector< EditableHbrFace* > mFaces;
  std::vector< BVHaabb > mTaabb;
  
  int mNumDataSets , mNumCurrentFaces;
  
  std::vector< EditableHbrFace* > mUpdateableFaces;
  std::vector< int > mUpdateableNodes;
  std::vector< EditableHbrVertex* > mTraversedVertices , mLoopVertices;
  std::vector< EditableHbrHalfedge* > mEdgesToSplit ;
  
  std::vector< int > mSplittableNodes;
  
  int mNumUpdateableFaces , mNumUpdateableNodes, mNumSplittableNodes , mNumSplittableEdges , mMaxEdgeSplit , 
      mNumTraversedVertices , mLoopStart , mLoopEnd;  
  
  int mDepth , mNodeSplitLimit;
  
  bool mIsDataSetInitialized;
  
  int mNumPolygons;
  
  int mRootNodeId;

  HalfEdgeMeshVBOManager *mVBOManager;
  
protected:
  
  void updateSortDataSets();

  void updateSortDataSets2();
  
  unsigned medianPartition(  BVHaabb& bounds, unsigned int first, unsigned int last,
                             std::vector< SortDataSet >::iterator triangles,
							 unsigned int depth, unsigned int& kind );
  
  void build(  BVHaabb& bounds, unsigned node, unsigned first,
               unsigned last, std::vector< SortDataSet >::iterator triangles, unsigned depth);
  
  void computeAABB( std::vector< SortDataSet >::iterator start, std::vector< SortDataSet >::iterator end, BVHaabb& aabb );//std::vector< SortDataSet >::iterator 

  void buildBVH();
  
  void rebuildBVH();

  void splitNodes(); 
  
  void splitEdge( EditableHbrHalfedge *h );

  void swapEdge( EditableHbrHalfedge *h );

  void mergeEdge(  EditableHbrHalfedge *h , unsigned char collapseMethod = 0  );
  
  void remesh();

  float edgeLengthSq( EditableHbrHalfedge *h );  

  void resetSearchData();
  void updateVBO();
  
  std::vector< SortDataSet >& dataSet(){ return mDataSet; } 

  bool isEdgeSwappable( EditableHbrHalfedge *h );
  
  void updateAAB( EditableHbrFace *face );

  void checkFaceDeleteValidity();
  
public:


  DynamicBVH( EditableHbrMesh *mesh , std::vector< HbrVertexData > &vertexData);
  
  void init();

  void setVBOManager( HalfEdgeMeshVBOManager *manager );
  
  std::vector< BVHNode >& getNodes();
  std::vector< SortDataSet >& getDataSet();


  bool intersect( const EditableHbrFace *face , Ray& ray ) const;
  bool intersect( int faceId , Ray& ray ) const;
  
  void updateNode( int nodeId );
  
  void insertNode( BVHaabb &aabb , int polygonId );
  
  void insertFace( EditableHbrFace *face , int leafNodeId );
  
  void addFaceForUpdate( EditableHbrFace *face );
  
  void addNodeForUpdate( int &nodeId );
  
  void removeFace( EditableHbrFace *face );
  
  int getNumFaces();
  
  EditableHbrFace* getFace( int id );

  std::vector< HbrVertexData >& vertexData();
  
  Eigen::Vector3f geometricCenter( EditableHbrFace* face );

  void radiusSearch( EditableHbrFace* face, Eigen::Vector3f& center, double radius );

  void applySplitting();
  
  void splitLongestEdge( EditableHbrFace *face );
  void swapLongestEdge( EditableHbrFace *face );
  void collapseShortestEdge( EditableHbrFace *face );
  
  void update();
  
  BVHNode& rootNode();
  
  Eigen::Vector3f objectCenter();
  float radius();
    
  ~DynamicBVH();
};


}
#endif // DYNAMICBVH_H
