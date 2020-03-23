#if 0

#include "iostream"
#include "fstream"
#include "opencvincludes.h"
#include "eigenincludes.h"
#include "vtkImageImport.h"
#include "vtkMatrix4x4.h"
#include "vtkImageData.h"
#include "vtkImageReslice2.h"
#include "vtkSmartPointer.h"
#include "vtkPointData.h"
#include "vtkDataArray.h"
#include "vtkPLYReader.h"
#include "vtkPolyData.h"
#include "display3droutines.h"


size_t intersectionWithXAxis(double *n, double* a, double wS, double hS, double dS,
	Eigen::Vector3d& xAxis, Eigen::Vector3d& yAxis, Eigen::Vector3d& zAxis, double voxelSize, Eigen::Vector3d& sliceOrigin);
size_t intersectionWithYAxis(double *n, double* a, double w, double h, double d, Eigen::Vector3d& xAxis, Eigen::Vector3d& yAxis, Eigen::Vector3d& zAxis, double voxelSize);
size_t intersectionWithZAxis(double *n, double* a, double w, double h, double d, Eigen::Vector3d& xAxis, Eigen::Vector3d& yAxis, Eigen::Vector3d& zAxis, double voxelSize);
size_t interpolateImage(Eigen::Vector3d xAxis, Eigen::Vector3d yAxis, Eigen::Vector3d origin, double step, int w, int h);

void intersectionWithXAxis(double *n, double* a, double wS, double hS, double dS,
	double* xAxis, double* yAxis, double* zAxis, double voxelSize, double* sliceOrigin
	, int& outputW, int& outputH);

void intersectionWithYAxis(double *n, double* a, double wS, double hS, double dS,
	double* xAxis, double* yAxis, double* zAxis, double voxelSize, double* sliceOrigin
	, int& outputW, int& outputH);

void intersectionWithZAxis(double *n, double* a, double wS, double hS, double dS,
	double* xAxis, double* yAxis, double* zAxis, double voxelSize, double* sliceOrigin
	, int outputW, int outputH);


void interpolateImage(Eigen::Vector3d xAxis, Eigen::Vector3d yAxis, Eigen::Vector3d origin, double step, int w, int h,
	int volumeW, int volumeH, int volumeD, unsigned short *volumeData, cv::Mat& interpolatedImage,
	double *transferFunction);

void interpolateImage( Eigen::Vector3d xAxis, Eigen::Vector3d yAxis, Eigen::Vector3d origin, double step, int w, int h,
	                   int volumeW, int volumeH, int volumeD, unsigned short *volumeData, cv::Mat& interpolatedImage,
	                   double *transferFunction );

void interpolateImage2(Eigen::Vector3d xAxis, Eigen::Vector3d yAxis, Eigen::Vector3d origin, double step, int w, int h,
	int volumeW, int volumeH, int volumeD, unsigned short *volumeData, cv::Mat& interpolatedImage,
	double *transferFunction);



void intersectPlaneWithBoxParallelSides( double *n, double* a, double *startPoints, int axisId, double wS, double hS, double dS,
	                                     double* xAxis, double* yAxis, double* zAxis, double voxelSize, double* sliceOrigin
	                                     , int& outputW, int& outputH );

void sliceFromVTK(double *coordSys, unsigned short *volumeDataPtr, unsigned int* voxelResolution, double *currentColorMap, double voxelSize);
void computeNewBoundingBox(Eigen::Vector3d& origin, Eigen::Vector3d& xAxis, Eigen::Vector3d& yAxis, Eigen::Vector3d& zAxis, 
	double wS, double hS, double dS, double voxelSize , Eigen::Matrix4d& transform );

void computeSliceDimensions(double wS, double hS, double dS, double voxelSize, Eigen::Matrix4d& transform, int& w, int& h);

void interpolateImage3(Eigen::Vector3d xAxis, Eigen::Vector3d yAxis, Eigen::Matrix4d& transform, double step, int w, int h,
	int volumeW, int volumeH, int volumeD, unsigned short *volumeData, cv::Mat& interpolatedImage,
	double *transferFunction);



