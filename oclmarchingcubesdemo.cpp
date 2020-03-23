

#include "iostream"
#include "iostream"
#include "QString"
#include "volumeinfo.h"
#include "volumeutility.h"
#include "QDebug"
#include "rawvolumedataio.h"
#include "volumeutility.h"
#include "display3droutines.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <memory>
#include <iostream>
#include <cassert>

#include "OpenCL/defines.h"
#include "OpenCL/tables.h"

// standard utility and system includes
#include "OpenCL/common/inc/oclUtils.h"
#include "OpenCL/shared/inc/shrQATest.h"
#include "OpenCL/shared/inc/cmd_arg_reader.h"
#include "OpenCL/shared/inc/shrUtils.h"
#include "CL/cl.hpp"
#include "OpenCL/oclScan_common.h"

// OpenCL vars
cl_platform_id cpPlatform;
cl_uint uiNumDevices;
cl_device_id* cdDevices;
cl_uint uiDeviceUsed;
cl_uint uiDevCount;
cl_context cxGPUContext;
cl_device_id device;
cl_command_queue cqCommandQueue;
cl_program cpProgram;
cl_kernel classifyVoxelKernel;
cl_kernel compactVoxelsKernel;
cl_kernel generateTriangles2Kernel;
cl_int ciErrNum;
char* cPathAndName = NULL;          // var for full paths to data, src, etc.
char* cSourceCL;                    // Buffer to hold source for compilation 
cl_bool g_glInterop = false;

int *pArgc = NULL;
char **pArgv = NULL;

class dim3 {
public:
	size_t x;
	size_t y;
	size_t z;

	dim3(size_t _x = 1, size_t _y = 1, size_t _z = 1) { x = _x; y = _y; z = _z; }
};

const char *volumeFilename = "Bucky.raw";

cl_uint gridSizeLog2[4] = { 5, 5, 5, 0 };
cl_uint gridSizeShift[4];
cl_uint gridSize[4];
cl_uint gridSizeMask[4];

cl_float voxelSize[4];
uint numVoxels = 0;
uint maxVerts = 0;
uint activeVoxels = 0;
uint totalVerts = 0;

float isoValue = 0.2f;
float dIsoValue = 0.005f;

cl_mem d_pos = 0;
cl_mem d_normal = 0;

cl_mem d_volume = 0;
cl_mem d_voxelVerts = 0;
cl_mem d_voxelVertsScan = 0;
cl_mem d_voxelOccupied = 0;
cl_mem d_voxelOccupiedScan = 0;
cl_mem d_compVoxelArray;

// tables
cl_mem d_numVertsTable = 0;
cl_mem d_triTable = 0;

// mouse controls
int mouse_old_x, mouse_old_y;
int mouse_buttons = 0;
cl_float rotate[4] = { 0.0, 0.0, 0.0, 0.0 };
cl_float translate[4] = { 0.0, 0.0, -3.0, 0.0 };

void Cleanup(int iExitCode);
void(*pCleanup)(int) = &Cleanup;


void initMC(int argc, char** argv);
void computeIsosurface();


void openclScan( cl_mem d_voxelOccupiedScan , cl_mem d_voxelOccupied , int numVoxels ) 
{
	scanExclusiveLarge( cqCommandQueue , d_voxelOccupiedScan , d_voxelOccupied , 1 , numVoxels );
}

void
launch_classifyVoxel(dim3 grid, dim3 threads, cl_mem voxelVerts, cl_mem voxelOccupied, cl_mem volume,
cl_uint gridSize[4], cl_uint gridSizeShift[4], cl_uint gridSizeMask[4], uint numVoxels,
cl_float voxelSize[4], float isoValue)
{
	ciErrNum = clSetKernelArg(classifyVoxelKernel, 0, sizeof(cl_mem), &voxelVerts);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	ciErrNum = clSetKernelArg(classifyVoxelKernel, 1, sizeof(cl_mem), &voxelOccupied);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	ciErrNum = clSetKernelArg(classifyVoxelKernel, 2, sizeof(cl_mem), &volume);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	ciErrNum = clSetKernelArg(classifyVoxelKernel, 3, 4 * sizeof(cl_uint), gridSize);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	ciErrNum = clSetKernelArg(classifyVoxelKernel, 4, 4 * sizeof(cl_uint), gridSizeShift);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	ciErrNum = clSetKernelArg(classifyVoxelKernel, 5, 4 * sizeof(cl_uint), gridSizeMask);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	ciErrNum = clSetKernelArg(classifyVoxelKernel, 6, sizeof(uint), &numVoxels);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	ciErrNum = clSetKernelArg(classifyVoxelKernel, 7, 4 * sizeof(cl_float), voxelSize);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	ciErrNum = clSetKernelArg(classifyVoxelKernel, 8, sizeof(float), &isoValue);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	ciErrNum = clSetKernelArg(classifyVoxelKernel, 9, sizeof(cl_mem), &d_numVertsTable);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);

	grid.x *= threads.x;
	ciErrNum = clEnqueueNDRangeKernel(cqCommandQueue, classifyVoxelKernel, 1, NULL, (size_t*)&grid, (size_t*)&threads, 0, 0, 0);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
}


