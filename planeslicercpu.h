#ifndef __PLANESLICERCPU_H__
#define __PLANESLICERCPU_H__

#include "vtkSmartPointer.h"
#include "vtkPolyData.h"

class PlaneSlicerCPU
{

public:

	void setMeshData(vtkSmartPointer< vtkPolyData > mesh);

	void computeSlice(float *position, float *normal, std::vector< float >& edges);

protected:

	std::vector< float > mTriangles;

};




#endif
