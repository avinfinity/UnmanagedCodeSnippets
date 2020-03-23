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

#include "dynamicbvh.h"
#include <halfedge/vertex.h>
//#include "tbb/parallel_sort.h"
#include "RenderObjects/halfedgemeshvbomanager.h"

#ifdef VC_WINDOWS

#undef min
#undef max

#endif

namespace vc{


unsigned int BVHaabb::largestAxis()
{
  Eigen::Vector3f d = ( mMax - mMin );

  return (d.x() > d.y() && d.x() > d.z() ) ? 0 : (d.y() > d.z() ? 1 : 2);
}

void BVHaabb::addPoint( Eigen::Array3f& point )
{
  mMin = mMin.min( point );
  mMax = mMax.max( point );
}

bool BVHaabb::includeAndCompare( BVHaabb &aabb )
{
  bool changed = false;

  for( int ii = 0; ii < 3; ii++ )
  {
    if( aabb.mMax( ii ) > mMax( ii ) )
    {
      mMax( ii ) = aabb.mMax( ii );

      changed = true;
    }

    if( aabb.mMin( ii ) < mMin( ii ) )
    {
      mMin( ii ) = aabb.mMin( ii );

      changed = true;
    }
  }

  return changed;
}

void BVHaabb::include( BVHaabb &aabb )
{
  mMin = mMin.min( aabb.mMin );
  mMax = mMax.max( aabb.mMax );
}

float BVHaabb::intersect( const Ray& ray ) const 
{
  float t0 = ray.minT;
  float t1 = ray.maxT;

  for (int i = 0; i < 3; ++i) 
  {
	float nearv = ( mMin( i ) - ray.mOrigin( i ) ) * ray.mInvDir( i );
	float farv  = ( mMax( i ) - ray.mOrigin( i ) ) * ray.mInvDir( i );
			
	if ( nearv > farv )
		std::swap( nearv , farv );
			
	if ( nearv > t0 ) t0 = nearv;
	if ( farv < t1 ) t1 = farv;
			
	if ( t0 > t1 )
		return ray.maxT;
   }
		
   return t0;  
}


Eigen::Vector3f BVHaabb::center()
{ 
  return ( mMin + mMax ) / 2 ;
}

void BVHaabb::reset()
{
  mMin = Eigen::Array3f(1e8f, 1e8f, 1e8f);
  mMax = Eigen::Array3f(-1e8f, -1e8f, -1e8f);
}




DynamicBVH::DynamicBVH( EditableHbrMesh* mesh, std::vector< HbrVertexData >& vertexData ) : mVertexData2( vertexData )

{
  mHbrMesh = mesh;
  
  mNumVertices = mVertexData2.size();

}


void DynamicBVH::init()
{
   int numVertices = mVertexData2.size();
   
   

   mLoopVertices.resize( numVertices );

   mLoopStart = 0;
   mLoopEnd = 0;

   for( int vv = 0; vv < numVertices ; vv++ )
   {
	  mVertexData2[ vv ].mIsSplittable = 0;
	  mVertexData2[ vv ].mIsUnderRegion = 0;
   }

   mNumVertices = mVertexData2.size();
   
   mNodeSplitLimit = 8;
   
   

  
   updateSortDataSets2();  
  
   mDepth = 0;
  
   BVHNode root = { 0, 0xffffffff , -1 , -1 , 0 , false };
	
   mNodes.push_back(root);
  
   BVHaabb bounds;
  
   bounds.reset(); 
  
   //double time = clock();
  
   computeAABB( mDataSet.begin() ,  mDataSet.end() , bounds );
  
   build( bounds , 0 , 0 , (unsigned)mDataSet.size() , mDataSet.begin() , 0 );
  
   mMaxEdgeSplit = 1000;
  
   mEdgesToSplit.resize( mMaxEdgeSplit );
  
   mNumSplittableEdges = 0;
   
   mNumNodes = mNodes.size();
   
   mSplittableNodes.resize( mNumNodes );
   
   mUpdateableFaces.resize( mHbrMesh->GetNumFaces() );
   
   mUpdateableNodes.resize( mHbrMesh->GetNumFaces() );
   
   mNumSplittableNodes = 0;    
   
   mNumUpdateableFaces = 0;
   
   mNumUpdateableNodes = 0;
   
   mNumDataSets = mDataSet.size();
  
   std::cout<<" num nodes : "<<mNodes.size()<<" , tree depth : "<<mDepth<<" "<<mNumNodes<<std::endl;
   
   int numContainedFaces = 0;
   
   for( int nd = 0; nd < mNumNodes ; nd++ )
   {
     if( !mNodes[ nd ].kind )
     {
       numContainedFaces += mNodes[ nd ].mLast - mNodes[ nd ].mFirst;  
     }
   }
   
   qDebug() << " num contained faces : "<<numContainedFaces<<" "<<mHbrMesh->GetNumFaces()<<endl;
  
}


void DynamicBVH::setVBOManager( HalfEdgeMeshVBOManager *manager )
{
	mVBOManager = manager;
}

BVHNode& DynamicBVH::rootNode()
{ 
  return mNodes[ 0 ];
}

Eigen::Vector3f DynamicBVH::objectCenter()
{
   BVHaabb aabb;
   
   aabb.reset();
   
   aabb.include( mNodes[ 0 ].mLeft );
   aabb.include( mNodes[ 0 ].mRight );
   
   Eigen::Vector3f center = ( aabb.mMin + aabb.mMax ) / 2;
   
   return center;
}


float DynamicBVH::radius()
{
   BVHaabb aabb;
   
   aabb.reset();
   
   aabb.include( mNodes[ 0 ].mLeft );
   aabb.include( mNodes[ 0 ].mRight );
   
   Eigen::Vector3f vec = ( aabb.mMax - aabb.mMin );
   
   return vec.norm() / 2;
}



std::vector< BVHNode >& DynamicBVH::getNodes()
{
  return mNodes;  
}

std::vector< SortDataSet >& DynamicBVH::getDataSet()
{
  return mDataSet;
}



bool DynamicBVH::intersect( const EditableHbrFace* face, Ray& ray ) const
{
  int id1 = face->GetVertex( 0 )->GetID();
  int id2 = face->GetVertex( 1 )->GetID();
  int id3 = face->GetVertex( 2 )->GetID();
  
  const Eigen::Vector3f &p0 = mVertexData2[ id1 ].mCoords;
  const Eigen::Vector3f &p1 = mVertexData2[ id2 ].mCoords; 
  const Eigen::Vector3f &p2 = mVertexData2[ id3 ].mCoords;
  
  const float epsilon = 1e-6f;
		
  Eigen::Vector3f d = ray.mDir;
  Eigen::Vector3f p = ray.mOrigin;
		
  Eigen::Vector3f e1 = p1 - p0;
  Eigen::Vector3f e2 = p2 - p0;
		
  Eigen::Vector3f n = e1.cross( e2);
  float s = -d.dot( n);
		
  if (std::fabs(s) < epsilon)
    return false;
		
  s = 1.0f / s;
		
  Eigen::Vector3f r = p - p0;
  
  float t = r.dot( n ) * s;
		
  if (t <= ray.minT || t >= ray.maxT)
	return false;
		
  Eigen::Vector3f q = -d.cross( r);
  float v = q.dot( e2) * s;
		
  if (v < 0.0f)
    return false;
		
  float w = -e1.dot( q) * s;
		
  if (w < 0.0f || v+w > 1.0f)
        return false;
		
  ray.maxT = t;

  return true;
}

bool DynamicBVH::intersect(int faceId, Ray& ray) const
{
 
    if( faceId >= mHbrMesh->GetNumFaces() )
    {
        qDebug() << " face id and faces : "<<faceId<<" "<<mHbrMesh->GetNumFaces()<<endl;
    }

	assert( faceId < mHbrMesh->GetNumFaces() );

    EditableHbrFace *face = mHbrMesh->GetFace( faceId );
    
  int id1 = face->GetVertex( 0 )->GetID();
  int id2 = face->GetVertex( 1 )->GetID();
  int id3 = face->GetVertex( 2 )->GetID();
  
  const Eigen::Vector3f &p0 = mVertexData2[ id1 ].mCoords;
  const Eigen::Vector3f &p1 = mVertexData2[ id2 ].mCoords; 
  const Eigen::Vector3f &p2 = mVertexData2[ id3 ].mCoords;
  
  const float epsilon = 1e-9f;
		
  Eigen::Vector3f d = ray.mDir;
  Eigen::Vector3f p = ray.mOrigin;
		
  Eigen::Vector3f e1 = p1 - p0;
  Eigen::Vector3f e2 = p2 - p0;
		
  Eigen::Vector3f n = e1.cross( e2);
  float s = -d.dot( n);
		
  if (std::fabs(s) < epsilon)
    return false;
		
  s = 1.0f / s;
		
  Eigen::Vector3f r = p - p0;
  
  float t = r.dot( n ) * s;
		
  if (t <= ray.minT || t >= ray.maxT)
	return false;
		
  Eigen::Vector3f q = -d.cross( r);
  float v = q.dot( e2) * s;
		
  if (v < 0.0f)
    return false;
		
  float w = -e1.dot( q) * s;
		
  if (w < 0.0f || v+w > 1.0f)
        return false;
		
  ray.maxT = t;

  return true;
}



struct MedianCompare
{  
  int mDim;
  
