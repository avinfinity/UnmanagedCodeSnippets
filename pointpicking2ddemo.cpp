

#include "iostream"
#include "opencvincludes.h"
#include "eigenincludes.h"
#include "vtkincludes.h"
#include "volumeutility.h"
#include "rawvolumedataio.h"
#include "QString"


unsigned short valueAt(float x, float y, unsigned int width, unsigned int height, unsigned short *sliceData);

void computeSearchRangeStatistics(Eigen::Vector3d& xAxis, Eigen::Vector3d& yAxis, Eigen::Vector3d& clickedPoint, Eigen::Vector3d& voxelSize, double searchRange, double angleStep);

float findBoundary(Eigen::Vector3d& dir, unsigned short *data, Eigen::Vector3d& startPosition, double step, int extent,
	Eigen::Vector3d& xAxis, Eigen::Vector3d& yAxis, Eigen::Vector3d& voxelSize, unsigned int width, unsigned int height,
	std::vector< std::pair< unsigned int, unsigned int > >& materials, std::vector< unsigned short >& materialIds);

void normalAtPoint(unsigned short *volume , int width , int height , int depth , double *voxelStep , double *position, double *normal);
unsigned short trilinearInterpolation(const unsigned short *volumeData, double x, double y, double z,
	unsigned int volumeW, unsigned int volumeH);


class SobelGradientOperator3x3
{



public:

	SobelGradientOperator3x3();

	void init();

	void convolve(float x, float y, float z, unsigned int volumeW, unsigned int  volumeH, unsigned int volumeD,
		unsigned short *volData, double *gradient);

protected:

	double _KernelGx[3][3][3], _KernelGy[3][3][3], _KernelGz[3][3][3];


};




