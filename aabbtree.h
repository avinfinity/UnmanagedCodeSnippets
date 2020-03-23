#ifndef __AABBTREE_H__
#define __AABBTREE_H__


#include "eigenincludes.h"
#include "voxel.h"

namespace imt{
	
	namespace volume{
		
			struct Ray{

			Eigen::Vector3f mOrigin, mDir , mInvDir;

			int mMinT, mMaxT;



		};


		struct AABB
		{


			AABB();

			Eigen::Vector3f mMin, mMax;

			void addPoint(Eigen::Vector3f point);

			void addVoxel(const Eigen::Vector3f& origin, const Eigen::Vector3f& step);

			unsigned int largestAxis();

			float intersect(const Ray& ray) const;

		};


		struct AABBNode
		{

			AABB mAabb;

			int start, end , parent , kind;

			float intersect(const Ray& ray) const;
		};



  class AABBTree
  {

  public:

	  AABBTree(std::vector< Voxel >& voxels);

	  void build();

	  bool intersect(Ray& r);

	  bool intersect2(Ray& r);

	  bool findClosestPoint(Ray& r, float searchAngle, Eigen::Vector3f& point);

	  bool findClosestVoxel(Ray& r, float searchAngle, Voxel& voxel);

	  void clear();

	  void build( int nodeId );

	  bool intersect( Voxel& voxel, Ray& ray );

	  std::vector< AABBNode >& getNodes();


  protected:

	  std::vector< Voxel >& mVoxels;

	  std::vector< AABBNode > mNodes;

	  Eigen::Vector3f mVolumeOrigin;
	  Eigen::Vector3f mVoxelStep;

  };	
		
		
		
		
		
		
	}
	
}



#endif