#include "aabbtree.h"
#include "iostream"

namespace imt{
	
	namespace volume
	{


		AABB::AABB()
		{
			mMin = Eigen::Vector3f(FLT_MAX, FLT_MAX, FLT_MAX);
			mMax = Eigen::Vector3f(FLT_MIN , FLT_MIN , FLT_MIN ) ;
		}


		float AABB::intersect(const Ray& ray) const
		{
			float t0 = ray.mMinT;
			float t1 = ray.mMaxT;

			for (int i = 0; i < 3; ++i)
			{
				float nearv = (mMin(i) - ray.mOrigin(i)) * ray.mInvDir(i);
				float farv = (mMax(i) - ray.mOrigin(i)) * ray.mInvDir(i);

				if (nearv > farv)
					std::swap(nearv, farv);

				if (nearv > t0) t0 = nearv;
				if (farv < t1) t1 = farv;

				if (t0 > t1)
					return ray.mMaxT;
			}

			return t0;
		}


		void AABB::addVoxel(const Eigen::Vector3f& origin, const Eigen::Vector3f& step)
		{
			mMin(0) = std::min( mMin(0) , origin(0));
			mMin(1) = std::min( mMin(1) , origin(1));
			mMin(2) = std::min( mMin(2) , origin(2));

			mMax(0) = std::max( mMax(0) , step(0) );
			mMax(1) = std::max( mMax(1) , step(1) );
			mMax(2) = std::max( mMax(2) , step(2) );

		}


		float AABBNode::intersect( const Ray& ray ) const
		{
			return mAabb.intersect(ray);
		}
		
		
		AABBTree::AABBTree(std::vector< Voxel >& voxels) : mVoxels( voxels )
		{
			mVolumeOrigin.setZero();
			mVoxelStep.setOnes();  
		}


	  void sortVoxels( std::vector< Voxel >::iterator start , std::vector< Voxel >::iterator end, int& sortAxis )
	  {
		  int xMin = std::numeric_limits< int >::max(), yMin = std::numeric_limits< int >::max(), zMin = std::numeric_limits< int >::max(), 
			  xMax = std::numeric_limits< int >::min(), yMax = std::numeric_limits< int >::min(), zMax = std::numeric_limits< int >::min();


		  for (auto it = start; it != end; it++)
		  {
			  xMin = std::min(xMin, (int)it->x);
			  yMin = std::min(yMin, (int)it->y);
			  zMin = std::min(zMin, (int)it->z);

			  xMax = std::min(xMax, (int)it->x);
			  yMax = std::min(yMax, (int)it->y);
			  zMax = std::min(zMax, (int)it->z);
		  }

		  int xDiff = xMax - xMin;
		  int yDiff = yMax - yMin;
		  int zDiff = zMax - zMin;

		  if ( xDiff > yDiff && xDiff > zDiff )
		  {
			  sortAxis = 0;

			  std::sort(start, end, [](const Voxel& v1, const Voxel& v2){ return v1.x <= v2.x; });
		  }
		  else if ( yDiff > zDiff )
		  {
			  sortAxis = 1;

			  std::sort(start, end, [](const Voxel& v1, const Voxel& v2){ return v1.y <= v2.y; });
		  }
		  else
		  {
			  sortAxis = 2;

			  std::sort(start, end, [](const Voxel& v1, const Voxel& v2){ return v1.z <= v2.z; });
		  }

      }

	  void AABBTree::build()
	  {
		  int numSurfVoxels = mVoxels.size();

		  int sortAxis = -1;

		  sortVoxels( mVoxels.begin() , mVoxels.end() , sortAxis );

		  mNodes.reserve(numSurfVoxels / 2);

		  AABBNode node;

		  node.start = 0;

		  node.end = numSurfVoxels;

		  node.kind = sortAxis;

		  node.parent = -1;  

		  mNodes.push_back(node);

		  build(0);

		  int numNodes = mNodes.size();

		  std::cout << " num nodes : " << numNodes << std::endl;


	  }