void
launch_compactVoxels(dim3 grid, dim3 threads, cl_mem compVoxelArray, cl_mem voxelOccupied, cl_mem voxelOccupiedScan, uint numVoxels)
{
	ciErrNum = clSetKernelArg(compactVoxelsKernel, 0, sizeof(cl_mem), &compVoxelArray);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	ciErrNum = clSetKernelArg(compactVoxelsKernel, 1, sizeof(cl_mem), &voxelOccupied);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	ciErrNum = clSetKernelArg(compactVoxelsKernel, 2, sizeof(cl_mem), &voxelOccupiedScan);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	ciErrNum = clSetKernelArg(compactVoxelsKernel, 3, sizeof(cl_uint), &numVoxels);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);

	grid.x *= threads.x;
	ciErrNum = clEnqueueNDRangeKernel(cqCommandQueue, compactVoxelsKernel, 1, NULL, (size_t*)&grid, (size_t*)&threads, 0, 0, 0);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
}




void
launch_generateTriangles2( dim3 grid, dim3 threads,
                           cl_mem pos, cl_mem norm, cl_mem compactedVoxelArray, cl_mem numVertsScanned, cl_mem volume,
                           cl_uint gridSize[4], cl_uint gridSizeShift[4], cl_uint gridSizeMask[4],
                           cl_float voxelSize[4], float isoValue, uint activeVoxels, uint maxVerts )
{
	ciErrNum = clSetKernelArg(generateTriangles2Kernel, 0, sizeof(cl_mem), &pos);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	ciErrNum = clSetKernelArg(generateTriangles2Kernel, 1, sizeof(cl_mem), &norm);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	ciErrNum = clSetKernelArg(generateTriangles2Kernel, 2, sizeof(cl_mem), &compactedVoxelArray);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	ciErrNum = clSetKernelArg(generateTriangles2Kernel, 3, sizeof(cl_mem), &numVertsScanned);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	ciErrNum = clSetKernelArg(generateTriangles2Kernel, 4, sizeof(cl_mem), &volume);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	ciErrNum = clSetKernelArg(generateTriangles2Kernel, 5, 4 * sizeof(cl_uint), gridSize);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	ciErrNum = clSetKernelArg(generateTriangles2Kernel, 6, 4 * sizeof(cl_uint), gridSizeShift);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	ciErrNum = clSetKernelArg(generateTriangles2Kernel, 7, 4 * sizeof(cl_uint), gridSizeMask);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	ciErrNum = clSetKernelArg(generateTriangles2Kernel, 8, 4 * sizeof(cl_float), voxelSize);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	ciErrNum = clSetKernelArg(generateTriangles2Kernel, 9, sizeof(float), &isoValue);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	ciErrNum = clSetKernelArg(generateTriangles2Kernel, 10, sizeof(uint), &activeVoxels);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	ciErrNum = clSetKernelArg(generateTriangles2Kernel, 11, sizeof(uint), &maxVerts);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);

	ciErrNum = clSetKernelArg(generateTriangles2Kernel, 12, sizeof(cl_mem), &d_numVertsTable);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	ciErrNum = clSetKernelArg(generateTriangles2Kernel, 13, sizeof(cl_mem), &d_triTable);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);

	grid.x *= threads.x;
	ciErrNum = clEnqueueNDRangeKernel(cqCommandQueue, generateTriangles2Kernel, 1, NULL, (size_t*)&grid, (size_t*)&threads, 0, 0, 0);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);

}