int main( int argc , char **argv )
{
	
	std::cout<<" point picking 2d demo "<<std::endl;

	cv::Mat sliceImage;
    
	std::vector< float > transferFunction(4 * 256);

	std::string sliceDataPath = "C:\\Data\\slice.dat";

	std::string transferFunctionPath = "C:\\Data\\colormap.dat";

	QString volumePath = "C:\\Projects\\MultiMaterialMeasurements\\Data\\Acceptance test\\Fuel_filter_0km_PR_1 2016-5-30 12-54.uint16_scv";

	imt::volume::VolumeInfo volume;

	imt::volume::RawVolumeDataIO::readUint16SCV(volumePath, volume);

	std::cout << volume.mWidth << " , " << volume.mHeight << " , " << volume.mDepth << std::endl;

	FILE *sliceDataFile = fopen(sliceDataPath.c_str(), "rb");
	FILE *transferFuncFile = fopen( transferFunctionPath.c_str() , "rb" );

	int sliceWidth = 0 , sliceHeight = 0;

	fread( &sliceWidth , sizeof( int ) , 1 , sliceDataFile );
	fread( &sliceHeight , sizeof(int) , 1 , sliceDataFile);

	std::cout << " slice width and height : " << sliceWidth << " " << sliceHeight << std::endl;

	std::vector< unsigned short > rawSliceData( sliceWidth * sliceHeight );

	fread(rawSliceData.data(), sizeof(unsigned short) , sliceWidth * sliceHeight , sliceDataFile);

	fread(transferFunction.data(), sizeof(float), 256 * 4, transferFuncFile);

	sliceImage.create(sliceHeight, sliceWidth, CV_8UC4);

	unsigned char *sliceImageData = sliceImage.data;

	for (int yy = 0; yy < sliceHeight; yy++)
		for (int xx = 0; xx < sliceWidth; xx++)
		{
			float grayValue = rawSliceData[yy * sliceWidth + xx];

			unsigned int histogramBin = grayValue / 256.0;

			sliceImageData[4 * yy * sliceWidth + 4 * xx] = transferFunction[4 * histogramBin + 2] * 255;
			sliceImageData[4 * yy * sliceWidth + 4 * xx + 1] = transferFunction[4 * histogramBin + 1] * 255;
			sliceImageData[4 * yy * sliceWidth + 4 * xx + 2] = transferFunction[4 * histogramBin] * 255;
			sliceImageData[4 * yy * sliceWidth + 4 * xx + 3] = transferFunction[4 * histogramBin + 3] * 255;
		}


	Eigen::Vector3d slicePosition, xAxis, yAxis, voxelSize,clickedPoint;

	fread(slicePosition.data(), sizeof(double), 3, sliceDataFile);
	fread(xAxis.data(), sizeof(double), 3, sliceDataFile);
	fread(yAxis.data(), sizeof(double), 3, sliceDataFile);
	fread(voxelSize.data(), sizeof(double), 3, sliceDataFile);
	fread(clickedPoint.data(), sizeof(double), 3, sliceDataFile);

	int numberOfMaterials = 0;

	fread(&numberOfMaterials, sizeof(int), 1, sliceDataFile);

	std::vector< std::pair< unsigned int, unsigned int > > materials( numberOfMaterials );

	fread(materials.data(), sizeof(std::pair< unsigned int, unsigned int >), numberOfMaterials, sliceDataFile);

	std::cout << " number of materials : " << numberOfMaterials << std::endl;

	for (int mm = 0; mm < numberOfMaterials; mm++)
	{
		std::cout << materials[mm].first << " " << materials[mm].second << std::endl;
	}

	//return 0;

	Eigen::Vector3d diff = clickedPoint - slicePosition;


	

	std::vector< unsigned short > materialIds(65535, 0);

	//now populate material ids

	int nMaterials = materials.size();

	for (int mm = 1; mm <= nMaterials; mm++)
	{
		int startVal = materials[mm - 1].first;
		int endVal = materials[mm - 1].second;

		for (int gg = startVal; gg < endVal; gg++)
		{
			materialIds[gg] = mm;
		}
	}


	float halfWidth = sliceWidth * 0.5;
	float halfHeight = sliceHeight * 0.5;

	std::cout << " diff vector : " << diff.transpose() << " " << diff.dot(xAxis) / voxelSize(0) + halfWidth << " " << diff.dot(yAxis) / voxelSize(0) + halfHeight << std::endl;

	cv::Point center(diff.dot(xAxis) / voxelSize(0) + halfWidth, diff.dot(yAxis) / voxelSize(0) + halfHeight);

	cv::circle(sliceImage, cv::Point(diff.dot(xAxis) / voxelSize(0) + halfWidth, diff.dot(yAxis) / voxelSize(0) + halfHeight), 3, cv::Scalar(0, 0, 255, 255), 2);

	//now generate 36 rays

	double slopeStep = 10.0 / 180.0 * M_PI;

	double searchRange = 10 * voxelSize(0);

	double angle = 0;


	float boundingBoxXExtent = 0 , boundingBoxYExtent = 0;

	double minValX = FLT_MAX, maxValX = 0;
	double minValY = FLT_MAX, maxValY = 0;

	double minDist = -1;

	Eigen::Vector3d minDistDir;


	for ( int ii = 0; ii < 36; ii++ )
	{
		Eigen::Vector3d dir = xAxis * cos(angle) + yAxis * sin(angle);

		angle += slopeStep;

		dir.normalize();

		Eigen::Vector3d otherEnd = diff + dir * searchRange;

		cv::Point end(otherEnd.dot(xAxis) / voxelSize(0) + halfWidth, otherEnd.dot(yAxis) / voxelSize(0) + halfHeight);

		double xProj = otherEnd.dot(xAxis);
		double yProj = otherEnd.dot(yAxis);

		minValX = std::min(minValX, xProj);
		maxValX = std::max(maxValX, xProj);

		minValY = std::min(minValY, yProj);
		maxValY = std::max(maxValY, yProj);

		//cv::line(sliceImage, center, end, cv::Scalar(255, 0, 0, 255));

		float dist = findBoundary(dir, rawSliceData.data(), diff, voxelSize(0) * 0.5, 20, xAxis, yAxis, voxelSize , sliceWidth, sliceHeight, materials, materialIds);
	
		if (dist > 0.0f)
		{


			if ( minDist < 0 || minDist > dist )
			{
				minDist = dist;

				minDistDir = dir;
			}

		}
	}

	Eigen::Vector3d boundaryPoint = diff + minDistDir * minDist;

	cv::circle(sliceImage, cv::Point(boundaryPoint.dot(xAxis) / voxelSize(0) + halfWidth, boundaryPoint.dot(yAxis) / voxelSize(0) + halfHeight),
		1, cv::Scalar(0, 255, 0, 255));

	std::cout << (maxValX - minValX) / voxelSize(0) << " " << (maxValY - minValY) / voxelSize(1) << std::endl;

	cv::namedWindow( "sliceImage" , 0 );
	//cv::moveWindow("sliceImage", 0, 0);
	//cv::setOpenGlContext("sliceImage");

	//cv::setOpenGlDrawCallback();
	
	cv::imshow("sliceImage", sliceImage);
	cv::waitKey();

	fclose(sliceDataFile);
	fclose(transferFuncFile);




	
	
	
	return 0;
}