int main( int argc , char **argv )
{
	//std::string plyPath = "C:/Data/T2_R2_Refined.ply";

	//vtkSmartPointer<vtkPLYReader> reader2 = vtkSmartPointer<vtkPLYReader>::New();

	//reader2->SetFileName(plyPath.c_str());

	//reader2->Update();

	//tr::Display3DRoutines::displayPolyData(reader2->GetOutput());

	std::string datasetPath = "C:/Projects/Wallthickness/data/separated_part_7.uint16_scv";

	unsigned int volumeW = 0, volumeH = 0, volumeD = 0;
	
	std::string savedVolumePath = "C:/Projects/sliceData.bin";

	int sliceW = 213;//209;//313; //211;//288;// 313;//
	int sliceH = 211;//288;// 185;// 213;//209;// 185;//

	std::fstream reader;

	reader.open(savedVolumePath, std::ios::in | std::ios::binary);

	reader.read((char*)&volumeW, sizeof(unsigned int)); //>> volumeW >> volumeH >> volumeD;
	reader.read((char*)&volumeH, sizeof(unsigned int));
	reader.read((char*)&volumeD, sizeof(unsigned int));

	std::cout << volumeW << " " << volumeH << " " << volumeD << std::endl;

	unsigned short *volumeData = new unsigned short[ volumeW * volumeH * volumeD ];

	reader.read((char*)volumeData, volumeW * volumeH * volumeD * sizeof(unsigned short));

	double transferFunction[65536];

	double voxelSize = 0.0871433;
	double coordSys[16];

	reader.read((char*)&coordSys, 16 * sizeof(double));

	for (int ii = 0; ii < 4; ii++)
		std::cout << coordSys[4 * ii] << " " << coordSys[4 * ii + 1] << " " 
		<< coordSys[4 * ii + 2] << " " << coordSys[4 * ii + 3] << std::endl;


	reader.read((char*)&transferFunction, 65536 * sizeof(double));

	cv::Mat im(sliceW, sliceH, CV_8UC1);

	unsigned char *sliceImData = im.data;

	unsigned short *gvs = new unsigned short[sliceW * sliceH];

	reader.read((char*)gvs, sliceW * sliceH * sizeof(unsigned short));

	for (int i = 0; i < sliceW * sliceH; i++)
	{
		unsigned short grayValue = gvs[i];
		double value = transferFunction[grayValue];

		sliceImData[i] = (unsigned char)(value * 255);
	}

	cv::namedWindow("slice", 0);
	cv::imshow("slice", im);
	cv::waitKey();


	//now compute the intersection between slice and volume box
	//There are three sets of 4 lines with which we desire to compute intersection

	//normal to the plane 
	double n[3] = { coordSys[2], coordSys[6], coordSys[10] };
	double a[3] = { coordSys[3], coordSys[7], coordSys[11] };

	std::cout << " normal to the plane : " << n[0] << " " << n[1] << " " << n[2] << std::endl;


	double wS = volumeW * voxelSize;
	double hS = volumeH * voxelSize;
	double dS = volumeD * voxelSize;

	Eigen::Vector3d origin(coordSys[3], coordSys[7], coordSys[11]) , so;
	Eigen::Vector3d xAxis(coordSys[0], coordSys[4], coordSys[8]), 
		            yAxis(coordSys[1], coordSys[5], coordSys[9]), 
					zAxis(coordSys[2], coordSys[6], coordSys[10]);

	std::cout << xAxis.norm() << " " << yAxis.norm() << " " << zAxis.norm() << std::endl;


	Eigen::Matrix4d transform ;

	Eigen::Matrix3d rotation;

	transform.setIdentity();

	for (int rr = 0; rr < 3; rr++)
		for (int cc = 0; cc < 3; cc++)
		{
			transform(rr, cc) = coordSys[4 * cc + rr];

			rotation(rr, cc) = coordSys[4 * cc + rr];
		}

	Eigen::Vector3d translation = -rotation * origin;

	transform.block(0,3,3,1) = translation;



	//transform(0, 3) *= -1;
	//transform(1, 3) *= -1;
	//transform(2, 3) *= -1;

	Eigen::Matrix4d invTransform = transform.inverse();

	std::cout << invTransform << std::endl;


	//computeNewBoundingBox(origin, xAxis, yAxis, zAxis, wS, hS, dS, voxelSize, invTransform);

	int sliceWidth = 0, sliceHeight = 0;

	computeSliceDimensions(wS, hS, dS, voxelSize, transform, sliceWidth, sliceHeight);//invTransform

	std::cout << " slice width and height : " << sliceWidth << " " << sliceHeight << std::endl;

	std::cout << " slice width and height VTK : " << sliceW << " " << sliceH << std::endl;

	cv::Mat interpolatedImage2;

	interpolateImage2( xAxis, yAxis, -origin, voxelSize, sliceWidth, sliceHeight,
		               volumeW, volumeH, volumeD, volumeData, interpolatedImage2,
		               transferFunction );

	//interpolateImage3(xAxis, yAxis, invTransform, voxelSize, sliceWidth, sliceHeight,
	//	volumeW, volumeH, volumeD, volumeData, interpolatedImage2,
	//	transferFunction);

	cv::namedWindow("NewSlice", 0);
	cv::imshow("NewSlice", interpolatedImage2);
	cv::waitKey();
	
	//intersection with X axis
	intersectionWithXAxis(n, a, wS, hS, dS, xAxis, yAxis, zAxis, voxelSize, so);

	//intersection with Y axis
	intersectionWithYAxis(n, a, wS, hS, dS, xAxis, yAxis, zAxis, voxelSize);

	//intersection with Z Axis
	intersectionWithZAxis(n, a, wS, hS, dS, xAxis, yAxis, zAxis, voxelSize);

	Eigen::Vector3d outputXAxis, outputYAxis;

	int outputW, outputH;

	int ow1, oh1, ow2, oh2, ow3, oh3;

	Eigen::Vector3d so1, so2, so3;

	intersectionWithXAxis( n, a, wS, hS, dS, xAxis.data(), yAxis.data(), zAxis.data(),
		                   voxelSize, so1.data(), ow1, oh1 );

	std::cout << ow1 << " " << oh1 << std::endl;

	intersectionWithYAxis( n, a, wS, hS, dS, xAxis.data(), yAxis.data(), zAxis.data(),
		                   voxelSize, so2.data(), ow2, oh2 );

	std::cout << ow2 << " " << oh2 << std::endl;

	intersectionWithZAxis( n, a, wS, hS, dS, xAxis.data(), yAxis.data(), zAxis.data(),
		                   voxelSize, so3.data(), ow3, oh3 );

	std::cout << ow3 << " " << oh3 << std::endl;

	int a1 = ow1 * oh1;
	int a2 = ow2 * oh2;
	int a3 = ow3 * oh3;

	outputW = ow1;
	outputH = oh1;

	so = so1;

	int  ar = a1;

	if (a2 < ar)
	{
		outputW = ow2;
		outputH = oh2;

		ar = a2;

		so = so2;
	}

	if ( a3 < ar)
	{
		outputW = ow3;
		outputH = oh3;

		ar = a3;

		so = so3;
	}

	std::cout << outputW << " " << outputH << std::endl;


	cv::Mat interpolatedImage;

	interpolateImage(xAxis, yAxis, so, voxelSize, outputW, outputH, volumeW,
		             volumeH, volumeD, volumeData, interpolatedImage, 
					 transferFunction);


	cv::namedWindow("interpolatedImage", 0);
	cv::imshow("interpolatedImage", interpolatedImage);
	cv::waitKey();


	unsigned int voxelResolution[3] = { volumeW, volumeH, volumeD };

	sliceFromVTK(coordSys, volumeData, voxelResolution, transferFunction, voxelSize);

	return 0;
}


inline double dot3(double *v1, double *v2)
{
	return (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]);
}