void initCL(int argc, char** argv) {
	//Get the NVIDIA platform
	ciErrNum = oclGetPlatformID(&cpPlatform);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);

	// Get the number of GPU devices available to the platform
	ciErrNum = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 0, NULL, &uiDevCount);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);

	// Create the device list
	cdDevices = new cl_device_id[uiDevCount];
	ciErrNum = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, uiDevCount, cdDevices, NULL);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);

	// Get device requested on command line, if any
	uiDeviceUsed = 0;
	unsigned int uiEndDev = uiDevCount - 1;
	if (shrGetCmdLineArgumentu(argc, (const char**)argv, "device", &uiDeviceUsed))
	{
		uiDeviceUsed = CLAMP(uiDeviceUsed, 0, uiEndDev);
		uiEndDev = uiDeviceUsed;
	}

	// Check if the requested device (or any of the devices if none requested) supports context sharing with OpenGL
	
	// No GL interop
	
	cl_context_properties props[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)cpPlatform, 0 };
	cxGPUContext = clCreateContext(props, 1, &cdDevices[uiDeviceUsed], NULL, NULL, &ciErrNum);

	oclPrintDevInfo(LOGBOTH, cdDevices[uiDeviceUsed]);

	// create a command-queue
	cqCommandQueue = clCreateCommandQueue(cxGPUContext, cdDevices[uiDeviceUsed], 0, &ciErrNum);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);

	// Program Setup
	size_t program_length;
	cPathAndName = shrFindFilePath("marchingCubes_kernel.cl", argv[0]);
	oclCheckErrorEX(cPathAndName != NULL, shrTRUE, pCleanup);
	cSourceCL = oclLoadProgSource(cPathAndName, "", &program_length);
	oclCheckErrorEX(cSourceCL != NULL, shrTRUE, pCleanup);

	// create the program
	cpProgram = clCreateProgramWithSource(cxGPUContext, 1,
		(const char **)&cSourceCL, &program_length, &ciErrNum);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);

	// build the program
	std::string buildOpts = "-cl-mad-enable";
	ciErrNum = clBuildProgram(cpProgram, 0, NULL, buildOpts.c_str(), NULL, NULL);
	if (ciErrNum != CL_SUCCESS)
	{
		// write out standard error, Build Log and PTX, then cleanup and return error
		shrLogEx(LOGBOTH | ERRORMSG, ciErrNum, STDERROR);
		oclLogBuildInfo(cpProgram, oclGetFirstDev(cxGPUContext));
		oclLogPtx(cpProgram, oclGetFirstDev(cxGPUContext), "oclMarchinCubes.ptx");
		Cleanup(EXIT_FAILURE);
	}

	// create the kernel
	classifyVoxelKernel = clCreateKernel(cpProgram, "classifyVoxel", &ciErrNum);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);

	compactVoxelsKernel = clCreateKernel(cpProgram, "compactVoxels", &ciErrNum);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);

	generateTriangles2Kernel = clCreateKernel(cpProgram, "generateTriangles2", &ciErrNum);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);

	// Setup Scan
	initScan(cxGPUContext, cqCommandQueue, (const char**)argv);
}





