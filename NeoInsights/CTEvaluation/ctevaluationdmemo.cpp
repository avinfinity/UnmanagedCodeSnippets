#include "iostream"
extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }

//extern "C" {
//	FILE _iob[3] = { __iob_func()[0], __iob_func()[1], __iob_func()[2] };
//}

#include "NeoInsights/CTEvaluation/CTPointCloudEvaluation.h"
#include "NeoInsights/CTEvaluation/CTPointCloudEvaluationDecorator.h"
#include "display3droutines.h"
#include "eigenincludes.h"
#include "vtkImageData.h"
#include "vtkMarchingCubes.h"
#include "display3droutines.h"
//#include "CTProfilsEvaluation2.h"
//#include "CTPointCloudEvaluation2.h"
#include "MultiResolutionMarchingCubes.h"
#include "multiresmarchingcubes.h"
#include "MultiResolutionMarchingCubes.h"
#include "rawvolumedataio.h"
#include "histogramfilter.h"

#include "CTPointCloudEvaluationCUDA.h"

#include "include/ipp.h"

void readVolumeParameters();

void viewData(std::string dataPath, std::vector< Eigen::Vector3f >& dataPoints);

void readVolume(std::string dataPath, unsigned int& width, unsigned int& height, unsigned int& depth,Eigen::Vector3f& origin , Eigen::Vector3f& voxelSize , std::vector< unsigned short  >& volumeData)
{
	std::fstream reader;
	reader.open(dataPath, std::ios::in|std::ios::binary);


	unsigned int w = 0, h = 0, d = 0;

	reader.read((char*)&w, sizeof(unsigned int));
	reader.read((char*)&h, sizeof(unsigned int));
	reader.read((char*)&d, sizeof(unsigned int));

	reader.read((char*)origin.data(), sizeof(float) * 3);
	reader.read((char*)voxelSize.data(), sizeof(float) * 3);

	std::cout << w << " " << h << " " << d << std::endl;

	volumeData.resize(w * h * d);
	reader.read((char*)volumeData.data() , volumeData.size() * sizeof( unsigned short) );
	reader.close();

	width = w;
	height = h;
	depth = d;

}

void extractSurfaceOnVolume( Eigen::Vector3f& origin , Eigen::Vector3f& voxelSize , std::vector< unsigned short >& volume , int w , int h ,int d  )
{
	vtkSmartPointer< vtkImageData > volumeData = vtkSmartPointer< vtkImageData >::New();


	volumeData->SetOrigin(origin(0), origin(1), origin(2));
	volumeData->SetSpacing(voxelSize(0), voxelSize(1), voxelSize(2));
	volumeData->SetDimensions(w, h, d);
	volumeData->AllocateScalars(VTK_UNSIGNED_SHORT, 1);

	long int nPoints = volumeData->GetNumberOfPoints();


	long int zStep = h * w;
	long int yStep = w;

	unsigned char *ptr = (unsigned char *)volumeData->GetScalarPointer();

	std::cout << w * h * d << " " << volume.size() << std::endl;

	memcpy(ptr, volume.data(), w * h * d * 2);

	vtkSmartPointer< vtkMarchingCubes > marchingCubes = vtkSmartPointer< vtkMarchingCubes >::New();

	marchingCubes->SetInputData(volumeData);

	marchingCubes->SetValue(0, 31818);

	marchingCubes->ComputeNormalsOn();

	marchingCubes->Update(0);

	vtkSmartPointer< vtkPolyData > isoSurface = vtkSmartPointer< vtkPolyData >::New();

	isoSurface->DeepCopy(marchingCubes->GetOutput());

	std::cout << isoSurface->GetNumberOfCells() << std::endl;

	tr::Display3DRoutines::displayPolyData(isoSurface);

}


void ippsConv( const float* pSrc1, int src1Len, const float* pSrc2, int src2Len, float* pDst)
{
	int dstLen = src1Len + src2Len - 1;

	for (int ll = 0; ll < dstLen; ll++)
	{
		float conv = 0;

		int start = std::max( 0 , ll - src2Len + 1);
		int end = std::min(ll, src1Len - 1);

		for (int kk = start; kk <= end; kk++)
		{
			int p = ll - kk;

			conv += pSrc1[kk] * pSrc2[ll - kk];
		}

		pDst[ll] = conv;
	}

}