size_t intersectionWithXAxis(double *n, double* a, double wS, double hS, double dS,
	Eigen::Vector3d& xAxis, Eigen::Vector3d& yAxis, Eigen::Vector3d& zAxis, double voxelSize , Eigen::Vector3d& sliceOrigin)
{
	// four start points ( 0 , 0 , 0 ) , ( 0 , h , 0 ) , ( 0 , h , d ) , ( 0 , 0 , d )
	double d[3] = { 1, 0, 0 }; // direction of x axis

	double ddotn = n[0];

	double wHalf = wS / 2;
	double hHalf = hS / 2;
	double dHalf = -dS / 2;

	double aminusp1[3] = { a[0] , a[1] , a[2]};
	double aminusp2[3] = { a[0], a[1] - hS, a[2]};
	double aminusp3[3] = { a[0], a[1] - hS, a[2] - dS};
	double aminusp4[3] = { a[0], a[1] , a[2] - dS};


	double t1 = dot3(aminusp1, n) / ddotn;
	double t2 = dot3(aminusp2, n) / ddotn;
	double t3 = dot3(aminusp3, n) / ddotn;
	double t4 = dot3(aminusp4, n) / ddotn;




	Eigen::Vector3d intersectedPoint1( t1  , 0 , 0 ), intersectedPoint2(t2, hS, 0) , 
		            intersectedPoint3( t3 , hS , dS ), intersectedPoint4(t4, 0, dS);




	Eigen::Vector3d origin(a[0], a[1], a[2]);

	Eigen::Vector3d p1, p2, p3, p4;

	p1(0) = (intersectedPoint1 - origin).dot(xAxis);
	p2(0) = (intersectedPoint2 - origin).dot(xAxis);
	p3(0) = (intersectedPoint3 - origin).dot(xAxis);
	p4(0) = (intersectedPoint4 - origin).dot(xAxis);

	p1(1) = (intersectedPoint1 - origin).dot(yAxis);
	p2(1) = (intersectedPoint2 - origin).dot(yAxis);
	p3(1) = (intersectedPoint3 - origin).dot(yAxis);
	p4(1) = (intersectedPoint4 - origin).dot(yAxis);

	p1(2) = (intersectedPoint1 - origin).dot(zAxis);
	p2(2) = (intersectedPoint2 - origin).dot(zAxis);
	p3(2) = (intersectedPoint3 - origin).dot(zAxis);
	p4(2) = (intersectedPoint4 - origin).dot(zAxis);

	//p1 = intersectedPoint1;
	//p2 = intersectedPoint2;
	//p3 = intersectedPoint3;
	//p4 = intersectedPoint4;

	double xMin = DBL_MAX, xMax = 0, yMin = DBL_MAX, yMax = 0, zMin = DBL_MAX, zMax = 0;

	xMin = std::min(xMin, p1(0));
	xMin = std::min(xMin, p2(0));
	xMin = std::min(xMin, p3(0));
	xMin = std::min(xMin, p4(0));

	yMin = std::min(yMin, p1(1));
	yMin = std::min(yMin, p2(1));
	yMin = std::min(yMin, p3(1));
	yMin = std::min(yMin, p4(1));

	zMin = std::min(zMin, p1(2));
	zMin = std::min(zMin, p2(2));
	zMin = std::min(zMin, p3(2));
	zMin = std::min(zMin, p4(2));

	xMax = std::max(xMax, p1(0));
	xMax = std::max(xMax, p2(0));
	xMax = std::max(xMax, p3(0));
	xMax = std::max(xMax, p4(0));

	yMax = std::max(yMax, p1(1));
	yMax = std::max(yMax, p2(1));
	yMax = std::max(yMax, p3(1));
	yMax = std::max(yMax, p4(1));

	zMax = std::max(zMax, p1(2));
	zMax = std::max(zMax, p2(2));
	zMax = std::max(zMax, p3(2));
	zMax = std::max(zMax, p4(2));

	//std::cout << p1.transpose() << std::endl;
	//std::cout << p2.transpose() << std::endl;
	//std::cout << p3.transpose() << std::endl;
	//std::cout << p4.transpose() << std::endl;

	//std::cout << xMin << " " << xMax << " " << yMin << " " << yMax << " " << zMin << " " << zMax << std::endl;

	std::cout << (xMax - xMin) / voxelSize << " " << (yMax - yMin) / voxelSize << " " << (zMax - zMin) / voxelSize << std::endl;

	sliceOrigin = origin + xMin * xAxis + yMin * yAxis;

	int d1 = (xMax - xMin) / voxelSize;
	int d2 = (yMax - yMin) / voxelSize;
	int d3 = (zMax - zMin) / voxelSize;

	size_t maxArea = std::max(d1 * d2, d2 * d3);

	maxArea = std::max(maxArea, (size_t)d3 * d1);

	return maxArea;

}


size_t intersectionWithYAxis(double *n, double* a, double wS, double hS, double dS,
	Eigen::Vector3d& xAxis, Eigen::Vector3d& yAxis, Eigen::Vector3d& zAxis, double voxelSize)
{
	// four start points ( 0 , 0 , 0 ) , ( w , 0 , 0 ) , ( w , 0 , d ) , ( 0 , 0 , d )
	double d[3] = { 0, 1, 0 }; // direction of x axis

	double ddotn = n[1];

	double aminusp1[3] = { a[0], a[1], a[2] };
	double aminusp2[3] = { a[0] - wS, a[1] , a[2]};
	double aminusp3[3] = { a[0] - wS, a[1] , a[2] - dS };
	double aminusp4[3] = { a[0], a[1], a[2] - dS};


	double t1 = dot3(aminusp1, n) / ddotn;
	double t2 = dot3(aminusp2, n) / ddotn;
	double t3 = dot3(aminusp3, n) / ddotn;
	double t4 = dot3(aminusp4, n) / ddotn;


	Eigen::Vector3d intersectedPoint1(0, t1, 0), intersectedPoint2(wS, t2, 0),
		intersectedPoint3(wS, t3, dS), intersectedPoint4(0, t4, dS);


	//std::cout << intersectedPoint1.transpose() << std::endl;
	//std::cout << intersectedPoint2.transpose() << std::endl;
	//std::cout << intersectedPoint3.transpose() << std::endl;
	//std::cout << intersectedPoint4.transpose() << std::endl;

	Eigen::Vector3d origin(a[0], a[1], a[2]);

	Eigen::Vector3d p1, p2, p3, p4;

	p1(0) = (intersectedPoint1 - origin).dot(xAxis);
	p2(0) = (intersectedPoint2 - origin).dot(xAxis);
	p3(0) = (intersectedPoint3 - origin).dot(xAxis);
	p4(0) = (intersectedPoint4 - origin).dot(xAxis);

	p1(1) = (intersectedPoint1 - origin).dot(yAxis);
	p2(1) = (intersectedPoint2 - origin).dot(yAxis);
	p3(1) = (intersectedPoint3 - origin).dot(yAxis);
	p4(1) = (intersectedPoint4 - origin).dot(yAxis);

	p1(2) = (intersectedPoint1 - origin).dot(zAxis);
	p2(2) = (intersectedPoint2 - origin).dot(zAxis);
	p3(2) = (intersectedPoint3 - origin).dot(zAxis);
	p4(2) = (intersectedPoint4 - origin).dot(zAxis);

	double xMin = DBL_MAX, xMax = 0, yMin = DBL_MAX, yMax = 0, zMin = DBL_MAX, zMax = 0;

	xMin = std::min(xMin, p1(0));
	xMin = std::min(xMin, p2(0));
	xMin = std::min(xMin, p3(0));
	xMin = std::min(xMin, p4(0));

	yMin = std::min(yMin, p1(1));
	yMin = std::min(yMin, p2(1));
	yMin = std::min(yMin, p3(1));
	yMin = std::min(yMin, p4(1));

	zMin = std::min(zMin, p1(2));
	zMin = std::min(zMin, p2(2));
	zMin = std::min(zMin, p3(2));
	zMin = std::min(zMin, p4(2));

	xMax = std::max(xMax, p1(0));
	xMax = std::max(xMax, p2(0));
	xMax = std::max(xMax, p3(0));
	xMax = std::max(xMax, p4(0));

	yMax = std::max(yMax, p1(1));
	yMax = std::max(yMax, p2(1));
	yMax = std::max(yMax, p3(1));
	yMax = std::max(yMax, p4(1));

	zMax = std::max(zMax, p1(2));
	zMax = std::max(zMax, p2(2));
	zMax = std::max(zMax, p3(2));
	zMax = std::max(zMax, p4(2));


	//std::cout << p1.transpose() << std::endl;
	//std::cout << p2.transpose() << std::endl;
	//std::cout << p3.transpose() << std::endl;
	//std::cout << p4.transpose() << std::endl;

	//std::cout << xMin << " " << xMax << " " << yMin << " " << yMax << " " << zMin << " " << zMax << std::endl;

	std::cout << (xMax - xMin) / voxelSize << " " << (yMax - yMin) / voxelSize << " " << (zMax - zMin) / voxelSize << std::endl;

	int d1 = (xMax - xMin) / voxelSize;
	int d2 = (yMax - yMin) / voxelSize;
	int d3 = (zMax - zMin) / voxelSize;

	size_t maxArea = std::max(d1 * d2, d2 * d3);

	maxArea = std::max(maxArea, (size_t)d3 * d1);

	return maxArea;
}


