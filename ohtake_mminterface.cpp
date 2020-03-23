//Standard C++
#include<iostream>
#include<fstream>
#include "limits.h"
#include<ctime>
#include<math.h>
#include<queue>
#include<list>
#include<omp.h>

//ITK
#include "itkImage.h"
#include "itkImageLinearIteratorWithIndex.h"
#include "itkTestingExtractSliceImageFilter.h"
#include "itkImageFileWriter.h"
#include "itkPNGImageIO.h"
#include "itkGradientImageFilter.h"
#include "itkCovariantVector.h"
#include "itkVectorLinearInterpolateImageFunction.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkBinaryReconstructionByDilationImageFilter.h"
#include "itkCannyEdgeDetectionImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkNeighborhoodIterator.h"
#include "itkDerivativeOperator.h"
#include "itkMatrix.h"
#include "itkNeighborhoodOperatorImageFunction.h"
#include "itkVectorNeighborhoodInnerProduct.h"
#include "itkVectorNeighborhoodOperatorImageFilter.h"
#include <itkVectorMagnitudeImageFilter.h>

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
#include <vtkMarchingCubes.h>
//OpenCV
#include "opencvincludes.h"

#include "GCoptimization.h"
#include "display3droutines.h"
#include "src\plugins\Application\ZeissViewer\SegmentationInterface\ZeissSegmentationInterface.hpp"
#include "poissonmeshgenerator.cpp"

#include "volumeutility.h"
#include "vtkImageData.h"

using namespace std;

typedef unsigned short InternalPixelType;
typedef unsigned char BinaryImagePixelType;
typedef float FloatImagePixelType;
typedef unsigned int IntImagePixelType;
const int dimensions = 3;
typedef itk::Image<IntImagePixelType, dimensions> IntImageType;
typedef itk::Image<InternalPixelType, dimensions> ImageType;
typedef itk::Image<BinaryImagePixelType, dimensions> BinaryImageType;
typedef itk::Image<InternalPixelType, 2> ImageType2D;
typedef itk::Image<BinaryImagePixelType, 2> BinaryImageType2D;
typedef itk::Image<FloatImagePixelType, dimensions> FloatImageType;
typedef itk::CovariantVector<FloatImagePixelType, dimensions> GradientCovariantType;
typedef itk::Image<GradientCovariantType, dimensions> GradientOutputImageType;
typedef itk::ContinuousIndex<double, 3> ContinuousIndexType;
typedef std::vector<ContinuousIndexType> ContinuousIndexListType;
typedef itk::ImageRegionIteratorWithIndex<FloatImageType> FloatRegionIteratorType;
typedef itk::ImageRegionIteratorWithIndex<GradientOutputImageType> GradientRegionIteratorType;

time_t tstart, tend;

//Function to print Slices
void saveSlices(const BinaryImageType::Pointer BinaryIntermediate, int* volumeSize, std::string name)
{
	typedef itk::Testing::ExtractSliceImageFilter<BinaryImageType, BinaryImageType2D> ExtractSliceFilterType;
	ExtractSliceFilterType::Pointer ExtractSliceFilter = ExtractSliceFilterType::New();
	typedef itk::ImageFileWriter<BinaryImageType2D> BinaryFileWriterType;
	BinaryFileWriterType::Pointer TestWriter = BinaryFileWriterType::New();
	BinaryImageType::IndexType SliceStart;
	BinaryImageType::SizeType SliceSize;
	BinaryImageType::RegionType SliceRegion;
	ExtractSliceFilter->SetInput(BinaryIntermediate);
	ExtractSliceFilter->SetDirectionCollapseToIdentity();
	TestWriter->SetImageIO(itk::PNGImageIO::New());
	std::ostringstream out;
	int i;
	for (i = 0;i<volumeSize[0];i++)
	{
		out.str(""); out.clear();
		out << name << i << ".png";
		SliceStart[0] = i; SliceStart[1] = 0; SliceStart[2] = 0;
		SliceSize[0] = 0; SliceSize[1] = volumeSize[1]; SliceSize[2] = volumeSize[2];
		SliceRegion.SetIndex(SliceStart);
		SliceRegion.SetSize(SliceSize);
		ExtractSliceFilter->SetExtractionRegion(SliceRegion);
		TestWriter->SetInput(ExtractSliceFilter->GetOutput());
		TestWriter->SetFileName(out.str());
		try
		{
			TestWriter->Update();

		}
		catch (itk::ExceptionObject &err)
		{
			cout << "Error Occured During Writing Slices" << endl;
			cout << err;
			return;
		}
	}
	return;
}

void displaySubVoxelPoints(ContinuousIndexListType SubVoxelList)
{
	tr::Display3DRoutines displayPointsRoutine;
	vector<Eigen::Vector3f> Points, Colors;

	

	for (ContinuousIndexListType::iterator it = SubVoxelList.begin(); it != SubVoxelList.end(); ++it)
	{
		ContinuousIndexType pos = *it;
		Eigen::Vector3f point, color;
		point[0] = pos[0]; point[1] = pos[1]; point[2] = pos[2];
		color[0] = 1; color[1] = 0; color[2] = 0;
		Points.push_back(point);
		Colors.push_back(color);
	}
	displayPointsRoutine.displayPointSet(Points, Colors);

}

