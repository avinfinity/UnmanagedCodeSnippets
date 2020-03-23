#ifndef __IMT_WALLTHICKNESSESTIMATOR_H__
#define __IMT_WALLTHICKNESSESTIMATOR_H__


#include "volumeinfo.h"
#include "vector"
#include "eigenincludes.h"
#include "voxel.h"
#include "aabbtree.h"
#include "embreeincludes.h"
#include "opencvincludes.h"

namespace imt{

	namespace volume{




class WallthicknessEstimator
{


public:

	struct EmbreeVertex
	{
		float x, y, z, a;
	};

	struct EmbreeTriangle { int v0, v1, v2; };


	WallthicknessEstimator( VolumeInfo& volume );

	void estimateWallThickness();

	void aabbtreeDemo();

	void normalEstimationDemo();

	void rayTracingDemo();

	void lineVoxelIteratorDemo();

	void initSurfaceVoxels(unsigned short* volume, std::vector< Voxel >& initSurfaceVoxels);
	void propagateThickness( unsigned short* volume, std::vector< Voxel >& initVoxels );

	void collectSurfaceVoxels(std::vector< Voxel >& surfaceVoxels);

	void computeNormals(std::vector< Voxel >& voxels, std::vector< Eigen::Vector3f >& normals);

	static void computeDistanceTransform( VolumeInfo& volinfo );

	static void computeDistanceTransformCUDA(VolumeInfo& volinfo);

	void computeBrepRayTraceThickness(VolumeInfo& volinfo);

	void computeOtherEnds( std::vector< Eigen::Vector3f >& points, std::vector< Eigen::Vector3f >& normals,
		                   std::vector< Eigen::Vector3f >& otherEnds ,
						   std::vector< Eigen::Vector3f >& otherEndNormals );

	void computeBrepRayTraceThicknessMin(VolumeInfo& volinfo);

	void computeBrepRayTraceThickness(VolumeInfo& volinfo, float angle);

	void computeBrepSphereThickness(VolumeInfo& volinfo);

	void collectFaces( Eigen::Vector3f& center, const Eigen::Vector3f& rayDir , const float& coeff, int faceId, std::vector< int >& collectedFaces,
		               std::vector< uchar >& faceMask , float& maxDist , float& minDist , int& maxDistFaceId , int& minDistFaceId );


	void compareMeshes( std::vector< Eigen::Vector3f >& refPoints, std::vector< unsigned int >& refIndices,
		                std::vector< Eigen::Vector3f >& comparePoints, std::vector< Eigen::Vector3f >& compareNormals,
						std::vector< unsigned int >& compareIndices);

	void fillRayTraceVolume(unsigned short *volume);

	void getWallthicknessSliceImage( cv::Mat& image , unsigned short *volume );

	float pointToTriangleDistance(const Eigen::Vector3f& point, const Eigen::Vector3f& a, const Eigen::Vector3f& b, const Eigen::Vector3f&  c);

protected:

	void peelSurface( const std::vector< Voxel >& surface , std::vector< Voxel >& peeledSurface );

	void rayTrace( std::vector< Voxel >& surface, std::vector< float >& thickness);

	void rayTrace2(std::vector< Voxel >& surface, std::vector< float >& thickness);


	

	




protected:


	VolumeInfo& mVolInfo;

	std::vector< std::vector< int > > mFaceAdjacency;

	std::vector< Eigen::Vector3f > mFaceCenters;

public:

	RTCScene mScene;

	unsigned int mGeomID;

	RTCDevice mDevice;

	size_t mZStep, mYStep;

	
};



	}


}



#endif