size_t intersectionWithZAxis(double *n, double* a, double wS, double hS, double dS, 
	Eigen::Vector3d& xAxis, Eigen::Vector3d& yAxis, Eigen::Vector3d& zAxis, double voxelSize)
{
	// four start points ( 0 , 0 , 0 ) , ( w , 0 , 0 ) , ( w , h , 0 ) , ( 0 , h , 0 )
	double d[3] = { 0, 0, 1 }; // direction of x axis

	double ddotn = n[2];

	double aminusp1[3] = { a[0], a[1], a[2] };
	double aminusp2[3] = { a[0] - wS, a[1] , a[2] };
	double aminusp3[3] = { a[0] - wS, a[1] - hS, a[2] };
	double aminusp4[3] = { a[0], a[1] - hS , a[2] };


	double t1 = dot3(aminusp1, n) / ddotn;
	double t2 = dot3(aminusp2, n) / ddotn;
	double t3 = dot3(aminusp3, n) / ddotn;
	double t4 = dot3(aminusp4, n) / ddotn;


	Eigen::Vector3d intersectedPoint1(0, 0, t1), intersectedPoint2(wS, 0, t2),
		intersectedPoint3(wS, hS, t3), intersectedPoint4(0, hS, t4);

	Eigen::Vector3d origin(a[0], a[1], a[2]);

	Eigen::Vector3d p1, p2, p3, p4;

	p1(0) = (intersectedPoint1 - origin).dot(xAxis);
	p2(0) = (intersectedPoint2 - origin).dot(xAxis);
	p3(0) = (intersectedPoint3 - origin).dot(xAxis);
	p4(0) = (intersectedPoint4 - origin).dot(xAxis);

	p1(1) = (intersectedPoint1 - origin).dot(yAxis);
	p2(1) = (intersectedPoint2 - origin).dot(yAxis);
	p3(1) = (intersectedPoint3 - origin).dot(yAxis);
	p4(1) = (intersectedPoint4 - origin).dot(yAxis);

	p1(2) = (intersectedPoint1 - origin).dot(zAxis);
	p2(2) = (intersectedPoint2 - origin).dot(zAxis);
	p3(2) = (intersectedPoint3 - origin).dot(zAxis);
	p4(2) = (intersectedPoint4 - origin).dot(zAxis);

	double xMin = DBL_MAX, xMax = 0, yMin = DBL_MAX, yMax = 0, zMin = DBL_MAX, zMax = 0;

	xMin = std::min(xMin, p1(0));
	xMin = std::min(xMin, p2(0));
	xMin = std::min(xMin, p3(0));
	xMin = std::min(xMin, p4(0));

	yMin = std::min(yMin, p1(1));
	yMin = std::min(yMin, p2(1));
	yMin = std::min(yMin, p3(1));
	yMin = std::min(yMin, p4(1));

	zMin = std::min(zMin, p1(2));
	zMin = std::min(zMin, p2(2));
	zMin = std::min(zMin, p3(2));
	zMin = std::min(zMin, p4(2));

	xMax = std::max(xMax, p1(0));
	xMax = std::max(xMax, p2(0));
	xMax = std::max(xMax, p3(0));
	xMax = std::max(xMax, p4(0));

	yMax = std::max(yMax, p1(1));
	yMax = std::max(yMax, p2(1));
	yMax = std::max(yMax, p3(1));
	yMax = std::max(yMax, p4(1));

	zMax = std::max(zMax, p1(2));
	zMax = std::max(zMax, p2(2));
	zMax = std::max(zMax, p3(2));
	zMax = std::max(zMax, p4(2));

	int d1 = (xMax - xMin) / voxelSize;
	int d2 = (yMax - yMin) / voxelSize;
	int d3 = (zMax - zMin) / voxelSize;

	size_t maxArea = std::max(d1 * d2, d2 * d3);

	maxArea = std::max(maxArea, (size_t)d3 * d1);

	return maxArea;
}