	  void AABBTree::build( int nodeId )
	  {
		  std::vector< Voxel >::iterator start = mVoxels.begin() + mNodes[nodeId].start;
		  std::vector< Voxel >::iterator end = mVoxels.begin() + mNodes[nodeId].end;

		  int numVoxels = mNodes[nodeId].end - mNodes[nodeId].start;

		  if (nodeId == 0)
		  {
			  std::cout << " num voxels : " << numVoxels 
				        << " " << mVoxels.size() << std::endl;
		  }
		    

		  int sortAxis = -1;

		  sortVoxels(start, end, sortAxis);

		  AABBNode& parentNode = mNodes[nodeId];

		  Eigen::Vector3f step = mVoxelStep;

		  for ( auto it = start; it != end; it++ )
		  {
			  Eigen::Vector3f point;

			  point(0) = it->x * mVoxelStep(0) + mVolumeOrigin(0);
			  point(1) = it->y * mVoxelStep(1) + mVolumeOrigin(1);
			  point(2) = it->z * mVoxelStep(2) + mVolumeOrigin(2);

			  parentNode.mAabb.addVoxel(point, step);
		  }

		  if ( mNodes[nodeId].end - mNodes[nodeId].start  < 10 )
			  return;


		  AABBNode node1 , node2;
		  node1.start = parentNode.start;
		  node1.end = parentNode.start + numVoxels / 2;
		  node1.kind = -1;
		  node1.parent = nodeId;

		  node2.start = parentNode.start + numVoxels / 2;
		  node2.end = parentNode.start + numVoxels;
		  node2.kind = -1;
		  node2.parent = nodeId;

		  mNodes[nodeId].start = mNodes.size() ;
		  mNodes[nodeId].end = mNodes.size() ;
		  mNodes[nodeId].kind = sortAxis;

		  mNodes.push_back(node1);
		  mNodes.push_back(node2);

		  build(mNodes[nodeId].start);
		  build(mNodes[nodeId].end);
		  
	  }





	  inline unsigned nearChild( const AABBNode& node , const Ray& ray )
	  {
		  if (ray.mDir(node.kind - 1) > 0.0f)
		  {
			  return node.start;
		  }
		  else
		  {
			  return node.end;
		  }
			  
	  }



	  bool AABBTree::intersect( Ray& r )
	  {
		  bool intersectionFound = false;

		  
		  const std::vector< AABBNode >& nodes = mNodes;

		  unsigned last = -1;
		  unsigned current = 0;


		  do {
				  const AABBNode& n = nodes[current];

				  assert( current < nodes.size() );

				  if ( n.kind < 0)
				  {
					  for ( unsigned i = n.start; i < n.end; ++i)
					  {
						  assert( i < mVoxels.size() && i >= 0);

						  if ( intersect( mVoxels[ i ] , r ) )
						  {

							  //face = mBvh.getFace(triangles[i].mId);

							  //if (!face)
							  //{
								 // qDebug() << " face id : " << triangles[i].mId << endl;
							  //}
						  }
					  }

					  // Go up.
					  last = current;

					  current = n.parent;

					  continue;
				  }

				  unsigned nearv = nearChild(n, r);

				  unsigned farv = nearv == n.start ? n.end : n.start;

				  if (last == farv)
				  {
					  // Already returned from far child − traverse up.
					  last = current;
					  current = n.parent;
					  continue;
				  }

				  // If coming from parent, try near child, else far child.
				  int tryChild = (last == n.parent) ? nearv : farv;
				  const AABBNode& bb = tryChild == n.start ? mNodes[ n.start ] : mNodes[ n.end ];

				  float maxT = r.mMaxT;

				  if ( bb.intersect(r) != maxT )
				  {
					  // If box was hit, descend.
					  last = current;
					  current = tryChild;
					  continue;
				  }

				  if (tryChild == nearv)
				  {
					  // Next is far.
					  last = nearv;
				  }
				  else 
				  {
					  // Go up instead.
					  last = current;
					  current = n.parent;
				  }

			  } while ( current != -1 );


		  return intersectionFound;
	  }


	  bool AABBTree::intersect2( Ray& r )
	  {
		  bool intersectionFound = false;

		  const std::vector< AABBNode >& nodes = mNodes;

		  return false;

	  }


	  bool AABBTree::intersect( Voxel& voxel, Ray& ray )
	  {

		  //initialize aabb for the voxel
		  AABB aabb;

		  int x = voxel.x;
		  int y = voxel.y;
		  int z = voxel.z;

		  aabb.mMin(0) = x * mVoxelStep(0) + mVolumeOrigin(0);
		  aabb.mMin(1) = y * mVoxelStep(1) + mVolumeOrigin(1);
		  aabb.mMin(2) = z * mVoxelStep(2) + mVolumeOrigin(2);

		  aabb.mMax(0) = ( x + 1 ) * mVoxelStep(0) + mVolumeOrigin(0);
		  aabb.mMax(1) = ( y + 1 ) * mVoxelStep(1) + mVolumeOrigin(1);
		  aabb.mMax(2) = ( z + 1 ) * mVoxelStep(2) + mVolumeOrigin(2);

		  float t = aabb.intersect(ray);

		  return t > 0;
	  }


	  bool AABBTree::findClosestPoint(Ray& r, float searchAngle, Eigen::Vector3f& point)
	  {
		  bool closestPointFound = false;



		  return closestPointFound;
	  }

	  bool AABBTree::findClosestVoxel( Ray& r , float searchAngle , Voxel& voxel )
	  {
		  bool closestVoxelFound = false;




		  return closestVoxelFound;
		  
	  }

	  void AABBTree::clear()
	  {
		   
	  }


	  std::vector< AABBNode >& AABBTree::getNodes()
	  {
		  return mNodes;
	  }
		
		
		
		
	}
	
	
}