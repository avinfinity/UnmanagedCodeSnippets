
#include "iostream"
#include "src\plugins\Application\ZeissViewer\SegmentationInterface\ZeissSegmentationInterface.hpp"
#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "itkPNGImageIO.h"
#include "eigenincludes.h"
#include "VolumeInfo.h"

typedef unsigned short InternalPixelType;
typedef Eigen::Vector3f GradientPixelType;
typedef unsigned char BinaryPixelType;
typedef Eigen::Vector3i IndexType;
typedef Eigen::Vector3f ContIndexType;

const float PI = 3.1415927;

//Assumes that the index provided is inside the volume
template<typename T>
T TrilinearInterpolation(typename const T* volume, const int* volumeSize, const ContIndexType index)
{
	const size_t zStep = volumeSize[0] * volumeSize[1], yStep = volumeSize[0];

	int x0 = floor(index[0]), x1 = ceil(index[0]), y0 = floor(index[1]), y1 = ceil(index[1]), z0 = floor(index[2]), z1 = ceil(index[2]);

	float xd = (x0 == x1) ? 0 : ((index[0] - x0) / (x1 - x0));
	float yd = (y0 == y1) ? 0 : ((index[1] - y0) / (y1 - y0));
	float zd = (z0 == z1) ? 0 : ((index[2] - z0) / (z1 - z0));

	float c00 = *(volume + x0 + y0*yStep + z0*zStep) * (1 - xd) + *(volume + x1 + y0*yStep + z0*zStep)*xd;
	float c01 = *(volume + x0 + y0*yStep + z1*zStep) * (1 - xd) + *(volume + x1 + y0*yStep + z1*zStep)*xd;
	float c10 = *(volume + x0 + y1*yStep + z0*zStep) * (1 - xd) + *(volume + x1 + y1*yStep + z0*zStep)*xd;
	float c11 = *(volume + x0 + y1*yStep + z1*zStep) * (1 - xd) + *(volume + x1 + y1*yStep + z1*zStep)*xd;

	float c0 = c00*(1 - yd) + c10*yd;
	float c1 = c01*(1 - yd) + c11*yd;

	return c0*(1 - zd) + c1*zd;
}

//Assumes that the index provided is inside the volume
template<typename T>
T VectorTrilinearInterpolation(typename const T* volume, const int* volumeSize, const ContIndexType index)
{
	const size_t zStep = volumeSize[0] * volumeSize[1], yStep = volumeSize[0];

	int x0 = floor(index[0]), x1 = ceil(index[0]), y0 = floor(index[1]), y1 = ceil(index[1]), z0 = floor(index[2]), z1 = ceil(index[2]);

	float xd = (x0 == x1) ? 0 : ((index[0] - x0) / (x1 - x0));
	float yd = (y0 == y1) ? 0 : ((index[1] - y0) / (y1 - y0));
	float zd = (z0 == z1) ? 0 : ((index[2] - z0) / (z1 - z0));

	T c00 = *(volume + x0 + y0*yStep + z0*zStep) * (1 - xd) + *(volume + x1 + y0*yStep + z0*zStep)*xd;
	T c01 = *(volume + x0 + y0*yStep + z1*zStep) * (1 - xd) + *(volume + x1 + y0*yStep + z1*zStep)*xd;
	T c10 = *(volume + x0 + y1*yStep + z0*zStep) * (1 - xd) + *(volume + x1 + y1*yStep + z0*zStep)*xd;
	T c11 = *(volume + x0 + y1*yStep + z1*zStep) * (1 - xd) + *(volume + x1 + y1*yStep + z1*zStep)*xd;

	T c0 = c00*(1 - yd) + c10*yd;
	T c1 = c01*(1 - yd) + c11*yd;

	T c = c0*(1 - zd) + c1*zd;
	return c;
}

