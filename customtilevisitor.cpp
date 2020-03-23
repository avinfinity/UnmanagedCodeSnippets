#include "customtilevisitor.h"

namespace imt{

	namespace volume{



		CustomTileVisitor::CustomTileVisitor() : SoLDMTileVisitor()
		{
			mTopoOctree = 0;
		}




		float CustomTileVisitor::getTileWeight( SoLDMTileID tileID, NodeStatus& status )
		{
			float tileWeight = SoLDMTileVisitor::getTileWeight(tileID, status);

			//std::cout << " tile weight " << tileWeight << std::endl;

			int fileId = mTopoOctree->getFileId( tileID.getID() );
			
			//std::cout << " file id : " << fileId <<" " << tileID.getID() << std::endl;
			
			auto node = mTopoOctree->getFileNode(fileId);

			//std::cout << node->_MinVal << " " << node->_MaxVal << std::endl;

			if (node->_MaxVal < mMinVal || node->_MinVal > mMaxVal)
			{
				status.m_inROI = false;

				tileWeight = NULL;
			}

			return tileWeight;
			
		}


		void CustomTileVisitor::setTopoOctree( imt::volume::VolumeTopoOctree *octree )
		{
			mTopoOctree = octree;
		}


		void CustomTileVisitor::setMinMaxThreshold( int minVal , int maxVal )
		{
			mMinVal = minVal;
			mMaxVal = maxVal;

		}

	}


}