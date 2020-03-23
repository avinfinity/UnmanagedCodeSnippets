#ifndef __PLANESLICER_CUH__
#define __PLANESLICER_CUH__

#include "cuda.h"
#include "cuda_runtime_api.h"
#include "cuda_runtime.h"


cudaError_t computeIntersectionWithPlane( int numTriangles, float3 *triangles, float3 planePosition, float3 planeNormal, float3 *edges, int *numEdges );



#endif