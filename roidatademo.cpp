
#include "vtkImageData.h"
#include "vtkMarchingCubes.h"
#include "iostream"
#include "rawvolumedataio.h"
#include "volumeinfo.h"
#include "histogramfilter.h"
#include "display3droutines.h"
#include "vtkCellData.h"

struct RotatedRoiData
{
	int64_t _Resolution;
	double _RoiWMM, _RoiHMM, _RoiDMM; //width , height and depth in milimeters
	int64_t _AxisAlignedW, _AxisAlignedH, _AxisAlignedD;

	int64_t _StartX, _StartY, _StartZ;

	Eigen::Vector3d _StartLocationMM;
	Eigen::Vector3d _XAxis, _YAxis, _ZAxis;


	CPUBuffer* _GrayValueBuffer, * _BinaryMaskBuffer;



};

void viewIsoSurface(imt::volume::VolumeInfo& volume, int isoVal);

void createCuboidCage(const Eigen::Vector3d& origin, const Eigen::Vector3d& xAxis, const Eigen::Vector3d& yAxis,
	const Eigen::Vector3d& zAxis, double w, double h, double d,
	vtkSmartPointer<vtkPoints> points, vtkSmartPointer<vtkPolyData> wireframe,
	vtkSmartPointer<vtkUnsignedCharArray> wireColors, int& pointId, const Eigen::Vector3f& color);


void viewIsoSurface(int w, int h, int d, unsigned short* grayValues, int isoVal);

int main(int argc, char** argv)
{


	QString dataPath = "D:\\Data\\alu_training_bhc_bin 2019-4-30 12-46-28.uint16_scv";//"C:\\Test Data\\Aluminum_diff_size\\Al-Teil_220kV_2272A_Sn3_133ms_8x_1700_Vx180 2019-4-8 21-9-47.uint16_scv";// "C:/projects/Wallthickness/data/CT multi/Calliper Assy 2016-6-9 10-12.uint16_scv"; //;

	imt::volume::RawVolumeDataIO io;

	imt::volume::VolumeInfo volInfo;

	//io.readHeader(dataPath, volInfo);

	io.readUint16SCV(dataPath, volInfo);

	imt::volume::HistogramFilter filter(&volInfo);

	std::vector< int64_t > histogram(std::numeric_limits< unsigned short >::max(), 0);

	unsigned short* vData = (unsigned short*)volInfo.mVolumeData;

	for (int64_t zz = 0; zz < volInfo.mDepth; zz++)
		for (int64_t yy = 0; yy < volInfo.mHeight; yy++)
			for (int64_t xx = 0; xx < volInfo.mWidth; xx++)
			{
				histogram[vData[zz * volInfo.mWidth * volInfo.mHeight + yy * volInfo.mWidth + xx]] += 1;
			}


	long maxHistogramVal = *(std::max_element(histogram.begin(), histogram.end()));

	long iso50Th = filter.ISO50Threshold(histogram); 	//fraunhoufferThreshold(volInfo.mWidth, volInfo.mHeight, volInfo.mDepth,
	
	//viewIsoSurface(volInfo, iso50Th);

	std::cout << " iso 50 threshold : " << iso50Th << std::endl;

	std::cout << volInfo.mWidth << " " << volInfo.mHeight << " " << volInfo.mHeight << std::endl;

	std::cout << " voxel step : " << volInfo.mVoxelStep.transpose() << std::endl;

	FILE* file = NULL;

	fopen_s(&file, "D:\\Data\\roidata.dat", "rb");

	RotatedRoiData roiData;

	fread(&roiData, sizeof(roiData), 1, file);

	unsigned short* grayBuffer = new unsigned short[ roiData._AxisAlignedW * roiData._AxisAlignedH * roiData._AxisAlignedD ];
	unsigned char* maskBuffer = new unsigned char[roiData._AxisAlignedW * roiData._AxisAlignedH * roiData._AxisAlignedD];

	int64_t requiredMaskBufferSize = roiData._AxisAlignedW * roiData._AxisAlignedH * roiData._AxisAlignedD;
	
	fread(grayBuffer, sizeof(unsigned short), requiredMaskBufferSize, file);
	fread(maskBuffer, sizeof(unsigned char), requiredMaskBufferSize, file);

	fclose(file);

	viewIsoSurface(roiData._AxisAlignedW, roiData._AxisAlignedH, roiData._AxisAlignedD, grayBuffer, iso50Th);

	std::cout << roiData._AxisAlignedW << " " << roiData._AxisAlignedH << " " << roiData._AxisAlignedD << std::endl;

	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkPolyData> wireframe = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkUnsignedCharArray> wireColors = vtkSmartPointer<vtkUnsignedCharArray>::New();

	points->Allocate(1000);
	wireframe->Allocate(1000);

	int pointId = 0;

	std::cout << volInfo.mWidth << " " << volInfo.mHeight << " " << volInfo.mDepth << std::endl;

	createCuboidCage( Eigen::Vector3d(0, 0, 0), Eigen::Vector3d(1, 0, 0), Eigen::Vector3d(0, 1, 0),
		              Eigen::Vector3d(0, 0, 1), volInfo.mWidth, volInfo.mHeight, volInfo.mDepth, points, 
		              wireframe, wireColors, pointId, Eigen::Vector3f(0, 1, 0));

	wireframe->GetCellData()->SetScalars(wireColors);
	wireframe->SetPoints(points);
	
	

	tr::Display3DRoutines::displayPolyData(wireframe);


	return 0;
}


