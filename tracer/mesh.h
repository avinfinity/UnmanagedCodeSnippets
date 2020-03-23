//
//  mesh.h
//  tracer
//
//  Created by Rasmus Barringer on 2012-10-06.
//  Copyright (c) 2012 Lund University. All rights reserved.
//

#ifndef tracer_mesh_h
#define tracer_mesh_h

#include "triangle.h"
#include "alignedallocator.h"
#include <vector>
#include "vtkSmartPointer.h"
#include "vtkPolyData.h"

class Mesh {
private:
	std::vector<Triangle, AlignedAllocator<Triangle, 64> > triangles;
	
public:
	Mesh(const char* filename, bool flipFaces);
	Mesh( vtkSmartPointer< vtkPolyData > polydata , bool flipFaces );
	
	std::vector<Triangle, AlignedAllocator<Triangle, 64> >& getTriangles();
};

#endif