unsigned short valueAt(float x, float y, unsigned int width, unsigned int height, unsigned short *sliceData);
#define grayValue(x , y , z)  volumeData[ zStep * z + yStep * y + x ] 



unsigned short trilinearInterpolation(const unsigned short *volumeData, double x, double y, double z,
	unsigned int volumeW, unsigned int volumeH )
{
	unsigned short interpolatedValue = 0;

	size_t zStep = volumeW * volumeH;
	size_t yStep = volumeW;

	int lx = (int)x;
	int ly = (int)y;
	int lz = (int)z;

	int ux = (int)std::ceil(x);
	int uy = (int)std::ceil(y);
	int uz = (int)std::ceil(z);

	double xV = x - lx;
	double yV = y - ly;
	double zV = z - lz;

	double c000 = grayValue(lx, ly, lz);
	double c100 = grayValue(ux, ly, lz);
	double c010 = grayValue(lx, uy, lz);
	double c110 = grayValue(ux, uy, lz);
	double c001 = grayValue(lx, ly, uz);
	double c101 = grayValue(ux, ly, uz);
	double c011 = grayValue(lx, uy, uz);
	double c111 = grayValue(ux, uy, uz);

	double interpolatedValF = c000 * (1.0 - xV) * (1.0 - yV) * (1.0 - zV) +
		c100 * xV * (1.0 - yV) * (1.0 - zV) +
		c010 * (1.0 - xV) * yV * (1.0 - zV) +
		c110 * xV * yV * (1.0 - zV) +
		c001 * (1.0 - xV) * (1.0 - yV) * zV +
		c101 * xV * (1.0 - yV) * zV +
		c011 * (1.0 - xV) * yV * zV +
		c111 * xV * yV * zV;

	interpolatedValue = (unsigned short)interpolatedValF;

	return interpolatedValue;
}




void generateROIImage(Eigen::Vector3d& xAxis, Eigen::Vector3d& yAxis, Eigen::Vector3d& roiOrigin,
	double step, int xDivisions, int yDivisions, unsigned short *volumeData, int volumeW, int volumeH, int volumeD, unsigned short *sliceData)
{

	for (int yy = 0; yy < yDivisions; yy++)
		for (int xx = 0; xx < xDivisions; xx++)
		{
			Eigen::Vector3d pixelCoordinates = roiOrigin + xx * step * xAxis + yy * step * yAxis;

			unsigned short interpolatedValue = trilinearInterpolation(volumeData, pixelCoordinates(0), pixelCoordinates(1), pixelCoordinates(2),volumeW, volumeH);

			sliceData[yy * xDivisions + xx] = interpolatedValue;
		}


}


