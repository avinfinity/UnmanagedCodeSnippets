
#include <Inventor/ViewerComponents/SoRenderAreaCore.h>
#include "OpenInventor/RenderArea.h"
#include "OpenInventor/RenderAreaInteractive.h"
#include <Inventor/SoDB.h>
#include <Inventor/nodes/SoCone.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/ViewerComponents/SoCameraInteractor.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/helpers/SbDataTypeMacros.h>
#include <VolumeViz/readers/SoVolumeReader.h>
#include <VolumeViz/converters/SoVolumeConverter.h>
#include <VolumeViz/readers/SoVRGenericFileReader.h> 
#include <VolumeViz/nodes/SoVolumeRender.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include "QApplication"



void readRawData()
{

	SoVRGenericFileReader *rawReader = new SoVRGenericFileReader();
	rawReader->setFilename("C:/projects/datasets/Bitron/separated_part_0.uint16_scv");

	int width = 212, height = 162, depth = 331;
	float voxelSize = 0.08714330;

	rawReader->setDataChar( SbBox3f(0.0f, 0.0f, 0.0f, width*voxelSize, height*voxelSize, depth*voxelSize) ,
		                    SoDataSet::UNSIGNED_SHORT, SbVec3i32((int)width, (int)height, (int)depth), 1024);




	rawReader->
	//rawReader->read(;


	//SoVolumeData *volume = new SoVolumeData;

	//volume->
}

int main( int argc , char **argv )
{



	QApplication app(argc, argv);

	SoDB::init();

	//readRawData();

	// create Open Inventor scene graph
	SoRef<SoSeparator> rootSceneGraph = new SoSeparator;
	SoPerspectiveCamera* camera = new SoPerspectiveCamera;
	SoVolumeRender *vr = new SoVolumeRender;
	SoCameraInteractor *interactor = new SoCameraInteractor( camera );
	rootSceneGraph->addChild(camera);
	rootSceneGraph->addChild(new SoDirectionalLight);
	rootSceneGraph->addChild(new SoCone);
	//rootSceneGraph->addChild(interactor);
	
	//vr->set
	//rootSceneGraph->addChild(vr);
	
	
	camera->viewAll(rootSceneGraph.ptr(), SbViewportRegion());
	
	RenderAreaInteractive *renderWidget = new RenderAreaInteractive(0);
	renderWidget->setSceneGraph(rootSceneGraph.ptr());

	renderWidget->showMaximized();



	
	return app.exec();
}



//void buildTransferFunction()
//{
//
//	SoTransferFunction transferFunction;// = node->RequestTransferFunction();
//
//	transferFunction.minValue.setValue(0);
//	transferFunction.maxValue.setValue(255);
//
//	//array<float>^ colorMap = renderingParams->CurrentColorMapWithOpacityMask;
//	
//	std::vector< float > colorMap;
//
//	bool isoRendering = true;
//	
//	transferFunction.enableNotify(FALSE);
//	int numEntries = 256;
//	for (int i = 0; i < numEntries; i++)
//	{
//		transferFunction.colorMap.set1Value(4 * i + 0, colorMap[4 * i + 0]); // red
//		transferFunction.colorMap.set1Value(4 * i + 1, colorMap[4 * i + 1]); // green
//		transferFunction.colorMap.set1Value(4 * i + 2, colorMap[4 * i + 2]); // blue	
//
//		float alpha = colorMap[4 * i + 3];
//
//		if (renderingParams->IsoRendering && dynamic_cast<GLOivVolumeNode*>(node) != nullptr)
//		{
//			if (i < renderingParams->ColorMapThreshold)
//				alpha = 0.0f;
//		}
//		else if (!renderingParams->ApplyTransparencyOnSlice && dynamic_cast<GLOivSliceNode*>(node) != nullptr && alpha < 0.004f)	// auf die Slices soll sich die Transparenz nicht auswirken
//		{
//			alpha = 0.004f;
//
//			if (renderingParams->Materials != nullptr)	// Sichtbarkeit der slices überprüfen
//			{
//				for (int m = 0; m < renderingParams->Materials->NumberOfMaterials; m++)
//				{
//					if (!renderingParams->Materials->Materials[m]->Visible)
//					{
//						double lowerBound = (double)renderingParams->Materials->Materials[m]->LowerBound / (65536 - 1) * (numEntries - 1);
//						double upperBound = (double)renderingParams->Materials->Materials[m]->UpperBound / (65536 - 1) * (numEntries - 1);
//						if (i >= lowerBound && i <= upperBound)
//							alpha = 0.0f;
//					}
//				}
//			}
//		}
//
//
//		transferFunction.colorMap.set1Value(4 * i + 3, alpha); // alpha
//
//	}
//	transferFunction.enableNotify(TRUE);
//	transferFunction.touch();
//
//}