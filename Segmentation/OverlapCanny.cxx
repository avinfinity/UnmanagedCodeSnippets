#include "iostream"
#include "ctime"
#include "stdio.h"
#include "string"
#include "volumeinfo.h"
#include "rawvolumedataio.h"
#include "Operators.h"
#include "src\plugins\Application\ZeissViewer\SegmentationInterface\ZeissSegmentationInterface.hpp"
#include "queue"
#include "itkImage.h"
#include "overlappedvoxelsmarchingcubes.h"
#include "display3droutines.h"

//ITK includes (only used to store image)
#include "itkImageFileWriter.h"
#include "itkPNGImageIO.h"

//VTK includes
#include "vtkImageData.h"
#include "vtkMarchingCubes.h"
#include "vtkPLYWriter.h"

#include "utils.cpp"

time_t tstart, tend;

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		cout << "Error!! Wrong usage" << endl;
		cout << "Usage: OverlapCanny <UINT16_FILE>" << endl;
		return EXIT_FAILURE;
	}

	//Reading uint16 file
	imt::volume::VolumeInfo vol;

	//Process to read .uint16_scv files
	imt::volume::RawVolumeDataIO::readUint16SCV(argv[1], vol);
	
	const int d = vol.mDepth, h = vol.mHeight, w = vol.mWidth;
	const size_t yStep = vol.mWidth, zStep = vol.mWidth*vol.mHeight;
	int volumeSize[3]; volumeSize[0] = vol.mWidth; volumeSize[1] = vol.mHeight; volumeSize[2] = vol.mDepth;
	Volume v;
	v.data = (InternalPixelType*)vol.mVolumeData;
	v.size[0] = volumeSize[0]; v.size[1] = volumeSize[1]; v.size[2] = volumeSize[2];
	ZeissSegmentationInterface ZeissSegmentation;
	ZeissSegmentation.setInputVolume(v);
	Materials M = ZeissSegmentation.getMaterialRegions();
	const unsigned int num_labels = M.regions.size();
	unsigned short *deviation;
	MultiResolutionHistograms *histograms = new MultiResolutionHistograms();
	*histograms = ZeissSegmentation.getMultiResolutionHistograms();
	//deviation = calculateDeviation(M, histograms->first_histogram_data);
	//std::cout << deviation[0] << " " << deviation[1] << " " << deviation[2] << std::endl;
	Regions regions(M, histograms->first_histogram_data);

	for (int i = 0; i < num_labels; i++)
	{
		std::cout << "Material " << i <<":" << std::endl;
		std::cout << "	Deviation "<< regions.deviation[i] << " Deviation Lower Limit " << regions.LowerLimits[i] << " Deviation Upper Limit " << regions.UpperLimits[i] << std::endl;
		std::cout << "	Lower Limit " << regions.regions[i].lower_bound << " Peak " << regions.regions[i].peak << " Upper Limit " << regions.regions[i].upper_bound << std::endl;
	}

	std::vector<unsigned short> peaks(num_labels);
	for (int i = 0; i < num_labels; i++)
		peaks[i] = M.regions[i].peak;

	float Epsilon = std::numeric_limits<InternalPixelType>::max();
	for (int i = 0; i < num_labels-1; i++)
	{
		if (Epsilon > (M.regions[i + 1].peak - M.regions[i].peak))
			Epsilon = (M.regions[i + 1].peak - M.regions[i].peak);
	}
	Epsilon = Epsilon;
	float UpperThreshold, LowerThreshold, NormalizedThreshold;
	UpperThreshold = Epsilon; LowerThreshold = UpperThreshold / 5;
	NormalizedThreshold = 0.05;
	std::cout << "Value of Upper Threshold = " << UpperThreshold << std::endl;
	std::cout << "Value of Lower Threshold = " << LowerThreshold << std::endl;
	std::cout << "Number of materials:" << num_labels << std::endl;

	//Calculating gradient vectors
	Eigen::Vector3f *VectorDerivatives;
	{
		
		tstart = time(0);
		/*InternalPixelType* temp = new InternalPixelType[vol.mWidth*vol.mHeight*vol.mDepth];
		imt::volume::GaussianOperator3x3 GaussianFilter;
		GaussianFilter.init(1); 
		GaussianFilter.apply(vol, temp);
		delete[] vol.mVolumeData;
		vol.mVolumeData = (unsigned char*)temp;*/
		
		InternalPixelType* temp = new InternalPixelType[d*h*w];
		imt::volume::MedianOperator3x3 MedianFilter;
		MedianFilter.init(3);
		MedianFilter.apply(vol, temp);
		delete[] vol.mVolumeData;
		vol.mVolumeData = (unsigned char*)temp;

		//VectorDerivatives = new Eigen::Vector3f[vol.mWidth*vol.mHeight*vol.mDepth];
		//imt::volume::SobelGradientOperator3x3 DerivativeOperator;
		//DerivativeOperator.init();
		//DerivativeOperator.apply(vol, VectorDerivatives);
		
		VectorDerivatives = new Eigen::Vector3f[w*h*d];
		imt::volume::CentralDerivativeOperator3x3x3 DerivativeOperator;
		DerivativeOperator.init();
		DerivativeOperator.apply(vol, VectorDerivatives);
		
		tend = time(0);
		std::cout << "Done applying Gaussian Filter and calculating Gradient vectors, time elipsed = " << difftime(tend, tstart) << std::endl;
	}

	//Non maximal suppression
	unsigned char* maximal = new unsigned char[vol.mWidth*vol.mHeight*vol.mDepth];
	std::fill_n(maximal, d*h*w, 0);
	InternalPixelType* GradientMag = new InternalPixelType[vol.mWidth*vol.mHeight*vol.mDepth];
	float* normalizedGradient = new float[w*h*d];
	{
		tstart = time(0);
		//#pragma omp parallel for
		for (int zz = 2; zz < d-2; zz++)
			for (int yy = 2; yy < h -2; yy++)
				for (int xx = 2; xx < w - 2; xx++)
				{
					Eigen::Vector3f derivative, unitVector;
					derivative = *(VectorDerivatives + xx + yy*yStep + zz*zStep);
					float derivativeMag = derivative.norm();
					*(GradientMag + xx + yy*yStep + zz*zStep) = derivativeMag;
					*(normalizedGradient + xx + yy*yStep + zz*zStep) = derivativeMag / (*((InternalPixelType*)vol.mVolumeData + xx + yy*yStep + zz*zStep));
					if (derivativeMag < LowerThreshold)
					{
						continue;
					}
					ContIndexType Uplus, Uminus, Ucenter;
					Ucenter[0] = xx; Ucenter[1] = yy; Ucenter[2] = zz;
					unitVector = derivative / derivativeMag;
					float max = (abs(unitVector[0]) > abs(unitVector[1])) ? abs(unitVector[0]) : abs(unitVector[1]);
					max = (abs(unitVector[2]) > max) ? abs(unitVector[2]) : max;
					float t = 1 / max;
					Uplus = Ucenter + unitVector*t;
					Uminus = Ucenter - unitVector*t;
					Eigen::Vector3f Gplus, Gminus;
					Gplus = VectorTrilinearInterpolation<Eigen::Vector3f>(VectorDerivatives, volumeSize, Uplus);
					Gminus = VectorTrilinearInterpolation<Eigen::Vector3f>(VectorDerivatives, volumeSize, Uminus);
					if ((derivativeMag > Gplus.norm()) && (derivativeMag > Gminus.norm()))
					{
						*(maximal + xx + yy*yStep + zz*zStep) = 255;
					}
				}
		tend = time(0);
		std::cout << "Done non-maximal supression, time elipsed = " << difftime(tend, tstart) << std::endl;
	}

	//Performing hysteris Thresholding
	std::vector<IndexType> D;
	std::queue<IndexType> Db;
	unsigned char *CannyEdge = new unsigned char[d*h*w];
	std::fill_n(CannyEdge, d*h*w, 0);
	{
		tstart = time(0);

		std::queue<IndexType> indexQueue;
		for (int zz = 0; zz < d; zz++)
			for (int yy = 0; yy < h; yy++)
				for (int xx = 0; xx < w; xx++)
				{
					if ((*(maximal + xx + yy*yStep + zz*zStep) == 255) && (*(GradientMag + xx + yy*yStep + zz *zStep) >= UpperThreshold))
					{
						indexQueue.push(Eigen::Vector3i(xx, yy, zz));
						*(CannyEdge + xx + yy*yStep + zz*zStep) = 255;
						D.push_back(Eigen::Vector3i(xx, yy, zz));
						Db.push(Eigen::Vector3i(xx, yy, zz));
					}
					else
						*(CannyEdge + xx + yy*yStep + zz*zStep) = 0;
				}
		while (!indexQueue.empty())
		{
			IndexType index = indexQueue.front();
			indexQueue.pop();
			
			for (int zz = index[2] - 1; zz <= index[2]+1; zz++)
				for (int yy = index[1] - 1; yy <= index[1]+1; yy++)
					for (int xx = index[0] - 1; xx <= index[0]+1; xx++)
					{
						if (xx < 0 || xx >= w || yy < 0 || yy >= h || zz < 0 || zz >= d)
							continue;
						
						if ((*(GradientMag + xx + yy*yStep + zz*zStep) >= LowerThreshold) && (*(CannyEdge + xx + yy*yStep + zz*zStep) == 0) && (*(maximal + xx + yy*yStep + zz*zStep) == 255))
						{
							indexQueue.push(Eigen::Vector3i(xx, yy, zz));
							*(CannyEdge + xx + yy*yStep + zz*zStep) = 255;
							D.push_back(Eigen::Vector3i(xx, yy, zz));
							Db.push(Eigen::Vector3i(xx, yy, zz));
						}
					
					}
		}
	}
	tend = time(0);
	delete[] GradientMag;
	std::cout << "Done Hysteris thresholding, time elipsed = " << difftime(tend, tstart) << std::endl;
	
	tstart = time(0);
	int count = 0;
	while (count < 10)
	{
		std::queue<IndexType> DbNew;
		while (!Db.empty())
		{
			Eigen::Vector3i index = Db.front();
			Db.pop();
			for (int zz = index[2] - 1; zz <= index[2] + 1; zz++)
				for (int yy = index[1] - 1; yy <= index[1] + 1; yy++)
					for (int xx = index[0] - 1; xx < index[0] + 1; xx++)
					{
						if ((xx < 0) || (xx >= w) || (yy < 0) || (yy >= h) || (zz < 0) || (zz >= d))
							continue;
						if (*(CannyEdge + xx + yy*yStep + zz*zStep) != 255 && *(normalizedGradient + xx + yy*yStep + zz*zStep) > NormalizedThreshold)
						{
							*(CannyEdge + xx + yy*yStep + zz*zStep) = 255;
							D.push_back(Eigen::Vector3i(xx, yy, zz));
							DbNew.push(Eigen::Vector3i(xx, yy, zz));
						}
					}
		}
		Db = DbNew;
		count++;
	}
	tend = time(0);
	std::cout << "Done Creeping Canny Edge, time elapsed = " << difftime(tend, tstart) << std::endl;
	std::queue<IndexType>().swap(Db);
	delete[] maximal;
	delete[] normalizedGradient;

	//Labelling certain regions
	unsigned char* Labels;
	{
		tstart = time(0);
		Labels = new unsigned char[w*h*d];
		std::fill_n(Labels, w*h*d, 0);
		for (int zz = 0; zz < d; zz++)
			for (int yy = 0; yy < h; yy++)
				for (int xx = 0; xx < w; xx++)
				{
					if ((xx <= 1) || (xx >= w - 2) || (yy <= 1) || (yy >= h - 2) || (zz <= 1) || (zz >= d - 2))
					{
						*(Labels + xx + yy*yStep + zz*zStep) = 1;
						continue;
					}
					if (*(CannyEdge + xx + yy*yStep + zz*zStep) != 255)
					{
						int i = regions.getRegion(*((unsigned short*)vol.mVolumeData + xx + yy*yStep + zz*zStep));
						if (i == -1)
						{
							D.push_back(IndexType(xx, yy, zz));
							*(CannyEdge + xx + yy*yStep + zz*zStep) = 255;
						}
						else
						{
							*(Labels + xx + yy*yStep + zz*zStep) = (1 << i);
						}
					}
				}
		for (int zz = 1; zz < d - 1; zz++)
			for (int yy = 1; yy < h - 1; yy++)
				for (int xx = 1; xx < w - 1; xx++)
				{
					unsigned char currentLabel = *(Labels + xx + yy*yStep + zz*zStep);
					if (currentLabel == 0)
						continue;
					unsigned int labelCount = 0; 
					
					for (int nx = xx - 1; nx <= xx + 1; nx = nx + 2)
						if (*(Labels + nx + yy*yStep + zz*zStep) == currentLabel)
							labelCount++;
					for (int ny = yy - 1; ny <= yy + 1; ny = ny + 2)
						if (*(Labels + xx + ny*yStep + zz*zStep) == currentLabel)
							labelCount++;
					for (int nz = zz - 1; nz <= zz + 1; nz = nz + 2)
						if (*(Labels + xx + yy*yStep + nz*zStep) == currentLabel)
							labelCount++;
					
					if (labelCount == 0)
					{
						*(Labels + xx + yy*yStep + zz*zStep) = 0;
						D.push_back(Eigen::Vector3i(xx, yy, zz));
						*(CannyEdge + xx + yy*yStep + zz*zStep) = 255;
					}
				}
		tend = time(0);
		std::cout << "Done labelling certain region, time elapsed = " << difftime(tend, tstart) << endl;
	}
	delete[] CannyEdge;

	//Label Propagation without Overlaping
	{
		tstart = time(0);
		std::vector<IndexType> Unmarked(D);
		unsigned int count = 0;
		while (1)
		{
			std::vector<IndexType> NewUmarked;
			std::vector<ValueChange> ChangeList;

			//#pragma omp parallel for
			for (long long int k = 0; k < Unmarked.size(); k++)
			{
				IndexType CurrentIndex = Unmarked[k];
				unsigned char temp = *(Labels + CurrentIndex[0] + CurrentIndex[1] * yStep + CurrentIndex[2] * zStep);

				for (int nx = -1; nx <= 1; nx = nx+2)
				temp |= *(Labels + (CurrentIndex[0] + nx) + CurrentIndex[1] * yStep + CurrentIndex[2] * zStep);
				for (int ny = -1; ny <= 1; ny = ny + 2)
				temp |= *(Labels + CurrentIndex[0] + (CurrentIndex[1] + ny)*yStep + CurrentIndex[2] * zStep);
				for (int nz = -1; nz <= 1; nz = nz + 2)
				temp |= *(Labels + CurrentIndex[0] + CurrentIndex[1] * yStep + (CurrentIndex[2] + nz)*zStep);

				/*for (int nz = -1; nz <= 1; nz++)
					for (int ny = -1; ny <= 1; ny++)
						for (int nx = -1; nx <= 1; nx++)
							temp |= *(Labels + (CurrentIndex[0] + nx) + (CurrentIndex[1] + ny)*yStep + (CurrentIndex[2] + nz)*zStep);*/
				{
					//#pragma omp critical 
					if (__popcnt(temp) == 0)
						NewUmarked.push_back(CurrentIndex);
					else
						ChangeList.push_back(ValueChange(CurrentIndex, temp));

				}
			}

			//#pragma omp parallel for
			for (long long int k = 0; k < ChangeList.size(); k++)
			{
				ValueChange temp = ChangeList[k];
				*(Labels + temp.index[0] + temp.index[1] * yStep + temp.index[2] * zStep) = temp.value;
			}
			if (NewUmarked.empty())
				break;
			std::vector<IndexType>().swap(Unmarked);
			std::vector<ValueChange>().swap(ChangeList);
			Unmarked = NewUmarked;
			count++;
		}
		tend = time(0);
		cout << "Done Label Propagation without Overlapping, time elapsed = " << difftime(tend, tstart) << endl;
	}

	//Labels Propagtion with overlapping
	{
		tstart = time(0);
		std::vector<IndexType> Unmarked(D);
		unsigned long long queueLenght = D.size();
		unsigned int count = 0;
		while (1)
		{
			std::vector<IndexType> NewUnmarked;
			std::vector<ValueChange> ChangeList;
			unsigned long long tempCount = 0;

			//#pragma omp parallel for 
			for (long long int k = 0; k < Unmarked.size(); k++)
			{
				IndexType CurrentIndex = Unmarked[k];
				unsigned char temp = *(Labels + CurrentIndex[0] + CurrentIndex[1] * yStep + CurrentIndex[2] * zStep);

				for (int nx = -1; nx <= 1; nx = nx + 2)
				temp |= *(Labels + (CurrentIndex[0] + nx) + CurrentIndex[1] * yStep + CurrentIndex[2] * zStep);
				for (int ny = -1; ny <= 1; ny = ny + 2)
				temp |= *(Labels + CurrentIndex[0] + (CurrentIndex[1] + ny)*yStep + CurrentIndex[2] * zStep);
				for (int nz = -1; nz <= 1; nz = nz + 2)
				temp |= *(Labels + CurrentIndex[0] + CurrentIndex[1] * yStep + (CurrentIndex[2] + nz)*zStep);

				/*for (int nz = -1; nz <= 1; nz++)
					for (int ny = -1; ny <= 1; ny++)
						for (int nx = -1; nx <= 1; nx++)
							temp |= *(Labels + (CurrentIndex[0] + nx) + (CurrentIndex[1] + ny)*yStep + (CurrentIndex[2] + nz)*zStep);*/

				{
					//#pragma omp critical 
					if (__popcnt(temp) == 1)
					{
						NewUnmarked.push_back(CurrentIndex);
						tempCount++;
					}
					else
						ChangeList.push_back(ValueChange(CurrentIndex, temp));

				}
			}

			for (long long int k = 0; k < ChangeList.size(); k++)
			{
				ValueChange temp = ChangeList[k];
				*(Labels + temp.index[0] + temp.index[1] * yStep + temp.index[2] * zStep) = temp.value;
			}
			if (tempCount == queueLenght)
				break;
			std::vector<IndexType>().swap(Unmarked);
			std::vector<ValueChange>().swap(ChangeList);
			queueLenght = tempCount;
			Unmarked = NewUnmarked;
			count++;
		}
		tend = time(0);
		cout << "Done Label Propagation with Overlapping, time elapsed = " << difftime(tend, tstart) << endl;
	}

	//Thresholding the labels
	tstart = time(0);
	unsigned char *Segmentation = new unsigned char[vol.mDepth*vol.mHeight*vol.mWidth];
	for (int zz = 0; zz < vol.mDepth; zz++)
		for (int yy = 0; yy < vol.mHeight; yy++)
			for (int xx = 0; xx < vol.mWidth; xx++)
			{
				int i = -1;
				unsigned char f = *(Labels + xx + yy*yStep + zz*zStep);
				InternalPixelType iso, g = *((InternalPixelType*)vol.mVolumeData + xx + yy * yStep + zz * zStep);
				for (int k = 0; k < num_labels; k++)
				{
					if ((f >> k) & 1)
					{
						if (i == -1)
							i = k;
						else
						{
							iso = (M.regions[i].peak + M.regions[k].peak) / 2;
							if (g < iso)
								break;
							i = k;
						}
					}
				}
				*(Segmentation + xx + yy*yStep + zz*zStep) = i;
			}
	tend = time(0);
	cout << "Done Segmenation, time elapsed = " << difftime(tend, tstart) << endl;
	saveSlice<BinaryPixelType>(Segmentation, volumeSize, "Hemanth_Segmentation\\SegmentationOld");
	//saveSlice<BinaryPixelType>(Labels, volumeSize, "Hemanth_Labels\\\Labels");
	
	////Refining Segmentaion
	//{
	//	BinaryPixelType *visited = new BinaryPixelType[w*h*d];
	//	std::fill_n(visited, w*h*d, 0);
	//	std::vector<IndexType> boundry;
	//	count = 0;
	//	for (int zz = 1; zz < d - 1; zz++)
	//		for (int yy = 1; yy < h - 1; yy++)
	//			for (int xx = 1; xx < w - 1; xx++)
	//			{
	//				bool flag = false;
	//				BinaryPixelType currentSegmentation = *(Segmentation + xx + yy*yStep + zz*zStep);
	//				for (int dz = zz - 1; (dz <= zz + 1) && !flag; dz++)
	//					for (int dy = yy - 1; (dy <= yy + 1) && !flag; dy++)
	//						for (int dx = xx - 1; (dx <= xx + 1) && !flag; dx++)
	//						{
	//							if (currentSegmentation != *(Segmentation + dx + dy*yStep + dz*zStep))
	//							{
	//								boundry.push_back(IndexType(xx, yy, zz));
	//								flag = true;
	//							}
	//						}
	//			}

	//	while (true)
	//	{
	//		count++;
	//		std::cout << count << ":";
	//		if (count > 100)
	//			break;
	//		std::queue<ValueChange> changed;
	//		for (std::vector<IndexType>::iterator it = boundry.begin(); it != boundry.end(); it++)
	//		{
	//			
	//			IndexType currentIndex = *it;
	//			int xx = currentIndex[0], yy = currentIndex[1], zz = currentIndex[2];
	//			if ((xx < 1) || (xx >= w - 1) || (yy < 1) || (yy >= h - 1) || (zz < 1) || (zz >= d - 1))
	//			{
	//				continue;
	//			}
	//			BinaryPixelType temp = MaxLogHMRFLikelihood(xx,yy,zz,yStep,zStep,vol,Segmentation,regions,2);
	//			if (temp != *(Segmentation + xx + yy*yStep + zz*zStep))
	//			{
	//				changed.push(ValueChange(IndexType(xx, yy, zz), temp));
	//			}
	//		}
	//		std::cout << changed.size() << std::endl;
	//		if (changed.empty())
	//			break;
	//		std::vector<IndexType>().swap(boundry);
	//		while (!changed.empty())
	//		{
	//			ValueChange temp = changed.front();
	//			changed.pop();
	//			int xx = temp.index[0], yy = temp.index[1], zz = temp.index[2];
	//			*(Segmentation + xx + yy*yStep + zz*zStep) = temp.value;
	//			for (int dx = xx - 1; dx <= xx + 1; dx++)
	//				for (int dy = yy - 1; dy <= yy + 1; dy++)
	//					for (int dz = zz - 1; dz <= zz + 1; dz++)
	//					{
	//						if (dx == xx && dy == yy && dz == zz)
	//							continue;
	//						if (*(visited + dx + dy*yStep + dz*zStep) == 0)
	//						{
	//							*(visited + dx + dy*yStep + dz*zStep) = std::numeric_limits<BinaryPixelType>::max();
	//							boundry.push_back(IndexType(dx, dy, dz));
	//						}
	//					}
	//		}
	//		for (std::vector<IndexType>::iterator it = boundry.begin(); it != boundry.end(); it++)
	//		{
	//			IndexType currentIndex = *it;
	//			*(visited + currentIndex[0] + currentIndex[1] * yStep + currentIndex[2] * zStep) = 0;
	//		}
	//	}
	//}
	//saveSlice<BinaryPixelType>(Segmentation, volumeSize, "Hemanth_Segmentation\\SegmentationNew");
	
	////Codes for rendering
	//vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
	//imageData->SetDimensions(volumeSize);
	//imageData->AllocateScalars(VTK_TYPE_UINT16, 1);
	//for (int xx = 0; xx < w; xx++)
	//	for (int yy = 0; yy < h; yy++)
	//		for (int zz = 0; zz < d; zz++)
	//		{
	//			unsigned short* pixel = static_cast<unsigned short*>(imageData->GetScalarPointer(xx, yy, zz));
	//			pixel[0] = *((unsigned short*)vol.mVolumeData + xx + yy*yStep + zz*zStep);
	//		}

	//vtkSmartPointer<vtkMarchingCubes> marchingCubes = vtkSmartPointer<vtkMarchingCubes>::New();
	//marchingCubes->SetInputData(imageData);
	//marchingCubes->SetValue(0, M.regions[1].lower_bound);
	//marchingCubes->Update();

	//std::vector<Eigen::Vector3f> vertices;
	//std::vector<unsigned int> indices;
	//imt::volume::OverlappedVoxelsMarchingCubes OverlapMarchingCubes;
	//OverlapMarchingCubes.computeAll(vol, Segmentation, num_labels, peaks, vertices, indices);
	//vtkSmartPointer<vtkPolyData> myMesh = tr::Display3DRoutines::displayMesh(vertices, indices);
	//std::vector<vtkSmartPointer<vtkPolyData>> meshes;
	//meshes.push_back(marchingCubes->GetOutput());
	//meshes.push_back(myMesh);
	//tr::Display3DRoutines::displayMultiPolyData(meshes);
	//
	//vtkSmartPointer<vtkPLYWriter> myMeshWriter = vtkSmartPointer<vtkPLYWriter>::New();
	//myMeshWriter->SetFileName("T2_R2_overlap.ply");
	//myMeshWriter->SetInputData(myMesh);
	//myMeshWriter->Update();
	//vtkSmartPointer<vtkPLYWriter> vtkMeshWriter = vtkSmartPointer<vtkPLYWriter>::New();
	//vtkMeshWriter->SetFileName("T2_R2_Marchingcubes.ply");
	//vtkMeshWriter->SetInputData(marchingCubes->GetOutput());
	//vtkMeshWriter->Update();

	//
	//for (int i = 1; i < num_labels; i++)
	//{
	//	char buffer1[50], buffer2[50];
	//	marchingCubes->SetValue(0, M.regions[i].lower_bound);
	//	marchingCubes->Update();
	//	sprintf(buffer1, "T2_R2_Marchingcubes_%d.ply", i);
	//	vtkMeshWriter->SetFileName(buffer1); 
	//	vtkMeshWriter->SetInputData(marchingCubes->GetOutput());
	//	vtkMeshWriter->Update();

	//	std::vector<Eigen::Vector3f>().swap(vertices);
	//	std::vector<unsigned int>().swap(indices);
	//	OverlapMarchingCubes.compute(vol, Segmentation, i, num_labels, peaks, vertices, indices);
	//	sprintf(buffer2, "T2_R2_overlap_%d.ply", i);
	//	myMeshWriter->SetFileName(buffer2);
	//	myMeshWriter->SetInputData(tr::Display3DRoutines::displayMesh(vertices, indices));
	//	myMeshWriter->Update();
	//}

	return EXIT_SUCCESS;
}