template<typename T>
void saveSlice(typename const T *volData, int* volSize, std::string name)
{
	typedef typename itk::Image<T, 2> Image2D;
	typedef typename Image2D::IndexType ImageIndexType;
	typedef typename Image2D::RegionType ImageRegionType;
	typedef typename Image2D::RegionType::SizeType ImageSizeType;
	typedef typename itk::ImageFileWriter<Image2D> ImageWriter;
	unsigned int yStep = volSize[0], zStep = volSize[1] * volSize[0];
	std::ostringstream out;
	for (unsigned int xx = 0; xx < volSize[0]; xx++)
	{
		out.str(""); out.clear();
		out << name << xx << ".png";
		ImageWriter::Pointer TestWriter = ImageWriter::New();
		TestWriter->SetImageIO(itk::PNGImageIO::New());
		ImageIndexType SliceStart; ImageSizeType SliceSize; ImageRegionType SliceRegion;
		SliceStart.Fill(0); SliceSize[0] = volSize[1]; SliceSize[1] = volSize[2];
		SliceRegion.SetIndex(SliceStart); SliceRegion.SetSize(SliceSize);
		Image2D::Pointer Slice = Image2D::New();
		Slice->SetRegions(SliceRegion); Slice->Allocate();
		for (unsigned int yy = 0; yy < volSize[1]; yy++)
			for (unsigned int zz = 0; zz < volSize[2]; zz++)
			{
				ImageIndexType temp;
				temp[0] = yy; temp[1] = zz;
				Slice->SetPixel(temp, *(volData + xx + yy*yStep + zz*zStep));
			}
		TestWriter->SetInput(Slice);
		TestWriter->SetFileName(out.str());
		try
		{
			TestWriter->Update();
		}
		catch (itk::ExceptionObject &err)
		{
			std::cout << "Error Occured During Writing Slice:" << xx << std::endl;
			std::cout << err;
			return;
		}
	}
}

struct Regions
{
public:
	InternalPixelType *deviation, *LowerLimits, *UpperLimits, *means, *meanDeviation;
	std::vector<MaterialRegion> regions;
	Regions(Materials M, unsigned long long* histogram)
	{
		regions = M.regions;
		//deviation = paramDeviation;
		LowerLimits = new InternalPixelType[regions.size()];
		UpperLimits = new InternalPixelType[regions.size()];
		deviation = new InternalPixelType[regions.size()];
		means = new InternalPixelType[regions.size()];
		meanDeviation = new InternalPixelType[regions.size()];
		for (int i = 0; i < regions.size(); i++)
		{
			InternalPixelType LowerBound, UpperBound;
			if (i == 0)
			{
				LowerBound = 0;
				UpperBound = regions[i].upper_bound;
			}
			else if (i == regions.size() - 1)
			{
				LowerBound = regions[i].lower_bound;
				UpperBound = std::numeric_limits<InternalPixelType>::max();
			}
			else
			{
				LowerBound = regions[i].lower_bound;
				UpperBound = regions[i].upper_bound;
			}
			float sum = 0, mean = 0, dev = 0;
			for (InternalPixelType x = LowerBound; (x <= UpperBound) && (x >= LowerBound); x++)
			{
				sum += histogram[x];
				mean += x*histogram[x];
				dev += pow((x - 1.0*regions[i].peak), 2)*histogram[x];
			}
			means[i] = ceil(mean / sum);
			deviation[i] = ceil(sqrt(dev / sum));
			dev = 0;
			for (InternalPixelType x = LowerBound; (x <= UpperBound) && (x >= LowerBound); x++)
			{
				dev += pow((x - 1.0*means[i]), 2)*histogram[x];
			}
			meanDeviation[i] = ceil(sqrt(dev / sum));
			if (i == 0)
			{
				LowerLimits[i] = 0;
				//UpperLimits[i] = regions[i].upper_bound;
				if (regions[i].upper_bound < (regions[i].peak + deviation[i]))
					UpperLimits[i] = regions[i].upper_bound;
				else
					UpperLimits[i] = regions[i].peak + deviation[i];
			}
			else if (i == (regions.size() - 1))
			{
				//LowerLimits[i] = regions[i].lower_bound;
				if (regions[i].lower_bound >(regions[i].peak - deviation[i]))
					LowerLimits[i] = regions[i].lower_bound;
				else
					LowerLimits[i] = regions[i].peak - deviation[i];

				UpperLimits[i] = std::numeric_limits <InternalPixelType>::max();
			}
			else
			{
				if (regions[i].lower_bound > (regions[i].peak - deviation[i]))
					LowerLimits[i] = regions[i].lower_bound;
				else
					LowerLimits[i] = regions[i].peak - deviation[i];
				if (regions[i].upper_bound < (regions[i].peak + deviation[i]))
					UpperLimits[i] = regions[i].upper_bound;
				else
					UpperLimits[i] = regions[i].peak + deviation[i];
			}
		}
	}
	int getRegion(InternalPixelType intensity) //returns -1 if doesn't belong to region
	{
		for (int i = 0; i < regions.size(); i++)
		{
			if ((intensity >= LowerLimits[i]) && (intensity <= UpperLimits[i]))
				return i;
		}
		return -1;
	}
};
struct ValueChange
{
public:
	IndexType index;
	unsigned char value;
	ValueChange(IndexType paramIndex, unsigned char paramValue) : index(paramIndex), value(paramValue) { }
};

