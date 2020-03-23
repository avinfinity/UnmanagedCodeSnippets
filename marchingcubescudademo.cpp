



#include "iostream"
#include <cuda_runtime_api.h>
#include <vector_types.h>
#include <vector_functions.h>
#include <helper_cuda.h>    // includes cuda.h and cuda_runtime_api.h
#include <helper_functions.h>
#include "opencvincludes.h"





extern "C" void
launch_classifyVoxel(dim3 grid, dim3 threads, uint* voxelVerts, uint *voxelOccupied, uchar *volume,
uint3 gridSize, uint3 gridSizeShift, uint3 gridSizeMask, uint numVoxels,
float3 voxelSize, float isoValue);

extern "C" void
launch_compactVoxels(dim3 grid, dim3 threads, uint *compactedVoxelArray, uint *voxelOccupied,
uint *voxelOccupiedScan, uint numVoxels);

extern "C" void
launch_generateTriangles(dim3 grid, dim3 threads,
float4 *pos, float4 *norm, uint *compactedVoxelArray, uint *numVertsScanned,
uint3 gridSize, uint3 gridSizeShift, uint3 gridSizeMask,
float3 voxelSize, float isoValue, uint activeVoxels, uint maxVerts);

extern "C" void
launch_generateTriangles2(dim3 grid, dim3 threads,
float4 *pos, float4 *norm, uint *compactedVoxelArray, uint *numVertsScanned, uchar *volume,
uint3 gridSize, uint3 gridSizeShift, uint3 gridSizeMask,
float3 voxelSize, float isoValue, uint activeVoxels, uint maxVerts);

extern "C" void allocateTextures(uint **d_edgeTable, uint **d_triTable, uint **d_numVertsTable);
extern "C" void bindVolumeTexture(uchar *d_volume);


int main( int argc , char **argv )
{
	
	
	
	
	return 0;
}