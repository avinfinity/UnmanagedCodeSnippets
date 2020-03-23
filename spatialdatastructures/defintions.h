#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define HALF_EDGE_MODULE 1

#ifndef VC_APPLE
//#include "OpenMesh/Core/Mesh/PolyMeshT.hh"
//#include "OpenMesh/Core/Mesh/TriMeshT.hh"
//#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
//#include "OpenMesh/Tools/Subdivider/Adaptive/Composite/CompositeTraits.hh"
#endif

#if HALF_EDGE_MODULE == 0
#include "hbr/mesh.h"
#include "hbr/halfedge.h"
#include "hbr/face.h"
#include "hbr/faceEdit.h"
#include "hbr/catmark.h"
#elif HALF_EDGE_MODULE == 1
#include "halfedge/mesh.h"
#include "halfedge/halfedge.h"
#include "halfedge/face.h"
#endif

#include "spatialdatastructures/editablevertex.h"
#include "Eigen/Dense"


#ifndef VC_APPLE

//typedef OpenMesh::TriMesh_ArrayKernelT< OpenMesh::Subdivider::Adaptive::CompositeTraits >  PolyMesh;
//typedef OpenMesh::DefaultTraits::Point PolyPoint;

#endif

#if HALF_EDGE_MODULE == 0
typedef OpenSubdiv::HbrMesh< EditableVertex >     EditableHbrMesh;
typedef OpenSubdiv::HbrVertex< EditableVertex >   EditableHbrVertex;
typedef OpenSubdiv::HbrFace< EditableVertex >     EditableHbrFace;
typedef OpenSubdiv::HbrHalfedge< EditableVertex > EditableHbrHalfedge;
#else
typedef Mesh< EditableVertex >     EditableHbrMesh;
typedef Vertex< EditableVertex >   EditableHbrVertex;
typedef Face< EditableVertex >     EditableHbrFace;
typedef HalfEdge< EditableVertex > EditableHbrHalfedge;
#endif

struct HbrVertexData
{
  Eigen::Vector3f mCoords;
  Eigen::Vector3f mVertexNormals;
  Eigen::Vector2f mTexCoords;
  
  bool mIsSplittable , mIsUnderRegion;
  
};

#endif