float findBoundary( Eigen::Vector3d& dir , unsigned short *data , Eigen::Vector3d& startPosition , double step , int extent ,
	               Eigen::Vector3d& xAxis, Eigen::Vector3d& yAxis , Eigen::Vector3d& voxelSize, unsigned int width, unsigned int height, 
				   std::vector< std::pair< unsigned int, unsigned int > >& materials, std::vector< unsigned short >& materialIds  )
{

	float halfWidth = width * 0.5f;
	float halfHeight = height * 0.5f;


	std::vector<int> grayValues( extent , 0 ) , rayMaterialIds( extent , 0 );

	bool boundaryFound = false;

	float distance = -1.0f;

	for ( int ee = 0; ee < extent; ee++ )
	{
		Eigen::Vector3d currentPos = startPosition + ee * step * dir;

		float x = currentPos.dot(xAxis) / voxelSize(0) + halfWidth;
		float y = currentPos.dot(yAxis) / voxelSize(0) + halfHeight;

		if (x < 1 || x > width - 2 || y < 1 || y > height - 2)
		{
			grayValues[ee] = -1;

			rayMaterialIds[ee] = -1;

			continue;
		}

		grayValues[ee] = valueAt(x, y, width, height, data);

		rayMaterialIds[ee] = materialIds[grayValues[ee]];

		if (ee == 0)
			continue;

		if ( rayMaterialIds[ee] != rayMaterialIds[ee - 1] )
		{
			boundaryFound = true;

			distance = ee * step;
			break;
		}
		else
		{
			//std::cout <<  rayMaterialIds[ee - 1] << " " << rayMaterialIds[ee] << " " << step * ee << std::endl;
		}
	}

	return distance;

}


unsigned short valueAt(float x, float y, unsigned int width, unsigned int height, unsigned short *sliceData)
{
	unsigned short grayValue = 0;

	int lx = (int)x;
	int ly = (int)y;

	int ux = lx + 1;
	int uy = ly + 1;

	float dx = x - lx;
	float dy = y - ly;

	float c00 = sliceData[ ly * width + lx];
	float c01 = sliceData[uy * width + lx];
	float c11 = sliceData[uy * width + ux];
	float c10 = sliceData[ ly * width + ux];

	float interpolatedValueF = c00 * (1 - dx) * (1 - dy) + c01 * (1 - dx) * dy + c11 * dx * dy + c10 * dx * (1 - dy);

	unsigned short interpolatedValue = (int)interpolatedValueF;

	return interpolatedValue;
}


void normalAtPoint(unsigned short *volume, int width, int height, int depth, double *voxelStep, double *position, double *normal)
{


}



SobelGradientOperator3x3::SobelGradientOperator3x3()
{
	init();
}



void SobelGradientOperator3x3::init()
{
	int hx[3] = { 1, 2, 1 }, hy[3] = { 1, 2, 1 }, hz[3] = { 1, 2, 1 };
	int hpx[3] = { 1, 0, -1 }, hpy[3] = { 1, 0, -1 }, hpz[3] = { 1, 0, -1 };

	//build the kernel
	for (int m = 0; m <= 2; m++)
		for (int n = 0; n <= 2; n++)
			for (int k = 0; k <= 2; k++)
			{
				_KernelGx[m][n][k] = hpx[m] * hy[n] * hz[k];
				_KernelGy[m][n][k] = hx[m] * hpy[n] * hz[k];
				_KernelGz[m][n][k] = hx[m] * hy[n] * hpz[k];
			}

}


//do note that this method does not perform any check whether elements accessed with this method exceeds the boundary of the image
void SobelGradientOperator3x3::convolve(float x, float y, float z, unsigned int volumeW, unsigned int  volumeH , unsigned int volumeD,
	unsigned short *volData, double *gradient)
{

	double sumx = 0, sumy = 0, sumz = 0;

	for ( int mm = -1; mm <=  1;  mm++)
		for ( int nn = -1; nn <= 1; nn++)
			for ( int kk = -1; kk <= 1; kk++)
			{
				unsigned short gVal = trilinearInterpolation(volData, x + kk, y + nn, z + mm, volumeW, volumeH); 

				sumx += _KernelGx[mm + 1][nn + 1][kk + 1] * gVal;
				sumy += _KernelGy[mm + 1][nn + 1][kk + 1] * gVal;
				sumz += _KernelGz[mm + 1][nn + 1][kk + 1] * gVal;
			}


	gradient[0] = sumx;
	gradient[1] = sumy;
	gradient[2] = sumz;

}


void findGradientMaximaPoint( double *initPoint , double* dir , unsigned short* volume  )
{



}


