#ifndef __IMT_CUSTOMTILEVISITOR_H__
#define __IMT_CUSTOMTILEVISITOR_H__


#include <Inventor/SbLinear.h>
#include <Inventor/STL/map>
#include <Inventor/STL/list>

#include <Inventor/nodes/SoNode.h>
#include <Inventor/fields/SoSubField.h>

#include <Inventor/fields/SoSFNode.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoSFUInt32.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoSFFilePathString.h>
#include <Inventor/threads/SbThread.h>
#include <Inventor/threads/SbThreadMutex.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <LDM/readers/SoLDMReader.h>
#include <LDM/SoLDMNodeFrontManager.h>
#include <LDM/SoLDMTileVisitor.h>
#include <VolumeViz/readers/SoVolumeReader.h>
#include <VolumeViz/readers/SoVRLdmFileReader.h>
#include "volumetopooctree.h"


namespace imt{

	namespace volume{


		class CustomTileVisitor : public SoLDMTileVisitor
		{



		public:

			CustomTileVisitor();

			virtual float getTileWeight(SoLDMTileID tileID, NodeStatus& status);

			void setTopoOctree(imt::volume::VolumeTopoOctree *octree);

			void setMinMaxThreshold(int minVal, int maxVal);


		protected:

			imt::volume::VolumeTopoOctree *mTopoOctree;

			int mMinVal, mMaxVal;

		};

	}

}




#endif