#ifndef __IMT_VOLUME_GRIDHIERARCYTREE_H__
#define __IMT_VOLUME_GRIDHIERARCYTREE_H__

#include <vector>

namespace imt {

	namespace volume {

		class GridHierarchyTree 
		{

		public:

			struct Edge
			{

				int _VertexId1, _VertexId2;
			};

			struct TreeNode
			{
				int _SubdivisionLevel;

				std::vector< std::vector< Edge > > _Edges;

				std::vector<TreeNode*> _Children;
		
				std::vector<TreeNode*> _Neighbors;
			};

			
			GridHierarchyTree( int fixedRefinementLevel , int hierarchicalRefinementLevel, int w , int h , int d  );


		protected:


			TreeNode* faceOppositeNode( TreeNode* node , int faceId );

			int _VolumeWidth , _VolumeHeight , _VolumeDepth;

			int _FixedRefinementLevel, _HierarchicalRefinementLevel;

			int _FixedSubdivX, _FixedSubdivY, _FixedSubdivZ;

			std::vector<TreeNode*> _FixedLevelHierarchyNodes;

		

		};

	}




}

#endif