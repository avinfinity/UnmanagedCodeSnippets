#include "gridhierarchytree.h"
#include <iostream>

namespace imt 
{


	namespace volume 
	{


		GridHierarchyTree::GridHierarchyTree( int fixedRefinementLevel , int hierarchicalRefinementLevel , int w, int h, int d )
		{
			_VolumeWidth = w;
			_VolumeHeight = h;
			_VolumeDepth = d;

			_FixedRefinementLevel = fixedRefinementLevel;
			_HierarchicalRefinementLevel = hierarchicalRefinementLevel;

			int blockSize = 1 << _FixedRefinementLevel;

			_FixedSubdivX = _VolumeWidth % blockSize == 0 ? _VolumeWidth / blockSize : _VolumeWidth / blockSize + 1;
			_FixedSubdivY = _VolumeHeight % blockSize == 0 ? _VolumeHeight / blockSize : _VolumeHeight / blockSize + 1;
			_FixedSubdivZ = _VolumeDepth % blockSize == 0 ? _VolumeDepth / blockSize : _VolumeDepth / blockSize + 1;

			_FixedLevelHierarchyNodes.resize( _FixedSubdivX * _FixedSubdivY * _FixedSubdivZ , 0 );

			//std::cout << " fixed hierarchy nodes :  " << _FixedLevelHierarchyNodes.size() << std::endl;
			
		    

		}


		GridHierarchyTree::TreeNode* GridHierarchyTree::faceOppositeNode(TreeNode* node, int faceId)
		{
			return NULL;
		}




	}


}