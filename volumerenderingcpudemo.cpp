/*
  Copyright (c) 2011-2014, Intel Corporation
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

	* Redistributions of source code must retain the above copyright
	  notice, this list of conditions and the following disclaimer.

	* Redistributions in binary form must reproduce the above copyright
	  notice, this list of conditions and the following disclaimer in the
	  documentation and/or other materials provided with the distribution.

	* Neither the name of Intel Corporation nor the names of its
	  contributors may be used to endorse or promote products derived from
	  this software without specific prior written permission.


   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#pragma warning(disable : 4244)
#pragma warning(disable : 4305)
#endif

#include "VolumeAnalytics/timing.h"
#include "VolumeAnalytics/volume_ispc.h"
#include <algorithm>
#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <string>

#include <iostream>
#include "volumeinfo.h"
#include "rawvolumedataio.h"
#include "volumesegmenter.h"
#include "display3droutines.h"
#include "histogramfilter.h"
#include "marchingcubes.h"
#include "stdio.h"
#include <fstream>
#include "meshviewer.h"
#include "multiresmarchingcubes.h"
#include "MultiResolutionMarchingCubes.h"
#include "MultiResolutionMarchingCubes2.h"
#include "VolumeAnalytics/volumerenderercpu.h"
#include "VolumeAnalytics/TransferFunction.h"
#include <vtkFillHolesFilter.h>
#include <vtkPLYWriter.h>
#include "vtkCellData.h"
#include "ipp.h"
#include "ippvm.h"



using namespace ispc;

extern void volume_serial(float density[], int nVoxels[3], const float raster2camera[4][4],
	const float camera2world[4][4], int width, int height, float image[]);

/* Write a PPM image file with the image */
static void writePPM(float* buf, int width, int height, const char* fn) {
	FILE* fp = fopen(fn, "wb");
	fprintf(fp, "P6\n");
	fprintf(fp, "%d %d\n", width, height);
	fprintf(fp, "255\n");
	for (int i = 0; i < width * height; ++i) {
		float v = buf[i] * 255.f;
		if (v < 0.f)
			v = 0.f;
		else if (v > 255.f)
			v = 255.f;
		unsigned char c = (unsigned char)v;
		for (int j = 0; j < 3; ++j)
			fputc(c, fp);
	}
	fclose(fp);
	printf("Wrote image file %s\n", fn);
}

/* Load image and viewing parameters from a camera data file.
   FIXME: we should add support to be able to specify viewing parameters
   in the program here directly. */
static void loadCamera(const char* fn, int* width, int* height, float raster2camera[4][4], float camera2world[4][4]) {
	FILE* f = fopen(fn, "r");
	if (!f) {
		perror(fn);
		exit(1);
	}
	if (fscanf(f, "%d %d", width, height) != 2) {
		fprintf(stderr, "Unexpected end of file in camera file\n");
		exit(1);
	}

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			if (fscanf(f, "%f", &raster2camera[i][j]) != 1) {
				fprintf(stderr, "Unexpected end of file in camera file\n");
				exit(1);
			}
		}
	}
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			if (fscanf(f, "%f", &camera2world[i][j]) != 1) {
				fprintf(stderr, "Unexpected end of file in camera file\n");
				exit(1);
			}
		}
	}
	fclose(f);
}

/* Load a volume density file.  Expects the number of x, y, and z samples
   as the first three values (as integer strings), then x*y*z
   floating-point values (also as strings) to give the densities.  */
static float* loadVolume(const char* fn, int n[3]) {
	FILE* f = fopen(fn, "r");
	if (!f) {
		perror(fn);
		exit(1);
	}

	if (fscanf(f, "%d %d %d", &n[0], &n[1], &n[2]) != 3) {
		fprintf(stderr, "Couldn't find resolution at start of density file\n");
		exit(1);
	}

	int count = n[0] * n[1] * n[2];
	float* v = new float[count];
	for (int i = 0; i < count; ++i) {
		if (fscanf(f, "%f", &v[i]) != 1) {
			fprintf(stderr, "Unexpected end of file at %d'th density value\n", i);
			exit(1);
		}
	}

	return v;
}



