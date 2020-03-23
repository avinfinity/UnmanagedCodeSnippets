#ifndef __IMT_DUALMARCHINGCUBES_H__
#define __IMT_DUALMARCHINGCUBES_H__

#include "volumeinfo.h"
#include "marchingcubes.h"

namespace imt {

	namespace volume {


		class DMCOctree {


		public:



			struct DMCOctreeNode
			{

				DMCOctreeNode *children[8];

				Eigen::Vector3f _Center;

				DMCVertex _SolvedVertex;

				float _Step;

				bool _IsLeaf;

				double sign;

				DMCOctreeNode(float step = 1.0);

				DMCVertex vertex() const;

				Eigen::Vector3i _Start;
				int step;

			};

			DMCOctree();

			void build( VolumeInfo& volume , int divisions = 4 );

		protected:

			DMCOctreeNode * _RootNode;

			std::vector<DMCOctreeNode*> _TreeNodes;


		};


		class DualMarchingCubes {

		public:

			DualMarchingCubes( imt::volume::VolumeInfo& volume, int isoThreshold , std::vector<Eigen::Vector3f>& vertices );

			void compute();




		protected:

			double valueAt(double x, double y, double z, unsigned int width, unsigned int height, unsigned short *volumeData);

			Eigen::Vector3f gradientAt( double x, double y, double z, unsigned int width, unsigned int height, unsigned short *volumeData , float scale );

			void buildTree( DMCOctree::DMCOctreeNode *node );

			bool computeOptimalPosition(std::vector<int64_t>& pointIndices , Eigen::Vector3f& optimalPosition , double &distance);

			void buildOctree(int division);

			void buildSubtree(DMCOctree::DMCOctreeNode* node, std::vector<int64_t>& pointIds , const Eigen::Vector3f min , const Eigen::Vector3f& max , int depth);

		protected:

			imt::volume::VolumeInfo &_VolumeInfo;

			int32_t _IsoThreshold;

			std::vector<Eigen::Vector3f> mOptimalPositions;

			std::vector<Eigen::Vector3f> mMarchingCubesVertices;
			std::vector<Eigen::Vector3f> mVertexGradients;

			int _UniformSubdivisionDepth;

			double _QuadricEnergyThreshold;

			std::vector<DMCOctree::DMCOctreeNode*> _AllNodes;


		};



	}




}




#endif