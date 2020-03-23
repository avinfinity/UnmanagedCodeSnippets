
#include "planeslicer.cuh"

//#include "stdio.h"

#define flt_eps  1.192092896e-07f


__device__ float dot(float3 a, float3 b)
{
	return (a.x * b.x + a.y * b.y + a.z * b.z);
}

__global__ void computeIntersectionWithPlane_Kernel( int numTriangles , float3 *triangles  , float3 *edges  , int *numEdges , float3 planePosition , float3 planeNormal )
{
   int tid = threadIdx.x + blockIdx.x * blockDim.x;
   
   if( tid > numTriangles - 1 )
      return;

   float3 *t3 = triangles + tid * 3;

   extern __shared__ float sData[];

   int *nIntersections = (int*)sData;
   int *tEid = nIntersections + 1;

   if (threadIdx.x == 0)
   {
	   *nIntersections = 0;

	   *tEid = 0;
   }

   __syncthreads();

   float3 *sIntersections = (float3*)( sData + 2 );
	  
   int p = 0, n = 0;

   float3 d1, d2, d3, l1, l2, l3;

   d1.x = planePosition.x - t3[0].x;
   d1.y = planePosition.y - t3[0].y;
   d1.z = planePosition.z - t3[0].z;

   d2.x = planePosition.x - t3[1].x;
   d2.y = planePosition.y - t3[1].y;
   d2.z = planePosition.z - t3[1].z;

   d3.x = planePosition.x - t3[2].x;
   d3.y = planePosition.y - t3[2].y;
   d3.z = planePosition.z - t3[2].z;

   l1.x = t3[0].x - t3[1].x;
   l1.y = t3[0].y - t3[1].y;
   l1.z = t3[0].z - t3[1].z;

   l2.x = t3[1].x - t3[2].x;
   l2.y = t3[1].y - t3[2].y;
   l2.z = t3[1].z - t3[2].z;

   l3.x = t3[2].x - t3[0].x;
   l3.y = t3[2].y - t3[0].y;
   l3.z = t3[2].z - t3[0].z;

   float f1 =  dot( d1 , planeNormal );
   float f2 =  dot( d2 , planeNormal );
   float f3 =  dot( d3 , planeNormal );

   p += f1 > 0;
   p += f2 > 0;
   p += f3 > 0;

   n += f1 < 0;
   n += f2 < 0;
   n += f3 < 0;

   if (p < 3 && p > 0)
   {

	   int id = atomicAdd(nIntersections, 1);

	   float lambda1 = f2 / dot(l1, planeNormal);
	   float lambda2 = f3 / dot(l2, planeNormal);
	   float lambda3 = f1 / dot(l3, planeNormal);

	   bool p1Found = false, p2Found = false;

	   if (lambda1 >= 0 && lambda1 <= 1)
	   {
		   sIntersections[2 * id].x = lambda1 * t3[0].x + (1 - lambda1) * t3[1].x;
		   sIntersections[2 * id].y = lambda1 * t3[0].y + (1 - lambda1) * t3[1].y;
		   sIntersections[2 * id].z = lambda1 * t3[0].z + (1 - lambda1) * t3[1].z;

		   p1Found = true;
	   }

	   if (lambda2 >= 0 && lambda2 <= 1)
	   {
		   if (!p1Found)
		   {
			   sIntersections[2 * id].x = lambda2 * t3[1].x + (1 - lambda2) * t3[2].x;
			   sIntersections[2 * id].y = lambda2 * t3[1].y + (1 - lambda2) * t3[2].y;
			   sIntersections[2 * id].z = lambda2 * t3[1].z + (1 - lambda2) * t3[2].z;

			   p1Found = true;

		   }
		   else
		   {
			   sIntersections[2 * id + 1].x = lambda2 * t3[1].x + (1 - lambda2) * t3[2].x;
			   sIntersections[2 * id + 1].y = lambda2 * t3[1].y + (1 - lambda2) * t3[2].y;
			   sIntersections[2 * id + 1].z = lambda2 * t3[1].z + (1 - lambda2) * t3[2].z;

			   p2Found = true;
		   }
	   }

	   if (lambda3 >= 0 && lambda3 <= 1)
	   {
		   if (!p2Found)
		   {
			   sIntersections[2 * id + 1].x = lambda3 * t3[2].x + (1 - lambda3) * t3[0].x;
			   sIntersections[2 * id + 1].y = lambda3 * t3[2].y + (1 - lambda3) * t3[0].y;
			   sIntersections[2 * id + 1].z = lambda3 * t3[2].z + (1 - lambda3) * t3[0].z;

			   p2Found = true;
		   }
	   }

	   if (!p1Found )
	   {
		   sIntersections[2 * id].x = sIntersections[2 * id + 1].x;
		   sIntersections[2 * id].y = sIntersections[2 * id + 1].y;
		   sIntersections[2 * id].z = sIntersections[2 * id + 1].z;
	   }

	   if (!p2Found)
	   {
		   sIntersections[2 * id + 1].x = sIntersections[2 * id].x;
		   sIntersections[2 * id + 1].y = sIntersections[2 * id].y;
		   sIntersections[2 * id + 1].z = sIntersections[2 * id].z;
	   }
   }


   __syncthreads();

   if (threadIdx.x == 0)
   {
	  *tEid = atomicAdd( numEdges , *nIntersections ) ;
   }
   
   __syncthreads();

   if ( *nIntersections > 0 && threadIdx.x < *nIntersections )
   {
	   edges[2 * (*tEid + threadIdx.x)]  = sIntersections[2 * threadIdx.x];
	   edges[2 * (*tEid + threadIdx.x) + 1] = sIntersections[2 * threadIdx.x + 1];
   }

}