void viewIsoSurface(imt::volume::VolumeInfo& volume, int isoVal)
{
	vtkSmartPointer< vtkImageData > volumeData = vtkSmartPointer< vtkImageData >::New();


	//volumeData->SetOrigin(volume.mVolumeOrigin(0), volume.mVolumeOrigin(1), volume.mVolumeOrigin(2));
	//volumeData->SetSpacing(volume.mVoxelStep(0), volume.mVoxelStep(1), volume.mVoxelStep(2));
	volumeData->SetDimensions(volume.mWidth, volume.mHeight, volume.mDepth);
	volumeData->SetExtent(0, volume.mWidth - 1, 0, volume.mHeight - 1, 0, volume.mDepth - 1);
	volumeData->AllocateScalars(VTK_UNSIGNED_SHORT, 1);

	long int nPoints = volumeData->GetNumberOfPoints();


	long int zStep = volume.mHeight * volume.mWidth;
	long int yStep = volume.mWidth;

	unsigned char* ptr = (unsigned char*)volumeData->GetScalarPointer();

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


void viewIsoSurface(int w , int h , int d , unsigned short* grayValues, int isoVal )
{
	vtkSmartPointer< vtkImageData > volumeData = vtkSmartPointer< vtkImageData >::New();

	volumeData->SetDimensions(w, h, d);
	volumeData->SetExtent(0, w - 1, 0, h - 1, 0, d - 1);
	volumeData->AllocateScalars(VTK_UNSIGNED_SHORT, 1);

	long int nPoints = volumeData->GetNumberOfPoints();


	long int zStep = h * w;
	long int yStep = w;

	unsigned char* ptr = (unsigned char*)volumeData->GetScalarPointer();

	memcpy(ptr, grayValues, w * h * d * 2);

	vtkSmartPointer< vtkMarchingCubes > marchingCubes = vtkSmartPointer< vtkMarchingCubes >::New();

	marchingCubes->SetInputData(volumeData);

	marchingCubes->SetValue(0, isoVal);

	marchingCubes->ComputeNormalsOn();

	marchingCubes->Update(0);

	vtkSmartPointer< vtkPolyData > isoSurface = vtkSmartPointer< vtkPolyData >::New();

	isoSurface->DeepCopy(marchingCubes->GetOutput());

	tr::Display3DRoutines::displayPolyData(isoSurface);

}


void createCuboidCage(const Eigen::Vector3d& origin, const Eigen::Vector3d& xAxis, const Eigen::Vector3d& yAxis,
	const Eigen::Vector3d& zAxis, double w, double h, double d,
	vtkSmartPointer<vtkPoints> points, vtkSmartPointer<vtkPolyData> wireframe,
	vtkSmartPointer<vtkUnsignedCharArray> wireColors, int& pointId, const Eigen::Vector3f& color)
{
	Eigen::Vector3d corners[8];

	corners[0] = origin;
	corners[1] = origin + xAxis * w;
	corners[2] = origin + xAxis * w + yAxis * h;
	corners[3] = origin + yAxis * h;

	corners[4] = origin + d * zAxis;
	corners[5] = origin + xAxis * w + d * zAxis;
	corners[6] = origin + xAxis * w + yAxis * h + d * zAxis;
	corners[7] = origin + yAxis * h + d * zAxis;

	for (int ii = 0; ii < 8; ii++)
	{
		points->InsertNextPoint(corners[ii].data());
	}

	vtkSmartPointer<vtkIdList> edge = vtkSmartPointer<vtkIdList>::New();

	unsigned char colorVal[] = { color(0) * 255.0 , color(1) * 255.0 , color(2) * 255.0 };



	for (int ii = 0; ii < 4; ii++)
	{
		//int id = ii + pointId;

		wireColors->InsertNextTypedTuple(colorVal);
		wireColors->InsertNextTypedTuple(colorVal);
		wireColors->InsertNextTypedTuple(colorVal);

		edge->InsertNextId(ii + pointId);
		edge->InsertNextId((ii + 1) % 4 + pointId);

		wireframe->InsertNextCell(VTK_LINE, edge);

		edge->Reset();

		edge->InsertNextId(ii + 4 + pointId);
		edge->InsertNextId((ii + 1) % 4 + 4 + pointId);

		wireframe->InsertNextCell(VTK_LINE, edge);

		edge->Reset();

		edge->InsertNextId(ii + pointId);
		edge->InsertNextId(ii + 4 + pointId);

		wireframe->InsertNextCell(VTK_LINE, edge);

		edge->Reset();
	}


	pointId += 8;
}