void convolutionTest()
{
	float x[5] = { -2,0,1,-1,3 }, h[3] = { 0.5,0.5,0.2 }, y[7];
	IppStatus st = ippsConv_32f(x, 5, h, 3, y);
	//printf_16s(“conv = ”, y, 6, st);

	std::cout << y[0] << " " << y[1] << " " << y[2] << " " << y[3] << " " << y[4] << " " << y[5]<<" "<<y[6] << std::endl;

	ippsConv(x, 5, h, 3, y);

	std::cout << y[0] << " " << y[1] << " " << y[2] << " " << y[3] << " " << y[4] << " " << y[5]<<" "<<y[6] << std::endl;
}



void visualizeMesh()
{

	std::string meshPath = "G:\\Projects\\NeoInsights\\NEO insights 2.0.00\\bin\\x64\\Release\\Test Pice 2017-9-7 10-16.uint16_scv";//"cube.uint16_scv";// "// "Test_Kupplung_1GB_Vx64µm 2015-6-2 9-10.uint16_scv";//"Test_Kupplung_4GB_Vx41 2015-6-2 13-46.uint16_scv";

	FILE *file = fopen(meshPath.c_str(), "rb");

	std::vector<double> vertices;
	std::vector<size_t> indices; 

	int nVertices , nIndices;


	fread(&nVertices, sizeof(int), 1, file);
	fread(&nIndices, sizeof(int), 1, file);

	vertices.resize(nVertices * 3);
	indices.resize(nIndices);

	fread(vertices.data(), sizeof(double) * 3, (size_t)nVertices, file);
	fread(indices.data(), sizeof(std::size_t), (size_t)nIndices, file);

	fclose(file);


	std::vector<Eigen::Vector3f> visualizationVertices( nVertices );
	std::vector< unsigned int > visualizationIndices( nIndices );

	for (int vv = 0; vv < nVertices; vv++)
	{
		visualizationVertices[vv](0) = vertices[3 * vv];
		visualizationVertices[vv](1) = vertices[3 * vv + 1];
		visualizationVertices[vv](2) = vertices[3 * vv + 2];
	}

	for (int ii = 0; ii < nIndices; ii++)
	{
		visualizationIndices[ii] = indices[ii];
	}
	
	tr::Display3DRoutines::displayMesh(visualizationVertices, visualizationIndices);
}



