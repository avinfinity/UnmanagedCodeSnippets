
#include <vtkVersion.h>
#include <vtkSmartPointer.h>

#include <string>

#include <vtkActor.h>
#include <vtkImageAccumulate.h>
#include <vtkImageData.h>
#include <vtkImageExtractComponents.h>
#include <vtkJPEGReader.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkXYPlotActor.h>
#include "iostream"
#include "vtkDoubleArray.h"
#include "vtkFieldData.h"

//#include "histogramfilter.h"
//#include "rawvolumedataio.h"

#include "opencvincludes.h"


//void plotHistogram()
//{
//	// Handle the arguments
//	//if (argc < 2)
//	//{
//	//	std::cout << "Usage: " << argv[0] << " Filename.jpg, [optional ignore zero:] <y/n>" << std::endl;
//	//	return EXIT_FAILURE;
//	//}
//
//
//	std::string fileName = "C:/Users/INASING1/Pictures/download.jpg";
//
//
//	int ignoreZero = 0;
//	/*if (argc == 3)
//	{
//		std::string ignore = argv[2];
//		std::cout << ignore << std::endl;
//		if (ignore == "y" || ignore == "Y")
//		{
//			ignoreZero = 1;
//		}
//	}
//*/
//	// Read a jpeg image
//	vtkSmartPointer<vtkJPEGReader> reader =
//		vtkSmartPointer<vtkJPEGReader>::New();
//	if (!reader->CanReadFile(fileName.c_str()))
//	{
//		std::cout << "Error: cannot read " << fileName << std::endl;
//		return;
//	}
//	reader->SetFileName(fileName.c_str());
//	reader->Update();
//
//	int numComponents = reader->GetOutput()->GetNumberOfScalarComponents();
//	if (numComponents > 3)
//	{
//		std::cout << "Error: cannot process an image with "
//			<< numComponents << " components!" << std::endl;
//		//return EXIT_FAILURE;
//	}
//
//	// Create a vtkXYPlotActor
//	vtkSmartPointer<vtkXYPlotActor> plot =
//		vtkSmartPointer<vtkXYPlotActor>::New();
//	plot->ExchangeAxesOff();
//	plot->SetLabelFormat("%g");
//	plot->SetXTitle("Level");
//	plot->SetYTitle("Frequency");
//	plot->SetXValuesToValue();
//
//	double xmax = 0.;
//	double ymax = 0.;
//
//	double colors[3][3] = {
//		{ 1, 0, 0 },
//		{ 0, 1, 0 },
//		{ 0, 0, 1 } };
//
//	const char* labels[3] = {
//		"Red", "Green", "Blue" };
//
//	// Process the image, extracting and plotting a histogram for each
//	// component
//	for (int i = 0; i < numComponents; ++i)
//	{
//		vtkSmartPointer<vtkImageExtractComponents> extract =
//			vtkSmartPointer<vtkImageExtractComponents>::New();
//		extract->SetInputConnection(reader->GetOutputPort());
//		extract->SetComponents(i);
//		extract->Update();
//
//		double range[2];
//		extract->GetOutput()->GetScalarRange(range);
//
//		vtkSmartPointer<vtkImageAccumulate> histogram =
//			vtkSmartPointer<vtkImageAccumulate>::New();
//		histogram->SetInputConnection(extract->GetOutputPort());
//		histogram->SetComponentExtent(
//			0,
//			static_cast<int>(range[1]) - static_cast<int>(range[0]) - 1, 0, 0, 0, 0);
//		histogram->SetComponentOrigin(range[0], 0, 0);
//		histogram->SetComponentSpacing(1, 0, 0);
//		histogram->SetIgnoreZero(ignoreZero);
//		histogram->Update();
//
//		if (range[1] > xmax)
//		{
//			xmax = range[1];
//		}
//		if (histogram->GetOutput()->GetScalarRange()[1] > ymax)
//		{
//			ymax = histogram->GetOutput()->GetScalarRange()[1];
//		}
//
//#if VTK_MAJOR_VERSION <= 5
//		plot->AddInput(histogram->GetOutput());
//#else
//		plot->AddDataSetInputConnection(histogram->GetOutputPort());
//#endif
//		if (numComponents > 1)
//		{
//			plot->SetPlotColor(i, colors[i]);
//			plot->SetPlotLabel(i, labels[i]);
//			plot->LegendOn();
//		}
//	}
//
//	plot->SetXRange(0, xmax);
//	plot->SetYRange(0, ymax);
//
//	// Visualize the histogram(s)
//	vtkSmartPointer<vtkRenderer> renderer =
//		vtkSmartPointer<vtkRenderer>::New();
//	renderer->AddActor(plot);
//
//	vtkSmartPointer<vtkRenderWindow> renderWindow =
//		vtkSmartPointer<vtkRenderWindow>::New();
//	renderWindow->AddRenderer(renderer);
//	renderWindow->SetSize(640, 480);
//
//	vtkSmartPointer<vtkRenderWindowInteractor> interactor =
//		vtkSmartPointer<vtkRenderWindowInteractor>::New();
//	interactor->SetRenderWindow(renderWindow);
//
//	// Initialize the event loop and then start it
//	interactor->Initialize();
//	interactor->Start();
//
//}