void displayNormals(ContinuousIndexListType SubVoxelList, std::vector<GradientCovariantType> NormalList, float scale = 1)
{
	tr::Display3DRoutines displayNormalRoutine;
	std::vector<Eigen::Vector3f> vertices, normals;
	ContinuousIndexListType::iterator VIt;
	std::vector<GradientCovariantType>::iterator NIt;
	for (VIt = SubVoxelList.begin(), NIt = NormalList.begin(); (VIt != SubVoxelList.end()) && (NIt != NormalList.end()); ++VIt, ++NIt)
	{
		Eigen::Vector3f vertex, normal;
		ContinuousIndexType index = *VIt;
		GradientCovariantType UnitVector = *NIt;
		vertex[0] = index[0]; vertex[1] = index[1]; vertex[2] = index[2];
		normal[0] = UnitVector[0]; normal[1] = UnitVector[1]; normal[2] = UnitVector[2];
		vertices.push_back(vertex);
		normals.push_back(normal);
	}
	displayNormalRoutine.displayNormals(vertices, normals, scale);
}
void displayOffsetPoints(ContinuousIndexListType SubVoxelList, std::vector<std::vector<ContinuousIndexType>> OffsetPointsList)
{
	tr::Display3DRoutines displayOffsetPointsRoutine;
	std::vector<Eigen::Vector3f> vertices, colors;
	ContinuousIndexListType::iterator VIT;
	std::vector<std::vector<ContinuousIndexType>>::iterator OIT;
	for (VIT = SubVoxelList.begin(), OIT = OffsetPointsList.begin(); (VIT != SubVoxelList.end()) && (OIT != OffsetPointsList.end()); ++VIT, ++OIT)
	{
		Eigen::Vector3f vertex, OffsetP, OffsetM, Vcolor, Pcolor, Mcolor;
		ContinuousIndexType VertexIndex, PIndex, MIndex;
		VertexIndex = *VIT;
		PIndex = (*OIT)[0]; MIndex = (*OIT)[1];
		vertex[0] = VertexIndex[0]; vertex[1] = VertexIndex[1]; vertex[2] = VertexIndex[2];
		OffsetP[0] = PIndex[0]; OffsetP[1] = PIndex[1]; OffsetP[2] = PIndex[2];
		OffsetM[0] = MIndex[0]; OffsetM[1] = MIndex[1]; OffsetM[2] = MIndex[2];
		Vcolor = { 1, 1, 1 };
		Pcolor = { 0, 1, 0 };
		Mcolor = { 1, 0, 0 };
		vertices.push_back(vertex);
		vertices.push_back(OffsetP);
		vertices.push_back(OffsetM);
		colors.push_back(Vcolor);
		colors.push_back(Pcolor);
		colors.push_back(Mcolor);
	}
	displayOffsetPointsRoutine.displayPointSet(vertices, colors);
}

InternalPixelType* calculateDeviation(Materials M, unsigned long long* histogram, int* volumeSize)
{
	InternalPixelType* deviation = new InternalPixelType[M.regions.size()];
	for (unsigned int i = 0; i < M.regions.size(); i++)
	{
		InternalPixelType LowerBound, peak, UpperBound;
		LowerBound = M.regions[i].lower_bound;
		UpperBound = M.regions[i].upper_bound;
		peak = M.regions[i].peak;
		double materialDeviation = 0;
		double voxelCount = 0;
		for (unsigned int j = LowerBound; j <= UpperBound; j++)
		{
			voxelCount += histogram[j];
			materialDeviation += pow((j - 1.0*peak), 2) * histogram[j];
		}
		deviation[i] = ceil(sqrt(materialDeviation/voxelCount));
	}
	return deviation;
}


void viewIsoSurface(imt::volume::VolumeInfo& volume, int isoVal);