void interpolateImage( Eigen::Vector3d xAxis, Eigen::Vector3d yAxis, Eigen::Vector3d origin, double step, int w, int h,
	                   int volumeW , int volumeH , int volumeD , unsigned short *volumeData , cv::Mat& interpolatedImage ,
					   double *transferFunction )
{

	size_t zStep = volumeW * volumeH;
	size_t yStep = volumeW;

	interpolatedImage.create(h, w, CV_8UC1);

	interpolatedImage.setTo(cv::Scalar(0));

	unsigned char *iiData = interpolatedImage.data;

	for (int yy = 0; yy < h; yy++)
		for (int xx = 0; xx < w; xx++)
		{
			Eigen::Vector3d coords = origin + yAxis * yy * step + xAxis * xx * step;

			int x3d = coords(0) / step;
			int y3d = coords(1) / step;
			int z3d = coords(2) / step;

			if ( y3d < 0 || y3d >= volumeW ||
				 y3d < 0 || y3d >= volumeH ||
				 z3d < 0 || z3d >= volumeD )
			{
				iiData[yy * w + xx] = 0;

				continue;
			}


			size_t voxelId = zStep * z3d + yStep * y3d + x3d;

			iiData[yy * w + xx] = transferFunction[ volumeData[voxelId] ] * 255.0;
			

		}

}




void interpolateImage2(Eigen::Vector3d xAxis, Eigen::Vector3d yAxis, Eigen::Vector3d origin, double step, int w, int h,
	int volumeW, int volumeH, int volumeD, unsigned short *volumeData, cv::Mat& interpolatedImage,
	double *transferFunction)
{

	size_t zStep = volumeW * volumeH;
	size_t yStep = volumeW;

	interpolatedImage.create(h, w, CV_8UC1);

	interpolatedImage.setTo(cv::Scalar(0));

	unsigned char *iiData = interpolatedImage.data;

	float shiftX =  w * 0.5;
	float shiftY =  h * 0.5;

	for (int yy = 0; yy < h; yy++)
		for (int xx = 0; xx < w; xx++)
		{
			Eigen::Vector3d coords = -origin + yAxis * ( yy - shiftY ) * step  + xAxis * ( xx - shiftX ) * step;//origin + 

			int x3d = coords(0) / step;
			int y3d = coords(1) / step;
			int z3d = coords(2) / step;

			if (x3d < 0 || x3d >= volumeW ||
				y3d < 0 || y3d >= volumeH ||
				z3d < 0 || z3d >= volumeD)
			{
				iiData[yy * w + xx] = 0;

				continue;
			}


			size_t voxelId = zStep * z3d + yStep * y3d + x3d;

			//triliear interpolation is required here
			iiData[yy * w + xx] = transferFunction[volumeData[voxelId]] * 255.0;
		}
}


void interpolateImage3(Eigen::Vector3d xAxis, Eigen::Vector3d yAxis, Eigen::Matrix4d& transform, double step, int w, int h,
	int volumeW, int volumeH, int volumeD, unsigned short *volumeData, cv::Mat& interpolatedImage,
	double *transferFunction)
{

	size_t zStep = volumeW * volumeH;
	size_t yStep = volumeW;

	interpolatedImage.create(h, w, CV_8UC1);

	interpolatedImage.setTo(cv::Scalar(0));

	unsigned char *iiData = interpolatedImage.data;

	float shiftY = w * 0.50;
	float shiftX = h * 0.50;

	std::cout << " slice w and h : " << w << " " << h << std::endl;

	std::cout << " volume w and h " << volumeW * step << " " << volumeH * step << std::endl;

	Eigen::Vector4d val = transform * Eigen::Vector4d(0, 0, 0, 1);

	std::cout << val.transpose() << std::endl;

	for (int yy = 0; yy < h; yy++)
		for (int xx = 0; xx < w; xx++)
		{
			Eigen::Vector4d coords( ( xx - shiftX) *
				step, ( yy - shiftY) * step, 0 , 1); //= -origin + yAxis * (yy - shiftY) * step + xAxis * (xx - shiftX) * step;//origin + 

			Eigen::Vector4d tc = transform * coords;

			double wc = tc(3);

			tc /= wc;

			int x3d = tc(0) / step;//coords(0) / step;
			int y3d = tc(1) / step;// coords(1) / step;
			int z3d = tc(2) / step;// coords(2) / step;

			if (x3d < 0 || x3d >= volumeW ||
				y3d < 0 || y3d >= volumeH ||
				z3d < 0 || z3d >= volumeD)
			{
				iiData[yy * w + xx] = 0;

				continue;
			}


			size_t voxelId = zStep * z3d + yStep * y3d + x3d;

			//triliear interpolation is required here
			iiData[yy * w + xx] = transferFunction[volumeData[voxelId]] * 255.0;
		}
}



void interpolateImage2(double* xAxis, double* yAxis, double* origin, double step, int w, int h,
	int volumeW, int volumeH, int volumeD, unsigned short *volumeData, unsigned char* interpolatedImage,
	double *transferFunction)
{

	size_t zStep = volumeW * volumeH;
	size_t yStep = volumeW;

	unsigned char *iiData = interpolatedImage;

	float shiftY = w * 0.5;
	float shiftX = h * 0.5;

	for (int yy = 0; yy < h; yy++)
		for (int xx = 0; xx < w; xx++)
		{
			double coords[3];

			coords[0] = origin[0] + yAxis[0] * (yy - shiftY) * step + xAxis[0] * (xx - shiftX) * step;
			coords[1] = origin[1] + yAxis[1] * (yy - shiftY) * step + xAxis[1] * (xx - shiftX) * step;
			coords[2] = origin[2] + yAxis[2] * (yy - shiftY) * step + xAxis[2] * (xx - shiftX) * step;

			int x3d = coords[0] / step;
			int y3d = coords[1] / step;
			int z3d = coords[2] / step;

			if (x3d < 0 || x3d >= volumeW ||
				y3d < 0 || y3d >= volumeH ||
				z3d < 0 || z3d >= volumeD)
			{
				iiData[yy * w + xx] = 0;

				continue;
			}


			size_t voxelId = zStep * z3d + yStep * y3d + x3d;

			//triliear interpolation is required here
			iiData[yy * w + xx] = transferFunction[volumeData[voxelId]] * 255.0;
		}
}




inline double dot3(double *v1, double *shift, double *v2)
{
	return ((v1[0] - shift[0]) * v2[0] + (v1[1] - shift[1]) * v2[1] + (v1[2] - shift[2]) * v2[2]);
}

void intersectionWithXAxis(double *n, double* a, double wS, double hS, double dS,
	double* xAxis, double* yAxis, double* zAxis, double voxelSize, double* sliceOrigin
	,  int& outputW, int& outputH)
{

	double startPoints[12] = { 0, 0, 0, 0, hS, 0, 0, hS, dS, 0, 0, dS };

	intersectPlaneWithBoxParallelSides( n, a, startPoints, 0, wS, hS, dS, xAxis, yAxis, zAxis, 
		                                voxelSize, sliceOrigin, outputW, outputH);
}