#if 0
int main(int argc, char* argv[]) {
	static unsigned int test_iterations[] = { 3, 7, 1 };
	//if (argc < 3) {
	//	fprintf(stderr, "usage: volume <camera.dat> <volume_density.vol> [ispc iterations] [tasks iterations] [serial "
	//		"iterations]\n");
	//	return 1;
	//}
	if (argc == 6) {
		for (int i = 0; i < 3; i++) {
			test_iterations[i] = atoi(argv[3 + i]);
		}
	}

	std::string cameraPath = "D:\\Data\\camera.dat";
	std::string volumePath = "D:\\Data\\density_highres.vol";

	//
	// Load viewing data and the volume density data
	//
	int width, height;
	float raster2camera[4][4], camera2world[4][4];
	//loadCamera(argv[1], &width, &height, raster2camera, camera2world);
	loadCamera(cameraPath.c_str(), &width, &height, raster2camera, camera2world);
	float* image = new float[width * height];

	int n[3];
	float* density = loadVolume(volumePath.c_str(), n);//loadVolume(argv[2], n);

	//
	// Compute the image using the ispc implementation; report the minimum
	// time of three runs.
	//
	double minISPC = 1e30;
	for (unsigned int i = 0; i < test_iterations[0]; ++i) {
		reset_and_start_timer();
		volume_ispc(density, n, raster2camera, camera2world, width, height, image);
		double dt = get_elapsed_mcycles();
		printf("@time of ISPC run:\t\t\t[%.3f] million cycles\n", dt);
		minISPC = std::min(minISPC, dt);
	}

	printf("[volume ispc 1 core]:\t\t[%.3f] million cycles\n", minISPC);
	writePPM(image, width, height, "D:\\Data\\volume-ispc-1core.ppm");

	// Clear out the buffer
	for (int i = 0; i < width * height; ++i)
		image[i] = 0.;

	//
	// Compute the image using the ispc implementation that also uses
	// tasks; report the minimum time of three runs.
	//
	double minISPCtasks = 1e30;
	for (unsigned int i = 0; i < test_iterations[1]; ++i) {
		reset_and_start_timer();
		volume_ispc_tasks(density, n, raster2camera, camera2world, width, height, image);
		double dt = get_elapsed_mcycles();
		printf("@time of ISPC + TASKS run:\t\t\t[%.3f] million cycles\n", dt);
		minISPCtasks = std::min(minISPCtasks, dt);
	}

	printf("[volume ispc + tasks]:\t\t[%.3f] million cycles\n", minISPCtasks);
	writePPM(image, width, height, "D:\\Data\\volume-ispc-tasks.ppm");

	// Clear out the buffer
	for (int i = 0; i < width * height; ++i)
		image[i] = 0.;

	//
	// And run the serial implementation 3 times, again reporting the
	// minimum time.
	//
	double minSerial = 1e30;
	for (unsigned int i = 0; i < test_iterations[2]; ++i) {
		reset_and_start_timer();
		volume_serial(density, n, raster2camera, camera2world, width, height, image);
		double dt = get_elapsed_mcycles();
		printf("@time of serial run:\t\t\t[%.3f] million cycles\n", dt);
		minSerial = std::min(minSerial, dt);
	}

	printf("[volume serial]:\t\t[%.3f] million cycles\n", minSerial);
	writePPM(image, width, height, "D:\\Data\\volume-serial.ppm");

	printf("\t\t\t\t(%.2fx speedup from ISPC, %.2fx speedup from ISPC + tasks)\n", minSerial / minISPC,
		minSerial / minISPCtasks);

	return 0;
}
#else