//InternalPixelType* calculateDeviation(Materials M, unsigned long long* histogram)
//{
//	InternalPixelType* deviation = new InternalPixelType[M.regions.size()];
//	for (unsigned int i = 0; i < M.regions.size(); i++)
//	{
//		InternalPixelType LowerBound, peak, UpperBound;
//		LowerBound = M.regions[i].lower_bound;
//		UpperBound = M.regions[i].upper_bound;
//		peak = M.regions[i].peak;
//		double materialDeviation = 0;
//		double voxelCount = 0;
//		for (unsigned int j = LowerBound; j <= UpperBound; j++)
//		{
//			voxelCount += histogram[j];
//			materialDeviation += pow((j - 1.0*peak), 2)*histogram[j];
//		} 
//		deviation[i] = ceil(sqrt(materialDeviation / voxelCount));
//	}
//	return deviation;
//}

//BinaryPixelType MaxLogHMRFLikelihood(int xx, int yy, int zz,size_t yStep, size_t zStep, imt::volume::VolumeInfo& volInfo, BinaryPixelType* Segmentation, Regions& regions, float beta)
//{
//	int num_labels = regions.regions.size();
//	InternalPixelType* VolumeData = (InternalPixelType*)volInfo.mVolumeData;
//	float minimum = std::numeric_limits<float>::max();
//	BinaryPixelType currentSegmentation = *(Segmentation + xx + yy*yStep + zz*zStep);
//	BinaryPixelType neigh = 0;
//	
//	for (int dz = zz - 1; dz <= zz + 1; dz++)
//		for (int dy = yy - 1; dy <= yy + 1; dy++)
//			for (int dx = xx - 1; dx <= xx + 1; dx++)
//			{
//				neigh |= (1 << *(Segmentation + dx + dy*yStep + dz*zStep));
//			}
//
//	for (int i = 0; i < num_labels; i++)
//	{
//		if ((neigh >> i) & 1)
//		{
//			float logLikelihood = std::log(regions.meanDeviation[i]) + std::pow(*(VolumeData + xx + yy*yStep + zz*zStep) - (1.0)*regions.means[i], 2) / (2 * std::pow(regions.meanDeviation[i], 2));
//			
//			for (int dz = zz - 1; dz <= zz + 1; dz++)
//				for (int dy = yy - 1; dy <= yy + 1; dy++)
//					for (int dx = xx - 1; dx <= xx + 1; dx++)
//					{
//						if ((dx == xx) && (dy == yy) && (dz == zz))
//							continue;
//						if (i == *(Segmentation + dx + dy*yStep + dz*zStep))
//							logLikelihood -= beta;
//					}
//			
//			if (logLikelihood < minimum)
//			{
//				minimum = logLikelihood;
//				currentSegmentation = i;
//			}
//		}
//	}
//	return currentSegmentation;
//}