int main(int argc, char **argv)
{


	// Create a vtkXYPlotActor
	vtkSmartPointer<vtkXYPlotActor> plot = vtkSmartPointer<vtkXYPlotActor>::New();
	plot->ExchangeAxesOff();

	plot->SetLabelFormat("%g");
	plot->SetXTitle("Level");
	plot->SetYTitle("Frequency");
	plot->SetXValuesToValue();


	vtkSmartPointer< vtkImageData > plotData = vtkSmartPointer< vtkImageData >::New();

	//plotData->Set


	//vtkSmartPointer< vtkDataArray > plotArray = vtkSmartPointer< vtkDataArray >::New();

	//plotArray->SetNumberOfComponents(1);

	//plotArray->InsertNextTuple1(0);
	//plotArray->InsertNextTuple1(1);
	//plotArray->InsertNextTuple1(2);
	//plotArray->InsertNextTuple1(3);
	//plotArray->InsertNextTuple1(3);
	//plotArray->InsertNextTuple1(2);
	//plotArray->InsertNextTuple1(1);
	//plotArray->InsertNextTuple1(0);

	vtkSmartPointer< vtkFieldData > plotField = vtkSmartPointer< vtkFieldData >::New();

	plotData->SetDimensions(8, 1, 1);

	plotData->AllocateScalars(VTK_UNSIGNED_SHORT, 1);

	unsigned short *plotArray = (unsigned short *)plotData->GetScalarPointer();

	plotArray[0] = 0;
	plotArray[1] = 1;
	plotArray[2] = 2;
	plotArray[3] = 3;

	plotArray[4] = 3;
	plotArray[5] = 2;
	plotArray[6] = 1;
	plotArray[7] = 0;

	plot->AddDataSetInput(plotData);

	// Visualize the histogram(s)
	vtkSmartPointer<vtkRenderer> renderer =
		vtkSmartPointer<vtkRenderer>::New();
	renderer->AddActor(plot);

	vtkSmartPointer<vtkRenderWindow> renderWindow =
		vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);
	renderWindow->SetSize(640, 480);

	vtkSmartPointer<vtkRenderWindowInteractor> interactor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	interactor->SetRenderWindow(renderWindow);

	// Initialize the event loop and then start it
	interactor->Initialize();
	interactor->Start();


	return 0;

}



#if 0
void writeBottleHeader();

int main( int argc , char **argv )
{


	std::string headerPath = "C:/projects/Wallthickness/data/bottle.dat";
	std::string dataPath = "C:/projects/Wallthickness/data/bottle.raw";

	//writeBottleHeader();

	//return 0;

	plotHistogram();

	imt::volume::RawVolumeDataIO io;

	imt::volume::VolumeInfo volInfo;

	io.readHeader(headerPath, volInfo);

	io.readData(dataPath, volInfo.mVolumeData);

	std::cout << volInfo.mWidth << " " << volInfo.mHeight << " " << volInfo.mHeight << std::endl;

	imt::volume::HistogramFilter filter( &volInfo );

	filter.apply();
	
	
	return 0;
}


void writeBottleHeader()
{
	std::string bottleHeaderPath = "C:/projects/Wallthickness/data/bottle.dat";

	imt::volume::VolumeInfo volInfo;

	volInfo.mWidth = 455;
	volInfo.mHeight = 485;
	volInfo.mDepth = 942;

	volInfo.mVoxelStep = Eigen::Vector3f(0.1650498, 0.1650498, 0.1650497);
	volInfo.mVolumeOrigin = Eigen::Vector3f(0, 0, 0);


	imt::volume::RawVolumeDataIO io;

	io.writeHeader(bottleHeaderPath, volInfo);

}

#endif