int main(int argc, char** argv)
{
	QString volumeFilePath = "C:\\Test Data\\1.Bitron\\separated_part_0.uint16_scv";

	imt::volume::RawVolumeDataIO io;

	imt::volume::VolumeInfo volInfo;

	io.readUint16SCV(volumeFilePath, volInfo);

	volInfo.mZStep = volInfo.mWidth * volInfo.mHeight;
	volInfo.mYStep = volInfo.mWidth;

	volInfo.mVolumeDataU16 = (unsigned short*)volInfo.mVolumeData;

	imt::volume::HistogramFilter filter(&volInfo);

	std::vector< int64_t > histogram(std::numeric_limits< unsigned short >::max(), 0);

	unsigned short* vData = (unsigned short*)volInfo.mVolumeData;

	for (int64_t zz = 0; zz < volInfo.mDepth; zz++)
		for (int64_t yy = 0; yy < volInfo.mHeight; yy++)
			for (int64_t xx = 0; xx < volInfo.mWidth; xx++)
			{
				histogram[vData[zz * volInfo.mWidth * volInfo.mHeight + yy * volInfo.mWidth + xx]] += 1;
			}


	long iso50Th = filter.ISO50Threshold(histogram); 	//fraunhoufferThreshold(volInfo.mWidth, volInfo.mHeight, volInfo.mDepth,
	
	std::cout << " iso 50 threshold : " << iso50Th << std::endl;

	TrackBallCamera camera;

	camera.setObjectCenter(QVector3D(volInfo.mWidth * 0.5, volInfo.mHeight * 0.5, volInfo.mDepth * 0.5));

	double r = 0.5  * sqrt(volInfo.mWidth * volInfo.mWidth + volInfo.mHeight * volInfo.mHeight + volInfo.mDepth * volInfo.mDepth) / sqrt(3.0);

	camera.setRadius(r);

	int viewPortW = 150, viewPortH = 100;

	camera.setViewPortDimension(viewPortW, viewPortH);

	camera.init();

	std::vector < Eigen::Vector3f > origins(viewPortW * viewPortH), rays(viewPortW * viewPortH);

	for(int yy = 0; yy < viewPortH; yy++)
		for (int xx = 0; xx < viewPortW; xx++)
		{
			QPointF pixel(xx, viewPortH - yy);

			Eigen::Vector3f origin, dir;

			camera.getRay(pixel, origin, dir);

			origins[yy * viewPortW + xx] = origin;
			rays[yy * viewPortW + xx] = dir;
		}

	std::vector<Eigen::Vector3f> displayPoints = origins;

	for (int yy = 0; yy < viewPortH; yy++)
		for (int xx = 0; xx < viewPortW; xx++)
		{

			Eigen::Vector3f otherEnd = origins[yy * viewPortW + xx] + rays[yy * viewPortW + xx] * r * 6;
			
			displayPoints.push_back(otherEnd);
		}


	for(int ii = 0; ii < 2; ii++)
		for(int jj = 0; jj < 2; jj++)
			for (int kk = 0; kk < 2; kk++)
			{
				displayPoints.push_back(Eigen::Vector3f(ii * volInfo.mWidth, jj * volInfo.mHeight, kk * volInfo.mDepth));
			}



	tr::Display3DRoutines::displayPointSet(displayPoints, std::vector<Eigen::Vector3f>(displayPoints.size() , Eigen::Vector3f(1,0,0)));

	imt::volume::VolumeRendererCPU volumeRenderer( (unsigned short*)vData, volInfo.mWidth , volInfo.mHeight , volInfo.mDepth);

	imt::volume::TransferFunction transferFunction(false);

	std::vector < imt::volume::Region* > regions(3);

	//for (int ii = 0; ii < 3; ii++)
	{
		imt::volume::ControlPosition cp1, cp2;

		cp1.first = 0;
		cp1.second = 0;
		
		cp2.first = 38784;
		cp2.second = 0;

		regions[0] = new imt::volume::Region(cp1 , cp2 ,1 , 255, 255, 255, true);
		
		cp1.first = 0;
		cp1.second = 0;

		cp2.first = 38784;
		cp2.second = 0;
		
		regions[1] = new imt::volume::Region(cp1, cp2, 1, 255, 255, 255, true);
		
		cp1.first = 0;
		cp1.second = 0;

		cp2.first = 38784;
		cp2.second = 0;
		
		regions[2] = new imt::volume::Region(cp1, cp2, 1, 255, 255, 255, true);
	}

	transferFunction.UpdateRegions(regions);

	volumeRenderer.setTransferFunction(&transferFunction);

	volumeRenderer.render(&camera);


	//volumeRenderer.s

	return 0;
}

#endif