void intersectionWithYAxis(double *n, double* a, double wS, double hS, double dS,
	double* xAxis, double* yAxis, double* zAxis, double voxelSize, double* sliceOrigin
	, int& outputW, int& outputH)
{

	double startPoints[12] = { 0, 0, 0, wS, 0, 0, wS, 0, dS, 0, 0, dS };

	intersectPlaneWithBoxParallelSides(n, a, startPoints, 1, wS, hS, dS, xAxis, yAxis, zAxis,
		voxelSize, sliceOrigin,  outputW, outputH);


}


void intersectionWithZAxis(double *n, double* a, double wS, double hS, double dS,
	double* xAxis, double* yAxis, double* zAxis, double voxelSize, double* sliceOrigin
	, int outputW, int outputH)
{
	double startPoints[12] = { 0, 0, 0, wS, 0, 0, wS, hS, 0, 0, hS, 0 };

	intersectPlaneWithBoxParallelSides(n, a, startPoints, 2, wS, hS, dS, xAxis, yAxis, zAxis,
		voxelSize, sliceOrigin, outputW, outputH); 

}


void intersectPlaneWithBoxParallelSides(double *n, double* a, double *startPoints , int axisId, double wS, double hS, double dS,
	double* xAxis, double* yAxis, double* zAxis, double voxelSize, double* sliceOrigin,
    int& outputW, int& outputH)
{
	double ddotn = n[axisId];

	double aminusp1[3] = { a[0] - startPoints[0], a[1] - startPoints[1], a[2] - startPoints[2] };
	double aminusp2[3] = { a[0] - startPoints[3], a[1] - startPoints[4], a[2] - startPoints[5] };
	double aminusp3[3] = { a[0] - startPoints[6], a[1] - startPoints[7], a[2] - startPoints[8] };
	double aminusp4[3] = { a[0] - startPoints[9], a[1] - startPoints[10], a[2] - startPoints[11] };


	double t1 = dot3(aminusp1, n) / ddotn;
	double t2 = dot3(aminusp2, n) / ddotn;
	double t3 = dot3(aminusp3, n) / ddotn;
	double t4 = dot3(aminusp4, n) / ddotn;


	double intersectedPoint1[3] = { startPoints[0], startPoints[1], startPoints[2] }, intersectedPoint2[3] = { startPoints[3], startPoints[4], startPoints[5] },
		   intersectedPoint3[3] = { startPoints[6], startPoints[7], startPoints[8] }, intersectedPoint4[3] = { startPoints[9], startPoints[10], startPoints[11]};

	intersectedPoint1[axisId] = t1;
	intersectedPoint2[axisId] = t2;
	intersectedPoint3[axisId] = t3;
	intersectedPoint4[axisId] = t4;


	double origin[3] = { a[0], a[1], a[2] };

	double p1[3], p2[3], p3[3], p4[3];

	p1[0] = dot3(intersectedPoint1, origin, xAxis);
	p2[0] = dot3(intersectedPoint2, origin, xAxis);
	p3[0] = dot3(intersectedPoint3, origin, xAxis);
	p4[0] = dot3(intersectedPoint4, origin, xAxis);

	p1[1] = dot3(intersectedPoint1, origin, yAxis);
	p2[1] = dot3(intersectedPoint2, origin, yAxis);
	p3[1] = dot3(intersectedPoint3, origin, yAxis);
	p4[1] = dot3(intersectedPoint4, origin, yAxis);

	p1[2] = dot3(intersectedPoint1, origin, zAxis);
	p2[2] = dot3(intersectedPoint2, origin, zAxis);
	p3[2] = dot3(intersectedPoint3, origin, zAxis);
	p4[2] = dot3(intersectedPoint4, origin, zAxis);

	double xMin = DBL_MAX, xMax = 0, yMin = DBL_MAX, yMax = 0, zMin = DBL_MAX, zMax = 0;

	xMin = std::min(xMin, p1[0]);
	xMin = std::min(xMin, p2[0]);
	xMin = std::min(xMin, p3[0]);
	xMin = std::min(xMin, p4[0]);

	yMin = std::min(yMin, p1[1]);
	yMin = std::min(yMin, p2[1]);
	yMin = std::min(yMin, p3[1]);
	yMin = std::min(yMin, p4[1]);

	zMin = std::min(zMin, p1[2]);
	zMin = std::min(zMin, p2[2]);
	zMin = std::min(zMin, p3[2]);
	zMin = std::min(zMin, p4[2]);

	xMax = std::max(xMax, p1[0]);
	xMax = std::max(xMax, p2[0]);
	xMax = std::max(xMax, p3[0]);
	xMax = std::max(xMax, p4[0]);

	yMax = std::max(yMax, p1[1]);
	yMax = std::max(yMax, p2[1]);
	yMax = std::max(yMax, p3[1]);
	yMax = std::max(yMax, p4[1]);

	zMax = std::max(zMax, p1[2]);
	zMax = std::max(zMax, p2[2]);
	zMax = std::max(zMax, p3[2]);
	zMax = std::max(zMax, p4[2]);

	sliceOrigin[0] = origin[0] + xMin * xAxis[0] + yMin * yAxis[0];
	sliceOrigin[1] = origin[1] + xMin * xAxis[1] + yMin * yAxis[1];
	sliceOrigin[2] = origin[2] + xMin * xAxis[2] + yMin * yAxis[2];

	outputW = (xMax - xMin) / voxelSize;
	outputH = (yMax - yMin) / voxelSize;
}


void interpolateImage(double* xAxis, double* yAxis, double* origin, double step, int w, int h,
	int volumeW, int volumeH, int volumeD, unsigned short *volumeData, unsigned char* interpolatedImage,
	double *transferFunction)
{

	size_t zStep = volumeW * volumeH;
	size_t yStep = volumeW;
	unsigned char *iiData = interpolatedImage;

	for (int yy = 0; yy < h; yy++)
		for (int xx = 0; xx < w; xx++)
		{
			double coords[3];

			coords[0] = origin[0] + yAxis[0] * yy * step + xAxis[0] * xx * step;
			coords[1] = origin[1] + yAxis[1] * yy * step + xAxis[1] * xx * step;
			coords[2] = origin[2] + yAxis[2] * yy * step + xAxis[2] * xx * step;

			int x3d = coords[0] / step;
			int y3d = coords[1] / step;
			int z3d = coords[2] / step;

			if (y3d < 0 || y3d >= volumeW ||
				y3d < 0 || y3d >= volumeH ||
				z3d < 0 || z3d >= volumeD)
			{
				iiData[yy * w + xx] = 0;
				continue;
			}

			size_t voxelId = zStep * z3d + yStep * y3d + x3d;

			iiData[3 * (yy * w + xx)] = transferFunction[volumeData[voxelId]] * 255.0;
			iiData[3 * (yy * w + xx) + 1] = transferFunction[volumeData[voxelId]] * 255.0;
			iiData[3 * (yy * w + xx) + 2] = transferFunction[volumeData[voxelId]] * 255.0;
		}

}


