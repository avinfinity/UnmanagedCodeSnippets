
#include "iostream"
#include "QString"
#include "volumeinfo.h"
#include "volumeutility.h"
#include "QDebug"

#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <VolumeViz/converters/SoVolumeConverter.h>
#include <LDM/converters/SoConverterParameters.h>
#include <Inventor/helpers/SbDataTypeMacros.h>
#include <LDM/readers/SoVRLdmFileReader.h>
#include <Inventor/devices/SoCpuBufferObject.h> 
#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <VolumeViz/nodes/SoVolumeData.h> 
#include <VolumeViz/nodes/SoTransferFunction.h>
#include <VolumeViz/readers/SoVRGenericFileReader.h> 
#include <LDM/converters/SoConverterParameters.h>
#include <LDM/readers/SoLDMReader.h>
#include <LDM/readers/SoVRLdmFileReader.h>
#include <LDM/SoLDMTopoOctree.h>
//#include <LDM/tiles/>
#include <LDM/fields/SoSFLDMResourceParameters.h>
#include "NeoInsightsLDMreader.h"
#include "NeoInsightsLDMWriter.h"
#include <VolumeViz/nodes/SoVolumeRender.h>

#include <Inventor/Xt/SoXt.h>
#include <Inventor/Xt/viewers/SoXtExaminerViewer.h>
#include <Inventor/nodes/SoSeparator.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/nodes/SoVolumeSkin.h>
#include <LDM/nodes/SoDataRange.h>
#include <LDM/nodes/SoTransferFunction.h>
#include <VolumeViz/nodes/SoVolumeRendering.h>

#include "QDomDocument"

#include "volumeconverter.h"
#include "opencvincludes.h"
#include "volumetopooctree.h"

#include "display3droutines.h"
#include "vtkImageData.h"
#include "vtkMarchingCubes.h"
#include "histogramfilter.h"
#include "QDebug"
#include "volumesegmenter.h"
#include "vtkSTLReader.h"
#include <vtkColorTransferFunction.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolumeProperty.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPLYWriter.h>

#include "rawvolumedataio.h"
#include "volumeutility.h"
#include <Inventor/nodes/SoFragmentShader.h>
#include <VolumeViz/nodes/SoVolumeRenderingQuality.h>
#include "customtilemanager.h"
#include "customtilevisitor.h"
#include "customnodefrontmanager.h"
#include "CustomTiledReader.h"

void viewVolumeSkin(SoVolumeData* pVolumeData, ushort minVal, ushort maxVal);

void viewIsoSurface(imt::volume::VolumeInfo& volume, int isoVal);

int main( int argc , char **argv )
{
	QString filePrefix = "C:/Data/WallthicknessData/11.1.17/Bizerba_2 2016-8-25 17-50-31";
	

	
	return 0;
}


