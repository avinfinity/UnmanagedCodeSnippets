#ifndef __VC_MULTIMATERIALSURFACEEXTRACTOR_H__
#define __VC_MULTIMATERIALSURFACEEXTRACTOR_H__


#include "volumeinfo.h"


namespace imt {

	namespace volume {


		class VolumeOctree 
		{

		public:

			VolumeOctree( int w, int h, int d, std::vector<Eigen::Vector3f>& points, std::vector<Eigen::Vector3f>& normals );
		protected:

			struct OctreeNode
			{
				int xi, yi, zi;

				int level;

				std::vector<int> mPointIndices;

				bool mIsLeaf;

				std::vector<OctreeNode*> mChildren;

			};
        



		protected:

			void buildVolumeOctree();


			void buildNode(OctreeNode* node);






		protected:


			std::vector<Eigen::Vector3f>& mPoints, &mNormals;

			int mWidth, mHeight, mDepth;


		};



		class MultiMaterialSurfaceExtractor 
		{


		public:


			class SobelGradientOperator3x3x3
			{
				int _KernelGx[3][3][3], _KernelGy[3][3][3], _KernelGz[3][3][3];

			public:

				SobelGradientOperator3x3x3();

				void init(size_t volumeW, size_t volumeH, size_t volumeD, double voxelSizeX,
					double voxelSizeY, double voxelSizeZ, unsigned short *volumeData);

				void apply( const Eigen::Vector3f& point, Eigen::Vector3f& gradient);

				void computeCentralDifferenceGradient( const Eigen::Vector3f& point, Eigen::Vector3f& gradient  );

			protected:


				size_t _VolumeW, _VolumeH, _VolumeD;

				double _VoxelSizeX, _VoxelSizeY, _VoxelSizeZ;

				unsigned short *_VolumeData;
			};


			struct MaterialRegion 
			{
				int _StartValue, _EndValue;
			};


			MultiMaterialSurfaceExtractor(imt::volume::VolumeInfo& volume , std::vector<MaterialRegion>& materialRegions );

			void compute();

			void compute(int startVal, int endVal , int isoThreshold);



		protected:

			bool isEdge( Eigen::Vector3f& point, Eigen::Vector3f& gradient , double gradientMagnitude , int checksize = 3 );

			bool computeProfile(Eigen::Vector3f& point, int halfProfileSie, Eigen::Vector3f& direction);

			void computeVolumeGradients();
			void buildConnectivity(std::vector<Eigen::Vector3f>& gradients, std::vector<unsigned char>& edgeVoxels);

			Eigen::Vector3f centralDifferenceGradientAtVoxel(int voxelX, int voxelY, int volxelZ) const;

			Eigen::Vector3f gradientAtCorner( int voxelX , int voxelY , int voxelZ );

			void filterNonEdgeVoxels(int z, std::vector<Eigen::Vector3f>& voxelGradients, std::vector<unsigned char>& edgeVoxelFlags);

			void computeIntersections(Eigen::Vector3f& gradient, Eigen::Vector3f& position);

			double intersectionWithXYPlane( const Eigen::Vector3f& planePosition, const Eigen::Vector3f& lineCenter, const Eigen::Vector3f& lineDirection) const;
			double intersectionWithYZPlane( const Eigen::Vector3f& planePosition, const Eigen::Vector3f& lineCenter, const Eigen::Vector3f& lineDirection) const;
			double intersectionWithZXPlane( const Eigen::Vector3f& planePosition, const Eigen::Vector3f& lineCenter, const Eigen::Vector3f& lineDirection) const;


			void traceMaximaVoxels( std::vector<unsigned char>& gradientMask );

		protected:


			double _SurfaceThreshold1, _SurfaceThreshold2, _IsoThreshold;

			std::vector<MaterialRegion> _MaterialRegions;

			imt::volume::VolumeInfo& _VolumeInfo;

			unsigned short *_VolumeData;

			std::vector<unsigned short*> _VolumeSlices;

			int64_t _ZStep, _YStep;

			SobelGradientOperator3x3x3 *_GradientEvaluator;



		};

		




	}




}


#endif