int main( int argc , char **argv )
{
   QString rawFilePath = "C:/projects/datasets/Bitron/separated_part_7.uint16_scv";
	
   imt::volume::VolumeInfo volume;

   imt::volume::RawVolumeDataIO::readUint16SCV(rawFilePath, volume);

  // std::cout << " volume dimensions : " << volume.mWidth << " " << volume.mHeight << " " << volume.mDepth << std::endl;
	
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// initialize marching cubes
////////////////////////////////////////////////////////////////////////////////
void
initMC(int argc, char** argv)
{
	// parse command line arguments
	int n;

	if ( shrGetCmdLineArgumenti(argc, (const char**)argv, "grid", &n) ) 
	{
		gridSizeLog2[0] = gridSizeLog2[1] = gridSizeLog2[2] = n;
	}
	
	if (shrGetCmdLineArgumenti(argc, (const char**)argv, "gridx", &n)) 
	{
		gridSizeLog2[0] = n;
	}
	
	if (shrGetCmdLineArgumenti(argc, (const char**)argv, "gridy", &n)) 
	{
		gridSizeLog2[1] = n;
	}
	
	if (shrGetCmdLineArgumenti(argc, (const char**)argv, "gridz", &n)) 
	{
		gridSizeLog2[2] = n;
	}

	char *filename;
	
	if (shrGetCmdLineArgumentstr(argc, (const char**)argv, "file", &filename)) 
	{
		volumeFilename = filename;
	}

	gridSize[0] = 1 << gridSizeLog2[0];
	gridSize[1] = 1 << gridSizeLog2[1];
	gridSize[2] = 1 << gridSizeLog2[2];


	gridSizeMask[0] = gridSize[0] - 1;
	gridSizeMask[1] = gridSize[1] - 1;
	gridSizeMask[2] = gridSize[2] - 1;

	gridSizeShift[0] = 0;
	gridSizeShift[1] = gridSizeLog2[0];
	gridSizeShift[2] = gridSizeLog2[0] + gridSizeLog2[1];

	numVoxels = gridSize[0] * gridSize[1] * gridSize[2];


	voxelSize[0] = 2.0f / gridSize[0];
	voxelSize[1] = 2.0f / gridSize[1];
	voxelSize[2] = 2.0f / gridSize[2];

	maxVerts = gridSize[0] * gridSize[1] * 100;

	shrLog("grid: %d x %d x %d = %d voxels\n", gridSize[0], gridSize[1], gridSize[2], numVoxels);
	shrLog("max verts = %d\n", maxVerts);

	// load volume data
	char* path = shrFindFilePath(volumeFilename, argv[0]);
	
	if (path == 0) 
	{
		shrLog("Error finding file '%s'\n", volumeFilename);
		exit(EXIT_FAILURE);
	}

	int size = gridSize[0] * gridSize[1] * gridSize[2] * sizeof(uchar);
	
	uchar *volume = 0;// loadRawFile(path, size);
	
	cl_image_format volumeFormat;
	
	volumeFormat.image_channel_order = CL_R;
	volumeFormat.image_channel_data_type = CL_UNORM_INT8;

	d_volume = clCreateImage3D( cxGPUContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &volumeFormat,
		                        gridSize[0] , gridSize[1] , gridSize[2] ,
		                        gridSize[0] , gridSize[0] * gridSize[1] , volume , &ciErrNum );

	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	
	free(volume);

	// allocate textures
	//allocateTextures(&d_triTable, &d_numVertsTable);

	// allocate device memory
	unsigned int memSize = sizeof(uint) * numVoxels;
	d_voxelVerts = clCreateBuffer(cxGPUContext, CL_MEM_READ_WRITE, memSize, 0, &ciErrNum);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	d_voxelVertsScan = clCreateBuffer(cxGPUContext, CL_MEM_READ_WRITE, memSize, 0, &ciErrNum);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	d_voxelOccupied = clCreateBuffer(cxGPUContext, CL_MEM_READ_WRITE, memSize, 0, &ciErrNum);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	d_voxelOccupiedScan = clCreateBuffer(cxGPUContext, CL_MEM_READ_WRITE, memSize, 0, &ciErrNum);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);
	d_compVoxelArray = clCreateBuffer(cxGPUContext, CL_MEM_READ_WRITE, memSize, 0, &ciErrNum);
	oclCheckErrorEX(ciErrNum, CL_SUCCESS, pCleanup);


}

void Cleanup(int iExitCode)
{
	if (d_triTable) clReleaseMemObject(d_triTable);
	if (d_numVertsTable) clReleaseMemObject(d_numVertsTable);

	if (d_voxelVerts) clReleaseMemObject(d_voxelVerts);
	if (d_voxelVertsScan) clReleaseMemObject(d_voxelVertsScan);
	if (d_voxelOccupied) clReleaseMemObject(d_voxelOccupied);
	if (d_voxelOccupiedScan) clReleaseMemObject(d_voxelOccupiedScan);
	if (d_compVoxelArray) clReleaseMemObject(d_compVoxelArray);

	if (d_volume) clReleaseMemObject(d_volume);

	closeScan();

	if (compactVoxelsKernel)clReleaseKernel(compactVoxelsKernel);
	if (compactVoxelsKernel)clReleaseKernel(generateTriangles2Kernel);
	if (compactVoxelsKernel)clReleaseKernel(classifyVoxelKernel);
	if (cpProgram)clReleaseProgram(cpProgram);

	if (cqCommandQueue)clReleaseCommandQueue(cqCommandQueue);
	if (cxGPUContext)clReleaseContext(cxGPUContext);

	// finalize logs and leave
//	shrQAFinish2(bQATest, *pArgc, (const char **)pArgv, (iExitCode == 0) ? QA_PASSED : QA_FAILED);
//
//	if ((g_bNoprompt) || (bQATest))
//	{
//		shrLogEx(LOGBOTH | CLOSELOG, 0, "%s Exiting...\n", cpExecutableName);
//	}
//	else
//	{
//		shrLogEx(LOGBOTH | CLOSELOG, 0, "%s Exiting...\nPress <Enter> to Quit\n", cpExecutableName);
//#ifdef WIN32
//		getchar();
//#endif
//	}
	exit(iExitCode);
}