  std::vector< Eigen::Array3f >& mCenters;
 
  MedianCompare( int dim , std::vector< Eigen::Array3f >& centers ) : mCenters( centers )
  {
    mDim = dim;
  }
  
  inline bool operator()( const SortDataSet& x, const SortDataSet& y ){ return mCenters[ x.mId ]( mDim ) < mCenters[ y.mId ]( mDim ); }
};



unsigned DynamicBVH::medianPartition( BVHaabb& bounds , unsigned first, unsigned last, std::vector< SortDataSet >::iterator triangles, unsigned depth, unsigned& kind )
{
   if( ( last - first ) < 5 )
	return 0xffffffff;
  
  	unsigned pivot = (first + last) / 2;
	
	unsigned bestDim = bounds.largestAxis();
	
	float bestSeparation = -1e+10f;
	
	for (unsigned dim = 0; dim < 3; ++dim) 
	{
		MedianCompare predicate( dim , mTCenter );// = { dim };
		
		//std::sort(triangles + first, triangles + last, predicate);
		std::sort(triangles + first, triangles + last, predicate); //tbb::parallel_
		
		BVHaabb left, right;
		
		left.reset();
		right.reset();
		
		computeAABB( triangles + first ,  triangles + pivot , left );
		computeAABB( triangles + pivot ,  triangles + last , right );
		
		float separation = right.center()[dim] - left.center()[dim];
		
		if (separation > bestSeparation) 
		{
			bestDim = dim;
			bestSeparation = separation;
		}
	}
	
	kind = bestDim + 1;
	
	MedianCompare predicate( bestDim , mTCenter );// = { bestDim };
	
	//std::sort( triangles + first, triangles + last, predicate );
	std::sort( triangles + first, triangles + last, predicate );//tbb::parallel_
	
	return pivot;
}

void DynamicBVH::build(  BVHaabb& bounds , unsigned node , unsigned first , 
			unsigned last , std::vector< SortDataSet >::iterator triangles , unsigned depth )
{
  	if ( mDepth < depth)
		mDepth = depth;
	
	if ( last - first < 8) 
	{
		mNodes[node].mFirst = first;
		mNodes[node].mLast = last;
		
		for( int ii = first ; ii < last ; ii++ )
		{
		  mHbrMesh->GetFace( mDataSet[ ii ].mId )->mSortDataID = ii;
		  mHbrMesh->GetFace( mDataSet[ ii ].mId )->mNodeID = node;
		  
		  mDataSet[ ii ].mNodeId = node;
		}
		
		return;
	}

	unsigned pivot;
	
	pivot = medianPartition( bounds , first , last , triangles, depth, mNodes[node].kind );
	
	if (pivot == 0xffffffff)
	{
		mNodes[ node ].mFirst = first;
		mNodes[ node ].mLast = last;
		
		for( int ii = first ; ii < last ; ii++ )
		{
		  mHbrMesh->GetFace( mDataSet[ ii ].mId )->mSortDataID = ii;
		  mHbrMesh->GetFace( mDataSet[ ii ].mId )->mNodeID = node;
		  
		  mDataSet[ ii ].mNodeId = node;
		  
		  
		}
		
		return;
	}
	
	unsigned left = (unsigned)mNodes.size();
	unsigned right = (unsigned)mNodes.size()+1;
	
	BVHaabb leftBounds, rightBounds;
	
	leftBounds.reset();
	rightBounds.reset();
	
	computeAABB( triangles + first ,  triangles + pivot , leftBounds );
	computeAABB( triangles + pivot ,  triangles + last , rightBounds );
	
	
	BVHNode child = { 0, node , -1 , -1 , 0 , false };
	
	mNodes.push_back(child);
	mNodes.push_back(child);
	
	mNodes[node].mFirst = left;
	mNodes[node].mLast = right;
	mNodes[node].mLeft = leftBounds;
	mNodes[node].mRight = rightBounds;
	
	build( leftBounds , left, first, pivot, triangles, depth + 1 );
	build( rightBounds , right, pivot, last, triangles, depth + 1 );
}


Eigen::Vector3f DynamicBVH::geometricCenter( EditableHbrFace *face )
{
    Eigen::Vector3f geometricCenter( 0 , 0 , 0 );

    for( int ii = 0; ii < 3; ii++ )
    {
      //EditableVertex &v = face->GetVertex( ii )->GetData();

      geometricCenter += mVertexData2[ face->GetVertex( ii )->GetID() ].mCoords;
    }

    geometricCenter /= 3;
    
    return geometricCenter;
}

void DynamicBVH::buildBVH()
{ 
  
}

void DynamicBVH::splitNodes()
{
  
}


void DynamicBVH::splitEdge( EditableHbrHalfedge* h )
{
  if( mNumVertices == mVertexData2.size() )
  {
    mVertexData2.resize( 2 * mNumVertices );
  }  

  EditableHbrHalfedge *oh = h->GetOpposite();

  if( !oh )
  {
    //boundary edge
	
      qDebug() << " boundary edge , returning "<<endl;
      return;
  }
  
  EditableHbrVertex* ov1 = h->GetDestVertex() ;
  EditableHbrVertex* ov2 = h->GetOrgVertex() ;
  
  int id1 = ov1->GetID();
  int id2 = ov2->GetID();

  EditableHbrVertex *vertex = mHbrMesh->NewVertex();
  
  int vId = vertex->GetID();
  
  mNumVertices = std::max( mNumVertices , vId );
  
  mVertexData2[ vId ].mIsSplittable = 0;
  mVertexData2[ vId ].mIsUnderRegion = 0;
  mVertexData2[ vId ].mCoords = ( mVertexData2[ id1 ].mCoords + mVertexData2[ id2 ].mCoords ) * 0.5 ;
  
  
  
  EditableHbrHalfedge* h1 = h->GetNext() ;
//  EditableHbrHalfedge* h2 = h1->GetNext();
  EditableHbrHalfedge* oh1 = oh->GetNext();
//  EditableHbrHalfedge* oh2 = oh1->GetNext();
  
  
  EditableHbrVertex *nv1 = h1->GetDestVertex();
  EditableHbrVertex *nv2 = oh1->GetDestVertex();

  //assert( vertex->GetID() == mNumVertices );

  
  
  EditableHbrFace *f1 = h->GetFace();//left face 
  EditableHbrFace *f2 = oh->GetFace();//right face

  EditableHbrHalfedge** edges1 = f1->edges();
  EditableHbrHalfedge** edges2 = f2->edges();

  int eid1 = -1 , eid2 = -1;

  for( int ii = 0; ii < 3; ii++ )
  {
	  if( edges1[ ii ] == h )
	  {
	    eid1 = ii;
	  }
  }

  for( int ii = 0; ii < 3; ii++ )
  {
	  if( edges2[ ii ] == oh )
	  {
	    eid2 = ii;
	  }
  }
  
  if( eid1 == -1 || eid2 == -1 )
  {
    //FATAL ERROR OCCURRED
	return;
  }

  EditableHbrHalfedge *nh1 = mHbrMesh->NewEdge();
  EditableHbrHalfedge *nh2 = mHbrMesh->NewEdge();

  EditableHbrHalfedge *nh3 = mHbrMesh->NewEdge();
  EditableHbrHalfedge *nh4 = mHbrMesh->NewEdge();

  EditableHbrHalfedge *nh5 = mHbrMesh->NewEdge();
  EditableHbrHalfedge *nh6 = mHbrMesh->NewEdge();

//  EditableHbrHalfedge *bh1 = edges1[ ( eid1 + 1 ) % 3 ];
//  EditableHbrHalfedge *bh2 = edges2[ ( eid2 + 1 ) % 3 ];

  //restructure face1
  h->setNext( nh1 );
  nh1->setPrev( h );
  nh1->setNext( edges1[ ( eid1 + 2 ) % 3 ] );
  
  assert( h1 == edges1[ ( eid1 + 1 ) % 3 ] );
  
  edges1[ ( eid1 + 2 ) % 3 ]->setPrev( nh1 );
  edges1[ ( eid1 + 1 ) % 3 ] = nh1;
  
  
  nh1->SetOpposite( nh2 );
  

  h->SetOpposite( nh4 );
  

  nh1->SetOrgVertex( vertex );

  nh1->setFace( f1 );

  vertex->addIncidentEdge( nh1 ); // incidence 1
  
  //restrcuture face2
  oh->setNext( nh5 );
  nh5->setPrev( oh );
  nh5->setNext( edges2[ ( eid2 + 2 ) % 3 ] );
  
//  assert( edges2[ ( eid2 + 2 ) % 3 ] == oh2 );
  
  edges2[ ( eid2 + 2 ) % 3 ]->setPrev( nh5 );
  edges2[ ( eid2 + 1 ) % 3 ] = nh5;

  nh5->SetOpposite( nh6 );
  oh->SetOpposite( nh3 );

  nh5->setFace( f2 );

  nh5->SetOrgVertex( vertex );
  vertex->addIncidentEdge( nh5 ); //incidence 2


  //create face3
  EditableHbrFace *f3 = mHbrMesh->NewFace( 3 );
  EditableHbrFace *f4 = mHbrMesh->NewFace( 3 );

  EditableHbrHalfedge** edges3 = f3->edges();
  EditableHbrHalfedge** edges4 = f4->edges();

  //construct face3 
  edges3[ 0 ] = nh3;
  edges3[ 1 ] = h1;
  edges3[ 2 ] = nh2;
  
  nh2->SetOrgVertex( nv1 );
  nh3->SetOrgVertex( vertex );

  nv1->addIncidentEdge( nh2 );
  vertex->addIncidentEdge( nh3 ); // incidence 3

  nh3->setNext( h1 );
  h1->setNext( nh2 );
  nh2->setNext( nh3 );

  nh3->setPrev( nh2 );
  nh2->setPrev( h1 );
  h1->setPrev( nh3 );

  nh3->setFace( f3 );
  h1->setFace( f3 );
  nh2->setFace( f3 );

  nh2->SetOpposite( nh1 );
  nh3->SetOpposite( oh );

  //construct face4 
  edges4[ 0 ] = nh4;
  edges4[ 1 ] = oh1;
  edges4[ 2 ] = nh6;

  nh6->SetOrgVertex( nv2 );
  nh4->SetOrgVertex( vertex );

  nv2->addIncidentEdge( nh6 );
  vertex->addIncidentEdge( nh4 ); //incidence 4

  nh4->setNext( oh1 );
  oh1->setNext( nh6 );
  nh6->setNext( nh4 );

  nh4->setPrev( nh6 );
  nh6->setPrev( oh1 );
  oh1->setPrev( nh4 );

  nh4->setFace( f4 );
  oh1->setFace( f4 );
  nh6->setFace( f4 ); 

  nh4->SetOpposite( h );
  nh6->SetOpposite( nh5 );

  //Rendering updates

  //update f1 and f2
//   GLuint indices[ 3 ];
// 
//   indices[ 0 ] = f1->GetVertexID( 0 );
//   indices[ 1 ] = f1->GetVertexID( 1 );
//   indices[ 2 ] = f1->GetVertexID( 2 );

//   mVBOManager->updateTriangle( f1->GetID() , indices ); //TODO
  
  
//  mTCenter[ f3->GetID() ] = geometricCenter( f3 );  
//  mTCenter[ f4->GetID() ] = geometricCenter( f4 );
    
   
 
 mVBOManager->addVertex( vId );
 mVBOManager->addFace( f1 );
 mVBOManager->addFace( f2 );
 mVBOManager->addFace( f3 );
 mVBOManager->addFace( f4 );
 
//  qDebug() << " sort data ids : "<<f1->sortDataId()<<" "<<f2->sortDataId()<<endl;
 
 insertFace( f3 , f1->mNodeID );
 insertFace( f4 , f2->mNodeID );
 
 f1->isUpdateNecessary() = true;
 f2->isUpdateNecessary() = true;
 f3->isUpdateNecessary() = true;
 f4->isUpdateNecessary() = true;
 
 
 addFaceForUpdate( f1 );
 addFaceForUpdate( f2 );
 addFaceForUpdate( f3 );
 addFaceForUpdate( f4 );
 
}

void DynamicBVH::swapEdge( EditableHbrHalfedge *h )
{
	EditableHbrHalfedge *hopp = h->GetOpposite();

	if( !hopp )
	  return;

	EditableHbrFace *f1 = h->GetFace();
	EditableHbrFace *f2 = hopp->GetFace();

	EditableHbrHalfedge** edges1 = f1->edges();
	EditableHbrHalfedge** edges2 = f2->edges();

	EditableHbrHalfedge* z2 = hopp->GetNext();
	EditableHbrHalfedge* z3 = z2->GetNext();

	EditableHbrHalfedge* z4 = h->GetNext();
	EditableHbrHalfedge* z1 = z4->GetNext();

	EditableHbrVertex *v1 = h->GetOrgVertex();
	EditableHbrVertex *v2 = hopp->GetOrgVertex();

	EditableHbrVertex* nv2 = z1->GetOrgVertex();
	EditableHbrVertex* nv1 = z3->GetOrgVertex();

	//Restructure Face1
	v1->RemoveIncidentEdge( h );
	nv1->addIncidentEdge( h );
	h->SetOrgVertex( nv1 );

	h->setNext( z1 );	
	z1->setNext( z2 );
	z2->setNext( h );

	z2->setPrev( z1 );
	z1->setPrev( h );
	h->setPrev( z2 );

	z2->setFace( f1 );

	edges1[ 0 ] = h;
	edges1[ 1 ] = z1;
	edges1[ 2 ] = z2;

	//Restructure Face2
	v2->RemoveIncidentEdge( hopp );
	nv2->addIncidentEdge( hopp );
	hopp->SetOrgVertex( nv2 );

	hopp->setNext( z3 );
	z3->setNext( z4 );
	z4->setNext( hopp );

	hopp->setPrev( z4 );
	z4->setPrev( z3 );
	z3->setPrev( hopp );

	z4->setFace( f2 );
	

	edges2[ 0 ] = hopp;
	edges2[ 1 ] = z3;
	edges2[ 2 ] = z4;

    mVBOManager->addFace( f1 );

    mVBOManager->addFace( f2 );
    
    f1->isUpdateNecessary() = true;
    f2->isUpdateNecessary() = true;
    
    addFaceForUpdate( f1 );
    addFaceForUpdate( f2 );
    
}

// delete v2 , retain v1 and update its coordinates if necessary. Delete face f1 and f2 and update
// the neighborhood accordingly. 

void DynamicBVH::mergeEdge( EditableHbrHalfedge* h, unsigned char collapseMethod )
{
  EditableHbrHalfedge *oh = h->GetOpposite();
  
  EditableHbrFace *f1 = h->GetFace();
  EditableHbrFace *f2 = oh->GetFace();
  
  EditableHbrVertex* v2 =  h->GetOrgVertex();
  EditableHbrVertex* v1 = oh->GetOrgVertex();
  
  
//  qDebug() << " before delete "<<endl;
//  
//  for( int ii = 0; ii < v1->GetNumIncidentEdges() ; ii++ )
//  {
//    qDebug() << v1->GetIncidentEdge( ii )->GetFace()->GetID() <<" , "<<v1->GetIncidentEdge( ii )->GetFace()->mNodeID<<" , "<<v1->GetIncidentEdge( ii )->GetFace()->mSortDataID; 
//  }
//  
//  qDebug() << endl;
//  
//  for( int ii = 0; ii < v2->GetNumIncidentEdges() ; ii++ )
//  {
//    qDebug() << v2->GetIncidentEdge( ii )->GetFace()->GetID() <<" , "<<v2->GetIncidentEdge( ii )->GetFace()->mNodeID<<" , "<<v2->GetIncidentEdge( ii )->GetFace()->mSortDataID; 
//  }
//  
//  qDebug() << endl;
  
  EditableHbrHalfedge *h1 = h->GetNext();
  EditableHbrHalfedge *h2 = h1->GetNext();
  
  EditableHbrHalfedge *oh1 = oh->GetNext();
  EditableHbrHalfedge *oh2 = oh1->GetNext();
  
  EditableHbrHalfedge *oph1 = h1->GetOpposite();
  EditableHbrHalfedge *oph2 = h2->GetOpposite();
  
  EditableHbrHalfedge *opoh1 = oh1->GetOpposite();
  EditableHbrHalfedge *opoh2 = oh2->GetOpposite();
  

  
  oph1->SetOpposite( oph2 );
  oph2->SetOpposite( oph1 );
  
  opoh1->SetOpposite( opoh2 );
  opoh2->SetOpposite( opoh1 );
  
//   qDebug() << " num incident edge before : "<<v1->GetNumIncidentEdges()<<" "<<v2->GetNumIncidentEdges()<<endl;
  
  v1->RemoveIncidentEdge( oh );
  v1->RemoveIncidentEdge( h1 );
  v2->RemoveIncidentEdge( h );
  v2->RemoveIncidentEdge( oh1 );
  
//   qDebug() << " num incident edge after : "<<v1->GetNumIncidentEdges()<<" "<<v2->GetNumIncidentEdges()<<endl;
  
  EditableHbrVertex *nv1 = h2->GetOrgVertex();
  EditableHbrVertex *nv2 = oh2->GetOrgVertex();
  
  nv1->RemoveIncidentEdge( h2 );
  nv2->RemoveIncidentEdge( oh2 );
  
  int nie = v2->GetNumIncidentEdges();
  
  std::vector< EditableHbrFace* > facesToUpdate( nie );
  
  for( int ie = 0; ie < nie ; ie++ )
  {
    EditableHbrHalfedge *edge = v2->GetIncidentEdge( ie );
    
    v1->addIncidentEdge( edge );
    
    edge->SetOrgVertex( v1 );
      
    assert( edge->GetFace() );
    
    facesToUpdate[ ie ] = edge->GetFace();
  }
  
  int id1 = v1->GetID();
  int id2 = v2->GetID();
  
//   qDebug()<< " org id match : "<<oph2->GetOrgVertexID()<<" "<<v1->GetID() << endl;
  
//   qDebug() << " added num incident edge after : "<<v1->GetNumIncidentEdges()<<endl;
  
  //mid point collapse
  if( collapseMethod == 0 )
  {
     mVertexData2[ id1 ].mCoords = ( mVertexData2[ id1 ].mCoords + mVertexData2[ id2 ].mCoords ) / 2; 
  }
  else if( collapseMethod == 1 ) // directional collapse to v1
  {
    
//     mVertexData2[ id2 ].mCoords = mVertexData2[ id1 ].mCoords ; 
  }
  else //directional collapse to v2
  {
     mVertexData2[ id1 ].mCoords = mVertexData2[ id2 ].mCoords ; 
  }

  addNodeForUpdate( f1->mNodeID );
  addNodeForUpdate( f2->mNodeID );
  
  removeFace( f1 );
  removeFace( f2 );
  
  mVBOManager->removeFace( f1 );
  mVBOManager->removeFace( f2 );
  mVBOManager->addVertex( v1->GetID() );

  for( int ff = 0; ff < nie ; ff++ )
  {
    mVBOManager->addFace( facesToUpdate[ ff ] );
  }
  
  
  
  int fId1 = f1->GetID();
  
  EditableHbrFace *lastFace = mHbrMesh->GetFace( mHbrMesh->GetNumFaces() - 1 );
  
  mHbrMesh->DeleteFace( f1 );
  
  if( fId1 < mHbrMesh->GetNumFaces() )
  {
    
//    assert( mHbrMesh->GetFace( fId1 ) == lastFace );
    
    mDataSet[ mHbrMesh->GetFace( fId1 )->mSortDataID ].mId = fId1; 
  }
  
  int fId2 = f2->GetID();
  
  lastFace = mHbrMesh->GetFace( mHbrMesh->GetNumFaces() - 1 );
  
  mHbrMesh->DeleteFace( f2 );
  
  if( fId2 < mHbrMesh->GetNumFaces() )
  {
//    assert( mHbrMesh->GetFace( fId2 ) == lastFace );
    
    mDataSet[ mHbrMesh->GetFace( fId2 )->mSortDataID ].mId = fId2; 
  }
  
  
  mHbrMesh->DeleteVertex( v2 );
  
//  qDebug() << " num incidence for merged vertex : "<<v1->GetNumIncidentEdges()<<" "<<nie<<endl;
  
  int niev1 = v1->GetNumIncidentEdges();
  
  for( int ii = 0; ii < niev1 ; ii++ )
  {
    addFaceForUpdate( v1->GetIncidentEdge( ii )->GetFace()  );
  }

//  checkFaceDeleteValidity();
  
//  qDebug() << " after delete "<<endl;
  
//  for( int ii = 0; ii < v1->GetNumIncidentEdges() ; ii++ )
//  {
//    qDebug() << v1->GetIncidentEdge( ii )->GetFace()->GetID() <<" , "<<v1->GetIncidentEdge( ii )->GetFace()->mNodeID<<" , "<<v1->GetIncidentEdge( ii )->GetFace()->mSortDataID;  
//  }
//  
//  qDebug() << endl;
  
//   qDebug() << " num faces after delete "<<mHbrMesh->GetNumFaces()<<endl;
  
}


void DynamicBVH::checkFaceDeleteValidity()
{
    int count = 0;
    
	for( int nn = 0 ; nn < mNodes.size() ; nn++ )
	{
		if( !mNodes[ nn ].kind )
		{
			for( int ii = mNodes[ nn ].mFirst ; ii < mNodes[ nn ].mLast ; ii++ )
			{
                count++;
                
				assert( mDataSet[ ii ].mId < mHbrMesh->GetNumFaces() );
			}
		}
	}
    
    qDebug() << " count bvh faces : "<<count<<" , num faces in mesh : "<<mHbrMesh->GetNumFaces()<<endl;
    
}

void DynamicBVH::remesh()
{
    //divide long edges
  
    //collapse short edges
  
    //equalize valence ( swap edges )
}


float DynamicBVH::edgeLengthSq( EditableHbrHalfedge *h )
{
	float length;

	int id1 = h->GetOrgVertexID();
	int id2 = h->GetDestVertexID();

	return ( mVertexData2[ id1 ].mCoords - mVertexData2[ id2 ].mCoords ).squaredNorm();
}


void DynamicBVH::radiusSearch( EditableHbrFace* face, Eigen::Vector3f& center, double radius ) 
{
	mVBOManager->bind();//TODO

	resetSearchData();

	double threshold = radius * radius;

	double splitTh = threshold / 25.0;

	//check if current face vertices lies inside the region
	EditableHbrVertex *v1 = face->GetVertex( 0 );
	EditableHbrVertex *v2 = face->GetVertex( 1 );
	EditableHbrVertex *v3 = face->GetVertex( 2 );

	double d1 = ( mVertexData2[ v1->GetID() ].mCoords - center ).squaredNorm();
	double d2 = ( mVertexData2[ v2->GetID() ].mCoords - center ).squaredNorm();
	double d3 = ( mVertexData2[ v3->GetID() ].mCoords - center ).squaredNorm();

	//std::cout<<" loop vertices size : "<<mLoopVertices.size() << std::endl;

	if( d1 > threshold && d2 > threshold && d3 > threshold )
	{
		//  
	}
	else
	{
		mLoopStart = 0;
		mLoopEnd = 0;

		if( d1 < threshold )
		{
			mLoopVertices[ mLoopEnd ] = v1;

			v1->isAdded() = true;

			mVertexData2[ v1->GetID() ].mIsUnderRegion = 1.0;

			mLoopEnd++;
		}

		if( d2 < threshold )
		{
			mLoopVertices[ mLoopEnd ] = v2;	

			v2->isAdded() = true;

			mVertexData2[ v2->GetID() ].mIsUnderRegion = 1.0;

			mLoopEnd++;
		}

		if( d3 < threshold )
		{
			//add vertex for ring traversal
			//v3->isTraversed() = true;
			mLoopVertices[ mLoopEnd ] = v3;

			mVertexData2[ v3->GetID() ].mIsUnderRegion = 1.0;

			v3->isAdded() = true;

			mLoopEnd++;
		}

		while( mLoopStart != mLoopEnd )
		{
			//find all the splittable edges and add them to the list
			EditableHbrVertex *v = mLoopVertices[ mLoopStart++ ];

			//std::cout<<mLoopStart-1<<std::endl;

			EditableHbrHalfedge* start = v->GetIncidentEdge() , *edge, *next;

			EditableHbrVertex *dstV = start->GetDestVertex();

			int orgId = start->GetOrgVertexID();
			int dstId = start->GetDestVertexID();

			if( !dstV->isAdded() && ( center - mVertexData2[ dstId ].mCoords ).squaredNorm() < threshold )
			{
				  mLoopVertices[ mLoopEnd++ ] = start->GetDestVertex();

				  start->GetDestVertex()->isAdded() = true;

				  mVertexData2[ dstId ].mIsUnderRegion = 1.0;
			  }

			  if( !dstV->isTraversed() && edgeLengthSq( start ) > splitTh )
			  {
			    //add edge to split list
				if( start->GetOpposite() && !start->GetOpposite()->isAdded() )
				{
					start->isAdded() = true;

			        mEdgesToSplit[ mNumSplittableEdges++ ] = start;					
				}

				mVertexData2[ dstId ].mIsSplittable = 1.0;
				mVertexData2[ orgId ].mIsSplittable = 1.0;
			  }
    
	        edge = start;
    
	        while ( edge ) 
	        {
		      next = v->GetNextEdge( edge );
        
		      if ( next == start ) 
		      {
                break;
              } 
		      else if ( !next ) 
		      {            
			     break;
              } 
		      else 
		      {
                edge = next;
              }

			  dstId = edge->GetDestVertexID();

			  dstV = edge->GetDestVertex();

			  if( !dstV->isAdded() && ( center - mVertexData2[ dstId ].mCoords ).squaredNorm() < threshold )
			  {
				  mLoopVertices[ mLoopEnd++ ] = edge->GetDestVertex();

				  dstV->isAdded() = true;

				  mVertexData2[ dstId ].mIsUnderRegion = 1.0;
			  }

			  if( !dstV->isTraversed() && edgeLengthSq( edge ) > splitTh )
			  {
			    //add edge to split list

				  if( edge->GetOpposite() && !edge->GetOpposite()->isAdded() )
				  {
			                mEdgesToSplit[ mNumSplittableEdges++ ] = edge;

					edge->isAdded() = true;
				  }

				mVertexData2[ dstId ].mIsSplittable = 1.0;
				mVertexData2[ orgId ].mIsSplittable = 1.0;
			  }

            }

			v->isTraversed() = true;
		}
	}

	updateVBO();

	mVBOManager->unbind();//TODO

	std::cout<<" num splittable edges : "<<mNumSplittableEdges<<std::endl;

}


void DynamicBVH::applySplitting()
{

	mVBOManager->bind();//TODO

	for( int ss = 0; ss < mNumSplittableEdges ; ss++ )
	{
		//if( ss < 8 )

		if( isEdgeSwappable( mEdgesToSplit[ ss ] ) )
		{
			swapEdge( mEdgesToSplit[ ss ] );
		}
		//else
		//{
		//	splitEdge( mEdgesToSplit[ ss ] );
		//}
		

		mEdgesToSplit[ ss ]->isAdded() = false;
	}

	mVBOManager->addTriangles();//TODO
	mVBOManager->addVertices();//TODO

	mVBOManager->unbind();//TODO
}


void DynamicBVH::splitLongestEdge( EditableHbrFace* face )
{
  
  
  EditableHbrHalfedge *h1 = face->GetFirstEdge();
  
  EditableHbrHalfedge *h2 = h1->GetNext();
  EditableHbrHalfedge *h3 = h2->GetNext();
  
  float l1 = edgeLengthSq( h1 );
  float l2 = edgeLengthSq( h2 );
  float l3 = edgeLengthSq( h3 );
  
  EditableHbrHalfedge *hmax = 0;
  
  if( l1 > l2 )
  {
     if( l1 > l3 )
     {
       hmax = h1;  
     }
     else
     {
       hmax = h3;
     }
  }
  else if( l2 > l3 )
  {
    hmax = h2;
  }
  else
  {
    hmax = h3;
  }
    

  splitEdge( hmax );
  
  
}
    
    
void DynamicBVH::swapLongestEdge( EditableHbrFace *face )
{
    EditableHbrHalfedge *h1 = face->GetFirstEdge();
    
    EditableHbrHalfedge *h2 = h1->GetNext();
    EditableHbrHalfedge *h3 = h2->GetNext();
    
    float l1 = edgeLengthSq( h1 );
    float l2 = edgeLengthSq( h2 );
    float l3 = edgeLengthSq( h3 );
    
    EditableHbrHalfedge *hmax = 0;
    
    if( l1 > l2 )
    {
        if( l1 > l3 )
        {
            hmax = h1;
        }
        else
        {
            hmax = h3;
        }
    }
    else if( l2 > l3 )
    {
        hmax = h2;
    }
    else
    {
        hmax = h3;
    }
    
    swapEdge( hmax );
}


void DynamicBVH::collapseShortestEdge( EditableHbrFace *face )
{
  EditableHbrHalfedge *h1 = face->GetFirstEdge();
  
  EditableHbrHalfedge *h2 = h1->GetNext();
  EditableHbrHalfedge *h3 = h2->GetNext();
  
  float l1 = edgeLengthSq( h1 );
  float l2 = edgeLengthSq( h2 );
  float l3 = edgeLengthSq( h3 );
  
  EditableHbrHalfedge *hmin = 0;
  
  if( l1 < l2 )
  {
     if( l1 < l3 )
     {
       hmin = h1;  
     }
     else
     {
       hmin = h3;
     }
  }
  else if( l2 < l3 )
  {
    hmin = h2;
  }
  else
  {
    hmin = h3;
  }
  
  mergeEdge( hmin );
  
  update();
  
  
}

void DynamicBVH::resetSearchData()
{

	for( int vv = 0; vv < mLoopEnd ; vv++ )
	{
		int vId = mLoopVertices[ vv ]->GetID();

		mLoopVertices[ vv ]->isTraversed() = false;
		mLoopVertices[ vv]->isAdded() = false; 

		mVertexData2[ vId ].mIsSplittable = 0;
		mVertexData2[ vId ].mIsUnderRegion = 0;

		mVBOManager->updateVertexData( vId , mVertexData2[ vId ] );//TODO
	}

	mLoopStart = 0;
	mLoopEnd = 0;

	mNumSplittableEdges  = 0;

}

void DynamicBVH::updateVBO()
{
	//std::cout<<" loop end : "<<mLoopEnd<<std::endl;

	for( int vv = 0; vv < mLoopEnd ; vv++ )
	{
		int vId = mLoopVertices[ vv ]->GetID();

		mVBOManager->updateVertexData( vId , mVertexData2[ vId ] );//TODO
	}
}

bool DynamicBVH::isEdgeSwappable( EditableHbrHalfedge *h )
{
	EditableHbrHalfedge *hopp = h->GetOpposite();

	if( !hopp )
		return false;

	EditableHbrHalfedge* z2 = hopp->GetNext();
	EditableHbrHalfedge* z3 = z2->GetNext();

	EditableHbrHalfedge* z4 = h->GetNext();
	EditableHbrHalfedge* z1 = z4->GetNext();

  	int v1 = h->GetOrgVertexID();
	int v2 = hopp->GetOrgVertexID();

	int nv2 = z1->GetOrgVertexID();
	int nv1 = z3->GetOrgVertexID();

	float l1 = ( mVertexData2[ v1 ].mCoords - mVertexData2[ v2 ].mCoords ).squaredNorm();
	float l2 = ( mVertexData2[ nv1 ].mCoords - mVertexData2[ nv2 ].mCoords ).squaredNorm();

	if( l2 < l1 )
	{
		return true;
	}
	else
	{
		return false;
	}
}


void DynamicBVH::updateAAB( EditableHbrFace *face )
{
  
    int id1 = face->GetVertex( 0 )->GetID();
    int id2 = face->GetVertex( 1 )->GetID();
    int id3 = face->GetVertex( 2 )->GetID();
    
    Eigen::Array3f a1 = mVertexData2[ id1 ].mCoords;
    Eigen::Array3f a2 = mVertexData2[ id2 ].mCoords;
    Eigen::Array3f a3 = mVertexData2[ id3 ].mCoords;
    
    int fId = face->GetID();
    
    mTaabb[ fId ].reset();
    
    mTaabb[ fId ].addPoint( a1 );
    mTaabb[ fId ].addPoint( a2 );
    mTaabb[ fId ].addPoint( a3 );
  
}

void DynamicBVH::computeAABB( std::vector< SortDataSet >::iterator start, std::vector< SortDataSet >::iterator end, BVHaabb& aabb )
{  
  for( std::vector< SortDataSet >::iterator it = start ; it != end ; it++ )
  {
    aabb.include( mTaabb[ it->mId ] );
  }  
  
}

 
void DynamicBVH::updateSortDataSets2()
{
  int numFaces = mHbrMesh->GetNumFaces();

  mDataSet.resize( numFaces );
  
  mTaabb.resize( 2 * numFaces );
  mTCenter.resize( numFaces );
  mFaces.resize( numFaces );

  mNumCurrentFaces = numFaces;

  int ff = 0;

  double coeff = 1.0 / 3.0;

  for( int ff = 0; ff < numFaces ;  ff++ )
  {

    mTaabb[ ff ].reset();
    
    //calculate the centroid
    EditableHbrFace *face = mHbrMesh->GetFace( ff );

    face->glID() = ff;

    Eigen::Vector3f geometricCenter( 0 , 0 , 0 );

    for( int ii = 0; ii < 3; ii++ )
    {
		geometricCenter += mVertexData2[ face->GetVertex( ii )->GetID() ].mCoords;
    }

    geometricCenter *= coeff;

    mTCenter[ ff ] = geometricCenter;
    
    mDataSet[ ff ].mId = ff;
    
//     mFaces[ ff ] = face;
    
    int id1 = face->GetVertex( 0 )->GetID();
    int id2 = face->GetVertex( 1 )->GetID();
    int id3 = face->GetVertex( 2 )->GetID();
    
	Eigen::Array3f a1 = mVertexData2[ id1 ].mCoords;
	Eigen::Array3f a2 = mVertexData2[ id2 ].mCoords;
	Eigen::Array3f a3 = mVertexData2[ id3 ].mCoords;
    
    mTaabb[ ff ].addPoint( a1 );
    mTaabb[ ff ].addPoint( a2 );
    mTaabb[ ff ].addPoint( a3 );
    
  }

}


void DynamicBVH::updateNode( int nodeId )
{
    if( mNodes[ nodeId ].kind )
        return;

    BVHNode &node = mNodes[ nodeId ];

    BVHaabb aabb;

    aabb.reset();    

    int currId = nodeId;

    bool exitLoop = false;

    while( 1 )
    {
       BVHNode &currnode = mNodes[ currId ];

       if( currnode.parent == -1 )
           break;

       BVHNode &parentNode = mNodes[ currnode.parent ];

       if( parentNode.mFirst == currId )
       {
	  BVHaabb tempaabb;
	  
	  tempaabb.reset();
	  
	  if( currnode.kind )
	  {
	    	  
	    tempaabb.include( currnode.mLeft );
	    tempaabb.include( currnode.mRight );
	  }
	  else
	  {
	    computeAABB( mDataSet.begin() + node.mFirst , mDataSet.begin() + node.mLast , tempaabb );
	  }
	 
          if( parentNode.mLeft.includeAndCompare( tempaabb ) )
          {
              currId = currnode.parent;
          }
          else
          {
              exitLoop = true;
          }
       }
       else
       {
	  BVHaabb tempaabb;
	  
	  tempaabb.reset();
	  
	  if( currnode.kind )
	  {
	    	  
	    tempaabb.include( currnode.mLeft );
	    tempaabb.include( currnode.mRight );
	  }
	  else
	  {
	    computeAABB( mDataSet.begin() + node.mFirst , mDataSet.begin() + node.mLast , tempaabb );
	  }
	 
           if( parentNode.mRight.includeAndCompare( tempaabb ) )
           {
               currId = currnode.parent;
           }
           else
           {
               exitLoop = true;
           }
       }

       if( exitLoop )
       {
           break;
       }
    }
    
    DataSetIterator it = mDataSet.begin() + node.mFirst;
    DataSetIterator end = mDataSet.begin() + node.mLast;
    
    for( ; it != end ; it++ )
    {
        
//       if( mHbrMesh->GetNumFaces() <= it->mId )
//         {
//             qDebug() << " problematic data set : face id " << it->mId << " , node id : " << it->mNodeId <<" , num faces : "<<mHbrMesh->GetNumFaces()<<endl;
//         }
      EditableHbrFace *face = mHbrMesh->GetFace( it->mId );
      
      face->isUpdateNecessary() = false;
    }
}
  

void DynamicBVH::addFaceForUpdate( EditableHbrFace* face )
{
  mUpdateableFaces[ mNumUpdateableFaces ] = face;
  
  face->isUpdateNecessary() = true;
  
  updateAAB( face );
  
  mNumUpdateableFaces++;
}


void DynamicBVH::addNodeForUpdate( int& nodeId )
{
  mUpdateableNodes[ mNumUpdateableNodes ] = nodeId;
  
  mNodes[ nodeId ].mNeedsUpdate = true;
  
  mNumUpdateableNodes++;
}


void DynamicBVH::insertFace( EditableHbrFace* face , int leafNodeId  )
{
//  qDebug() << " inserting face " << leafNodeId<<" "<<mNodes.size()<< "  "<< mNodes[ leafNodeId ].mLast - mNodes[ leafNodeId ].mFirst<< endl;
  
  if( leafNodeId < 0 || leafNodeId >= mNumNodes || mNodes[ leafNodeId ].kind )
  {
    return;
  }
  
  
  BVHNode &node = mNodes[ leafNodeId ];
  
  DataSetIterator start = mDataSet.begin() + node.mFirst;
  DataSetIterator end = mDataSet.begin() + node.mLast;
  
//  qDebug() <<" current node : "<< node.mLast << "  " << mNumDataSets << endl;
  
  if( node.mLast == mNumDataSets )
  {
    node.mExtraSpace += 5;
    
    mNumDataSets += 5;
    
    if( mNumDataSets > mDataSet.size() )
    {
      mDataSet.resize( mDataSet.size() + 1000 );

      start = mDataSet.begin() + node.mFirst; 
      end = mDataSet.begin() + node.mLast;
    }
  }
  
  if( node.mExtraSpace > 0 )
  {
//     mFaces[ end->mId ] = face;
//     
//     mTCenter[ end->mId ] = geometricCenter( face );
    
    *end = *start;
    
    end->mNodeId = leafNodeId;
    
    end->mId = face->GetID();
    
    node.mExtraSpace--;
    
    face->mNodeID = leafNodeId;
    
    node.mLast++;
    
//    qDebug() << " id match : " << end->mId<< " " << face->GetID() << endl;
    
//    qDebug() <<" has extra space "<<endl;  
  }
  else
  {
    
//    qDebug() << " making extra space  "<<endl;  
    
    int nextNodeId = ( *end ).mNodeId; 
    
    BVHNode &nextNode = mNodes[ nextNodeId ];
    
    if( nextNode.kind )
      qDebug() << " fatal error "<<endl;
    
    DataSetIterator nextDataSetStart = mDataSet.begin() + nextNode.mFirst;
    DataSetIterator nextDataSetEnd = mDataSet.begin() + nextNode.mLast; 
    
    if( nextNode.mExtraSpace > 0 )
    {
      //copy       
      *nextDataSetEnd = *nextDataSetStart;
      
      nextDataSetStart->mId = face->GetID();
      nextDataSetStart->mNodeId = leafNodeId;
      
      nextNode.mExtraSpace--;

      face->mNodeID = leafNodeId;//node.mLast;
      
      node.mLast++;    
      nextNode.mFirst++;
      nextNode.mLast++;
      
    }
    else
    {
      int numElems = nextNode.mLast - nextNode.mFirst;
      
//      qDebug() << " num data set : "<<mNumDataSets<< " " <<numElems << endl;
      
      if( mDataSet.size() < ( mNumDataSets + numElems ) )
      {
	    //create more data sets
	    mDataSet.resize( mDataSet.size() + 1000 );
	    
	    nextDataSetStart = mDataSet.begin() + nextNode.mFirst;
	    nextDataSetEnd = mDataSet.begin() + nextNode.mLast; 
	    
	    start = mDataSet.begin() + node.mFirst;
	    end = mDataSet.begin() + node.mLast;
      }
      
      DataSetIterator dataSetsEnd = mDataSet.begin() + mNumDataSets;
      
      std::copy( nextDataSetStart , nextDataSetEnd , dataSetsEnd );  
      
      DataSetIterator it = dataSetsEnd;
      
      DataSetIterator newEnd = dataSetsEnd + numElems;
        
      nextNode.mFirst = mNumDataSets;
      
      for( ; it != newEnd ; it++ )
      {
	    mNumDataSets++;
      }
        
      nextNode.mLast = mNumDataSets;

      end->mNodeId = leafNodeId;
    
      end->mId = face->GetID();
      
      face->mNodeID = leafNodeId;
      
      node.mLast++;
      
//      qDebug() << " id match : " << end->mId<< " " << face->GetID() << endl;
      
      node.mExtraSpace += ( numElems - 1 );
      
    }
    
  }
  
  if( ( node.mLast - node.mFirst ) > mNodeSplitLimit )
  {
    mSplittableNodes[ mNumSplittableNodes ] = leafNodeId;
    
    mNumSplittableNodes++;
  }
  
}


void DynamicBVH::removeFace( EditableHbrFace* face )
{
   
//    int dataSetId = face->sortDataId();
   
   int nodeId = face->mNodeID;
   
   BVHNode &node = mNodes[ nodeId ];
   
   DataSetIterator it = mDataSet.begin() + node.mFirst;
   DataSetIterator end = mDataSet.begin() + node.mLast;
   
   DataSetIterator end_ = end - 1;

   int incr = 0;
   
   for( ; it != end ; it++ , incr++ )
   {
       
       EditableHbrFace *currFace = mHbrMesh->GetFace( it->mId );
       
     if( currFace == face )
     {
       
//        qDebug() << " deleting data set : data set id : "<< node.mFirst + incr << " , face id : "<< currFace->GetID() << " , node id : "<< nodeId<< " "<<end_->mNodeId << endl;
       
        *it = *end_;
	
	
	mHbrMesh->GetFace( it->mId )->mSortDataID = node.mFirst + incr;
//         EditableHbrFace *endFace = mHbrMesh->GetFace( it->mId );
	
        node.mLast--;
        node.mExtraSpace++;
	     
	
         
        break;
     }
   }
    
    

   
}

int DynamicBVH::getNumFaces()
{
  return mHbrMesh->GetNumFaces();
}



EditableHbrFace* DynamicBVH::getFace( int id )
{
  if( id < 0 || id >= mHbrMesh->GetNumFaces() )
  {
      qDebug() << " invalid id for face "<<endl;
      
    return 0;
  }
  
  return mHbrMesh->GetFace( id );
  
}

std::vector< HbrVertexData >& DynamicBVH::vertexData()
{
	return mVertexData2;
}


void DynamicBVH::update()
{
  //update faces
  
//   qDebug() << " updating faces "<<mNumUpdateableFaces<<endl;
  
  
  for(  ; mNumUpdateableNodes > 0 ; mNumUpdateableNodes-- )
  {
    int nodeId = mUpdateableNodes[ mNumSplittableNodes ];
    
     if( !mNodes[ nodeId ].kind && mNodes[ nodeId ].mNeedsUpdate )
     {
       updateNode( nodeId );  
       
       mNodes[ nodeId ].mNeedsUpdate = false;
     }
  }
  
  
  for(  ; mNumUpdateableFaces > 0 ; mNumUpdateableFaces-- )
  {
    EditableHbrFace* face = mUpdateableFaces[ mNumUpdateableFaces - 1 ];
    
    if( face->isUpdateNecessary() )
    {
       int nodeId = face->mNodeID;
      
       updateNode( nodeId );
       
//        qDebug() << " add node for update "<<endl;
    }
  }
}

  

DynamicBVH::~DynamicBVH()
{

}


}