int main( int argc , char **argv )
{

	//visualizeMesh();

	//return 0;

	//convolutionTest();

	//return 0;

	//testNormals();

	QString volumeFilePath = "C:/Data/Kupplung_4GB/Test_Kupplung_4GB_Vx41 2015-6-2 13-46.uint16_scv";//"G:/Data/bottle_01 2016-7-6 17-14.uint16_scv"; //"G:\\Projects\\Data\\Connector\\Connector_1.uint16_scv";//"G:\\Data\\separated_part_0.uint16_scv";//
	//"G:\\Projects\\Data\\Kupplung\\VV_Test_Kupplung_7GB_Vx31 2015-6-19 9-16.uint16_scv";
	//"G:/Data/separated_part_0.uint16_scv";
	////"G:\\Data\\07.06.17\\Test Pice_BHC 2017-9-7 12-40.uint16_scv";
	////"G:/Projects/Data/CADCube_10GB/Test cube_10gb_BHC 2018-1-25 9-51-35.uint16_scv";// ////;


	//first load the volume
	imt::volume::RawVolumeDataIO io;

	imt::volume::VolumeInfo volInfo;

	io.readUint16SCV(volumeFilePath, volInfo);

	volInfo.mZStep = volInfo.mWidth * volInfo.mHeight;
	volInfo.mYStep = volInfo.mWidth;

	volInfo.mVolumeDataU16 = (unsigned short*)volInfo.mVolumeData;

	imt::volume::HistogramFilter filter(&volInfo);

	std::vector< int64_t > histogram(std::numeric_limits< unsigned short >::max(), 0);

	unsigned short *vData = (unsigned short*)volInfo.mVolumeData;

	for (int64_t zz = 0; zz < volInfo.mDepth; zz++)
		for (int64_t yy = 0; yy < volInfo.mHeight; yy++)
			for (int64_t xx = 0; xx < volInfo.mWidth; xx++)
			{
				histogram[vData[zz * volInfo.mWidth * volInfo.mHeight + yy * volInfo.mWidth + xx]] += 1;
			}


	long maxHistogramVal = *(std::max_element(histogram.begin(), histogram.end()));

	long iso50Th = filter.fraunhoufferThreshold(volInfo.mWidth, volInfo.mHeight, volInfo.mDepth, volInfo.mVoxelStep(0), 
		volInfo.mVoxelStep(1), volInfo.mVoxelStep(2), (unsigned short*)volInfo.mVolumeData); //filter.ISO50Threshold(histogram);

	std::cout << " iso 50 threshold : " << iso50Th << std::endl;

	double coeff = 1024.0 * 1024.0 * 1024.0 / ((double)volInfo.mWidth * (double)volInfo.mHeight * (double)volInfo.mDepth * 2.0);

	coeff = std::pow(coeff, 0.3333333);

	std::cout << " volume reduction coefficient : " << coeff << std::endl;

	imt::volume::MultiResMarchingCubes mrcm(volInfo, iso50Th, 2);
	
	Zeiss::IMT::NG::NeoInsights::Volume::MeshGenerator::MultiResolutionMarchingCubes marchingCubes((unsigned short*)volInfo.mVolumeData, volInfo.mWidth,
		volInfo.mHeight, volInfo.mDepth, volInfo.mVoxelStep(0), volInfo.mVoxelStep(1), volInfo.mVoxelStep(2));

	std::vector<double> vertices, vertexNormals;
	std::vector<unsigned int> surfaceIndices;


	double initT = cv::getTickCount();

	marchingCubes.compute(iso50Th, vertices, vertexNormals, surfaceIndices);
	
	std::vector<Eigen::Vector3f> visualizationVertices, visualizationNormals;

	int numVertices = vertices.size() / 3;

	visualizationVertices.resize(numVertices);
	visualizationNormals.resize(numVertices);

	for (int vv = 0; vv < numVertices; vv++)
	{
		visualizationVertices[vv](0) = vertices[3 * vv];
		visualizationVertices[vv](1) = vertices[3 * vv + 1];
		visualizationVertices[vv](2) = vertices[3 * vv + 2];

		visualizationNormals[vv](0) = vertexNormals[3 * vv];
		visualizationNormals[vv](1) = vertexNormals[3 * vv + 1];
		visualizationNormals[vv](2) = vertexNormals[3 * vv + 2];
	}

	tr::Display3DRoutines::displayMesh(visualizationVertices, surfaceIndices);

	bool ConsiderStaticThreshold = true;//false;
	bool UseAutoParams = true;
	double GradientThreshold = 0.05;
	double Sigma = 1.5;

	int InterpolationMethod = IPPI_INTER_LINEAR;
	bool UseAngleCriterium = true;
	double AngleCriterium = 45;//0.25 * M_PI;

	unsigned long long QualityThreshold = 0;

	bool UseBeamHardeningCorrection = false;// true;

	double gradThreshold = 0;

	void** data = new void*[volInfo.mDepth];

	for (long i = 0; i < volInfo.mDepth; ++i)
	{
		data[i] = (((unsigned short*)volInfo.mVolumeData) + ((int64)i) * volInfo.mWidth * volInfo.mHeight); 
	}




	std::cout << " number of visualization of vertices : " << visualizationVertices.size() << std::endl;


	imt::volume::cuda::CTPointCloudEvaluationCUDA pointCloudEvaluationCUDA( volInfo , visualizationVertices , visualizationNormals , surfaceIndices) ;


	pointCloudEvaluationCUDA.compute();

	return 0;

	//CCTPointCloudEvaluationDecorator *cCTPointCloudEvaluationAlgo = new CCTPointCloudEvaluationDecorator();

	//l3d volumeSize = { volInfo.mWidth , volInfo.mHeight , volInfo.mDepth };
	//d3d voxelSize = { volInfo.mVoxelStep(0) , volInfo.mVoxelStep(1) , volInfo.mVoxelStep(2) };

	//int nVertices = vertices.size() / 3;

	//for (int vv = 0; vv < 3 * nVertices; vv++)
	//{
	//	vertices[vv] /= volInfo.mVoxelStep(0);

	//	vertexNormals[vv] *= -1;
	//}

	//double SearchRange = 10.0 * volInfo.mVoxelStep(0);
	//double SearchRangeAirSide = 10.0 * volInfo.mVoxelStep(0);

	////cCTPointCloudEvaluationAlgo->SwitchVoxelVolume(data.c_ptr(), bitDepth, size);

	//std::vector<double> refinedPoints(3 * nVertices), refinedNormals(3 * nVertices), qualities(nVertices);

	//QualityThreshold = nVertices * 2;

	//std::cout << "number of vertices : " << nVertices << std::endl;

	//d3d* v1 = (d3d*)vertices.data();

	//int i = 1200;
	//std::cout << v1[i][0] << " " << v1[i][1] << " " << v1[i][2] << " " << vertices[3 * i] << " " << vertices[3 * i + 1] << " " << vertices[3 * i + 2] << std::endl;

	//initT = cv::getTickCount();

	//long result = cCTPointCloudEvaluationAlgo->DoMeasureData(
	//	data,16 , volumeSize , voxelSize , InterpolationMethod,
	//	UseAutoParams , GradientThreshold, Sigma,
	//	UseBeamHardeningCorrection,
	//	ConsiderStaticThreshold, iso50Th,
	//	UseAngleCriterium, AngleCriterium,
	//	static_cast<unsigned long long>(QualityThreshold),
	//	SearchRange, SearchRangeAirSide, nVertices, (d3d*)vertices.data(), 
	//	(d3d*)vertexNormals.data(), (d3d*)refinedPoints.data(), (d3d*)refinedNormals.data(), qualities.data());

	//std::cout << "time spent in vertex refinement " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;

	//for (int vv = 0; vv < numVertices; vv++)
	//{
	//	if (qualities[vv] > 0.001)
	//	{
	//		visualizationVertices[vv](0) = refinedPoints[3 * vv];
	//		visualizationVertices[vv](1) = refinedPoints[3 * vv + 1];
	//		visualizationVertices[vv](2) = refinedPoints[3 * vv + 2];

	//		visualizationNormals[vv](0) = refinedNormals[3 * vv];
	//		visualizationNormals[vv](1) = refinedNormals[3 * vv + 1];
	//		visualizationNormals[vv](2) = refinedNormals[3 * vv + 2];
	//	}
	//	else
	//	{
	//		visualizationVertices[vv](0) = vertices[3 * vv];
	//		visualizationVertices[vv](1) = vertices[3 * vv + 1];
	//		visualizationVertices[vv](2) = vertices[3 * vv + 2];

	//		visualizationNormals[vv](0) = vertexNormals[3 * vv];
	//		visualizationNormals[vv](1) = vertexNormals[3 * vv + 1];
	//		visualizationNormals[vv](2) = vertexNormals[3 * vv + 2];
	//	}

	//}


	//tr::Display3DRoutines::displayMesh(visualizationVertices, surfaceIndices);

#if 0
	readVolumeParameters();

	return 0;

	std::string volumeDataPath = "C:/projects/DebugData/vol.dat";

	unsigned int width = 0, height = 0, depth = 0;

	std::vector< unsigned short > volumeData;

	Eigen::Vector3f origin, voxelSize;

	readVolume( volumeDataPath, width, height, depth, origin , voxelSize , volumeData );

	extractSurfaceOnVolume(origin, voxelSize, volumeData, width, height, depth);

	std::cout << " volume dimensions : " << width << " " << height << " " << depth << std::endl;
	
	std::cout << " test application ct evaluation " << std::endl;

	//CCTPointCloudEvaluation *evaluator = new CCTPointCloudEvaluation( )

	std::string nominalDataPath = "C:/projects/DebugData/nominalData.dat";
	std::string actualDataPath = "C:/projects/DebugData/actualData.dat";

	std::vector< Eigen::Vector3f > nominalPoints, actualPoints;

	viewData( nominalDataPath , nominalPoints);
	viewData( actualDataPath , actualPoints);

	std::vector< Eigen::Vector3f > nominalAndActualPoints , nominalAndActualColors;

	std::vector< Eigen::Vector3f > nominalColors( nominalPoints.size() , Eigen::Vector3f(1 , 0 , 0) ), actualColors( actualPoints.size() , Eigen::Vector3f( 0 , 1 , 0 ) );

	nominalAndActualPoints.insert(nominalAndActualPoints.end(), nominalPoints.begin(), nominalPoints.end());
	nominalAndActualPoints.insert(nominalAndActualPoints.end(), actualPoints.begin(), actualPoints.end());

	nominalAndActualColors.insert(nominalAndActualColors.end(), nominalColors.begin(), nominalColors.end());
	nominalAndActualColors.insert(nominalAndActualColors.end(), actualColors.begin(), actualColors.end());

	tr::Display3DRoutines::displayPointSet(nominalAndActualPoints, nominalAndActualColors);
	
#endif



	
	return 0;
}

