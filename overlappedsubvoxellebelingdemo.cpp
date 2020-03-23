//Standard C++
#include<iostream>
#include<fstream>
#include "limits.h"
#include <ctime>
#include<math.h>
#include<queue> 
#include<list>
#include<omp.h>
#include<intrin.h>

//ITK
#include "itkImage.h"
#include "itkImageLinearIteratorWithIndex.h"
#include "itkGradientMagnitudeImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkNeighborhoodIterator.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkTestingExtractSliceImageFilter.h"
#include "itkImageFileWriter.h"
#include "itkPNGImageIO.h"
#include "itkDiscreteGaussianImageFilter.h"

#include "display3droutines.h"
#include "ZeissSegmentationInterface.hpp"
//#include "ZeissSeparationInterface.hpp"

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
typedef ImageType::IndexType IndexType;
typedef itk::ImageRegionIteratorWithIndex<BinaryImageType> BinaryImageRegionIteratorType;
typedef itk::ImageRegionIteratorWithIndex<ImageType> ImageRegionIteratorType;
typedef itk::ImageRegionIteratorWithIndex<FloatImageType> FloatImageRegionIteratorType;
typedef itk::NeighborhoodIterator<BinaryImageType> BinaryNeighIteratorType;
typedef itk::NeighborhoodIterator<ImageType> ImageNeighIteratorType;
typedef itk::NeighborhoodIterator<FloatImageType> FloatNeighIteratorType;
time_t tstart, tend;

struct Region : public MaterialRegion
{
	public:
		InternalPixelType deviation, deviationUpperBound, deviationLowerBound;
		Region(MaterialRegion param, InternalPixelType paramDeviation)
		{
			deviation = paramDeviation;
			lower_bound = param.lower_bound;
			upper_bound = param.upper_bound;
			peak = param.peak;
			deviationUpperBound = ((peak + deviation) < upper_bound) ? (peak + deviation) : upper_bound;
			deviationLowerBound = ((peak - deviation) > lower_bound) ? (peak - deviation) : lower_bound;

		}

		bool isInsideDeviation(InternalPixelType I)
		{
			return ((I <= deviationUpperBound) && (I >= deviationLowerBound));
		}
		bool isInsideMaterialRange(InternalPixelType I)
		{
			return ((I <= upper_bound) && (I >= lower_bound));
		}
};

struct ValueChange
{
	public:
		IndexType index;
		InternalPixelType value;
		ValueChange(IndexType paramIndex, InternalPixelType paramValue) : index(paramIndex), value(paramValue) { }
};
//Function to print Slices
template <typename T>
void saveSlices(typename T::Pointer Image, std::string name)
{
	typedef typename itk::Image<T::PixelType,2> Image2DType;
	typedef typename itk::Testing::ExtractSliceImageFilter<T, Image2DType> ExtractSliceFilterType;
	typedef typename itk::ImageFileWriter<Image2DType> Image2DWriter;
	typedef typename T::IndexType ImageIndexType;
	typedef typename T::RegionType ImageRegionType;
	typedef typename T::RegionType::SizeType ImageSizeType;
	ImageSizeType volumeSize = Image->GetLargestPossibleRegion().GetSize();
	ImageIndexType SliceStart;
	ImageRegionType SliceRegion;
	ImageSizeType SliceSize;
	ExtractSliceFilterType::Pointer ExtractSliceFilter = ExtractSliceFilterType::New();
	ExtractSliceFilter->SetInput(Image);
	ExtractSliceFilter->SetDirectionCollapseToIdentity();
	Image2DWriter::Pointer TestWriter = Image2DWriter::New();
	TestWriter->SetImageIO(itk::PNGImageIO::New());
	std::ostringstream out;
	for (unsigned int i = 0; i < volumeSize[0]; i++)
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
			materialDeviation += pow((j - 1.0*peak), 2)*histogram[j];
		}
		deviation[i] = ceil(sqrt(materialDeviation/voxelCount));
	}
	return deviation;
}