void sliceFromVTK(double *coordSys , unsigned short *volumeDataPtr , unsigned int* voxelResolution , double *currentColorMap , double voxelSize )
{
	vtkSmartPointer<vtkImageImport> import = vtkSmartPointer<vtkImageImport>::New();
	import->SetReleaseDataFlag(1);
	import->SetImportVoidPointer(static_cast<void*>(volumeDataPtr));
	import->SetDataExtent(0, voxelResolution[0] - 1, 0, voxelResolution[1] - 1, 0, voxelResolution[2] - 1);
	import->SetWholeExtent(0, voxelResolution[0] - 1, 0, voxelResolution[1] - 1, 0, voxelResolution[2] - 1);
	import->SetDataSpacing(voxelSize, voxelSize, voxelSize);
	import->SetDataOrigin(0.0, 0.0, 0.0);

	std::cout << voxelResolution[0] << " " << voxelResolution[1] << " " << voxelResolution[2] << std::endl;

	import->SetDataScalarTypeToUnsignedShort();

	vtkSmartPointer<vtkMatrix4x4> vtkCoord = vtkSmartPointer<vtkMatrix4x4>::New();
	vtkCoord->DeepCopy(coordSys);

	vtkCoord->Print(std::cout);

	vtkSmartPointer<vtkImageReslice2> vtkReslice = vtkSmartPointer<vtkImageReslice2>::New();

	vtkReslice->SetInputConnection(import->GetOutputPort());
	vtkReslice->SetOutputDimensionality(2);
	vtkReslice->SetResliceAxes(vtkCoord);

	vtkReslice->Update();

	auto vtkImage = vtkReslice->GetOutput();

	int dims[3];
	
	vtkImage->GetDimensions(dims);

	std::cout << " vtk dim " << dims[1] << " " << dims[0] << std::endl;

	if (dims[0] == 0 || dims[1] == 0 || dims[2] == 0)
		return;


	cv::Mat im(dims[1], dims[0], CV_8UC3);

	

	unsigned char *imageData = im.data;

	unsigned short *gvs = new unsigned short[dims[0] * dims[1]];

	for (int i = 0; i < dims[0] * dims[1]; i++)
	{
		unsigned short grayValue = static_cast<unsigned short>(vtkImage->GetPointData()->GetScalars()->GetTuple1(i));
		double value = currentColorMap[grayValue];

		imageData[3 * i + 0] = (unsigned char)(value * 255);
		imageData[3 * i + 1] = (unsigned char)(value * 255);
		imageData[3 * i + 2] = (unsigned char)(value * 255);

		//writer << grayValue;

		gvs[i] = grayValue;
	}

	cv::namedWindow("image", 0);
	cv::imshow("image", im);
	cv::waitKey();

}


void computeNewBoundingBox(Eigen::Vector3d& origin, Eigen::Vector3d& xAxis, Eigen::Vector3d& yAxis, Eigen::Vector3d& zAxis, 
	double wS, double hS, double dS, double voxelSize,Eigen::Matrix4d& transform )
{

	double xMin = DBL_MAX, xMax = -FLT_MAX, yMin = DBL_MAX, yMax = -FLT_MAX, zMin = DBL_MAX, zMax = -FLT_MAX;;

	double xMin2 = DBL_MAX, xMax2 = -FLT_MAX, yMin2 = DBL_MAX, yMax2 = -FLT_MAX, zMin2 = DBL_MAX, zMax2 = -FLT_MAX;;

	for (int ii = 0; ii < 2; ii++)
		for (int jj = 0; jj < 2; jj++)
			for (int kk = 0; kk < 2; kk++)
			{
				Eigen::Vector3d corner = origin + (ii  * wS - wS / 2) * xAxis +
					(jj  * hS - hS / 2) * yAxis + (kk  * dS - dS / 2) * zAxis;

				Eigen::Vector4d corner4(ii * wS, jj * hS, kk * dS , 1);

				Eigen::Vector4d proj4 = transform * corner4;

				proj4 /= proj4(3);

				double xProj = proj4(0);
				double yProj = proj4(1);
				double zProj = proj4(2);

				xMin = std::min(xMin, xProj);
				xMax = std::max(xMax, xProj);

				yMin = std::min(yMin, yProj);
				yMax = std::max(yMax, yProj);

				zMin = std::min(zMin, zProj);
				zMax = std::max(zMax, zProj);

				xMin2 = std::min(corner(0), xMin2);
				xMax2 = std::max(corner(0), xMax2);

				yMin2 = std::min(corner(1), yMin2);
				yMax2 = std::max(corner(1), yMax2);

				zMin2 = std::min(corner(2), zMin2);
				zMax2 = std::max(corner(2), zMax2);
			}


	       //int w = (xMax - xMin) / voxelSize;

	       std::cout << " extent : " << (xMax2 - xMin2) / voxelSize << " " << (yMax2 - yMin2) / voxelSize << 
			         " " << (zMax2 - zMin2) / voxelSize << std::endl;

		   //std::cout << " slice computed center : " << (xMax + xMin) * 0.5 / voxelSize << " " << (yMin + yMax) * 0.5 / voxelSize << std::endl;

	

}


