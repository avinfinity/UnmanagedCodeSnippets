#ifndef __IMT_CUSTOMNODEFRONTMANAGER_H__
#define __IMT_CUSTOMNODEFRONTMANAGER_H__

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
#include <VolumeViz/readers/SoVolumeReader.h>
#include <VolumeViz/readers/SoVRLdmFileReader.h>

namespace imt {

	namespace volume{

		class CustomNodeFrontManager : public SoLDMNodeFrontManager
		{



		public:

			CustomNodeFrontManager();



		};

	}

}


#endif