int main( int argc, char** argv)
{
	//if (argc != 2)
	//{
	//	cout << "Wrong Usage!!" << endl;
	//	cout << "Usage: HelloWorld.exe <UNIT16_SCV_FILE_NAME>" << endl;
	//	return EXIT_FAILURE;
	//}
	//ITK use constants and TypeDefs
	unsigned int numThreads = cv::getNumberOfCPUs();
	ImageType::Pointer volume = ImageType::New(); //Acquire Smart Pointer to volume
	unsigned short *data;
	unsigned char* header;
	int volumeSize[3];
	double resolution[3];
	char *temp = new char(2);
	unsigned long count;
	unsigned int headerSize;
	tstart = time(0);

	std::string filePath = "C:\\Projects\\Wallthickness\\data\\Datasets for Multi-material\\MultiMaterial\\MAR_Ref_wBHC_wMAR-Artefaktred-0,574 2012-9-14 12-58.uint16_scv";

	ifstream uint16(filePath, ios::binary);
	if (uint16.is_open())
	{
		uint16.read(reinterpret_cast<char *>(&headerSize), sizeof(headerSize));
		header = new unsigned char[headerSize - sizeof(headerSize)];
		uint16.read(reinterpret_cast<char *>(header), headerSize - sizeof(headerSize));

		//std::cout << " size of header : "

		memcpy(volumeSize, header + 8, 12);
		memcpy(resolution, header + 20, 24);
		std::cout << "Size of the volume is:" << volumeSize[0] << "x" << volumeSize[1] << "x" << volumeSize[2] << endl;
		std::cout << "Resolution of the volume is:" << resolution[0] << " " << resolution[1] << " " << resolution[2] << endl;
 		delete[] header;
		ImageType::IndexType start;
		start[0] = 0;start[1] = 0;start[2] = 0;
		ImageType::SizeType size;
		size[0] = volumeSize[0];size[1] = volumeSize[1];size[2] = volumeSize[2];
		ImageType::RegionType region;
		region.SetIndex(start);
		region.SetSize(size);
		volume->SetRegions(region);
		volume->Allocate();
		data = new unsigned short[(unsigned)(volumeSize[0] * volumeSize[1] * volumeSize[2])];
		typedef itk::ImageLinearIteratorWithIndex<ImageType> IteratorType;
		IteratorType volumeIt(volume, volume->GetLargestPossibleRegion());
		volumeIt.SetDirection(0);
		count = 0;
		for (volumeIt.GoToBegin();!volumeIt.IsAtEnd();volumeIt.NextLine())
		{
			volumeIt.GoToBeginOfLine();
			while (!volumeIt.IsAtEndOfLine())
			{
				uint16.read(temp, 2);
				volumeIt.Set((InternalPixelType)(((temp[1] << 8) & 0xFF00) | (temp[0] & 0x00FF)));
				data[count] = volumeIt.Get();
				++volumeIt;
				count++;
			}
		}
		if (count != (unsigned)(volumeSize[0] * volumeSize[1] * volumeSize[2]))
		{
			std::cerr << "Not enough bytes read" << endl;
			return EXIT_FAILURE;
		}
	}
	else
	{
		std::cerr << "File couldn't be opened" << endl;
		return EXIT_FAILURE;
	}
	tend = time(0);
	cout << "Read the File, Time Elapsed= " << difftime(tend, tstart) << endl;

	//Getting Material Information
	Volume	v;
	v.data = data;
	v.size[0] = volumeSize[0]; v.size[1] = volumeSize[1]; v.size[2] = volumeSize[2];
	v.voxel_size[0] = resolution[0]; v.voxel_size[1] = resolution[1]; v.voxel_size[2] = resolution[2];
	ZeissSegmentationInterface ZeissSegmentation;
	ZeissSegmentation.setInputVolume(v);
	Materials M = ZeissSegmentation.getMaterialRegions();
	const IntImagePixelType num_labels = M.regions.size();
	InternalPixelType *peaks = new InternalPixelType[num_labels];
	InternalPixelType *deviation;
	InternalPixelType *UpperBounds, *LowerBounds;
	UpperBounds = new InternalPixelType[num_labels];
	LowerBounds = new InternalPixelType[num_labels];
	FloatImagePixelType d = 4;
	MultiResolutionHistograms *histograms = new MultiResolutionHistograms();
	//ZeissSegmentation.getMultiResolutionHistograms(histograms);
	//deviation = calculateDeviation(M, histograms->first_histogram_data,volumeSize);

	//imt::volume::VolumeInfo vol;

	

	deviation = new InternalPixelType[num_labels];
	double deviationPercentage = 0.8;
	for (int i = 0; i < num_labels; i++)
	{
		peaks[i] = M.regions[i].peak;
		UpperBounds[i] = M.regions[i].upper_bound;
		LowerBounds[i] = M.regions[i].lower_bound;
		deviation[i] = deviationPercentage *(((peaks[i] - LowerBounds[i]) >(UpperBounds[i] - peaks[i])) ? (UpperBounds[i] - peaks[i]) : (peaks[i] - LowerBounds[i]));
	}
	InternalPixelType UpperThreshold = itk::NumericTraits<InternalPixelType>::max(), LowerThreshold;
	for (int i = 0; i < num_labels - 1; i++)
		if ((peaks[i + 1] - peaks[i]) / (2 * d) < UpperThreshold)
			UpperThreshold = (peaks[i + 1] - peaks[i]) / (2 * d);

	cout << "Number of materials:" << num_labels << endl;
	cout << "Upper Threshold:" << UpperThreshold << endl;
	LowerThreshold = UpperThreshold / 20;
	delete[] data;

	

	//Calculating Gradients
	GradientOutputImageType::Pointer volumeGradient;
	{
		typedef itk::DiscreteGaussianImageFilter<ImageType, ImageType> volumeGaussianFilterType;
		volumeGaussianFilterType::Pointer volumeGaussianFilter = volumeGaussianFilterType::New();
		volumeGaussianFilter->SetVariance(1);
		volumeGaussianFilter->SetInput(volume); 
		typedef itk::GradientImageFilter<ImageType, FloatImagePixelType, FloatImagePixelType, GradientOutputImageType> GradientImageFilterType;
		GradientImageFilterType::Pointer GradientImageFilter = GradientImageFilterType::New();
		GradientImageFilter->SetInput(volumeGaussianFilter->GetOutput());
		try
		{
			tstart = time(0);
			GradientImageFilter->Update();
			tend = time(0);
			cout << "Done calculating Gradient Vectors, time elapsed = " << difftime(tend, tstart) << endl;
			volumeGradient = GradientImageFilter->GetOutput();
			volume = volumeGaussianFilter->GetOutput(); 
		}
		catch (itk::ExceptionObject &err)
		{
			cout << "ITK error occured during Gradient Calculation" << endl;
			cout << err;
		}
		catch (std::exception err)
		{
			cout << "STD error occured during Gradeint Calculation" << endl;
			cout << err.what();
		}
	}

	//A Float Image to store Gradient Magnitude of local Maximum points
	FloatImageType::Pointer GradientMaximum = FloatImageType::New();
	{
		FloatImageType::IndexType start;
		start.Fill(0);
		FloatImageType::SizeType size = volume->GetLargestPossibleRegion().GetSize();
		FloatImageType::RegionType region;
		region.SetIndex(start);
		region.SetSize(size);
		GradientMaximum->SetRegions(region);
		GradientMaximum->Allocate();
	}
	FloatImageType::Pointer GradientMag = FloatImageType::New();
	{
		typedef itk::VectorMagnitudeImageFilter<GradientOutputImageType, FloatImageType> GradientMagImageFilterType;
		GradientMagImageFilterType::Pointer GradientMagImageFilter = GradientMagImageFilterType::New();
		GradientMagImageFilter->SetInput(volumeGradient);
		GradientMagImageFilter->Update();
		GradientMag = GradientMagImageFilter->GetOutput();
	}
	//Trilinear Interpolator for Gradient Vectors
	typedef itk::VectorLinearInterpolateImageFunction<GradientOutputImageType> GradientLinearInterpolatorType;
	GradientLinearInterpolatorType::Pointer GradientLinearInterpolator = GradientLinearInterpolatorType::New();
	GradientLinearInterpolator->SetInputImage(volumeGradient); 
	typedef itk::ImageLinearIteratorWithIndex<FloatImageType> FloatIteratorType;
	typedef itk::ImageLinearIteratorWithIndex<GradientOutputImageType> GradientVectorIteratorType;
	//Non maximal supression
	GradientVectorIteratorType GradientVectorIT(volumeGradient, volumeGradient->GetLargestPossibleRegion());
	GradientVectorIT.SetDirection(0);
	FloatIteratorType GradientIT(GradientMaximum, GradientMaximum->GetLargestPossibleRegion());
	GradientIT.SetDirection(0);
	tstart = time(0);
	for (GradientVectorIT.GoToBegin(), GradientIT.GoToBegin(); !(GradientVectorIT.IsAtEnd() || GradientIT.IsAtEnd()); GradientVectorIT.NextLine(), GradientIT.NextLine())
	{
		GradientVectorIT.GoToBeginOfLine();
		GradientIT.GoToBeginOfLine();
		while (!(GradientVectorIT.IsAtEndOfLine() || GradientIT.IsAtEndOfLine()))
		{
			if (GradientVectorIT.Get().GetNorm() < LowerThreshold)
			{
				GradientIT.Set(0);
				++GradientVectorIT;
				++GradientIT;
				continue;
			}
			GradientOutputImageType::ValueType UnitVector, CenterPos, UplusPos, UminusPos;
			UnitVector = GradientVectorIT.Get();
			UnitVector.Normalize();
			GradientCovariantType::ValueType max = (abs(UnitVector[0]) > abs(UnitVector[1])) ? abs(UnitVector[0]) : abs(UnitVector[1]);
			max = (abs(UnitVector[2]) > max) ? abs(UnitVector[2]) : max;
			GradientCovariantType::ValueType t = 1 / max;
			FloatImageType::IndexType pos = GradientIT.GetIndex();
			CenterPos[0] = pos[0]; CenterPos[1] = pos[1]; CenterPos[2] = pos[2];
			UplusPos = CenterPos + UnitVector*t;
			UminusPos = CenterPos - UnitVector*t;
			GradientLinearInterpolatorType::ContinuousIndexType Uplusindex, Uminusindex;
			Uplusindex[0] = UplusPos[0]; Uplusindex[1] = UplusPos[1]; Uplusindex[2] = UplusPos[2];
			Uminusindex[0] = UminusPos[0]; Uminusindex[1] = UminusPos[1]; Uminusindex[2] = UminusPos[2];
			bool UminusInside, UplusInside;
			UminusInside = ((Uminusindex[0] >= 0) && (Uminusindex[0] <= volumeSize[0] - 1)) && ((Uminusindex[1] >= 0) && (Uminusindex[1] <= volumeSize[1] - 1)) && ((Uminusindex[2] >= 0) && (Uminusindex[2] <= volumeSize[2] - 1));
			UplusInside = ((Uplusindex[0] >= 0) && (Uplusindex[0] <= volumeSize[0] - 1)) && ((Uplusindex[1] >= 0) && (Uplusindex[1] <= volumeSize[1] - 1)) && ((Uplusindex[2] >= 0) && (Uplusindex[2] <= volumeSize[2] - 1));
			try
			{
				if (UminusInside && UplusInside && (GradientVectorIT.Get().GetNorm() > GradientLinearInterpolator->EvaluateAtContinuousIndex(Uminusindex).GetNorm())
					&& (GradientVectorIT.Get().GetNorm() > GradientLinearInterpolator->EvaluateAtContinuousIndex(Uplusindex).GetNorm()))
				{
					GradientIT.Set(GradientVectorIT.Get().GetNorm());
				}
				else
				{
					GradientIT.Set(0);
				}
			}
			catch (itk::ExceptionObject &err)
			{
				cout << "ITK Error:" << endl;
				cout << "Error at Center Index:" << pos << ",Uminus:" << Uminusindex << ",Uplus:" << Uplusindex << endl;
				cout << err;
			}
			catch (std::exception err)
			{
				cout << "STD Error" << endl;
				cout << "Error at Center Index:" << pos << ",Uminus:" << Uminusindex << ",Uplus:" << Uplusindex << endl;
				cout << err.what();
			}
			++GradientVectorIT;
			++GradientIT;
		}
	}
	tend = time(0);
	cout << "Done Non Maxima Supression, Time Elapsed : " << difftime(tend, tstart) << endl;
	tstart = time(0);
	BinaryImageType::Pointer CannyEdge = BinaryImageType::New();
	{
		BinaryImageType::IndexType start;
		BinaryImageType::RegionType::SizeType size;
		BinaryImageType::RegionType region;
		start.Fill(0);
		size = volume->GetLargestPossibleRegion().GetSize();
		region.SetIndex(start);
		region.SetSize(size);
		CannyEdge->SetRegions(region);
		CannyEdge->Allocate();
		CannyEdge->FillBuffer(itk::NumericTraits<BinaryImageType::PixelType>::Zero);
	}
	IntImageType::Pointer IndexImage = IntImageType::New();
	{
		IntImageType::IndexType start;
		IntImageType::RegionType::SizeType size;
		IntImageType::RegionType region;
		start.Fill(0);
		size = volume->GetLargestPossibleRegion().GetSize();
		region.SetIndex(start);
		region.SetSize(size);
		IndexImage->SetRegions(region);
		IndexImage->Allocate();
		IndexImage->FillBuffer(itk::NumericTraits<BinaryImageType::PixelType>::Zero);

	}
	//Hysteris Thresholding
	tstart = time(0);
	typedef vector<BinaryImageType::IndexType> BinaryIndexListType;
	BinaryIndexListType CannyList;
	typedef queue<BinaryImageType::IndexType> BinaryIndexQueueType;
	BinaryIndexQueueType indexQueue;
	typedef itk::ImageLinearIteratorWithIndex<BinaryImageType> BinaryIteratorType;
	BinaryIteratorType CannyEdgeIT(CannyEdge, CannyEdge->GetLargestPossibleRegion());
	typedef itk::ImageLinearIteratorWithIndex<IntImageType> IntIteratorType;
	IntIteratorType IndexIT(IndexImage, IndexImage->GetLargestPossibleRegion());
	CannyEdgeIT.SetDirection(0);
	IndexIT.SetDirection(0);
	typedef std::vector<list<IntImagePixelType>> CannyNeighListType;
	CannyNeighListType EdgePointNeigh;
	IntImagePixelType EdgePointCount = 0;
	for (GradientIT.GoToBegin(), IndexIT.GoToBegin(), CannyEdgeIT.GoToBegin(); !(GradientIT.IsAtEnd() || CannyEdgeIT.IsAtEnd() || IndexIT.IsAtEnd());
		GradientIT.NextLine(), CannyEdgeIT.NextLine(), IndexIT.NextLine())
	{
		GradientIT.GoToBeginOfLine();
		CannyEdgeIT.GoToBeginOfLine();
		IndexIT.GoToBeginOfLine();
		while (!(GradientIT.IsAtEndOfLine() || CannyEdgeIT.IsAtEndOfLine() || IndexIT.IsAtEndOfLine()))
		{
			if (GradientIT.Get() >= UpperThreshold)
			{
				CannyEdgeIT.Set(255);
				IndexIT.Set(EdgePointCount);
				indexQueue.push(CannyEdgeIT.GetIndex());
				CannyList.push_back(CannyEdgeIT.GetIndex());
				list<IntImagePixelType> CurrentNeighList;
				EdgePointNeigh.push_back(CurrentNeighList);
				EdgePointCount++;
			}
			else
				CannyEdgeIT.Set(0);
			++CannyEdgeIT;
			++GradientIT;
			++IndexIT;
		}
	}
	typedef itk::NeighborhoodIterator<IntImageType> IntNeighborhoodIteratorType;
	IntNeighborhoodIteratorType::RadiusType Iradius; Iradius.Fill(1);
	IntNeighborhoodIteratorType IndexNeighIT(Iradius, IndexImage, IndexImage->GetLargestPossibleRegion());
	typedef itk::NeighborhoodIterator<BinaryImageType> BinaryNeighborhoodIteratorType;
	BinaryNeighborhoodIteratorType::RadiusType Bradius; Bradius.Fill(1);
	BinaryNeighborhoodIteratorType CannyEdgeNeighIT(Bradius, CannyEdge, CannyEdge->GetLargestPossibleRegion());
	typedef itk::NeighborhoodIterator<FloatImageType> FloatNeighborhoodIteratorType;
	FloatNeighborhoodIteratorType::RadiusType Fradius; Fradius.Fill(1);
	FloatNeighborhoodIteratorType GradientNeighIT(Fradius, GradientMaximum, GradientMaximum->GetLargestPossibleRegion());
	IntImagePixelType queueCount = 0;
	try
	{
		while (!indexQueue.empty())
		{
			BinaryImageType::IndexType index = indexQueue.front();
			indexQueue.pop();
			CannyEdgeNeighIT.SetLocation(index);
			GradientNeighIT.SetLocation(index);
			IndexNeighIT.SetLocation(index);
			for (unsigned int i = 0; i < CannyEdgeNeighIT.Size(); i++)
			{
				if (!(CannyEdgeNeighIT.IndexInBounds(i) && GradientNeighIT.IndexInBounds(i) && IndexNeighIT.IndexInBounds(i)))
				{
					continue;
				}
				if ((CannyEdgeNeighIT.GetPixel(i) != 0) && (IndexNeighIT.GetCenterPixel() < IndexNeighIT.GetPixel(i)))
				{
					EdgePointNeigh[queueCount].push_back(IndexNeighIT.GetPixel(i));
				}
				else if ((CannyEdgeNeighIT.GetPixel(i) == 0) && (GradientNeighIT.GetPixel(i) >= LowerThreshold) && (GradientNeighIT.GetPixel(i) < UpperThreshold))
				{

					CannyEdgeNeighIT.SetPixel(i, 255);
					IndexNeighIT.SetPixel(i, EdgePointCount);
					indexQueue.push(CannyEdgeNeighIT.GetIndex());
					CannyList.push_back(CannyEdgeNeighIT.GetIndex());
					EdgePointNeigh[queueCount].push_back(EdgePointCount);
					std::list<IntImagePixelType> CurrentNeighList;
					EdgePointNeigh.push_back(CurrentNeighList);
					EdgePointCount++;
				}
			}
			queueCount++;
		}
	}
	catch (itk::ExceptionObject &err)
	{
		cout << err;
		return EXIT_SUCCESS;
	}
	catch (exception e)
	{
		cout << e.what();
	}
	tend = time(0);
	cout << "Done Hysterisis Theholding, Time Elapsed = " << difftime(tend, tstart) << endl;
	CannyEdge->ReleaseData();
	CannyEdge = NULL;
	IndexImage->ReleaseData();
	IndexImage = NULL;
	cout << EdgePointCount << endl;
	//Getting Edge Points with SubVoxel Precision
	tstart = time(0);
	ContinuousIndexListType SubVoxelList(CannyList.size());
	for (long long int k = 0; k < CannyList.size(); k++)
	{
		BinaryImageType::IndexType pos = CannyList[k];
		GradientCovariantType CenterPos, UminusPos, UplusPos, UnitVector;
		GradientVectorIT.SetIndex(pos);
		UnitVector = GradientVectorIT.Get();
		UnitVector.Normalize();
		CenterPos[0] = pos[0]; CenterPos[1] = pos[1]; CenterPos[2] = pos[2];
		GradientCovariantType::ValueType max = (abs(UnitVector[0]) > abs(UnitVector[1])) ? abs(UnitVector[0]) : abs(UnitVector[1]);
		max = (max > abs(UnitVector[2])) ? max : abs(UnitVector[2]);
		GradientCovariantType::ValueType t = 1 / max;
		UplusPos = CenterPos + UnitVector*t;
		UminusPos = CenterPos - UnitVector*t;
		GradientLinearInterpolatorType::ContinuousIndexType UplusIndex, UminusIndex;
		UplusIndex[0] = UplusPos[0]; UplusIndex[1] = UplusPos[1]; UplusIndex[2] = UplusPos[2];
		UminusIndex[0] = UminusPos[0]; UminusIndex[1] = UminusPos[1]; UminusIndex[2] = UminusPos[2];
		GradientCovariantType SubVoxelPos;
		SubVoxelPos = CenterPos + UnitVector * (GradientLinearInterpolator->EvaluateAtContinuousIndex(UplusIndex).GetNorm() -
			GradientLinearInterpolator->EvaluateAtContinuousIndex(UminusIndex).GetNorm()) * (UplusPos - UminusPos).GetNorm() / (4 * (
			GradientLinearInterpolator->EvaluateAtContinuousIndex(UplusIndex).GetNorm() +
			GradientLinearInterpolator->EvaluateAtContinuousIndex(UminusIndex).GetNorm() - 2 * GradientVectorIT.Get().GetNorm()));
		ContinuousIndexType SubVoxelIndex;
		SubVoxelIndex[0] = SubVoxelPos[0]; SubVoxelIndex[1] = SubVoxelPos[1]; SubVoxelIndex[2] = SubVoxelPos[2];
		SubVoxelList[k]= SubVoxelIndex;
	}
	tend = time(0);
	cout << "Done SubVoxel Position Calculation, Time Elapsed : " << difftime(tend, tstart) << endl;
	//Finding Normals
	tstart = time(0);
	typedef std::vector<GradientCovariantType> NormalListType;
	NormalListType NormalList(SubVoxelList.size());
	count = 0;
	#pragma omp parallel for
	for (long long int k = 0; k < SubVoxelList.size(); k++)
	{
		ContinuousIndexType SubVoxelIndex = SubVoxelList[k];
		GradientCovariantType UnitVector;
		typedef itk::DerivativeOperator<FloatImagePixelType, dimensions> DerivativeOperatorType;
		typedef itk::NeighborhoodOperatorImageFunction<FloatImageType, FloatImagePixelType> FloatNeighOperatorFunctionType;
		FloatImageType::IndexType Firststart,Secondstart,Thirdstart;
		FloatImageType::RegionType::SizeType Firstsize,Secondsize,Thirdsize;
		FloatImageType::RegionType Firstregion,Secondregion,Thirdregion;
		for (int i = 0; i < dimensions; i++)
		{
			Firststart[i] = ((floor(SubVoxelIndex[i])-2) < 0) ? 0 : (floor(SubVoxelIndex[i])-2);
			Firstsize[i] = ((ceil(SubVoxelIndex[i])+2) > (volumeSize[i] - 1)) ? (volumeSize[i]-1) : (ceil(SubVoxelIndex[i])+2) - Firststart[i] + 1;
			Secondstart[i] = ((floor(SubVoxelIndex[i]) - 1) < 0) ? 0 : (floor(SubVoxelIndex[i]) - 1);
			Secondsize[i] = (((ceil(SubVoxelIndex[i]) + 1) > (volumeSize[i] - 1)) ? (volumeSize[i] - 1) : (ceil(SubVoxelIndex[i]) + 1)) - Secondstart[i] +1;
			Thirdstart[i] = (floor(SubVoxelIndex[i]) < 0) ? 0 : floor(SubVoxelIndex[i]);
			Thirdsize[i] = ((ceil(SubVoxelIndex[i]) > (volumeSize[i] - 1)) ? (volumeSize[i] - 1) : ceil(SubVoxelIndex[i])) - Thirdstart[i] + 1;
		}
		Firstregion.SetIndex(Firststart);
		Firstregion.SetSize(Firstsize);
		Secondregion.SetIndex(Secondstart);
		Secondregion.SetSize(Secondsize);
		Thirdregion.SetSize(Thirdsize);
		Thirdregion.SetIndex(Thirdstart);
		FloatImageType::Pointer LevelSetFunction = FloatImageType::New();
		LevelSetFunction->SetRegions(Secondregion);
		LevelSetFunction->Allocate();
		LevelSetFunction->FillBuffer(0);
		typedef itk::ImageRegionIteratorWithIndex<FloatImageType> FloatImageRegionIteratorType;
		FloatImageRegionIteratorType LevelSetIT(LevelSetFunction, Secondregion);
		typedef itk::NeighborhoodIterator<GradientOutputImageType> CovariantNeighIteratorType;
		CovariantNeighIteratorType::RadiusType NeighRadius; NeighRadius.Fill(1);
		CovariantNeighIteratorType GradiantVectorNeighIT(NeighRadius, volumeGradient, Secondregion);
		typedef itk::VectorNeighborhoodInnerProduct<GradientOutputImageType> VectorInnerProductType;
		VectorInnerProductType IP;
		typedef itk::DerivativeOperator<FloatImagePixelType,3> VectorDerivativeType;
		std::vector<VectorDerivativeType> VectorDerivativeOperators;
		for (int i = 0; i < dimensions; i++)
		{
			VectorDerivativeType directionalDerivativeOP;
			directionalDerivativeOP.SetDirection(i);
			directionalDerivativeOP.SetOrder(1);
			VectorDerivativeType::RadiusType radius; radius.Fill(1);
			directionalDerivativeOP.CreateToRadius(radius);
			VectorDerivativeOperators.push_back(directionalDerivativeOP);
		}
		GradiantVectorNeighIT.GoToBegin();
		LevelSetIT.GoToBegin();
		while (!(GradiantVectorNeighIT.IsAtEnd() || LevelSetIT.IsAtEnd()))
		{
			GradientCovariantType DerivativeDirection, He;
			DerivativeDirection = GradiantVectorNeighIT.GetCenterPixel();
			DerivativeDirection.Normalize();
			for (int i = 0; i < dimensions; i++)
			{
				CovariantNeighIteratorType::PixelType Hrow = IP(GradiantVectorNeighIT,VectorDerivativeOperators[i]);
				He[i] = Hrow * DerivativeDirection;
			}
			LevelSetIT.Set(DerivativeDirection*He);
			++LevelSetIT;
			++GradiantVectorNeighIT;
		}
		GradientOutputImageType::Pointer LevelSetFunctionDerivative = GradientOutputImageType::New();
		LevelSetFunctionDerivative->SetRegions(Thirdregion);
		LevelSetFunctionDerivative->Allocate();
		typedef itk::ImageRegionIteratorWithIndex<GradientOutputImageType> GradientVectorRegionIteratorType;
		GradientVectorRegionIteratorType LevelSetDerivativeIT(LevelSetFunctionDerivative, Thirdregion);
		FloatNeighborhoodIteratorType LevelSetNeighIT(NeighRadius, LevelSetFunction, Thirdregion);
		typedef itk::NeighborhoodInnerProduct<FloatImageType> FloatNeighInnerProductType;
		FloatNeighInnerProductType LevelSetInnerProduct;
		for (int i = 0; i < dimensions; i++)
		{
			LevelSetDerivativeIT.GoToBegin();
			LevelSetNeighIT.GoToBegin();
			DerivativeOperatorType directionalDerivativeOperator; 
			directionalDerivativeOperator.SetDirection(i);
			directionalDerivativeOperator.SetOrder(1);
			DerivativeOperatorType::RadiusType radius;
			radius.Fill(1);
			directionalDerivativeOperator.CreateToRadius(radius);
			while (!(LevelSetDerivativeIT.IsAtEnd() || LevelSetNeighIT.IsAtEnd()))
			{
				GradientCovariantType temp;
				temp = LevelSetDerivativeIT.Get();
				temp[i] = LevelSetInnerProduct(LevelSetNeighIT, directionalDerivativeOperator);
				LevelSetDerivativeIT.Set(temp);
				++LevelSetDerivativeIT;
				++LevelSetNeighIT;
			}
		}
		GradientLinearInterpolatorType::Pointer LevelSetGradientInterpolator = GradientLinearInterpolatorType::New();
		LevelSetGradientInterpolator->SetInputImage(LevelSetFunctionDerivative);
		UnitVector = LevelSetGradientInterpolator->EvaluateAtContinuousIndex(SubVoxelIndex);
		UnitVector.Normalize();
		NormalList[k] = UnitVector;
		count++;
	}
	tend = time(0);
	cout << "Done Normal Calculation, Time Elapsed : " << difftime(tend,tstart) << endl;
	//Finding OffsetPoints
	ContinuousIndexListType::iterator PIT;
	NormalListType::iterator NIT;
	typedef std::vector<std::vector<ContinuousIndexType>> OffsetPointsListType;
	OffsetPointsListType OffsetPointsList;
	{
		tstart = time(0);
		typedef itk::GradientImageFilter<FloatImageType, FloatImagePixelType, FloatImagePixelType, GradientOutputImageType> GradientMagDerivativeFilterType;
		GradientMagDerivativeFilterType::Pointer GradientMagDerivateFilter = GradientMagDerivativeFilterType::New();
		GradientMagDerivateFilter->SetInput(GradientMag);
		GradientMagDerivateFilter->Update();
		GradientOutputImageType::Pointer GradientMagDerivative = GradientMagDerivateFilter->GetOutput();
		GradientLinearInterpolatorType::Pointer GradientMagDerivativeInterpolator = GradientLinearInterpolatorType::New();
		GradientMagDerivativeInterpolator->SetInputImage(GradientMagDerivative);
		IntImagePixelType Icount = 0;
		for (PIT = SubVoxelList.begin(), NIT = NormalList.begin(); (PIT != SubVoxelList.end()) && (NIT != NormalList.end()); ++PIT, ++NIT)
		{
			ContinuousIndexType OffsetPlusIndex, OffsetMinusIndex, EdgePointIndex;
			GradientCovariantType normal, OffsetPlus, OffsetMinus, EdgePoint;
			normal = *NIT;
			EdgePointIndex = *PIT;
			EdgePoint[0] = EdgePointIndex[0]; EdgePoint[1] = EdgePointIndex[1]; EdgePoint[2] = EdgePointIndex[2];
			float dt = d/2;
			OffsetPlus = EdgePoint + dt*normal;
			OffsetMinus = EdgePoint - dt*normal;
			std::vector<GradientCovariantType> OffsetPoints = { OffsetPlus, OffsetMinus };
			std::vector<ContinuousIndexType> OffSetPointsIndex(2);
			for (int i = 0; i < OffsetPoints.size(); i++)
			{	
				GradientCovariantType PreviousDisplacement, Displacement;
				GradientCovariantType OffsetPoint = OffsetPoints[i];
				if (i == 0)
					PreviousDisplacement = -dt*normal;
				else
					PreviousDisplacement = dt*normal;
				ContinuousIndexType temp;
				count = 0;
				while (1)
				{
					count++;
					temp[0] = OffsetPoint[0]; temp[1] = OffsetPoint[1]; temp[2] = OffsetPoint[2];
					if (!GradientMagDerivative->GetLargestPossibleRegion().IsInside(temp))
					{
						OffsetPoint = OffsetPoint + PreviousDisplacement;
						temp[0] = OffsetPoint[0]; temp[1] = OffsetPoint[1]; temp[2] = OffsetPoint[2];
						break;
					}
					Displacement = GradientMagDerivativeInterpolator->EvaluateAtContinuousIndex(temp);
					Displacement.Normalize();
					Displacement = Displacement * (double)dt;
					if ((Displacement * PreviousDisplacement < 0) || (count > 100))
					{
						break;
					}
					OffsetPoint = OffsetPoint - Displacement;
					PreviousDisplacement = Displacement;
				}
				OffSetPointsIndex[i] = temp;
			}
			OffsetPointsList.push_back(OffSetPointsIndex);
			Icount++;
		}
		tend = time(0);	
		cout << "Done Finding Offset Points, Time Elapsed = " << difftime(tend, tstart) << endl;
	}

	//Graph Optimization
	int *labels;
	{
		InternalPixelType UpperWeight = 4*UpperThreshold;
		InternalPixelType LowerWeight = UpperThreshold;
		typedef itk::LinearInterpolateImageFunction<FloatImageType> GradientMagInterpolatorType;
		GradientMagInterpolatorType::Pointer GradientMagInterpolator = GradientMagInterpolatorType::New();
		GradientMagInterpolator->SetInputImage(GradientMag);
		typedef itk::LinearInterpolateImageFunction<ImageType> volumeInterpolatorType;
		volumeInterpolatorType::Pointer volumeInterpolator = volumeInterpolatorType::New();
		volumeInterpolator->SetInputImage(volume);
		IntImagePixelType num_vertices = 2 * SubVoxelList.size();
		int *data = new int[num_vertices*num_labels];
		for (unsigned int i = 0; i < num_vertices; i++)
		{
			for (unsigned int l = 0; l < num_labels; l++)
			{
				ContinuousIndexType index = OffsetPointsList[i / 2][i % 2];
				InternalPixelType intensity = volumeInterpolator->EvaluateAtContinuousIndex(index);
				FloatImagePixelType gradientMagnitude = GradientMagInterpolator->EvaluateAtContinuousIndex(index);
				if ((intensity >= (peaks[l] - deviation[l])) && (intensity <= (peaks[l] + deviation[l])))
					data[i*num_labels + l] = (gradientMagnitude > UpperWeight) ? UpperWeight : gradientMagnitude;
				else
					data[i*num_labels + l] = UpperWeight;
			}
		}
		int *smooth = new int[num_labels*num_labels];
		for (unsigned int l1 = 0; l1 < num_labels; l1++)
		{
			for (unsigned int l2 = 0; l2 < num_labels; l2++)
			{
				if (l1 != l2) smooth[l1 + num_labels*l2] = 1;
				else smooth[l1 + num_labels*l2] = 0;
			}
		}
		try
		{
			GCoptimizationGeneralGraph *gc = new GCoptimizationGeneralGraph(num_vertices, num_labels);
			gc->setDataCost(data);
			gc->setSmoothCost(smooth);
			for (unsigned int i = 0; i < SubVoxelList.size(); i++)
			{
				ContinuousIndexType CurrentOffsetPIndex, CurrentOffsetMIndex;
				GradientCovariantType CurrentOffsetP, CurrentOffsetM,currentNormal;
				CurrentOffsetPIndex = OffsetPointsList[i][0];
				CurrentOffsetMIndex = OffsetPointsList[i][1];
				CurrentOffsetP[0] = CurrentOffsetPIndex[0]; CurrentOffsetP[1] = CurrentOffsetPIndex[1]; CurrentOffsetP[2] = CurrentOffsetPIndex[2];
				CurrentOffsetM[0] = CurrentOffsetMIndex[0]; CurrentOffsetM[1] = CurrentOffsetMIndex[1]; CurrentOffsetM[2] = CurrentOffsetMIndex[2];
				currentNormal = NormalList[i];
				InternalPixelType CurrentOffsetPIntensity, CurrentOffsetMIntensity;
				CurrentOffsetPIntensity = volumeInterpolator->EvaluateAtContinuousIndex(CurrentOffsetPIndex);
				CurrentOffsetMIntensity = volumeInterpolator->EvaluateAtContinuousIndex(CurrentOffsetMIndex);
				std::list<IntImagePixelType> NeighList = EdgePointNeigh[i];
				for (std::list<IntImagePixelType>::iterator it = NeighList.begin(); it != NeighList.end(); it++)
				{
					IntImagePixelType j = *it;
					ContinuousIndexType NeighOffsetPIndex, NeighOffsetMIndex;
					NeighOffsetPIndex = OffsetPointsList[j][0];
					NeighOffsetMIndex = OffsetPointsList[j][1];
					GradientCovariantType NeighOffsetP, NeighOffsetM, neighNormal;
					NeighOffsetP[0] = NeighOffsetPIndex[0]; NeighOffsetP[1] = NeighOffsetPIndex[1]; NeighOffsetP[2] = NeighOffsetPIndex[2];
					NeighOffsetM[0] = NeighOffsetMIndex[0]; NeighOffsetM[1] = NeighOffsetMIndex[1]; NeighOffsetM[2] = NeighOffsetMIndex[2];
					neighNormal = NormalList[j];
					InternalPixelType NeighOffsetPIntensity, NeighOffsetMIntensity;
					NeighOffsetPIntensity = volumeInterpolator->EvaluateAtContinuousIndex(NeighOffsetPIndex);
					NeighOffsetMIntensity = volumeInterpolator->EvaluateAtContinuousIndex(NeighOffsetMIndex);
					if (currentNormal*neighNormal >= 0)
					{
						double weight1, weight2;
						if (CurrentOffsetP == NeighOffsetP) weight1 = UpperWeight;
						else 
							weight1 = UpperWeight - abs(CurrentOffsetPIntensity - NeighOffsetMIntensity) / (CurrentOffsetP - NeighOffsetP).GetNorm();
						if (CurrentOffsetM == NeighOffsetM) weight2 = UpperWeight;
						else
							weight2 = UpperWeight - abs(CurrentOffsetMIntensity - NeighOffsetMIntensity) / (CurrentOffsetM - NeighOffsetM).GetNorm();
						
						gc->setNeighbors(2 * i + 0, 2 * j + 0, (weight1>0)?weight1:0);
						gc->setNeighbors(2 * i + 1, 2 * j + 1, (weight2>0)?weight2:0);
					}
					else
					{
						double weight1, weight2;
						if (CurrentOffsetP == NeighOffsetM) weight1 = UpperWeight;
						else
							weight1 = UpperWeight - abs(CurrentOffsetPIntensity - NeighOffsetMIntensity) / (CurrentOffsetP - NeighOffsetM).GetNorm();
						if (CurrentOffsetM == NeighOffsetP) weight1 = UpperWeight;
						else
							weight2 = UpperWeight - abs(CurrentOffsetMIntensity - NeighOffsetPIntensity) / (CurrentOffsetM - NeighOffsetP).GetNorm();
						gc->setNeighbors(2 * i + 0, 2 * j + 1, (weight1 > 0) ? weight1 : 0);
						gc->setNeighbors(2 * i + 1, 2 * j + 0, (weight2 > 0) ? weight2 : 0);
					}
				}
			}
			unsigned int old_energy = gc->compute_energy();
			unsigned int new_energy;
			cout << "Energy Before Optimization " << old_energy << endl;
			while (1)
			{
				gc->expansion(10);
				new_energy = gc->compute_energy();
				if ((new_energy-old_energy) == 0)
				{
					break;
				}
				old_energy = new_energy;
			}
			cout << "Energy After Optimization " << gc->compute_energy() << endl;
			labels = new int[num_vertices];
			for (unsigned int i = 0; i < num_vertices; i++)
			{
				labels[i] = gc->whatLabel(i);
			}
		}
		catch (GCException e)
		{
			e.Report();
		}
		catch (itk::ExceptionObject &err)
		{
			cout << err;
		}
	}
	std::vector<std::vector<std::vector<Eigen::Vector3f>>> points(num_labels, std::vector<std::vector<Eigen::Vector3f>>(num_labels)), colors(num_labels, std::vector<std::vector<Eigen::Vector3f>>(num_labels));
	
	BinaryImageType::Pointer EdgeMap = BinaryImageType::New();
	{
		EdgeMap->SetRegions(volume->GetLargestPossibleRegion());
		EdgeMap->Allocate();
		EdgeMap->FillBuffer(itk::NumericTraits<BinaryImagePixelType>::Zero);
	}

	for (unsigned int k = 0; k < SubVoxelList.size(); k++)
	{
		unsigned int l1 = labels[2 * k], l2 = labels[2 * k + 1];
		for (unsigned int i = 0; i < num_labels; i++)
		{
			for (unsigned int j = 0; j <= i; j++)
			{
				if ((l1 == i && l2 == j) || (l1 == j && l2 == i))
				{
					points[i][j].push_back(Eigen::Vector3f(SubVoxelList[k][0], SubVoxelList[k][1], SubVoxelList[k][2]));
					colors[i][j].push_back(Eigen::Vector3f(1, 1, 1));
				}
			}
		}
		if (l1 != l2)
		{
			if ((l1 == 0 && l2 == 1) || (l1 == 1 && l2 == 0))
			{
				EdgeMap->SetPixel(CannyList[k], 1);
			}
			else if ((l1 == 0 && l2 == 2) || (l1 == 2 && l1 == 0))
			{
				EdgeMap->SetPixel(CannyList[k], 2);
			}
			else if ((l1 == 1 && l2 == 2) || (l1 == 2 && l2 == 1))
			{
				EdgeMap->SetPixel(CannyList[k], 3);
			}
		}
		else
		{
			if (l1 == 0)
			{
				EdgeMap->SetPixel(CannyList[k], 4);
			}
			else if (l1 == 1)
			{
				EdgeMap->SetPixel(CannyList[k], 5);
			}
			else if (l1 == 2)
			{
				EdgeMap->SetPixel(CannyList[k], 6);
			}
		}
	}
	
	//saveSlices(EdgeMap, volumeSize,"EdgeProfile");


	for (unsigned int i = 0; i < num_labels; i++)
	{
		for (unsigned int j = 0; j <= i; j++)
		{
			cout << "Displaying interferce between material:" << i << " and matrerial:" << j << endl;
			tr::Display3DRoutines::displayPointSet(points[i][j], colors[i][j]);
		}
	}
	return EXIT_SUCCESS;
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


	int nPoints = isoSurface->GetNumberOfPoints();
	int nCells = isoSurface->GetNumberOfCells();

	std::vector< float > points(nPoints * 3), normals(nPoints * 3);
	std::vector< unsigned int > indices(nCells * 3);

	vtkSmartPointer< vtkDataArray > isoSurfaceNormals = isoSurface->GetPointData()->GetNormals();

	std::cout << " normals size : " << isoSurfaceNormals->GetNumberOfTuples() << " " << isoSurfaceNormals->GetNumberOfComponents() << std::endl;

	for (int pp = 0; pp < nPoints; pp++)
	{
		double pt[3], n[3];

		isoSurface->GetPoint(pp, pt);

		isoSurfaceNormals->GetTuple(pp, n);

		points[3 * pp] = pt[0];
		points[3 * pp + 1] = pt[1];
		points[3 * pp + 2] = pt[2];

		normals[3 * pp] = n[0];
		normals[3 * pp + 1] = n[1];
		normals[3 * pp + 2] = n[2];
	}

	vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();

	for (int tt = 0; tt < nCells; tt++)
	{
		isoSurface->GetCellPoints(tt, triangle);

		indices[3 * tt] = triangle->GetId(0);
		indices[3 * tt + 1] = triangle->GetId(1);
		indices[3 * tt + 2] = triangle->GetId(2);
	}


	FILE *file = fopen("C:/Data/ZxoData/bitron.tri", "wb");

	fwrite(&nPoints, 1, sizeof(int), file);
	fwrite(&nCells, 1, sizeof(int), file);
	fwrite(points.data(), points.size(), sizeof(float), file);
	fwrite(normals.data(), normals.size(), sizeof(float), file);
	fwrite(indices.data(), indices.size(), sizeof(unsigned int), file);

	fclose(file);

	tr::Display3DRoutines::displayPolyData(isoSurface);

}


