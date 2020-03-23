
#include "planeslicercuda.h"
#include "vtkIdList.h"
#include "planeslicer.cuh"
#include "cudahelper.h"



void PlaneSlicerCUDA::setMeshData( vtkSmartPointer< vtkPolyData > mesh )
{

	cudaSafeCall( cudaFree(0) );

	int nTris = mesh->GetNumberOfCells();

	mTriangles.resize(9 * nTris);

	vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();

	for ( int tt = 0; tt < nTris; tt++ )
	{
		mesh->GetCellPoints( tt , triangle );

		double p1[3], p2[3], p3[3];

		mesh->GetPoint( triangle->GetId(0) , p1 );
		mesh->GetPoint( triangle->GetId(1) , p2 );
		mesh->GetPoint( triangle->GetId(2) , p3 );


		mTriangles[ 9 * tt] = p1[0];
		mTriangles[ 9 * tt + 1] = p1[1];
		mTriangles[ 9 * tt + 2] = p1[2];

		mTriangles[ 9 * tt + 3] = p2[0];
		mTriangles[ 9 * tt + 4] = p2[1];
		mTriangles[ 9 * tt + 5] = p2[2];

		mTriangles[ 9 * tt + 6] = p3[0];
		mTriangles[ 9 * tt + 7] = p3[1];
		mTriangles[ 9 * tt + 8] = p3[2];
	}


	cudaSafeCall( cudaMalloc( (void**)&mNumEdgesGPU, sizeof(int)) );
	cudaSafeCall( cudaMalloc( (void**)&mTrianglesGPU, mTriangles.size() * sizeof(float)) );
	cudaSafeCall( cudaMalloc( (void**)&mEdgesGPU, mTriangles.size() / 18 * sizeof(float)) );
	cudaSafeCall( cudaHostAlloc( (void**)&mEdgesMappedCPU, mTriangles.size() / 18 * sizeof(float) , cudaHostAllocDefault) );
	//cudaSafeCall( cudaHostGetDevicePointer( (void**)&mEdgesMappedGPU , mEdgesMappedCPU, 0));
	cudaSafeCall( cudaMemcpy( mTrianglesGPU, mTriangles.data(), mTriangles.size() * sizeof(float), cudaMemcpyHostToDevice));


}


void PlaneSlicerCUDA::computeSlice( float *position, float *normal, std::vector< float >& edges )
{

	int numTriangles = mTriangles.size() / 9;

	int numEdges = 0;

	float3 pos, n;

	pos.x = position[0];
	pos.y = position[1];
	pos.z = position[2];

	n.x = normal[0];
	n.y = normal[1];
	n.z = normal[2];

	cudaEvent_t start, stop;

	cudaSafeCall(cudaEventCreate(&start));
	cudaSafeCall(cudaEventCreate(&stop));
	cudaSafeCall(cudaEventRecord(start , 0));

	cudaSafeCall( cudaMemset( mNumEdgesGPU , 0 , sizeof( int ) ) );

	cudaSafeCall( computeIntersectionWithPlane( numTriangles, ( float3* )mTrianglesGPU , pos , n , ( float3* )mEdgesGPU, mNumEdgesGPU ) );

	cudaSafeCall( cudaMemcpy( &mNumEdges, mNumEdgesGPU, sizeof(int), cudaMemcpyDeviceToHost));

	cudaSafeCall( cudaMemcpy( mEdgesMappedCPU, mEdgesGPU, mNumEdges * 6 * sizeof(float), cudaMemcpyDeviceToHost));

	cudaSafeCall(cudaEventRecord(stop, 0));
	cudaSafeCall(cudaEventSynchronize(stop));

	float elapsedTime;

	cudaSafeCall(cudaEventElapsedTime(&elapsedTime, start, stop));

	std::cout << " elapsed time : " << elapsedTime << std::endl;

	

	//cudaSafeCall( cudaDeviceSynchronize() );

	//std::cout << " num intersected edges : " << mNumEdges << std::endl;

	edges.resize(mNumEdges * 6);

	memcpy(edges.data(), mEdgesMappedCPU, mNumEdges * 6 * sizeof(float));

}