__global__ void computeIntersectionWithStandardPlane_Kernel( int numTriangles, float3 *triangles , float3* direction , float3 *edges , float3 *edgePointNormals , float3* collectedTris , 
	                                                         int *numEdges, float3 sliceValue , int planeType )
{
	int tid = threadIdx.x + blockIdx.x * blockDim.x;

	if ( tid > numTriangles - 1 )
		return;


	extern __shared__ float sData[];

	int *nIntersections = (int*)sData;
	int *tEid = nIntersections + 1;

	if ( threadIdx.x == 0 )
	{
		*nIntersections = 0;

		*tEid = 0;
	}

	__syncthreads();

	float sv = 0;

	if ( planeType == 0 )
	{
		sv = sliceValue.x;
	}
	else if ( planeType == 1 )
	{
		sv = sliceValue.y;
	}
	else
	{
		sv = sliceValue.z;
	}

	
	float *t = (float*)( triangles + 3 * tid );

	float3 *n3 = direction + 3 * tid;

	float v1 = t[ planeType ] - sv;
	float v2 = t[3 + planeType] - sv;
	float v3 = t[6 + planeType] - sv;

	bool notIntersected = (v1 > 0 && v2 > 0 && v3 > 0) || (v1 < 0 && v2 < 0 && v3 < 0);

	if ( notIntersected )
	{
		return;
	}

	float t1 = v1 / ( t[planeType] - t[3 + planeType] );
	float t2 = v2 / ( t[3 + planeType] - t[6 + planeType]);
	float t3 = v3 / ( t[6 + planeType] - t[planeType] );
	
	float3 end1, end2;
	float interpN1[3], interpN2[3];

	if ( t1 > 0 && t1 < 1 )
	{
		end1.x = t[ 0 ] * t1 + t[ 3 ] * (1 - t1);
		end1.y = t[ 1 ] * t1 + t[ 4 ] * (1 - t1);
		end1.z = t[ 2 ] * t1 + t[ 5 ] * (1 - t1);

		interpN1[0] = n3[0].x * t1 + n3[1].x * (1 - t1);
		interpN1[1] = n3[0].y * t1 + n3[1].y * (1 - t1);
		interpN1[2] = n3[0].z * t1 + n3[1].z * (1 - t1);

		//interpWt1 = wt[0] * t1 + wt[1] * (1 - t1);

		if (t2 > 0 && t2 < 1)
		{
			//end2 = pos2 * t2 + pos3 * (1 - t2);

			end2.x = t[3] * t2 + t[6] * (1 - t2);
			end2.y = t[4] * t2 + t[7] * (1 - t2);
			end2.z = t[5] * t2 + t[8] * (1 - t2);

			interpN2[0] = n3[1].x * t2 + n3[2].x * (1 - t2);
			interpN2[1] = n3[1].y * t2 + n3[2].y * (1 - t2);
			interpN2[2] = n3[1].z * t2 + n3[2].z * (1 - t2);

		}
		else
		{
			//end2 = pos3 * t3 + pos1 * (1 - t3);

			end2.x = t[6] * t3 + t[0] * (1 - t3);
			end2.y = t[7] * t3 + t[1] * (1 - t3);
			end2.z = t[8] * t3 + t[2] * (1 - t3);

			//interpN2 = n3 * t3 + n1 * (1 - t3);

			interpN2[0] = n3[1].x * t3 + n3[2].x * (1 - t3);
			interpN2[1] = n3[1].y * t3 + n3[2].y * (1 - t3);
			interpN2[2] = n3[1].z * t3 + n3[2].z * (1 - t3);

		}

	}
	else if (t2 > 0 && t2 < 1)
	{

		end1.x = t[3] * t2 + t[6] * (1 - t2);
		end1.y = t[4] * t2 + t[7] * (1 - t2);
		end1.z = t[5] * t2 + t[8] * (1 - t2);

		end2.x = t[3] * t2 + t[6] * (1 - t2);
		end2.y = t[4] * t2 + t[7] * (1 - t2);
		end2.z = t[5] * t2 + t[8] * (1 - t2);

		//end1 = pos2 * t2 + pos3 * (1 - t2);
		//end2 = pos3 * t3 + pos1 * (1 - t3);

		//interpN1 = n2 * t2 + n3 * (1 - t2);
		//interpN2 = n3 * t3 + n1 * (1 - t3);

		interpN1[0] = n3[1].x * t2 + n3[2].x * (1 - t2);
		interpN1[1] = n3[1].y * t2 + n3[2].y * (1 - t2);
		interpN1[2] = n3[1].z * t2 + n3[2].z * (1 - t2);

		interpN2[0] = n3[2].x * t3 + n3[0].x * (1 - t3);
		interpN2[1] = n3[2].y * t3 + n3[0].y * (1 - t3);
		interpN2[2] = n3[2].z * t3 + n3[0].z * (1 - t3);


	}
	
	int id = atomicAdd( nIntersections , 2 );

	edges[2 * id] = end1;
	edges[2 * id + 1] = end2;

	edgePointNormals[2 * id].x = interpN1[0];
	edgePointNormals[2 * id].y = interpN1[1];
	edgePointNormals[2 * id].z = interpN1[2];

	edgePointNormals[2 * id + 1].x = interpN2[0];
	edgePointNormals[2 * id + 1].y = interpN2[1];
	edgePointNormals[2 * id + 1].z = interpN2[2];

	__syncthreads();

	atomicAdd( numEdges , *nIntersections );
	

}