//
//void viewData(std::string dataPath, std::vector< Eigen::Vector3f >& dataPoints)
//{
//
//	
//
//	std::fstream reader;
//
//	reader.open(dataPath, std::ios::in | std::ios::binary);
//
//	int nNominalPositions = 0;
//
//	reader >> nNominalPositions;
//
//	std::vector< double > nPos(nNominalPositions);
//
//	reader.read((char*)nPos.data(), nNominalPositions * sizeof(double));
//
//	reader.close();
//
//	std::vector< Eigen::Vector3f > colors(nNominalPositions / 3, Eigen::Vector3f(1, 0, 0));
//
//	dataPoints.resize(nNominalPositions / 3);
//
//	for (int ii = 0; ii < nNominalPositions / 3; ii++)
//	{
//		dataPoints[ii](0) = nPos[3 * ii];
//		dataPoints[ii](1) = nPos[3 * ii + 1];
//		dataPoints[ii](2) = nPos[3 * ii + 2];
//
//		//std::cout << points[ii].transpose() << std::endl;
//	}
//
//	std::cout << " number of points : " << dataPoints.size() << std::endl;
//
//	tr::Display3DRoutines::displayPointSet(dataPoints, colors);
//
//
//}
//
//void testPointCloudEvaluationDecorator( int w , int h , int d  , Eigen::Vector3f& voxelSideLengths , std::vector< Eigen::Vector3f >& positions ,
//	std::vector< Eigen::Vector3f >& dir , std::vector< unsigned short >& volume )
//{ 
//
//	unsigned short *volumeData = volume.data();
//
//	void **sliceData = new void *[d] ;
//
//	for (int zz = 0; zz < d; zz++)
//	{
//		sliceData[zz] = volumeData + (w * h * zz);
//	}
//
//	unsigned int bitDepth = 16;
//	
//	l3d size;
//	d3d voxelSize;
//	int interpolationMethod;
//	bool useAutoParams;
//	double gradientThreshold , sigma;
//	bool useBeamHardeningCorrection , considerStaticThreshold;
//	long staticThreshold;
//	bool useAngleCriteria;
//	double angleCriteria;
//	unsigned long long qualityThreshold;
//	double searchRange , searchRangeAirSide;
//	long count;
//
//	d3d *nominalPositions, *searchDirections , *results , *normals;
//
//	double *qualities;
//	ICTEvaluationProgress *progress = 0;
//
//	size[0] = w;
//	size[1] = h;
//	size[2] = d;
//
//	voxelSize[0] = voxelSideLengths(0);
//	voxelSize[1] = voxelSideLengths(1);
//	voxelSize[2] = voxelSideLengths(2);
//
//	
//
//	CCTPointCloudEvaluationDecorator::DoMeasureData( sliceData , bitDepth , size , voxelSize , interpolationMethod , useAutoParams , gradientThreshold ,
//		                                             sigma , useBeamHardeningCorrection , considerStaticThreshold , staticThreshold , useAngleCriteria , 
//													 angleCriteria , qualityThreshold , searchRange , searchRangeAirSide , count , nominalPositions , searchDirections ,
//													 results , normals , qualities , progress);
//
//
//}
//
//
//void readVolumeParameters()
//{
//	std::string path1 = "C:/projects/DebugData/volume_dimensions.bin";
//	std::string path2 = "C:/projects/DebugData/volume_params.bin";
//	std::string path3 = "C:/projects/DebugData/volume_vox.bin";
//
//	l3d size;
//	d3d voxelSize;
//	int interpolationMethod;
//	bool useAutoParams;
//	double gradientThreshold, sigma;
//	bool useBeamHardeningCorrection = false, considerStaticThreshold;
//	long staticThreshold;
//	bool useAngleCriteria;
//	double angleCriteria;
//	unsigned long long qualityThreshold;
//	double searchRange, searchRangeAirSide;
//	long count;
//
//	FILE* pDimensionsBin;
//	fopen_s(&pDimensionsBin, "C:\\projects\\DebugData\\volume_dimensions.bin", "rb");
//	fread(size, sizeof(long), 3, pDimensionsBin);
//	fread(&voxelSize, sizeof(double), 3, pDimensionsBin);
//	fclose(pDimensionsBin);
//
//	std::cout << voxelSize[0] << " " << voxelSize[1] << " " << voxelSize[2] << std::endl;
//
//	std::vector< unsigned short > volume;
//
//	int w = size[0];
//	int h = size[1];
//	int d = size[2];
//
//	volume.resize(w * h * d);
//
//	unsigned short *volumeData = volume.data();
//
//	void **sliceData = new void *[d];
//
//	for (int zz = 0; zz < d; zz++)
//	{
//		sliceData[zz] = volumeData + (w * h * zz);
//	}
//
//	FILE* pParamsBin;
//	fopen_s(&pParamsBin, "C:\\projects\\DebugData\\volume_params.bin", "rb");
//	fread(&searchRange, sizeof(double), 1, pParamsBin);
//	fread(&searchRangeAirSide, sizeof(double), 1, pParamsBin);
//	fread(&interpolationMethod, sizeof(int), 1, pParamsBin);
//	fread(&sigma, sizeof(double), 1, pParamsBin);
//	fread(&useAngleCriteria, sizeof(bool), 1, pParamsBin);
//	fread(&angleCriteria, sizeof(double), 1, pParamsBin);
//	fread(&useBeamHardeningCorrection, sizeof(bool), 1, pParamsBin);
//	fread(&useAutoParams, sizeof(bool), 1, pParamsBin);
//	fread(&gradientThreshold, sizeof(double), 1, pParamsBin);
//	fread(&qualityThreshold, sizeof(unsigned long long), 1, pParamsBin);
//	fread(&considerStaticThreshold, sizeof(bool), 1, pParamsBin);
//	fread(&staticThreshold, sizeof(long), 1, pParamsBin);
//	fclose(pParamsBin);
//
//	std::cout << " quality threshold " << qualityThreshold << std::endl;
//	std::cout << " interpolation method : " << interpolationMethod << std::endl;
//
//	FILE* pVoxBin;
//	fopen_s(&pVoxBin, "C:\\projects\\DebugData\\volume_vox.bin", "rb");
//	int sliceSize = size[0] * size[1];
//	for (int i = 0; i < size[2]; i++)
//	{
//		if (!fread(sliceData[i], sizeof(short), sliceSize, pVoxBin))
//		{
//			break;
//		}
//	}
//
//	fclose(pVoxBin);
//
//	long countLong = 0;// static_cast<long>(count);
//
//	extractSurfaceOnVolume(Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(voxelSize[0], voxelSize[1], voxelSize[2]), volume, w, h, d);
//
//	FILE* pVPointsAndNormalsBin;
//	fopen_s(&pVPointsAndNormalsBin, "C:\\projects\\DebugData\\pointsAndNormals.bin", "rb");
//	fread(&countLong, sizeof(long), 1, pVPointsAndNormalsBin);
//	count = countLong;
//
//	d3d *nominalPositions = new d3d[count];
//	d3d *searchDirections = new d3d[count];
//
//	//std::cout << " point count : " << count << std::endl;
//
//	fread(nominalPositions, 3 * sizeof(double), count, pVPointsAndNormalsBin);
//	fread(searchDirections, 3 * sizeof(double), count, pVPointsAndNormalsBin);
//	fclose(pVPointsAndNormalsBin);
//
//	unsigned int bitDepth = 16;
//
//	d3d* results = new d3d[count];
//	d3d* normals = new d3d[count];
//	double* qualities = new double[count];
//
//	int nSlices = int( count / 1000);
//	
//	if (count % 1000) 
//		nSlices++;
//	int PointsPerSlice = 1000;
//	
//	if ( count < 3000 )
//	{
//		nSlices = int( count / 300 );
//		
//		PointsPerSlice = 300;
//
//		if ( count % 300 ) 
//			nSlices++;
//	}
//
//
//	
//	int nParts = count / PointsPerSlice + 1;
//
//	float colorStep = 1.0 / nParts;
//
//	int cc = 0;
//
//	std::vector< Eigen::Vector3f > sliceColors , slicePoints;
//
//	for (int pp = 0; pp < count; pp += PointsPerSlice , cc++)
//	{
//		int end = std::min(pp + PointsPerSlice, (int)count);
//
//		std::vector< Eigen::Vector3f > dpts;
//
//		float zCol = cc * colorStep;
//
//		Eigen::Vector3f color((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f);
//
//		for (int pp1 = pp; pp1 < end; pp1++)
//		{
//			slicePoints.push_back(Eigen::Vector3f(nominalPositions[pp1][0], nominalPositions[pp1][1], nominalPositions[pp1][2]));
//
//			sliceColors.push_back(color);
//		}
//
//	}
//
//	//tr::Display3DRoutines::displayPointSet(slicePoints, sliceColors);
//
//	//return;
//
//	std::cout << searchRange << " " << searchRangeAirSide << std::endl;
//
//	//return;
//
//
//
//	//long numEstimatedPoints = CCTPointCloudEvaluationDecorator::DoMeasureData( sliceData, bitDepth, size, voxelSize, interpolationMethod, useAutoParams, gradientThreshold,
//	//	                                                                       sigma, useBeamHardeningCorrection, considerStaticThreshold, staticThreshold, useAngleCriteria,
//	//	                                                                       angleCriteria, qualityThreshold, searchRange, searchRangeAirSide, count, nominalPositions, searchDirections,
//	//	                                                                       results, normals, qualities, 0);
//
//
//	//LXLogger* log = LXManager::Manager().GetLogger("CCTPointCloudEvaluationDecorator");
//	std::stringstream msg;
//
//	//CCTPointCloudEvaluation evaluation(sliceData, (CCTProfilsEvaluation::vxType)2, size);
//
//	CCTPointCloudEvaluation evaluation( sliceData, (CCTProfilsEvaluation::vxType)2, size);
//
//	evaluation.SetSigma(sigma);
//	evaluation.SetThreshold(gradientThreshold);
//	bool usingQuality = count <= qualityThreshold;
//
//	std::cout << " sigma and gradient threshold " << sigma << " " << gradientThreshold << std::endl;
//
//
//	if ((!usingQuality || evaluation.nMaterial > 1) && evaluation.detectedMaterials < evaluation.nMaterial)
//	{
//		unsigned int const shortestDimension = std::min(std::min(size[0], size[1]), size[2]);
//		unsigned int const SliceSkipRate = 25;
//		int evpts = evaluation.MaterialAnalysis(1, shortestDimension / SliceSkipRate);
//
//		std::cout << "MaterialAnalysis with " << evpts << " evaluated points found " << evaluation.detectedMaterials 
//			      << " materials and optimal static threshold: " << evaluation.staticThreshold<<std::endl;
//	
//		std::cout << "MaterialAnalysis: air threshold: " << evaluation.materialThresholds[0] << " gradient noise: " << evaluation.materialGradientThresholds[0] << std::endl;
//
//		for (int i = 1; i <= evaluation.detectedMaterials; i++)
//		{
//			std::cout << "MaterialAnalysis: " << i << ". Material threshold: " << evaluation.materialThresholds[i] 
//				      << " gradient threshold: " << evaluation.materialGradientThresholds[i]<<std::endl;
//		}
//		if (evaluation.detectedMaterials > 1)
//		{
//			std::cout << "Material analysis detected " << evaluation.detectedMaterials << " materials. Using the global threshold!"<<std::endl;
//		}
//	}
//
//	std::cout << " search range and search range air side : " << searchRange << " " << searchRangeAirSide << std::endl;
//
//	//log->Info(System::String::Format("Trying to extract profiles ({0}|{1})", searchRangeVoxel, searchRangeAirSideVoxel));
//	evaluation.extractProfiles(count, (d3d*)nominalPositions, (d3d*)searchDirections, searchRange, searchRangeAirSide, interpolationMethod);
//
//	if (useAngleCriteria)
//	{
//		std::cout << "Using Angle criteria (" << angleCriteria << ")" << std::endl;
//		evaluation.SetAngleCriterium(180.0 * angleCriteria / M_PI);
//	}
//
//
//
//	if (useAutoParams || useBeamHardeningCorrection)
//	{
//		bool useAutoGrad = useAutoParams;
//		// Bei sehr vielen Messpunkten Basis der Statistik etwas ausdünnen (Rechenzeit verringern!)
//		long step = (count > 5000) ? count / 5000 : 1;
//		
//		std::cout << "Using " << step << " as step size"<<std::endl;
//		
//		if (evaluation.detectedMaterials > evaluation.nMaterial)
//		{
//			evaluation.SetThreshold(evaluation.globalGradThreshold / evaluation.upperLimit);
//			useAutoGrad = false;
//		}
//
//		bool success = evaluation.AutoParam(step, useBeamHardeningCorrection, useBeamHardeningCorrection, useAutoParams, useAutoGrad);
//		
//		if (success)
//			std::cout << "AutoParam successful!"<<std::endl;
//		else
//			std::cout << "AutoParam failed!"<<std::endl;
//
//		sigma = evaluation.sigma;
//		std::cout<<"Sigma = " << evaluation.sigma<<std::endl;
//		
//		gradientThreshold = evaluation.relThreshold;
//		std::cout << "GradientThreshold = " << evaluation.relThreshold << std::endl;
//		
//	}
//
//	if (considerStaticThreshold)
//	{
//		evaluation.checkStaticThreshold4Measure = true;
//		evaluation.staticThreshold = static_cast<double>(staticThreshold);
//	}
//
//
//	std::cout << "Starting to measure " << count << " points (Quality: " << ( usingQuality ? "true)..." : "false)..." )<<std::endl;
//
//	long valid = evaluation.Measure(usingQuality, nullptr);
//	std::cout << valid << " points grabbed from volume"<<std::endl;
//
//
//	double const origSearchRangeVoxelSquared = searchRange * searchRange;
//	double const origSearchRangeAirSideVoxelSquared = searchRangeAirSide * searchRangeAirSide;
//	for (int i = 0; i < count; ++i)
//	{
//		if (evaluation.getResult(i, results[i], normals[i], &qualities[i]))
//		{
//			//log->Error(System::String::Format("Invalid Point {0}: {1} {2} {3} | {4} {5} {6}", i, positions[i][0], positions[i][1], positions[i][2], searchDirections[i][0], searchDirections[i][1], searchDirections[i][2] ));
//
//			results[i][0] = 0.0;
//			results[i][1] = 0.0;
//			results[i][2] = 0.0;
//
//			normals[i][0] = 0.0;
//			normals[i][1] = 0.0;
//			normals[i][2] = 0.0;
//
//			qualities[i] = 0.0;
//		}
//		//else if (origSearchRangeVoxel != searchRangeVoxel || origSearchRangeAirSideVoxel != searchRangeAirSideVoxel)
//		//{
//		//	//Removing points which are not in range [-origSearchRangeAirSideVoxel, origSearchRangeVoxel]
//		//	double squaredDev = CCTPointCloudEvaluationDecorator::CalculateSignedSquaredDeviation( nominalPositions[i], searchDirections[i], results[i], normals[i]);
//		//	if (squaredDev == std::numeric_limits<double>::quiet_NaN() || squaredDev > origSearchRangeVoxelSquared || -squaredDev > origSearchRangeAirSideVoxelSquared)
//		//	{
//		//		results[i][0] = 0.0;
//		//		results[i][1] = 0.0;
//		//		results[i][2] = 0.0;
//
//		//		normals[i][0] = 0.0;
//		//		normals[i][1] = 0.0;
//		//		normals[i][2] = 0.0;
//
//		//		qualities[i] = 0.0;
//		//	}
//		//}
//	}

	//double const origSearchRangeVoxelSquared = origSearchRangeVoxel * origSearchRangeVoxel;
	//double const origSearchRangeAirSideVoxelSquared = origSearchRangeAirSideVoxel * origSearchRangeAirSideVoxel;


	//std::cout << " number of estimated points : " << numEstimatedPoints << std::endl;

	//std::vector< Eigen::Vector3f > estimatedPoints(numEstimatedPoints);

	//for (int ee = 0; ee < numEstimatedPoints; ee++)
	//{
	//	estimatedPoints[ee](0) = results[ee][0];
	//	estimatedPoints[ee](1) = results[ee][1];
	//	estimatedPoints[ee](2) = results[ee][2];
	//}

	//std::vector< Eigen::Vector3f > colors(estimatedPoints.size(), Eigen::Vector3f(1, 0, 0));

	//tr::Display3DRoutines::displayPointSet(estimatedPoints, colors);


//}