int main(int argc, char** argv)
{
	//if (argc != 2)
	//{
	//	cout << "Wrong Usage!!" << endl;
	//	cout << "Usage: HelloWorld.exe <UNIT16_SCV_FILE_NAME>" << endl;
	//	return EXIT_FAILURE;
	//}
	//ITK use constants and TypeDefs
	ImageType::Pointer volume = ImageType::New(); //Acquire Smart Pointer to volume
	unsigned short *data;
	unsigned char* header;
	int volumeSize[3];
	double resolution[3];
	char *temp = new char(2);
	unsigned long count;
	unsigned int headerSize;
	tstart = time(0);

	std::string filePath = "C:\\Projects\\Wallthickness\\data\\CT multi\\Hemanth_ Sugar free 2016-6-6 11-33.uint16_scv";//"C:\Projects\Wallthickness\data / Datasets for Multi - material / Beretta Result Oberkochen";

	ifstream uint16(filePath, ios::binary);//argv[1]
	if (uint16.is_open())
	{
		uint16.read(reinterpret_cast<char *>(&headerSize), sizeof(headerSize));
		header = new unsigned char[headerSize - sizeof(headerSize)];
		uint16.read(reinterpret_cast<char *>(header), headerSize - sizeof(headerSize));
		memcpy(volumeSize, header + 8, 12);
		memcpy(resolution, header + 20, 24);
		std::cout << "Size of the volume is:" << volumeSize[0] << "x" << volumeSize[1] << "x" << volumeSize[2] << endl;
		std::cout << "Resolution of the volume is:" << resolution[0] << " " << resolution[1] << " " << resolution[2] << endl;
		delete[] header;
		ImageType::IndexType start;
		start[0] = 0; start[1] = 0; start[2] = 0;
		ImageType::SizeType size;
		size[0] = volumeSize[0]; size[1] = volumeSize[1]; size[2] = volumeSize[2];
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
		for (volumeIt.GoToBegin(); !volumeIt.IsAtEnd(); volumeIt.NextLine())
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
	Volume v;
	v.data = data;
	v.size[0] = volumeSize[0]; v.size[1] = volumeSize[1]; v.size[2] = volumeSize[2];
	v.voxel_size[0] = resolution[0]; v.voxel_size[1] = resolution[1]; v.voxel_size[2] = resolution[2];
	ZeissSegmentationInterface ZeissSegmentation;
	ZeissSegmentation.setInputVolume(v);
	Materials M = ZeissSegmentation.getMaterialRegions();
	const IntImagePixelType num_labels = M.regions.size();
	InternalPixelType *deviation;
	MultiResolutionHistograms *histograms = new MultiResolutionHistograms();
	ZeissSegmentation.getMultiResolutionHistograms(histograms);
	deviation = calculateDeviation(M,histograms->first_histogram_data,volumeSize);
	InternalPixelType Epsilon = itk::NumericTraits<InternalPixelType>::max();
	std::vector<Region> regions;
	for (unsigned int i = 0; i < num_labels; i++)
	{
		MaterialRegion temp;
		temp = M.regions[i];
		if (i == 0)
		{
			temp.lower_bound = 0;
		}
		else if (i == (num_labels - 1))
		{
			temp.upper_bound = itk::NumericTraits<InternalPixelType>::max();
		}
		regions.push_back(Region(temp, deviation[i]));
	}
	FloatImagePixelType d = 2;
	for (unsigned int i = 0; i < (num_labels - 1); i++)
	{
		if ((M.regions[i + 1].peak - M.regions[i].peak) / (2 * d) < Epsilon)
		{
			Epsilon = (M.regions[i + 1].peak - M.regions[i].peak) / (2 * d);
		}
	}
	Epsilon = Epsilon / 5;
	cout << "Epsilon = " << Epsilon << endl;
	//finding Gradient Magnitude Image
	FloatImageType::Pointer GradientMag;
	{
		tstart = time(0);
		typedef itk::DiscreteGaussianImageFilter<ImageType, ImageType> GaussianFilterType;
		GaussianFilterType::Pointer GaussianFilter = GaussianFilterType::New();
		GaussianFilter->SetInput(volume);
		GaussianFilter->SetVariance(1);
		typedef itk::GradientMagnitudeImageFilter<ImageType, FloatImageType> GradientMagnitudeImageFilterType;
		GradientMagnitudeImageFilterType::Pointer GradientMagnitudeImageFilter = GradientMagnitudeImageFilterType::New();
		GradientMagnitudeImageFilter->SetInput(GaussianFilter->GetOutput());
		//GradientMagnitudeImageFilter->SetInput(volume);
		try
		{
			GradientMagnitudeImageFilter->Update();
			tend = time(0);
			cout << "Done Gradient Magnitude Calculation, time elapsed = " << difftime(tend, tstart) << endl;
			GradientMag = GradientMagnitudeImageFilter->GetOutput();
			volume = GaussianFilter->GetOutput();
		}
		catch (itk::ExceptionObject &err)
		{
			cout << "Error Occured during gradient magnitude calculation:" << endl;
			cout << err;
		}
	}
	cout << regions[2].deviationLowerBound << " " << regions[2].deviationUpperBound << endl;
	ImageType::Pointer Labels; //Images to store labels 
	std::vector<ImageType::IndexType> D; //vector of all uncertain points; 
	{
		tstart = time(0);
		{
			Labels = ImageType::New();
			Labels->SetRegions(volume->GetLargestPossibleRegion());
			Labels->Allocate();
			Labels->FillBuffer(itk::NumericTraits<InternalPixelType>::Zero);
		}
		BinaryNeighIteratorType::RadiusType Nradius; Nradius.Fill(1);
		FloatNeighIteratorType GradientMagNeighIT(Nradius, GradientMag, GradientMag->GetLargestPossibleRegion());
		ImageRegionIteratorType LabelIT(Labels, Labels->GetLargestPossibleRegion());
		ImageRegionIteratorType volumeIT(volume, volume->GetLargestPossibleRegion());
		LabelIT.GoToBegin();
		volumeIT.GoToBegin();
		GradientMagNeighIT.GoToBegin();
		while (!(LabelIT.IsAtEnd() || volumeIT.IsAtEnd() || GradientMagNeighIT.IsAtEnd()))
		{
			if (GradientMagNeighIT.GetCenterPixel() < Epsilon)
			{
				unsigned int tempCount = 0;
				for (unsigned int i = 0; i < GradientMagNeighIT.Size(); i++)
				{
					if (!GradientMagNeighIT.IndexInBounds(i))
					{
						continue;
					}
					if (GradientMagNeighIT.GetPixel(i) < Epsilon)
						tempCount++;
				}
				if (tempCount <= 1)
				{
					LabelIT.Set(itk::NumericTraits<InternalPixelType>::Zero);	
					D.push_back(LabelIT.GetIndex());
				}
				else
				{
					bool flag = false;
					for (unsigned int i = 0; i < regions.size(); i++)
					{
						if (regions[i].isInsideDeviation(volumeIT.Get()))
						{
							flag = true;
							LabelIT.Set(1 << i);
							break;
						}
					}
					if (!flag)
					{
						LabelIT.Set(itk::NumericTraits<InternalPixelType>::Zero);
						D.push_back(LabelIT.GetIndex());
					}
				}
			}
			else
			{
				LabelIT.Set(itk::NumericTraits<InternalPixelType>::Zero);
				D.push_back(LabelIT.GetIndex());
			}
			++LabelIT;
			++volumeIT;
			++GradientMagNeighIT;
		}
		tend = time(0);
		cout << "Done determining L and D, and labels for points in L, time elapsed = " << difftime(tend, tstart) << endl;
	}
	//Label propagation without Overlap
	{
		tstart = time(0);
		ImageNeighIteratorType::RadiusType Nradius; Nradius.Fill(1);
		ImageNeighIteratorType LabelsNeighIT(Nradius, Labels, Labels->GetLargestPossibleRegion());
		std::queue<IndexType, std::deque<IndexType>> Unmarked(std::deque<IndexType>(D.begin(), D.end()));
		while (1)
		{
			std::queue<IndexType> NewUnmarked = std::queue<IndexType>();
			std::queue<ValueChange> ChangeList = std::queue<ValueChange>();
			while (!Unmarked.empty())
			{
				IndexType CurrentIndex = Unmarked.front();
				Unmarked.pop();
				LabelsNeighIT.SetLocation(CurrentIndex);
				InternalPixelType temp = LabelsNeighIT.GetCenterPixel();
				for (unsigned int i = 0; i < LabelsNeighIT.Size(); i++)
				{
					if (!LabelsNeighIT.IndexInBounds(i))
						continue;
					temp |= LabelsNeighIT.GetPixel(i);
				}
				if (__popcnt16(temp) == 0)
					NewUnmarked.push(CurrentIndex);
				else
					ChangeList.push(ValueChange(CurrentIndex, temp));
			}
			while (!ChangeList.empty())
			{
				ValueChange temp = ChangeList.front();
				ChangeList.pop();
				Labels->SetPixel(temp.index, temp.value);
			}
			if (NewUnmarked.empty())
				break;
			Unmarked = NewUnmarked;
		}
		tend = time(0);
		cout << "Done Label propagation without Overlapping, time elapsed = " << difftime(tend, tstart) << endl;
	}
	//Label propagation with overlap
	{
		tstart = time(0);
		ImageNeighIteratorType::RadiusType Nradius; Nradius.Fill(1);
		ImageNeighIteratorType LabelsNeighIT(Nradius, Labels, Labels->GetLargestPossibleRegion());
		std::queue<IndexType, std::deque<IndexType>>Unmarked(std::deque<IndexType>(D.begin(), D.end()));
		unsigned long long queueLenght = D.size();
		while (1)
		{
			std::queue<IndexType> NewUnmarked = std::queue<IndexType>();
			std::queue<ValueChange> ChangeList = std::queue<ValueChange>();
			unsigned long long tempCount = 0;
			while (!Unmarked.empty())
			{
				IndexType CurrentIndex = Unmarked.front();
				Unmarked.pop();
				LabelsNeighIT.SetLocation(CurrentIndex);
				InternalPixelType temp = LabelsNeighIT.GetCenterPixel();
				for (unsigned int i = 0; i < LabelsNeighIT.Size(); i++)
				{
					if (!LabelsNeighIT.IndexInBounds(i))
						continue;
					else
						temp |= LabelsNeighIT.GetPixel(i);
				}
				if (__popcnt16(temp) == 1)
				{
					NewUnmarked.push(CurrentIndex);
					tempCount++;
				}
				else
					ChangeList.push(ValueChange(CurrentIndex, temp));
			}
			while (!ChangeList.empty())
			{
				ValueChange temp = ChangeList.front();
				ChangeList.pop();
				Labels->SetPixel(temp.index, temp.value);
			}
			if (tempCount == queueLenght)
				break;
			queueLenght = tempCount;
			Unmarked = NewUnmarked;
		}
		tend = time(0);
		cout << "Done Label propagation with Overlapping, time elapsed = " << difftime(tend, tstart) << endl;
	}
	//Assigning labels
	tstart = time(0);
	BinaryImageType::Pointer Segmentation = BinaryImageType::New();
	{
		Segmentation->SetRegions(volume->GetLargestPossibleRegion());
		Segmentation->Allocate();
	}
	ImageRegionIteratorType LabelsIT(Labels, Labels->GetLargestPossibleRegion());
	ImageRegionIteratorType volumeIT(volume, volume->GetLargestPossibleRegion());
	BinaryImageRegionIteratorType SegmentationIT(Segmentation, Segmentation->GetLargestPossibleRegion());
	LabelsIT.GoToBegin();
	volumeIT.GoToBegin();
	SegmentationIT.GoToBegin();
	while (!(LabelsIT.IsAtEnd() || volumeIT.IsAtEnd() || SegmentationIT.IsAtEnd()))
	{
		int i = -1;
		InternalPixelType f = LabelsIT.Get(), iso, g = volumeIT.Get();
		/*if (f >> 2 & 1)
			cout << f << " ";*/
		for (unsigned int k = 0; k < num_labels; k++)
		{
			if ((f >> k) & 1)
			{
				if (i == -1)
					i = k;
				else
				{
					iso = (regions[i].peak + regions[k].peak) / 2;
					if (g < iso)
						break;
					i = k;
				}
			}
		}
		SegmentationIT.Set(i);
		++LabelsIT;
		++volumeIT;
		++SegmentationIT;
	}
	tend = time(0);
	cout << "Done Segmentation, time elapsed = " << difftime(tend, tstart) << endl;
	saveSlices<BinaryImageType>(Segmentation, "Segmentation");
	saveSlices<ImageType>(Labels, "labels");
	return EXIT_SUCCESS;
}