cudaError_t computeIntersectionWithStandardPlane(int numTriangles, float3 *triangles, float3 *edges, float3 *oppositeEnds , float3* collectedTris, int *numEdges, float3 sliceValue, int planeType)
{

	dim3 threads(128, 1);

	int wB = (numTriangles + threads.x - 1) / threads.x;
	int hB = 1;

	dim3 blocks(wB, hB);

	int sharedMemorySize = threads.x * 2 * sizeof(float3) + 2 * sizeof(int);

	//computeIntersectionWithPlane_Kernel << < blocks, threads, sharedMemorySize >> >(numTriangles, triangles, edges, numEdges, sliceValue, planeNormal);

	return cudaGetLastError();
}


cudaError_t computeIntersectionWithPlane(int numTriangles, float3 *triangles, float3 planePosition, float3 planeNormal, float3 *edges, int *numEdges)
{
    
	dim3 threads( 128 , 1 );

	int wB = ( numTriangles + threads.x - 1 ) / threads.x;
	int hB = 1;

	dim3 blocks(wB, hB);

	int sharedMemorySize =  threads.x * 2 * sizeof(float3) + 2 * sizeof( int );

	computeIntersectionWithPlane_Kernel << < blocks, threads , sharedMemorySize >> >(numTriangles, triangles, edges, numEdges, planePosition, planeNormal);

	return cudaGetLastError();
}