void viewVolumeSkin(SoVolumeData* pVolumeData, ushort minVal, ushort maxVal)
{

	// If necessary, specify the actual range of the data values.
	//    By default VolumeViz maps the entire range of the voxel data type
	//    (e.g. 0..65535 for unsigned short) into the colormap.  This works
	//    great for byte (8 bit) voxels, but not so well for 16 bit voxels
	//    and not at all for floating point voxels. So it's not actually
	//    necessary for this data set, but shown here for completeness.
	//    NOTE: Min/max values are stored in the header for LDM format
	//    files, but for other formats the getMinMax query can take a
	//    long time because VolumeViz has to examine every voxel.
	SoDataRange *pRange = new SoDataRange();

	int voxelSize = pVolumeData->getDataSize();


	//std::cout << " voxel size : " << voxelSize << " " << pVolumeData->getDataType() << std::endl;

	//if (voxelSize > 1) 
	{
		//double minval, maxval;
		//pVolumeData->getMinMax(minval, maxval);
		pRange->min = minVal;
		pRange->max = maxVal;
	}

	// Use a predefined colorMap with the SoTransferFunction
	SoTransferFunction* pTransFunc = new SoTransferFunction;
	pTransFunc->predefColorMap = SoTransferFunction::STANDARD;//AIRWAY //NONE; //STANDARD;//TEMPERATURE;//

	pTransFunc->minValue = minVal;
	pTransFunc->maxValue = maxVal;


	//NONE,
	//	/** Grey (Default). This is constant intensity (white) with a 0..1 alpha ramp.
	//	*                  A good initial color map for volume rendering. */
	//	GREY,
	//	/** Gray (Synonym of grey) */
	//	GRAY = GREY,
	//	/** Temperature (opaque). */
	//	TEMPERATURE,
	//	/** Physics (opaque). */
	//	PHYSICS,
	//	/** Standard (opaque). */
	//	STANDARD,
	//	/** Glow (opaque). This is similar to "hot iron" type color maps. */
	//	GLOW,
	//	/** Blue red (opaque). */
	//	BLUE_RED,
	//	/** Seismic */
	//	SEISMIC,
	//	/** Blue white red (opaque). */
	//	BLUE_WHITE_RED,
	//	/** Intensity (opaque). This is an intensity ramp (gray scale) with constant alpha (1).
	//	*                      A good initial color map for slice rendering. */
	//	INTENSITY,
	//	/** 256 labels (opaque). A good color map for rendering label volumes. */
	//	LABEL_256,
	//	/** VolRenRed (opacity ramp). Suitable for volume rendering. **/
	//	VOLREN_RED,
	//	/** VolRenGreen (opacity ramp). Suitable for volume rendering. **/
	//	VOLREN_GREEN,
	//	/** Airway.
	//	* This colormap is especially adapted to display airways of CT Scan datasets.
	//	* Ranges of data may have to be adapted depending on your acquisition device's calibration.
	//	* Please refer to SoDataRange node for details. */
	//	AIRWAY,
	//	/** Airway surfaces.
	//	* This colormap is especially adapted to display airways and soft tissues of CT Scan datasets.
	//	* Ranges of data may have to be adapted depending on your acquisition device's calibration.
	//	* Please refer to SoDataRange node for details. */
	//	AIRWAY_SURFACES

	// Node in charge of drawing the volume
	SoVolumeSkin *pVolRender = new SoVolumeSkin;

	// Node in charge of drawing the volume
	SoVolumeRender* pVolRender2 = new SoVolumeRender;
	pVolRender2->samplingAlignment = SoVolumeRender::DATA_ALIGNED;

	// Assemble the scene graph
	// Note: SoVolumeSkin must appear after the SoVolumeData node.
	SoSeparator *root = new SoSeparator;
	root->ref();
	root->addChild(pVolumeData);
	//root->addChild(pRange);
	root->addChild(pTransFunc);
	root->addChild(pVolRender2);

	Widget myWindow = SoXt::init("");

	// Set up viewer:
	SoXtExaminerViewer *myViewer = new SoXtExaminerViewer(myWindow);
	myViewer->setSceneGraph(root);
	myViewer->setTitle("Volume Skin");
	myViewer->show();


	SoXt::show(myWindow);
	SoXt::mainLoop();
	SoVolumeRendering::finish();
	SoXt::finish();


}


void viewIsoSurface(imt::volume::VolumeInfo& volume, int isoVal)
{
	vtkSmartPointer< vtkImageData > volumeData = vtkSmartPointer< vtkImageData >::New();


	volumeData->SetOrigin(volume.mVolumeOrigin(0), volume.mVolumeOrigin(1), volume.mVolumeOrigin(2));
	volumeData->SetSpacing(volume.mVoxelStep(0), volume.mVoxelStep(1), volume.mVoxelStep(2));
	volumeData->SetDimensions(volume.mWidth, volume.mHeight, volume.mDepth);
	volumeData->SetExtent(0, volume.mWidth - 1, 0, volume.mHeight - 1, 0, volume.mDepth - 1);
	volumeData->AllocateScalars(VTK_UNSIGNED_SHORT, 1);

	long int zStep = volume.mHeight * volume.mWidth;
	long int yStep = volume.mWidth;

	unsigned char *ptr = (unsigned char *)volumeData->GetScalarPointer();

	std::cout << volume.mWidth << " " << volume.mHeight << " " << volume.mDepth << std::endl;

	memcpy(ptr, volume.mVolumeData, volume.mWidth * volume.mHeight * volume.mDepth * 2);

	vtkSmartPointer< vtkMarchingCubes > marchingCubes = vtkSmartPointer< vtkMarchingCubes >::New();

	marchingCubes->SetInputData(volumeData);

	marchingCubes->SetValue(0, isoVal);

	marchingCubes->ComputeNormalsOn();

	marchingCubes->Update(0);

	vtkSmartPointer< vtkPolyData > isoSurface = vtkSmartPointer< vtkPolyData >::New();

	isoSurface->DeepCopy(marchingCubes->GetOutput());

	tr::Display3DRoutines::displayPolyData(isoSurface);

}

