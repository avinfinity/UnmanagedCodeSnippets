#ifndef __PLANESLICERCUDA_H__
#define __PLANESLICERCUDA_H__

#include "vtkSmartPointer.h"
#include "vtkPolyData.h"

class PlaneSlicerCUDA
{
	
	public:
	
	void setMeshData( vtkSmartPointer< vtkPolyData > mesh );
	void computeSlice( float *position , float *normal , std::vector< float >& edges );


protected:


	std::vector< float > mTriangles;

	float *mTrianglesMappedCPU, *mEdgesMappedCPU;
	float *mTrianglesMappedGPU, *mEdgesMappedGPU , *mTrianglesGPU , *mEdgesGPU;

	int *mNumEdgesGPU;

	int mNumEdges;
	
	
};



#endif