void computeSliceDimensions( double wS, double hS, double dS, double voxelSize, Eigen::Matrix4d& transform , int& w , int& h )
{

	double xMin = DBL_MAX, xMax = -FLT_MAX, yMin = DBL_MAX,
		   yMax = -FLT_MAX, zMin = DBL_MAX, zMax = -FLT_MAX;;

	for (int ii = 0; ii < 2; ii++)
		for (int jj = 0; jj < 2; jj++)
			for (int kk = 0; kk < 2; kk++)
			{
				Eigen::Vector3d corner(ii * wS, jj * hS, kk * dS);



				Eigen::Vector4d corner4(ii * wS , jj * hS , kk * dS, 1);

				Eigen::Vector4d proj4 = transform * corner4;

				proj4 /= proj4(3);

				double xProj = proj4(0);
				double yProj = proj4(1);
				double zProj = proj4(2);

				xMin = std::min(xMin, xProj);
				xMax = std::max(xMax, xProj);

				yMin = std::min(yMin, yProj);
				yMax = std::max(yMax, yProj);

				zMin = std::min(zMin, zProj);
				zMax = std::max(zMax, zProj);
			}

	w = (xMax - xMin) / voxelSize;
	h = (yMax - yMin) / voxelSize;

	std::cout << " slice center " << (xMax + xMin) * 0.5 << " " << (yMax + yMin) * 0.5<<" "<<( zMax + zMin ) * 0.5 << std::endl;

}


//Matrix is m x n and vector is n x 1
void matrixVectorMultiplication(int m, int n, const double *matrix, const double *vec , double *outputVec)
{
	for (int rr = 0; rr < m; rr++)
	{
		float sumVal = 0;

		const double *currentRow = matrix + rr * n;

		for (int cc = 0; cc < n; cc++)
		{
			sumVal += currentRow[cc] * vec[cc];
		}

		outputVec[rr] = sumVal;
	}


}

void computeSliceDimensions(double wS, double hS, double dS, double voxelSize, double* transform, int& w, int& h)
{

	double xMin = DBL_MAX, xMax = -FLT_MAX, yMin = DBL_MAX,
		yMax = -FLT_MAX, zMin = DBL_MAX, zMax = -FLT_MAX;;

	for (int ii = 0; ii < 2; ii++)
		for (int jj = 0; jj < 2; jj++)
			for (int kk = 0; kk < 2; kk++)
			{
				//double corner[3] = { ii * wS, jj * hS, kk * dS };

				double corner4[4] = { ii * wS, jj * hS, kk * dS, 1 };

				//Eigen::Vector4d proj4 = transform * corner4;

				double proj4[4];

				matrixVectorMultiplication(4, 4, transform, corner4, proj4);

				proj4[0] /= proj4[3];
				proj4[1] /= proj4[3];
				proj4[2] /= proj4[3];

				double xProj = proj4[0];
				double yProj = proj4[1];
				double zProj = proj4[2];

				xMin = std::min(xMin, xProj);
				xMax = std::max(xMax, xProj);

				yMin = std::min(yMin, yProj);
				yMax = std::max(yMax, yProj);

				zMin = std::min(zMin, zProj);
				zMax = std::max(zMax, zProj);
			}

	w = (xMax - xMin) / voxelSize;
	h = (yMax - yMin) / voxelSize;

}
#else

#include <iostream>
#include <vector>
#include <assert.h>
#include <cmath>
#include "opencvincludes.h"
//#include <png++/png.hpp>

using namespace std;

typedef vector<double> Array;
typedef vector<Array> Matrix;
typedef vector<Matrix> Image;

Matrix getGaussian(int height, int width, double sigma)
{
	Matrix kernel(height, Array(width));
	double sum = 0.0;
	int i, j;

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			kernel[i][j] = exp(-(i * i + j * j) / (2 * sigma * sigma)) / (2 * M_PI * sigma * sigma);
			sum += kernel[i][j];
		}
	}

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			kernel[i][j] /= sum;
		}
	}

	return kernel;
}

Image loadImage(const char* filename)
{
	//png::image<png::rgb_pixel> image(filename);

	cv::Mat image = cv::imread(filename);

	Image imageMatrix(3, Matrix(image.rows, Array(image.cols)));

	int h, w;
	for (h = 0; h < image.rows; h++) {
		for (w = 0; w < image.cols; w++) {

			cv::Vec3b col = image.at<cv::Vec3b>(h, w);

			imageMatrix[0][h][w] = col[0];//image[h][w].red;
			imageMatrix[1][h][w] = col[1];//image[h][w].green;
			imageMatrix[2][h][w] = col[2];//image[h][w].blue;
		}
	}

	return imageMatrix;
}

void saveImage(Image& image, const char* filename)
{
	assert(image.size() == 3);

	int height = image[0].size();
	int width = image[0][0].size();
	int x, y;

	//png::image<png::rgb_pixel> imageFile(width, height);

	cv::Mat saveImage(height, width, CV_8UC3);

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {

			auto& col = saveImage.at<cv::Vec3b>(y, x);
			col[0] = image[0][y][x];
			col[1] = image[1][y][x];
			col[2] = image[2][y][x];
		}
	}
	//imageFile.write(filename);

	cv::imwrite(filename, saveImage);
}

Image applyFilter(Image& image, Matrix& filter) {
	assert(image.size() == 3 && filter.size() != 0);

	int height = image[0].size();
	int width = image[0][0].size();
	int filterHeight = filter.size();
	int filterWidth = filter[0].size();
	int newImageHeight = height - filterHeight + 1;
	int newImageWidth = width - filterWidth + 1;
	int d, i, j, h, w;

	Image newImage(3, Matrix(newImageHeight, Array(newImageWidth)));

	for (d = 0; d < 3; d++) {
		for (i = 0; i < newImageHeight; i++) {
			for (j = 0; j < newImageWidth; j++) {
				for (h = i; h < i + filterHeight; h++) {
					for (w = j; w < j + filterWidth; w++) {
						newImage[d][i][j] += filter[h - i][w - j] * image[d][h][w];
					}
				}
			}
		}
	}

	return newImage;
}

Image applyFilter(Image& image, Matrix& filter, int times)
{
	Image newImage = image;
	for (int i = 0; i < times; i++) {
		newImage = applyFilter(newImage, filter);
	}
	return newImage;
}

int main(int argc, char** argv)
{
	int height = 7;// atoi(argv[1]);
	int width = 7;// atoi(argv[2]);
	int sigma = 2.0;// atoi(argv[3]);
	Matrix filter = getGaussian(height, width, sigma);

	cout << "Loading image..." << endl;
	Image image = loadImage("D:\\Projects\\CPP_CPU\\Untitled Folder\\image.png");
	cout << "Applying filter..." << endl;
	Image newImage = applyFilter(image, filter);
	cout << "Saving image..." << endl;
	saveImage(newImage, "D:\\Projects\\CPP_CPU\\Untitled Folder\\newImage2.png");
	cout << "Done!" << endl;
}

#endif
