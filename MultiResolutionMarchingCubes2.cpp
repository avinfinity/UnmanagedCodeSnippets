#include "stdafx.h"
#include "MultiResolutionMarchingCubes2.h"
#include "MarchingCubesLookupTable.h"
#include "SobelGradientOperator.h"
#include <algorithm>
#include <iostream>
#include "eigenincludes.h"
#include "display3droutines.h"

//#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
//
//#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
//#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
//#include <OpenMesh/Tools/Utils/Timer.hh>
//#include <OpenMesh/Tools/Decimater/DecimaterT.hh>
//#include <OpenMesh/Tools/Decimater/ModAspectRatioT.hh>
//#include <OpenMesh/Tools/Decimater/ModEdgeLengthT.hh>
//#include <OpenMesh/Tools/Decimater/ModHausdorffT.hh>
//#include <OpenMesh/Tools/Decimater/ModNormalDeviationT.hh>
//#include <OpenMesh/Tools/Decimater/ModNormalFlippingT.hh>
//#include <OpenMesh/Tools/Decimater/ModQuadricT.hh>
//#include <OpenMesh/Tools/Decimater/ModProgMeshT.hh>
//#include <OpenMesh/Tools/Decimater/ModIndependentSetsT.hh>
//#include <OpenMesh/Tools/Decimater/ModRoundnessT.hh>
#include "opencvincludes.h"
#include "halfedgemesh.h"
#include "display3droutines.h"
#include "gridhierarchytree.h"



//struct MeshTraits : public OpenMesh::DefaultTraits
//{
//	VertexAttributes(OpenMesh::Attributes::Status);
//	FaceAttributes(OpenMesh::Attributes::Status);
//	EdgeAttributes(OpenMesh::Attributes::Status);
//};
//
//typedef OpenMesh::TriMesh_ArrayKernelT<MeshTraits> SurfaceMesh;

//#define IMT_DISABLE_OMP


//void indicesToOpenMesh(std::vector< double >& vertices, std::vector< unsigned int >& indices, SurfaceMesh* surfaceMesh)
//{
//	int numVertices = vertices.size() / 3;
//	int numCells = indices.size() / 3;
//
//	int numEdges = 0;
//
//	int fid = 0;
//
//
//
//	surfaceMesh->clear();
//
//
//
//	int numFaces = indices.size() / 3;
//
//	std::cout << " indices to openMesh : " << vertices.size()
//		<< " " << indices.size() << std::endl;
//
//	std::vector< SurfaceMesh::VertexHandle > vhandle(numVertices);
//
//
//	for (int vv = 0; vv < numVertices; vv++)
//	{
//		vhandle[vv] = surfaceMesh->add_vertex(SurfaceMesh::Point(vertices[3 * vv], vertices[3 * vv + 1], vertices[3 * vv + 2]));
//
//	}
//
//	for (int ff = 0; ff < numFaces; ff++)
//	{
//		std::vector< SurfaceMesh::VertexHandle >  face_vhandles;// , face_vhandles2;
//
//		face_vhandles.push_back(vhandle[indices[3 * ff]]);
//		face_vhandles.push_back(vhandle[indices[3 * ff + 1]]);
//		face_vhandles.push_back(vhandle[indices[3 * ff + 2]]);
//
//		//face_vhandles2.push_back( vhandle[ indices[ 3 * ff ] ] );
//		//       face_vhandles2.push_back( vhandle[ indices[ 3 * ff + 2 ] ] );
//		//       face_vhandles2.push_back( vhandle[ indices[ 3 * ff + 1 ] ] );
//
//		std::vector< SurfaceMesh::VertexHandle >::const_iterator it, it2, end(face_vhandles.end());
//
//		bool skip = false;
//
//		// test for valid vertex indices
//		for (it = face_vhandles.begin(); it != end; ++it)
//			if (!surfaceMesh->is_valid_handle(*it))
//			{
//				skip = true;
//			}
//
//		if (skip)
//			continue;
//
//
//		// don't allow double vertices
//		for (it = face_vhandles.begin(); it != end; ++it)
//			for (it2 = it + 1; it2 != end; ++it2)
//				if (*it == *it2)
//				{
//					skip = true;
//				}
//
//
//		if (skip)
//			continue;
//
//
//		// try to add face
//		SurfaceMesh::FaceHandle fh = surfaceMesh->add_face(face_vhandles);
//
//		if (!fh.is_valid())
//		{
//			//fh = surfaceMesh->add_face(face_vhandles2);
//
//			/*if (fh.is_valid())
//			{
//				std::cout << " reverse face is valid " << std::endl;
//			}*/
//			//continue;
//		}
//
//	}
//
//	int nHalfEdges =  surfaceMesh->n_halfedges();
//
//	std::vector<unsigned char> halfedgeMasks(nHalfEdges, 0);
//
//	SurfaceMesh::HalfedgeIter  h_it, h_end(surfaceMesh->halfedges_end());
//
//
//	int nHoles = 0;
//
//
//	std::vector<unsigned int> addedTriangles;
//
//	for (h_it = surfaceMesh->halfedges_begin(); h_it != h_end; ++h_it)
//	{
//		bool is_boundary = surfaceMesh->is_boundary(h_it);
//
//		auto current_h = *h_it;
//
//		if (!is_boundary || halfedgeMasks[current_h.idx()])
//			continue;
//
//		halfedgeMasks[current_h.idx()] = 1;
//
//		std::vector< int > holeVertexIds;
//
//		//auto e = surfaceMesh->vertex()
//
//		auto vh = surfaceMesh->to_vertex_handle(current_h);
//		
//		holeVertexIds.push_back(vh.idx());
//
//
//		nHoles++;
//
//		while(1)
//		{
//			current_h = surfaceMesh->next_halfedge_handle(current_h);
//
//			if ( current_h == *h_it)
//				      break;
//
//			vh = surfaceMesh->to_vertex_handle(current_h);
//
//			holeVertexIds.push_back(vh.idx());
//
//		}
//
//		//now generate the triangles
//		int nHoleVertices = holeVertexIds.size();
//
//		//std::cout << "hole size : " << nHoleVertices << std::endl;
//
//		if (nHoleVertices > 7)
//			continue;
//
//		for ( int hh = 1; hh < nHoleVertices - 1; hh++ )
//		{
//			addedTriangles.push_back(holeVertexIds[0]);
//			
//			addedTriangles.push_back(holeVertexIds[hh + 1]);
//			addedTriangles.push_back(holeVertexIds[hh]);
//		}
//
//	}
//
//
//	std::cout << "number of found holes : " << nHoles << std::endl;
//
//
//	indices.insert(indices.end(), addedTriangles.begin(), addedTriangles.end());
//
//
//	//surfaceMesh->request_vertex_normals();
//	//surfaceMesh->request_face_normals();
//	//surfaceMesh->update_normals();
//	//surfaceMesh->release_face_normals();
//}



namespace Zeiss {
	namespace IMT {

		namespace NG {

			namespace NeoInsights {

				namespace Volume {

					namespace MeshGenerator {



#define IS_NAN(vec) (vec.x != vec.x || vec.y != vec.y || vec.z != vec.z)

						MultiResolutionMarchingCubes2::MultiResolutionMarchingCubes2(unsigned short* volumeData, int32_t volumeWidth, int32_t volumeHeight, int32_t volumeDepth,
							double voxelStepX, double voxelStepY, double voxelStepZ) :
							_VolumeData(volumeData), _VolumeWidth(volumeWidth), _VolumeHeight(volumeHeight), _VolumeDepth(volumeDepth),
							_VoxelStepX(voxelStepX), _VoxelStepY(voxelStepY), _VoxelStepZ(voxelStepZ)
						{
							_ZStep = (int64_t)_VolumeWidth * (int64_t)_VolumeHeight;
							_YStep = (int64_t)_VolumeWidth;

							_DeviationThreshold = cos(10.0 / 180 * M_PI);

							_GridHierarchyTree = new imt::volume::GridHierarchyTree(3, 0, _VolumeWidth, _VolumeHeight, _VolumeDepth);


						}


						void MultiResolutionMarchingCubes2::reinterpolateSlices(int z, int strip, float *reinterpolatedSlices)
						{
							for (int zz = 0; zz < strip; zz++)
								for (int yy = 0; yy < _VolumeHeight; yy++)
									for (int xx = 0; xx < _VolumeWidth; xx++)
									{
										reinterpolatedSlices[zz * _ZStep + yy * _YStep + xx] = (float)grayValueAtCorner(xx, yy, z + zz);
									}
						}


#define grayValueAt( x  , y , z ) _VolumeData[ _ZStep * (z) + _YStep * (y) + (x)]


						float MultiResolutionMarchingCubes2::grayValueAtCorner(int x, int y, int z)
						{

							int64_t id = _ZStep * z + _YStep * y + x;

							float grayValue = _VolumeData[id];

							//int64_t maxId = _ZStep * _VolumeDepth;

							//for (int64_t zz = z; zz <= z + 1; zz++)
							//	for (int64_t yy = y; yy <= y + 1; yy++)
							//		for (int64_t xx = x; xx <= x + 1; xx++)
							//		{
							//			int64_t id = _ZStep * zz + _YStep * yy + xx;

							//			grayValue += _VolumeData[id];
							//		}

							//grayValue /= 8.0;

							return grayValue;

						}


						//sample corners at resolution level 2
						void MultiResolutionMarchingCubes2::sampleCorners2(int x, int y, int z, std::vector<float>& grayValues)
						{
							int xx = 4 * x;
							int yy = 4 * y;
							int zz = 4 * z;

							//grayValues.resize(8);

							sampleCorners(xx, yy, zz, grayValues, 4);
						}

						//sample corners at resolution level 1
						void MultiResolutionMarchingCubes2::sampleCorners1(int x, int y, int z, std::vector<float>& grayValues)
						{
							int xx = 2 * x;
							int yy = 2 * y;
							int zz = 2 * z;

							sampleCorners(x, y, z, grayValues, 2);
						}


						//sample corners at resolution level 0
						void MultiResolutionMarchingCubes2::sampleCorners(int x, int y, int z, std::vector<float>& grayValues, int step)
						{
							grayValues.resize(8);

							grayValues[0] = grayValueAt(x, y, z);
							grayValues[1] = grayValueAt(x + step, y, z);
							grayValues[2] = grayValueAt(x + step, y, z + step);
							grayValues[3] = grayValueAt(x, y, z + step);
							grayValues[4] = grayValueAt(x, y + step, z);
							grayValues[5] = grayValueAt(x + step, y + step, z);
							grayValues[6] = grayValueAt(x + step, y + step, z + step);
							grayValues[7] = grayValueAt(x, y + step, z + step);
						}

						bool MultiResolutionMarchingCubes2::doesGridIntersect(int x, int y, int z, int step)
						{
							int minValue = 65536;
							int maxValue = 0;

							for(int zz = z ; zz <= z + step; zz++)
								for(int yy = y; yy <= y + step ; yy++)
									for (int xx = x; xx <= x + step; xx++)
									{
										int64_t id = zz * _ZStep + yy * _YStep + xx;

										minValue = std::min(minValue, (int)_VolumeData[id]);
										maxValue = std::max(maxValue, (int)_VolumeData[id]);

									}

							if (minValue < _IsoThreshold && maxValue > _IsoThreshold)
							{
								return true;
							}								
							else
							{
								return false;
							}

						}



						MultiResolutionMarchingCubes2::Vector3d::Vector3d()
						{
							x = 0;
							y = 0;
							z = 0;
						}

						MultiResolutionMarchingCubes2::Vector3d::Vector3d(double xVal, double yVal, double zVal)
						{
							x = xVal;
							y = yVal;
							z = zVal;
						}


						int64_t planeIdInCube( int id1, int id2 , MultiResolutionMarchingCubes2::Index3d &index_3d , int w , int h , std::vector<unsigned char>& planeMasks )
						{
							
							int planeId = -1;

							unsigned char* mask1 = planeMasks.data() + 3 * id1;
							unsigned char* mask2 = planeMasks.data() + 3 * id2;

							bool m1, m2, m3;

							m1 = mask1[0] != 3 && mask2[0] != 3 && mask1[0] == mask2[0];
							m2 = mask1[1] != 3 && mask2[1] != 3 && mask1[1] == mask2[1];
							m3 = mask1[2] != 3 && mask2[2] != 3 && mask1[2] == mask2[2];

							bool isPlanerIntersection = m1 || m2 || m3;

							int64_t zStep = w * h;

							int64_t offset = zStep * index_3d.z + index_3d.y * w + index_3d.x;

							//if (!isPlanerIntersection)
								//return -1;

							if (m1)
							{
								planeId = offset + mask1[0] * 3;
							}

							if (m2)
							{
								planeId = offset + 1 + mask1[1] * 3 * w;
							}

							if (m3)
							{
								planeId = offset + 2 + mask1[2] * 3 * zStep;
							}

							return planeId;
						}


						bool planeVoxelAndType(int id1, int id2, MultiResolutionMarchingCubes2::Index3d &index_3d, 
							                   int w, int h, std::vector<unsigned char>& planeMasks , 
							                   MultiResolutionMarchingCubes2::Index3d& planeVoxel , int& planeType , int step)
						{

							int planeId = -1;

							unsigned char* mask1 = planeMasks.data() + 3 * id1;
							unsigned char* mask2 = planeMasks.data() + 3 * id2;

							bool m1, m2, m3;

							m1 = mask1[0] != 3 && mask2[0] != 3 && mask1[0] == mask2[0];//yz plane
							m2 = mask1[1] != 3 && mask2[1] != 3 && mask1[1] == mask2[1];//zx plane	
							m3 = mask1[2] != 3 && mask2[2] != 3 && mask1[2] == mask2[2];//xy plane

							planeVoxel = index_3d;

							bool edgeOnPlane = false;

							int count = 0;

							if (m1)
							{
								if (mask1[0] == 2)
									planeVoxel.x += step;

								edgeOnPlane = true;

								planeType = 0;

								count++;
							}

							if (m2)
							{
								if(mask1[1] == 2)
								planeVoxel.y += step;

								edgeOnPlane = true;

								planeType = 1;

								count++;
							}

							if (m3)
							{
								if (mask1[2] == 2)
									planeVoxel.z += step;

								edgeOnPlane = true;

								planeType = 2;

								count++;
							}


							return edgeOnPlane;
						}



#define DOT(v1 , v2) v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]


						bool MultiResolutionMarchingCubes2::createSurface(std::vector< float > &leaf_node,
							MultiResolutionMarchingCubes2::Index3d &index_3d, std::vector<MultiResolutionMarchingCubes2::EdgeInfo>& edgeInfos, 
							std::vector<VertexProperty<4>>& planeInfo , SobelGradientOperator* gradientOperator, int step)
						{
							int cubeindex = 0;
							Vector3d vertex_list[12];

							double checkThreshold = _IsoThreshold;
							bool needFurtherSubdivision = false;

							if (leaf_node[0] < checkThreshold) cubeindex |= 1;
							if (leaf_node[1] < checkThreshold) cubeindex |= 2;
							if (leaf_node[2] < checkThreshold) cubeindex |= 4;
							if (leaf_node[3] < checkThreshold) cubeindex |= 8;
							if (leaf_node[4] < checkThreshold) cubeindex |= 16;
							if (leaf_node[5] < checkThreshold) cubeindex |= 32;
							if (leaf_node[6] < checkThreshold) cubeindex |= 64;
							if (leaf_node[7] < checkThreshold) cubeindex |= 128;

							// Cube is entirely in/out of the surface
							if (MarchingCubesLookupTable::edgeTable[cubeindex] == 0)
							{

								if ( step < 2 )
									return false;

								int sum = 0;

								for (int zScale = 0; zScale < step; zScale++)
									for (int yScale = 0; yScale < step; yScale++)
										for (int xScale = 0; xScale < step; xScale++)
										{
											Index3d newIndex;

											newIndex.x = index_3d.x + xScale ;
											newIndex.y = index_3d.y + yScale ;
											newIndex.z = index_3d.z + zScale ;

											std::vector<float> newLeafValues;

											//sample the eight corners of the cube and check their gray values
											sampleCorners(newIndex.x, newIndex.y, newIndex.z, newLeafValues, 1);

											sum += createSurface(newLeafValues, newIndex, edgeInfos, planeInfo, gradientOperator, 1);
										}


								return true;							
							}
								

							int32_t x = index_3d.x;
							int32_t y = index_3d.y;
							int32_t z = index_3d.z;

							EdgeInfo eif[12] = {
								{ 0 , _ZStep * z + _YStep * y + x },//x , y , z  ---  x + 1 , y , z
							{ 2 , _ZStep * z + _YStep * y + (x + step) },//x + 1 , y , z ---  x + 1 , y , z + 1
							{ 0 , _ZStep * (z + step) + _YStep * y + x },//x + 1 , y , z + 1 --- x , y , z + 1
							{ 2 , _ZStep *  z + _YStep * y + x },// x,y, z + 1  --- x , y , z
							{ 0 , _ZStep * z + _YStep * (y + step) + x },//  x, y + 1 , z -- x + 1 , y + 1 , z
							{ 2 , _ZStep * z + _YStep * (y + step) + x + step },//  x + 1 , y + 1 , z --- x + 1 , y + 1 , z + 1
							{ 0 , _ZStep * (z + step) + _YStep * (y + step) + x },//  x + 1 , y + 1 , z + 1 --- x , y + 1 , z + 1
							{ 2 , _ZStep *  z + _YStep * (y + step) + x },// x , y + 1, z + 1  --- x , y + 1 , z
							{ 1 , _ZStep * z + _YStep * y + x },// x , y , z --- x , y + 1 , z
							{ 1 , _ZStep * z + _YStep * y + x + step },// x + 1 , y , z --- x + 1 , y + 1 , z
							{ 1 , _ZStep * (z + step) + _YStep * y + x + step },// x + 1 , y , z + 1 --- x + 1 , y + 1 , z + 1
							{ 1 , _ZStep * (z + step) + _YStep * y + x },// x , y , z + 1 --- x , y + 1 , z + 1	
							};



							Vector3d center;
							center.x = index_3d.x * _VoxelStepX;
							center.y = index_3d.y * _VoxelStepY;
							center.z = index_3d.z * _VoxelStepZ;

							std::vector<unsigned char> changeMask(12 * 3 , 0);


							//// Find the vertices where the surface intersects the cube
							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 1) //x , y , z  ---  x + 1 , y , z
							{
								Index3d voxelPosition(x, y, z);

								int xEdgeShift = interpolateEdgeX(voxelPosition, leaf_node[0], leaf_node[1], vertex_list[0], step);

								eif[0]._EdgeId += xEdgeShift;

								changeMask[0] = 3;
								changeMask[1] = 1;
								changeMask[2] = 1;
							}


							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 2) //x + 1 , y , z ---  x + 1 , y , z + 1
							{
								Index3d voxelPosition(x + step, y, z);

								int zEdgeShift = interpolateEdgeZ(voxelPosition, leaf_node[1], leaf_node[2], vertex_list[1], step);
							
								eif[1]._EdgeId += zEdgeShift * _ZStep;

								changeMask[3 + 0] = 2;
								changeMask[3 + 1] = 1;
								changeMask[3 + 2] = 3;
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 4)//x + 1 , y , z + 1 --- x , y , z + 1
							{

#if 0
								Index3d voxelPosition(x + step, y, z + step);

								int xEdgeShift = interpolateEdgeX(voxelPosition, leaf_node[2], leaf_node[3], vertex_list[2], -step);
#else
								Index3d voxelPosition(x , y, z + step);

								int xEdgeShift = interpolateEdgeX(voxelPosition, leaf_node[3], leaf_node[2],  vertex_list[2], step);

#endif

								eif[2]._EdgeId += xEdgeShift;
							
								changeMask[3 * 2 + 0] = 3; // x + 1 -> x
								changeMask[3 * 2 + 1] = 1; // y -> y
								changeMask[3 * 2 + 2] = 2; //z + 1 -> z + 1
							
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 8)// x,y, z + 1  --- x , y , z
							{
#if 0
								Index3d voxelPosition(x, y, z + step);

								int zEdgeShift = interpolateEdgeZ(voxelPosition, leaf_node[3], leaf_node[0], vertex_list[3], -step);

#else
								Index3d voxelPosition(x, y, z);

								int zEdgeShift = interpolateEdgeZ(voxelPosition, leaf_node[0],  leaf_node[3],  vertex_list[3], step);
#endif
								eif[3]._EdgeId += zEdgeShift * _ZStep;

								changeMask[3 * 3 + 0] = 1;
								changeMask[3 * 3 + 1] = 1;
								changeMask[3 * 3 + 2] = 3;
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 16)//  x, y + 1 , z -- x + 1 , y + 1 , z
							{
								Index3d voxelPosition(x, y + step, z);

								int xEdgeShift = interpolateEdgeX(voxelPosition, leaf_node[4], leaf_node[5], vertex_list[4], step);

								eif[4]._EdgeId += xEdgeShift;

								changeMask[3 * 4 + 0] = 3;
								changeMask[3 * 4 + 1] = 2;
								changeMask[3 * 4 + 2] = 1;
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 32)//  x + 1 , y + 1 , z --- x + 1 , y + 1 , z + 1
							{
								Index3d voxelPosition(x + step, y + step, z);

								int zEdgeShift = interpolateEdgeZ(voxelPosition, leaf_node[5], leaf_node[6], vertex_list[5], step);

								eif[5]._EdgeId += zEdgeShift * _ZStep;

								changeMask[3 * 5 + 0] = 2;
								changeMask[3 * 5 + 1] = 2;
								changeMask[3 * 5 + 2] = 3;
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 64)//  x + 1 , y + 1 , z + 1 --- x , y + 1 , z + 1
							{

#if 0
								Index3d voxelPosition(x + step, y + step, z + step);

								int xEdgeShift = interpolateEdgeX(voxelPosition, leaf_node[6], leaf_node[7], vertex_list[6], -step);

#else
								Index3d voxelPosition(x , y + step, z + step);

								int xEdgeShift = interpolateEdgeX(voxelPosition, leaf_node[7], leaf_node[6],  vertex_list[6], step);
#endif
								eif[6]._EdgeId += xEdgeShift;

								changeMask[3 * 6 + 0] = 3;
								changeMask[3 * 6 + 1] = 2;
								changeMask[3 * 6 + 2] = 2;

							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 128)// x , y + 1, z + 1  --- x , y + 1 , z
							{
#if 0
								Index3d voxelPosition(x, y + step, z + step);

								int zEdgeShift = interpolateEdgeZ(voxelPosition, leaf_node[7], leaf_node[4], vertex_list[7], -step);

#else
								Index3d voxelPosition(x, y + step, z );

								int zEdgeShift = interpolateEdgeZ(voxelPosition, leaf_node[4],  leaf_node[7],  vertex_list[7], step);
#endif
								eif[7]._EdgeId += zEdgeShift * _ZStep;


								changeMask[3 * 7 + 0] = 1;
								changeMask[3 * 7 + 1] = 2;
								changeMask[3 * 7 + 2] = 3;

							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 256)// x , y , z --- x , y + 1 , z
							{
								Index3d voxelPosition(x, y, z);

								int yEdgeShift = interpolateEdgeY(voxelPosition, leaf_node[0], leaf_node[4], vertex_list[8], step);

								eif[8]._EdgeId += yEdgeShift * _YStep;

								changeMask[3 * 8 + 0] = 1;
								changeMask[3 * 8 + 1] = 3;
								changeMask[3 * 8 + 2] = 1;
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 512)// x + 1 , y , z --- x + 1 , y + 1 , z
							{
								Index3d voxelPosition(x + step, y, z);

								int yEdgeShift = interpolateEdgeY(voxelPosition, leaf_node[1], leaf_node[5], vertex_list[9], step);

								eif[9]._EdgeId += yEdgeShift * _YStep;

								changeMask[3 * 9 + 0] = 2;
								changeMask[3 * 9 + 1] = 3;
								changeMask[3 * 9 + 2] = 1;
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 1024)// x + 1 , y , z + 1 --- x + 1 , y + 1 , z + 1
							{

								Index3d voxelPosition(x + step, y, z + step);

								int yEdgeShift = interpolateEdgeY(voxelPosition, leaf_node[2], leaf_node[6], vertex_list[10], step);

								eif[10]._EdgeId += yEdgeShift * _YStep;

								changeMask[3 * 10 + 0] = 2;
								changeMask[3 * 10 + 1] = 3;
								changeMask[3 * 10 + 2] = 2;
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 2048)// x , y , z + 1 --- x , y + 1 , z + 1
							{
								Index3d voxelPosition(x, y, z + step);

								int yEdgeShift = interpolateEdgeY(voxelPosition, leaf_node[3], leaf_node[7], vertex_list[11], step);

								eif[11]._EdgeId += yEdgeShift * _YStep;

								changeMask[3 * 11 + 0] = 1;
								changeMask[3 * 11 + 1] = 3;
								changeMask[3 * 11 + 2] = 2;
							}

							bool foundIntersection = false;
							
							int nProducedTriangles = 0;

							std::vector<bool> reverseOrientation;

							//if (step > 1)
							{
								// Create the triangle
								for (int i = 0; MarchingCubesLookupTable::triTable[cubeindex][i] != -1; i += 3)
								{

									EdgeInfo e1, e2, e3;
									Vector3d p1, p2, p3;

									int id1 = MarchingCubesLookupTable::triTable[cubeindex][i];
									int id2 = MarchingCubesLookupTable::triTable[cubeindex][i + 1];
									int id3 = MarchingCubesLookupTable::triTable[cubeindex][i + 2];
									
									p1 = vertex_list[id1];									
									p2 = vertex_list[id2];
									p3 = vertex_list[id3];

									//int64_t planeId1 = planeIdInCube( id1, id2 , index_3d, _VolumeWidth, _VolumeHeight, changeMask);
									//int64_t planeId2 = planeIdInCube( id2, id3 , index_3d, _VolumeWidth, _VolumeHeight, changeMask);
									//int64_t planeId3 = planeIdInCube( id3, id1 , index_3d, _VolumeWidth, _VolumeHeight, changeMask);

									//we need to estimate plane ids here
									if (IS_NAN(p1) || IS_NAN(p2) || IS_NAN(p3))
									{
										std::cout << "nan value found" << std::endl;
									}

									double g1[3], g2[3], g3[3];

									Vector3d p1Voxel( p1.x / _VoxelStepX , p1.y / _VoxelStepY , p1.z / _VoxelStepZ );
									Vector3d p2Voxel(p2.x / _VoxelStepX, p2.y / _VoxelStepY, p2.z / _VoxelStepZ);
									Vector3d p3Voxel(p3.x / _VoxelStepX, p3.y / _VoxelStepY, p3.z / _VoxelStepZ);

									Eigen::Vector3d side1( p2.x - p1.x , p2.y - p1.y , p2.z - p1.z ),
										side2( p3.x - p3.x , p3.y - p2.y , p3.z - p2.z ) ;

									side1.normalize();
									side2.normalize();

									Eigen::Vector3d triangleNormal = side1.cross(side2);

									triangleNormal.normalize();


									//	//compute gradient at p1 , p2 , p3
									gradientOperator->apply((double*)&p1Voxel, g1, true);
									gradientOperator->apply((double*)&p2Voxel, g2, true);
									gradientOperator->apply((double*)&p3Voxel, g3, true);

									double d1 = DOT(triangleNormal.data(), g1);
									double d2 = DOT(triangleNormal.data(), g2);
									double d3 = DOT(triangleNormal.data(), g3);

									if ((d1 + d2 + d3) > 0)
									{
										//reverse orientation 
										reverseOrientation.push_back(true);
									}
									else
									{
										reverseOrientation.push_back(false);
									}


									//check for angle deviation between three direction
									double diff1 = DOT(g1, g2);
									double diff2 = DOT(g2, g3);
									double diff3 = DOT(g3, g1);

									nProducedTriangles++;

									if (diff1 < _DeviationThreshold || diff2 < _DeviationThreshold || diff3 < _DeviationThreshold)
									{
										//the intersection resides at the edge or corner of the object , we need to further subdivide the 
										//the voxel at higher resolution(if possible)

										if(step > 1)
										needFurtherSubdivision = true;
										break;
									}

								}
							}

							if ( nProducedTriangles == 0 && step == 4 )
							{
								int sum = 0;


								for (int zScale = 0; zScale < step; zScale++)
									for (int yScale = 0; yScale < step; yScale++)
										for (int xScale = 0; xScale < step; xScale++)
										{
											Index3d newIndex;

											newIndex.x = index_3d.x + xScale;
											newIndex.y = index_3d.y + yScale;
											newIndex.z = index_3d.z + zScale;

											std::vector<float> newLeafValues;

											//sample the eight corners of the cube and check their gray values
											sampleCorners(newIndex.x, newIndex.y, newIndex.z, newLeafValues, 1);

											sum += createSurface(newLeafValues, newIndex, edgeInfos, planeInfo, gradientOperator, 1);
										}

								return true;
							}

								if (needFurtherSubdivision && step > 1)//(false)//(step > 1)//
								{
									int newStep = std::max(1, step / 2);

									
									for(int zScale = 0; zScale < 2; zScale++)
										for(int yScale = 0; yScale < 2; yScale++)
											for (int xScale = 0; xScale < 2; xScale++)
											{
												Index3d newIndex;

												newIndex.x = index_3d.x + xScale * newStep;
												newIndex.y = index_3d.y + yScale * newStep;
												newIndex.z = index_3d.z + zScale * newStep;

												std::vector<float> newLeafValues;

												//sample the eight corners of the cube and check their gray values
												sampleCorners(newIndex.x, newIndex.y, newIndex.z, newLeafValues , newStep );

												createSurface(newLeafValues , newIndex, edgeInfos, planeInfo , gradientOperator, newStep);
											}
									
								}
								else
								{
									int ptc = 0;

									// Create the triangle
									for (int i = 0; MarchingCubesLookupTable::triTable[cubeindex][i] != -1; i += 3)
									{

										EdgeInfo e1, e2, e3;
										Vector3d p1, p2, p3;

										int id1 = MarchingCubesLookupTable::triTable[cubeindex][i];
										int id2 = MarchingCubesLookupTable::triTable[cubeindex][i + 1];
										int id3 = MarchingCubesLookupTable::triTable[cubeindex][i + 2];

										//if (reverseOrientation[ptc])
										//{
										//	std::swap(id1, id3);
										//}

										ptc++;

										p1 = vertex_list[id1];
										p2 = vertex_list[id2];
										p3 = vertex_list[id3];

										//int planeId1 = planeIdInCube(id1, id2, index_3d, _VolumeWidth, _VolumeHeight, changeMask);
										//int planeId2 = planeIdInCube(id2, id3, index_3d, _VolumeWidth, _VolumeHeight, changeMask);
										//int planeId3 = planeIdInCube(id3, id1, index_3d, _VolumeWidth, _VolumeHeight, changeMask);

										foundIntersection = true;

										double g1[3], g2[3], g3[3];

										MultiResolutionMarchingCubes2::Index3d edgeVoxel1, edgeVoxel2, edgeVoxel3;
										int edgePlane1 = -1, edgePlane2 = -1, edgePlane3 = -1;

										VertexProperty<4> vp1, vp2, vp3;

										//vp1._PlaneId = planeId1;
										//vp2._PlaneId = planeId2;
										//vp3._PlaneId = planeId3;

										vp1.step = step;
										vp2.step = step;
										vp3.step = step;

										if ( planeVoxelAndType(id1, id2, index_3d, _VolumeWidth, _VolumeHeight, changeMask, edgeVoxel1, edgePlane1 , step) )
										{

										}

										if (planeVoxelAndType(id2, id3, index_3d, _VolumeWidth, _VolumeHeight, changeMask, edgeVoxel2, edgePlane2, step))
										{

										}

										if (planeVoxelAndType(id3, id1, index_3d, _VolumeWidth, _VolumeHeight, changeMask, edgeVoxel3, edgePlane3, step))
										{

										}

										vp1._Voxel = edgeVoxel1;
										vp2._Voxel = edgeVoxel2;
										vp3._Voxel = edgeVoxel3;

										vp1._PlaneType = edgePlane1;
										vp2._PlaneType = edgePlane2;
										vp3._PlaneType = edgePlane3;

#ifndef IMT_DISABLE_OMP
#pragma omp critical (edgeInsertion)
#endif				
										{

											e1 = eif[id1];
											e1._Coords = p1;
											e1._TrianglePointId = edgeInfos.size();

											e2 = eif[id2];
											e2._Coords = p2;
											e2._TrianglePointId = edgeInfos.size() + 1;

											e3 = eif[id3];
											e3._Coords = p3;
											e3._TrianglePointId = edgeInfos.size() + 2;

											edgeInfos.push_back(e1);
											edgeInfos.push_back(e2);
											edgeInfos.push_back(e3);

											

											planeInfo.push_back(vp1);
											planeInfo.push_back(vp2);
											planeInfo.push_back(vp3);

										}
									}
								}
							

							//now compute gradient at each point and compare if the voxel lies at corner
							return foundIntersection;
						}



						bool MultiResolutionMarchingCubes2::createPlaneEdges(std::vector< float > &leaf_node,
							MultiResolutionMarchingCubes2::Index3d &index_3d, std::vector<MultiResolutionMarchingCubes2::EdgeInfo>& edgeInfos,
							std::vector<VertexProperty<4>>& planeInfo, SobelGradientOperator* gradientOperator, int step, std::vector<double>& edgePoints)
						{
							int cubeindex = 0;
							Vector3d vertex_list[12];

							double checkThreshold = _IsoThreshold;
							bool needFurtherSubdivision = false;

							if (leaf_node[0] < checkThreshold) cubeindex |= 1;
							if (leaf_node[1] < checkThreshold) cubeindex |= 2;
							if (leaf_node[2] < checkThreshold) cubeindex |= 4;
							if (leaf_node[3] < checkThreshold) cubeindex |= 8;
							if (leaf_node[4] < checkThreshold) cubeindex |= 16;
							if (leaf_node[5] < checkThreshold) cubeindex |= 32;
							if (leaf_node[6] < checkThreshold) cubeindex |= 64;
							if (leaf_node[7] < checkThreshold) cubeindex |= 128;

							// Cube is entirely in/out of the surface
							if (MarchingCubesLookupTable::edgeTable[cubeindex] == 0)
							{

								if (step < 2)
									return false;

								int sum = 0;

								for (int zScale = 0; zScale < step; zScale++)
									for (int yScale = 0; yScale < step; yScale++)
										for (int xScale = 0; xScale < step; xScale++)
										{
											Index3d newIndex;

											newIndex.x = index_3d.x + xScale;
											newIndex.y = index_3d.y + yScale;
											newIndex.z = index_3d.z + zScale;

											std::vector<float> newLeafValues;

											//sample the eight corners of the cube and check their gray values
											sampleCorners(newIndex.x, newIndex.y, newIndex.z, newLeafValues, 1);

											sum += createSurface(newLeafValues, newIndex, edgeInfos, planeInfo, gradientOperator, 1);
										}


								return true;
							}


							int32_t x = index_3d.x;
							int32_t y = index_3d.y;
							int32_t z = index_3d.z;

							EdgeInfo eif[12] = {
								{ 0 , _ZStep * z + _YStep * y + x },//x , y , z  ---  x + 1 , y , z
							{ 2 , _ZStep * z + _YStep * y + (x + step) },//x + 1 , y , z ---  x + 1 , y , z + 1
							{ 0 , _ZStep * (z + step) + _YStep * y + x },//x + 1 , y , z + 1 --- x , y , z + 1
							{ 2 , _ZStep *  z + _YStep * y + x },// x,y, z + 1  --- x , y , z
							{ 0 , _ZStep * z + _YStep * (y + step) + x },//  x, y + 1 , z -- x + 1 , y + 1 , z
							{ 2 , _ZStep * z + _YStep * (y + step) + x + step },//  x + 1 , y + 1 , z --- x + 1 , y + 1 , z + 1
							{ 0 , _ZStep * (z + step) + _YStep * (y + step) + x },//  x + 1 , y + 1 , z + 1 --- x , y + 1 , z + 1
							{ 2 , _ZStep *  z + _YStep * (y + step) + x },// x , y + 1, z + 1  --- x , y + 1 , z
							{ 1 , _ZStep * z + _YStep * y + x },// x , y , z --- x , y + 1 , z
							{ 1 , _ZStep * z + _YStep * y + x + step },// x + 1 , y , z --- x + 1 , y + 1 , z
							{ 1 , _ZStep * (z + step) + _YStep * y + x + step },// x + 1 , y , z + 1 --- x + 1 , y + 1 , z + 1
							{ 1 , _ZStep * (z + step) + _YStep * y + x },// x , y , z + 1 --- x , y + 1 , z + 1	
							};



							Vector3d center;
							center.x = index_3d.x * _VoxelStepX;
							center.y = index_3d.y * _VoxelStepY;
							center.z = index_3d.z * _VoxelStepZ;

							std::vector<unsigned char> changeMask(12 * 3, 0);


							//// Find the vertices where the surface intersects the cube
							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 1) //x , y , z  ---  x + 1 , y , z
							{
								Index3d voxelPosition(x, y, z);

								int xEdgeShift = interpolateEdgeX(voxelPosition, leaf_node[0], leaf_node[1], vertex_list[0], step);

								eif[0]._EdgeId += xEdgeShift;

								changeMask[0] = 3;
								changeMask[1] = 1;
								changeMask[2] = 1;
							}


							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 2) //x + 1 , y , z ---  x + 1 , y , z + 1
							{
								Index3d voxelPosition(x + step, y, z);

								int zEdgeShift = interpolateEdgeZ(voxelPosition, leaf_node[1], leaf_node[2], vertex_list[1], step);

								eif[1]._EdgeId += zEdgeShift * _ZStep;

								changeMask[3 + 0] = 2;
								changeMask[3 + 1] = 1;
								changeMask[3 + 2] = 3;
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 4)//x + 1 , y , z + 1 --- x , y , z + 1
							{

								Index3d voxelPosition(x, y, z + step);

								int xEdgeShift = interpolateEdgeX(voxelPosition, leaf_node[3], leaf_node[2], vertex_list[2], step);

								eif[2]._EdgeId += xEdgeShift;

								changeMask[3 * 2 + 0] = 3; // x + 1 -> x
								changeMask[3 * 2 + 1] = 1; // y -> y
								changeMask[3 * 2 + 2] = 2; //z + 1 -> z + 1

							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 8)// x,y, z + 1  --- x , y , z
							{
								Index3d voxelPosition(x, y, z);

								int zEdgeShift = interpolateEdgeZ(voxelPosition, leaf_node[0], leaf_node[3], vertex_list[3], step);
								eif[3]._EdgeId += zEdgeShift * _ZStep;

								changeMask[3 * 3 + 0] = 1;
								changeMask[3 * 3 + 1] = 1;
								changeMask[3 * 3 + 2] = 3;
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 16)//  x, y + 1 , z -- x + 1 , y + 1 , z
							{
								Index3d voxelPosition(x, y + step, z);

								int xEdgeShift = interpolateEdgeX(voxelPosition, leaf_node[4], leaf_node[5], vertex_list[4], step);

								eif[4]._EdgeId += xEdgeShift;

								changeMask[3 * 4 + 0] = 3;
								changeMask[3 * 4 + 1] = 2;
								changeMask[3 * 4 + 2] = 1;
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 32)//  x + 1 , y + 1 , z --- x + 1 , y + 1 , z + 1
							{
								Index3d voxelPosition(x + step, y + step, z);

								int zEdgeShift = interpolateEdgeZ(voxelPosition, leaf_node[5], leaf_node[6], vertex_list[5], step);

								eif[5]._EdgeId += zEdgeShift * _ZStep;

								changeMask[3 * 5 + 0] = 2;
								changeMask[3 * 5 + 1] = 2;
								changeMask[3 * 5 + 2] = 3;
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 64)//  x + 1 , y + 1 , z + 1 --- x , y + 1 , z + 1
							{

								Index3d voxelPosition(x, y + step, z + step);

								int xEdgeShift = interpolateEdgeX(voxelPosition, leaf_node[7], leaf_node[6], vertex_list[6], step);

								eif[6]._EdgeId += xEdgeShift;

								changeMask[3 * 6 + 0] = 3;
								changeMask[3 * 6 + 1] = 2;
								changeMask[3 * 6 + 2] = 2;

							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 128)// x , y + 1, z + 1  --- x , y + 1 , z
							{
								Index3d voxelPosition(x, y + step, z);

								int zEdgeShift = interpolateEdgeZ(voxelPosition, leaf_node[4], leaf_node[7], vertex_list[7], step);

								eif[7]._EdgeId += zEdgeShift * _ZStep;


								changeMask[3 * 7 + 0] = 1;
								changeMask[3 * 7 + 1] = 2;
								changeMask[3 * 7 + 2] = 3;

							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 256)// x , y , z --- x , y + 1 , z
							{
								Index3d voxelPosition(x, y, z);

								int yEdgeShift = interpolateEdgeY(voxelPosition, leaf_node[0], leaf_node[4], vertex_list[8], step);

								eif[8]._EdgeId += yEdgeShift * _YStep;

								changeMask[3 * 8 + 0] = 1;
								changeMask[3 * 8 + 1] = 3;
								changeMask[3 * 8 + 2] = 1;
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 512)// x + 1 , y , z --- x + 1 , y + 1 , z
							{
								Index3d voxelPosition(x + step, y, z);

								int yEdgeShift = interpolateEdgeY(voxelPosition, leaf_node[1], leaf_node[5], vertex_list[9], step);

								eif[9]._EdgeId += yEdgeShift * _YStep;

								changeMask[3 * 9 + 0] = 2;
								changeMask[3 * 9 + 1] = 3;
								changeMask[3 * 9 + 2] = 1;
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 1024)// x + 1 , y , z + 1 --- x + 1 , y + 1 , z + 1
							{

								Index3d voxelPosition(x + step, y, z + step);

								int yEdgeShift = interpolateEdgeY(voxelPosition, leaf_node[2], leaf_node[6], vertex_list[10], step);

								eif[10]._EdgeId += yEdgeShift * _YStep;

								changeMask[3 * 10 + 0] = 2;
								changeMask[3 * 10 + 1] = 3;
								changeMask[3 * 10 + 2] = 2;
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 2048)// x , y , z + 1 --- x , y + 1 , z + 1
							{
								Index3d voxelPosition(x, y, z + step);

								int yEdgeShift = interpolateEdgeY(voxelPosition, leaf_node[3], leaf_node[7], vertex_list[11], step);

								eif[11]._EdgeId += yEdgeShift * _YStep;

								changeMask[3 * 11 + 0] = 1;
								changeMask[3 * 11 + 1] = 3;
								changeMask[3 * 11 + 2] = 2;
							}

							bool foundIntersection = false;

							int nProducedTriangles = 0;

							std::vector<bool> reverseOrientation;

							//if (step > 1)
							{
								// Create the triangle
								for (int i = 0; MarchingCubesLookupTable::triTable[cubeindex][i] != -1; i += 3)
								{

									EdgeInfo e1, e2, e3;
									Vector3d p1, p2, p3;

									int id1 = MarchingCubesLookupTable::triTable[cubeindex][i];
									int id2 = MarchingCubesLookupTable::triTable[cubeindex][i + 1];
									int id3 = MarchingCubesLookupTable::triTable[cubeindex][i + 2];

									p1 = vertex_list[id1];
									p2 = vertex_list[id2];
									p3 = vertex_list[id3];

									//int64_t planeId1 = planeIdInCube( id1, id2 , index_3d, _VolumeWidth, _VolumeHeight, changeMask);
									//int64_t planeId2 = planeIdInCube( id2, id3 , index_3d, _VolumeWidth, _VolumeHeight, changeMask);
									//int64_t planeId3 = planeIdInCube( id3, id1 , index_3d, _VolumeWidth, _VolumeHeight, changeMask);

									//we need to estimate plane ids here
									if (IS_NAN(p1) || IS_NAN(p2) || IS_NAN(p3))
									{
										std::cout << "nan value found" << std::endl;
									}

									double g1[3], g2[3], g3[3];

									Vector3d p1Voxel(p1.x / _VoxelStepX, p1.y / _VoxelStepY, p1.z / _VoxelStepZ);
									Vector3d p2Voxel(p2.x / _VoxelStepX, p2.y / _VoxelStepY, p2.z / _VoxelStepZ);
									Vector3d p3Voxel(p3.x / _VoxelStepX, p3.y / _VoxelStepY, p3.z / _VoxelStepZ);

									Eigen::Vector3d side1(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z),
										side2(p3.x - p3.x, p3.y - p2.y, p3.z - p2.z);

									side1.normalize();
									side2.normalize();

									Eigen::Vector3d triangleNormal = side1.cross(side2);

									triangleNormal.normalize();


									//	//compute gradient at p1 , p2 , p3
									gradientOperator->apply((double*)&p1Voxel, g1, true);
									gradientOperator->apply((double*)&p2Voxel, g2, true);
									gradientOperator->apply((double*)&p3Voxel, g3, true);

									double d1 = DOT(triangleNormal.data(), g1);
									double d2 = DOT(triangleNormal.data(), g2);
									double d3 = DOT(triangleNormal.data(), g3);

									if ((d1 + d2 + d3) > 0)
									{
										//reverse orientation 
										reverseOrientation.push_back(true);
									}
									else
									{
										reverseOrientation.push_back(false);
									}


									//check for angle deviation between three direction
									double diff1 = DOT(g1, g2);
									double diff2 = DOT(g2, g3);
									double diff3 = DOT(g3, g1);

									nProducedTriangles++;

									if (diff1 < _DeviationThreshold || diff2 < _DeviationThreshold || diff3 < _DeviationThreshold)
									{
										//the intersection resides at the edge or corner of the object , we need to further subdivide the 
										//the voxel at higher resolution(if possible)

										if (step > 1)
											needFurtherSubdivision = true;
										break;
									}

								}
							}

							if (nProducedTriangles == 0 && step == 4)
							{
								int sum = 0;


								for (int zScale = 0; zScale < step; zScale++)
									for (int yScale = 0; yScale < step; yScale++)
										for (int xScale = 0; xScale < step; xScale++)
										{
											Index3d newIndex;

											newIndex.x = index_3d.x + xScale;
											newIndex.y = index_3d.y + yScale;
											newIndex.z = index_3d.z + zScale;

											std::vector<float> newLeafValues;

											//sample the eight corners of the cube and check their gray values
											sampleCorners(newIndex.x, newIndex.y, newIndex.z, newLeafValues, 1);

											sum += createPlaneEdges(newLeafValues, newIndex, edgeInfos, planeInfo, gradientOperator, 1 , edgePoints);
										}

								return true;
							}

							if (needFurtherSubdivision && step > 1)//(false)//(step > 1)//
							{
								int newStep = std::max(1, step / 2);


								for (int zScale = 0; zScale < 2; zScale++)
									for (int yScale = 0; yScale < 2; yScale++)
										for (int xScale = 0; xScale < 2; xScale++)
										{
											Index3d newIndex;

											newIndex.x = index_3d.x + xScale * newStep;
											newIndex.y = index_3d.y + yScale * newStep;
											newIndex.z = index_3d.z + zScale * newStep;

											std::vector<float> newLeafValues;

											//sample the eight corners of the cube and check their gray values
											sampleCorners(newIndex.x, newIndex.y, newIndex.z, newLeafValues, newStep);

											createPlaneEdges(newLeafValues, newIndex, edgeInfos, planeInfo, gradientOperator, newStep , edgePoints);
										}

							}
							else
							{
								int ptc = 0;

								// Create the triangle
								for (int i = 0; MarchingCubesLookupTable::triTable[cubeindex][i] != -1; i += 3)
								{

									EdgeInfo e1, e2, e3;
									Vector3d p1, p2, p3;

									int id1 = MarchingCubesLookupTable::triTable[cubeindex][i];
									int id2 = MarchingCubesLookupTable::triTable[cubeindex][i + 1];
									int id3 = MarchingCubesLookupTable::triTable[cubeindex][i + 2];

									//if (reverseOrientation[ptc])
									//{
									//	std::swap(id1, id3);
									//}

									ptc++;

									p1 = vertex_list[id1];
									p2 = vertex_list[id2];
									p3 = vertex_list[id3];

									//int planeId1 = planeIdInCube(id1, id2, index_3d, _VolumeWidth, _VolumeHeight, changeMask);
									//int planeId2 = planeIdInCube(id2, id3, index_3d, _VolumeWidth, _VolumeHeight, changeMask);
									//int planeId3 = planeIdInCube(id3, id1, index_3d, _VolumeWidth, _VolumeHeight, changeMask);

									foundIntersection = true;

									double g1[3], g2[3], g3[3];

									MultiResolutionMarchingCubes2::Index3d edgeVoxel1, edgeVoxel2, edgeVoxel3;
									int edgePlane1 = -1, edgePlane2 = -1, edgePlane3 = -1;

									VertexProperty<4> vp1, vp2, vp3;

									//vp1._PlaneId = planeId1;
									//vp2._PlaneId = planeId2;
									//vp3._PlaneId = planeId3;

									vp1.step = step;
									vp2.step = step;
									vp3.step = step;

									if (planeVoxelAndType(id1, id2, index_3d, _VolumeWidth, _VolumeHeight, changeMask, edgeVoxel1, edgePlane1, step))
									{
#pragma omp critical (edgeInsertion)
										{

											edgePoints.push_back(p1.x);
											edgePoints.push_back(p1.y);
											edgePoints.push_back(p1.z);

											edgePoints.push_back(p2.x);
											edgePoints.push_back(p2.y);
											edgePoints.push_back(p2.z);
										}

									}

									if (planeVoxelAndType(id2, id3, index_3d, _VolumeWidth, _VolumeHeight, changeMask, edgeVoxel2, edgePlane2, step))
									{
#pragma omp critical (edgeInsertion)
										{
											edgePoints.push_back(p2.x);
											edgePoints.push_back(p2.y);
											edgePoints.push_back(p2.z);

											edgePoints.push_back(p3.x);
											edgePoints.push_back(p3.y);
											edgePoints.push_back(p3.z);
										}
									}

									if (planeVoxelAndType(id3, id1, index_3d, _VolumeWidth, _VolumeHeight, changeMask, edgeVoxel3, edgePlane3, step))
									{
#pragma omp critical (edgeInsertion)
										{
											edgePoints.push_back(p3.x);
											edgePoints.push_back(p3.y);
											edgePoints.push_back(p3.z);

											edgePoints.push_back(p1.x);
											edgePoints.push_back(p1.y);
											edgePoints.push_back(p1.z);
										}
									}

									vp1._Voxel = edgeVoxel1;
									vp2._Voxel = edgeVoxel2;
									vp3._Voxel = edgeVoxel3;

									vp1._PlaneType = edgePlane1;
									vp2._PlaneType = edgePlane2;
									vp3._PlaneType = edgePlane3;

#ifndef IMT_DISABLE_OMP
#pragma omp critical (edgeInsertion)
#endif				
									{
										e1 = eif[id1];
										e1._Coords = p1;
										e1._TrianglePointId = edgeInfos.size();

										e2 = eif[id2];
										e2._Coords = p2;
										e2._TrianglePointId = edgeInfos.size() + 1;

										e3 = eif[id3];
										e3._Coords = p3;
										e3._TrianglePointId = edgeInfos.size() + 2;

										edgeInfos.push_back(e1);
										edgeInfos.push_back(e2);
										edgeInfos.push_back(e3);

										planeInfo.push_back(vp1);
										planeInfo.push_back(vp2);
										planeInfo.push_back(vp3);
									}
								}
							}


							//now compute gradient at each point and compare if the voxel lies at corner
							return foundIntersection;


						}






						bool MultiResolutionMarchingCubes2::isCorner(Index3d index , int res)
						{
							return false;
						}



						void MultiResolutionMarchingCubes2::patchSurface( std::vector<unsigned char>& subdivisionLevel , std::vector<SurfaceEdge*>* surfaceEdges , int reductionBand )
						{
							//detecting the faces that need to be patched

							int wLimit, hLimit;

							for ( int planeId = 0; planeId < 3; planeId++)
							{
								for( int yy = 0; yy < hLimit; yy++ )
									for ( int xx = 0; xx < wLimit; xx++ )
									{

								    }
							}


						}




						void MultiResolutionMarchingCubes2::compute(int32_t isoThreshold, std::vector<double>& vertices, std::vector<double>& vertexNormals,
							std::vector<unsigned int>& surfaceIndices)
						{

							_MaxLevel = 2;

							if (!_VolumeData)
								return;


							if (!_VolumeWidth || !_VolumeHeight || !_VolumeDepth)
								return;


							_IsoThreshold = isoThreshold + 0.5;

							int reductionFactor = 4;

							int32_t reducedWidth = _VolumeWidth / reductionFactor; 
							int32_t reducedHeight = _VolumeHeight / reductionFactor; 
							int32_t reducedDepth = _VolumeDepth / reductionFactor; 

							std::vector<double> surface;
							std::vector<EdgeInfo> edgeInfos;
							std::vector< VertexProperty<4> > vertexProperties;

							int32_t bandWidth = 3 * reductionFactor;

							int32_t strip = 1 << _MaxLevel;

							//compute the vertex gradients now using sobel operator
							SobelGradientOperator gradientOperator(_VolumeData, _VolumeWidth, _VolumeHeight, _VolumeDepth);

							std::vector<unsigned char> subDivisionLevel(reductionFactor * 2 * _VolumeWidth * _VolumeHeight, 0);
							std::vector<SurfaceEdge*> surfaceEdges[6];

#ifndef IMT_DISABLE_OMP
#pragma omp parallel for
#endif
							for (int32_t rz = 2; rz < reducedDepth - 2; rz++)
							{
								//int32_t zz = (rz + 1) * strip;

								for (int32_t ry = 2; ry < reducedHeight - 2; ry++)
									for (int32_t rx = 2; rx < reducedWidth - 2; rx++)
									{
										std::vector<float> leafValues;

										int x = rx * reductionFactor;
										int y = ry * reductionFactor;
										int z = rz * reductionFactor;

										if ( !doesGridIntersect(x, y, z , reductionFactor) )
											    continue;

										//sample the eight corners of the cube and check their gray values
										sampleCorners(rx * reductionFactor, ry * reductionFactor, rz * reductionFactor, leafValues ,reductionFactor);

										Index3d index3d(rx * reductionFactor, ry * reductionFactor, rz * reductionFactor);

										if ( createSurface( leafValues, index3d, edgeInfos, vertexProperties, &gradientOperator, reductionFactor ) )
										{
											//intersection found , we need to check if this is a corner

										}


									}
							}

							//std::vector<double> vertices;
							surfaceIndices.resize(edgeInfos.size(), 0);

							mergeDuplicateVertices(edgeInfos, vertices, surfaceIndices);

							identifyContours( edgeInfos , vertexProperties, vertices, surfaceIndices );


							int numVertices = (int)vertices.size() / 3;

							vertexNormals.resize(vertices.size());

							double *vertexData = vertices.data();
							double *normalData = vertexNormals.data();
#ifndef IMT_DISABLE_OMP
#pragma omp parallel for
#endif
							for (int vv = 0; vv < numVertices; vv++)
							{
								double voxelCoordiantes[3] = { vertexData[3 * vv] / _VoxelStepX , vertexData[3 * vv + 1] / _VoxelStepY , vertexData[3 * vv + 2] / _VoxelStepZ };

								gradientOperator.apply(voxelCoordiantes, normalData + vv * 3, true);
							}

							double initT = cv::getTickCount();

							imt::volume::HalfEdgeMesh hem(surfaceIndices, vertices, vertexNormals);

							//hem.initialize();

							std::cout << "number of vertices and faces : " << vertices.size() / 3 << " " << surfaceIndices.size() / 3 << std::endl;

							//SurfaceMesh sm;

							//indicesToOpenMesh(vertices, surfaceIndices, &sm);

							//std::cout << "time spent in building half edge : " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;
						}



						void MultiResolutionMarchingCubes2::computeWithGridHierarchy( int32_t isoThreshold, std::vector<double>& vertices, std::vector<double>& vetexNormals, 
							std::vector<unsigned int>& surfaceIndices)
						{
							_MaxLevel = 3;

							if (!_VolumeData)
								return;


							if (!_VolumeWidth || !_VolumeHeight || !_VolumeDepth)
								return;


							_IsoThreshold = isoThreshold + 0.5;

							int reductionFactor = 16;

							int32_t reducedWidth = _VolumeWidth / reductionFactor;
							int32_t reducedHeight = _VolumeHeight / reductionFactor;
							int32_t reducedDepth = _VolumeDepth / reductionFactor;

							std::vector<double> surface;
							std::vector<EdgeInfo> edgeInfos;
							std::vector< VertexProperty<4> > vertexProperties;

							int32_t bandWidth = 3 * reductionFactor;

							int32_t strip = 1 << _MaxLevel;

							//compute the vertex gradients now using sobel operator
							SobelGradientOperator gradientOperator(_VolumeData, _VolumeWidth, _VolumeHeight, _VolumeDepth);

							std::vector<double> edgeVertices;


#ifndef IMT_DISABLE_OMP
#pragma omp parallel for
#endif
							for (int32_t rz = 2; rz < reducedDepth - 2; rz++)
							{
								//int32_t zz = (rz + 1) * strip;

								for ( int32_t ry = 2; ry < reducedHeight - 2; ry++ )
									for ( int32_t rx = 2; rx < reducedWidth - 2; rx++ )
									{
										std::vector<float> leafValues;

										int x = rx * reductionFactor;
										int y = ry * reductionFactor;
										int z = rz * reductionFactor;

										//if (!doesGridIntersect(x, y, z, reductionFactor))
										//	continue;

										//sample the eight corners of the cube and check their gray values
										sampleCorners(rx * reductionFactor, ry * reductionFactor, rz * reductionFactor, leafValues, reductionFactor);

										Index3d index3d(rx * reductionFactor, ry * reductionFactor, rz * reductionFactor);

										if (createPlaneEdges(leafValues, index3d, edgeInfos, vertexProperties, &gradientOperator, reductionFactor , edgeVertices))
										{
											//intersection found , we need to check if this is a corner

										}


									}
							}




							std::cout << "number of edge vertices : " << edgeVertices.size() / 3 << std::endl;


							tr::Display3DRoutines::displayEdges(edgeVertices);


						}




						void MultiResolutionMarchingCubes2::mergeDuplicateVertices(std::vector< MultiResolutionMarchingCubes2::EdgeInfo >& edgeInfos, std::vector<double>& surfaceVertices,
							std::vector<unsigned int>& surfaceIndices)
						{
							std::vector<EdgeInfo> axisEdgeInfos[3];

							int64_t nAllPoints = edgeInfos.size();

							for (int64_t pp = 0; pp < nAllPoints; pp++)
							{
								axisEdgeInfos[edgeInfos[pp]._EdgeType].push_back(edgeInfos[pp]);
							}


							std::fill(surfaceIndices.begin(), surfaceIndices.end(), 0);

							std::vector<unsigned int> duplicateIndices(surfaceIndices.size());
							std::vector<Eigen::Vector3f> duplicateVertices;

							//now sort three types of edges and merge them
							for (int ii = 0; ii < 3; ii++)
							{

								int nEdges = (int)axisEdgeInfos[ii].size();

								if (nEdges == 0)
									continue;

								std::sort(axisEdgeInfos[ii].begin(), axisEdgeInfos[ii].end(), [](const EdgeInfo& e1, const EdgeInfo& e2)->bool {

									return e1._EdgeId <= e2._EdgeId;
								});


								//all the edgeinfo with same edge id should be merged (The assumption here is that the surface cuts one singel voxel edge at most once)
								surfaceVertices.push_back(axisEdgeInfos[ii][0]._Coords.x);
								surfaceVertices.push_back(axisEdgeInfos[ii][0]._Coords.y);
								surfaceVertices.push_back(axisEdgeInfos[ii][0]._Coords.z);

								surfaceIndices[axisEdgeInfos[ii][0]._TrianglePointId] = (unsigned int)surfaceVertices.size() / 3 - 1;

								duplicateVertices.push_back(Eigen::Vector3f(axisEdgeInfos[ii][0]._Coords.x, axisEdgeInfos[ii][0]._Coords.y, axisEdgeInfos[ii][0]._Coords.z));
								duplicateIndices[axisEdgeInfos[ii][0]._TrianglePointId] = duplicateVertices.size() - 1;


								for (int64_t ee = 0; ee < nEdges - 1; ee++)
								{
									if (axisEdgeInfos[ii][ee]._EdgeId != axisEdgeInfos[ii][ee + 1]._EdgeId)
									{
										surfaceVertices.push_back(axisEdgeInfos[ii][ee + 1]._Coords.x);
										surfaceVertices.push_back(axisEdgeInfos[ii][ee + 1]._Coords.y);
										surfaceVertices.push_back(axisEdgeInfos[ii][ee + 1]._Coords.z);
									}

									surfaceIndices[axisEdgeInfos[ii][ee + 1]._TrianglePointId] = (unsigned int)surfaceVertices.size() / 3 - 1;

									duplicateVertices.push_back(Eigen::Vector3f(axisEdgeInfos[ii][ee + 1]._Coords.x, axisEdgeInfos[ii][ee + 1]._Coords.y, axisEdgeInfos[ii][ee + 1]._Coords.z));
									duplicateIndices[axisEdgeInfos[ii][ee + 1]._TrianglePointId] = duplicateVertices.size() - 1;
								}

							}

							//std::cout << "display duplicate mesh" << std::endl;
							//tr::Display3DRoutines::displayMesh(duplicateVertices, duplicateIndices);
						}



						bool MultiResolutionMarchingCubes2::findConnectedVertices( int id1, int id2 , std::vector<unsigned int>& surfaceIndices, 
							std::vector<MultiResolutionMarchingCubes2::VertexProperty<4>>& edgeAttributes , std::vector<int>* vertexIncidences,
							std::vector<int>& vertexConnection)
						{
							int vId1 = surfaceIndices[id1];
							int vId2 = surfaceIndices[id2];

							auto incidence = vertexIncidences[vId1];

							//auto vp1 = edgeAttributes[id1];

							auto vp2 = edgeAttributes[id2];

							//std::vector<int> vertexConnection;

							int planeType = vp2._PlaneType;
							auto voxelIndex = vp2._Voxel;
							int step = vp2.step;

							vertexConnection.push_back(vId1);//(id1);//

							if (planeType == -1)
								std::cout << "invalid plane type" << std::endl;

							//std::cout << " match vertex id " << vId2 << std::endl;

							//std::cout << vId1 << " ";

							bool exitLoop = false;

							std::cout << id2 << " " << id1 << " -- ";

							while (1)
							{
								int incidenceSize = incidence.size();

								bool nextVertexFound = false;

								
								for ( int ii = 0; ii < incidenceSize; ii++ )
								{
									auto iid = incidence[ii];

									auto& vp = edgeAttributes[iid];
										 
									if ( vp._PlaneType != vp2._PlaneType )
										       continue;

									int d[3];

									d[0] = vp._Voxel.x - vp2._Voxel.x;
									d[1] = vp._Voxel.y - vp2._Voxel.y;
									d[2] = vp._Voxel.z - vp2._Voxel.z;

									if (d[vp2._PlaneType] != 0 || vp.step >= vp2.step)//
									{
										//std::cout << "invalid transition found" << std::endl;

										std::cout << d[0] << " " << d[1] << " " << d[2] << std::endl;

										std::cout << "plane type : " << vp._PlaneType<<" "<<vp.step << std::endl;
										continue;
									}
										
									

									if (d[0] < 0 || d[0] > step || d[1] < 0 || d[1] > step || d[2] < 0 || d[2] > step)
										continue;

									//found a connection
									int vId = surfaceIndices[iid];

									std::cout << "connection at : (" << ii<<" , "<<iid << " , " << incidenceSize<<" , "<<vp.step<<" "<<vp2._PlaneType << ") ";

									vertexConnection.push_back(vId);//(iid);//

									nextVertexFound = true;

									//std::cout << vId << " ";

									if (vId == vId2)
									{
										exitLoop = true;

										break;
									}
										

									incidence = vertexIncidences[vId];

									break;

								}

								std::cout << std::endl;

								if (vertexConnection.size() > 20)
								{
									std::cout << "invalid chain found "<< vertexConnection.size()<<" "<<vId2<<" "<<vp2._PlaneType << std::endl;

									//for (auto id : vertexConnection)
									//{
									//	std::cout << id << " ";
									//}

									//std::cout << std::endl;

									return false;

									break;
								}

								if (!nextVertexFound)
								{
									std::cout << "next vertex not found "<< vertexConnection.size() << std::endl;

									return false;

									break;
								}

								if (exitLoop)
								{

									std::cout << "connection found : " << vertexConnection.size() << std::endl;

									return true;

									break;
								}

							}

							//std::cout << std::endl;

						}


						bool findId(std::vector<int>& duplicateIds, std::vector<unsigned int>& surfaceIndices, int searchId)
						{
							bool found = false;

							int nDuplicateIds = duplicateIds.size();

							for (int dd = 0; dd < nDuplicateIds; dd++)
							{
								int vId = surfaceIndices[duplicateIds[dd]];

								if (vId == searchId)
								{
									found = true;
								}
							}

							return found;
						}



						void MultiResolutionMarchingCubes2::identifyContours( std::vector< MultiResolutionMarchingCubes2::EdgeInfo >& edgeInfos, 
							                                                  std::vector< MultiResolutionMarchingCubes2::VertexProperty<4> >& planeInfo,
							                                                  std::vector<double>& surfaceVertices, std::vector<unsigned int>& surfaceIndices )
						{


							std::cout << "size of plane info and number of indices : " << planeInfo.size() << " " << surfaceIndices.size() << std::endl;


							int nTriangles = planeInfo.size() / 3;

							int nVertices = surfaceVertices.size() / 3;

							std::vector<int>* vertexIncidences = new std::vector<int>[nVertices];

							std::vector<unsigned char> usedEdgeMask(nTriangles * 3, 0);

							std::vector<Eigen::Vector3f> displayVertices(nVertices), faceColors(nTriangles, Eigen::Vector3f(1, 1, 1));

							for (int vv = 0; vv < nVertices; vv++)
							{
								displayVertices[vv](0) = surfaceVertices[3 * vv];
								displayVertices[vv](1) = surfaceVertices[3 * vv + 1];
								displayVertices[vv](2) = surfaceVertices[3 * vv + 2];
							}

							//first we need to compute vertex incidences
							for ( int tt = 0; tt < nTriangles; tt++ )
							{
								int id1 = 3 * tt;
								int id2 = 3 * tt + 1;
								int id3 = 3 * tt + 2;

								int vId1 = surfaceIndices[id1];
								int vId2 = surfaceIndices[id2];
								int vId3 = surfaceIndices[id3];

								//create vertex incidences
								vertexIncidences[vId1].push_back(id2);
								vertexIncidences[vId2].push_back(id3);
								vertexIncidences[vId3].push_back(id1);
							}

							int nInconsistencies = 0;

							int firstEdge = 0;

							//now find each edges where there is a possibility to split and form a contour.
							for ( int tt = 0; tt < nTriangles; tt++ )
							{
								int id1 = 3 * tt;
								int id2 = 3 * tt + 1;
								int id3 = 3 * tt + 2;

								int vId1 = surfaceIndices[id1];
								int vId2 = surfaceIndices[id2];
								int vId3 = surfaceIndices[id3];

								auto& a1 = vertexIncidences[vId1];
								auto& a2 = vertexIncidences[vId2];
								auto& a3 = vertexIncidences[vId3];

								auto& vp1 = planeInfo[id1];
								auto& vp2 = planeInfo[id2];
								auto& vp3 = planeInfo[id3];

								Eigen::Vector3f faceColor(1, 1, 1);

								//if ( vp1.step == 2) //nInconsistencies == 0 &&
								//	faceColor = Eigen::Vector3f(1, 0, 0);

								//if (vp1.step == 4) //nInconsistencies == 0 &&
								//	faceColor = Eigen::Vector3f(0, 1, 0);

								//now check for existence of opposite edge
								if ( !findId( a2 , surfaceIndices , vId1 ))//( !(std::find(a1.begin(), a1.end(), vId3) != a1.end()) )
								{
									if (vp1.step == 4)
									{
										nInconsistencies++;

										if (vp1._PlaneType == -1)
										{
											std::cout << "bad plane type for completion " << std::endl;
										}

										if (firstEdge < 2)
										{
											
											std::vector<int> vertexConnections;

											if (!findConnectedVertices(id2, id1, surfaceIndices, planeInfo, vertexIncidences , vertexConnections))
											{
												faceColor = Eigen::Vector3f(0, 1, 0);

												//std::cout << displayVertices[vId1].transpose() << " -- " << displayVertices[vertexConnections[0]].transpose() << " - " << displayVertices[vertexConnections[1]].transpose() << " " <<
												//	displayVertices[vertexConnections[2]].transpose() << std::endl;
											}
											else
											{
												

												faceColor = Eigen::Vector3f(1, 0, 0);
											}
										
										}
										firstEdge++; //= false;

										
									}
									

									faceColors[tt] = faceColor;
								}

								if ( !findId(a3, surfaceIndices, vId2) )//!(std::find(a2.begin(), a2.end(), vId1) != a2.end()))
								{
									if (vp2.step == 4)
										nInconsistencies++;
									
									faceColors[tt] = faceColor;
								}

								if ( !findId(a1, surfaceIndices, vId3) )//!(std::find(a3.begin(), a3.end(), vId2) != a3.end()))
								{
									if (vp3.step == 4)
										nInconsistencies++;

									faceColors[tt] = faceColor;
								}
							}


							std::cout << "number of inconsistencies : " << nInconsistencies << std::endl;

							tr::Display3DRoutines::displayMeshWithFaceColors(displayVertices, surfaceIndices, faceColors);

						}


#define INTERPOLATE_EDGE_X( p1 , p2 , valP1 , valP2 , output ) { double mu = (_IsoThreshold - valP1) / (valP2 - valP1); \
                                                                 output.x = p1.x + mu * (p2.x - p1.x); \
                                                                 }
						//std::cout<<"value of mu : "<<mu<<std::endl;
#define INTERPOLATE_EDGE_Y( p1 , p2 , valP1 , valP2 , output ) { double mu = (_IsoThreshold - valP1) / (valP2 - valP1); \
                                                                 output.y = p1.y + mu * (p2.y - p1.y); }


#define INTERPOLATE_EDGE_Z( p1 , p2 , valP1 , valP2 , output ) { double mu = (_IsoThreshold - valP1) / (valP2 - valP1); \
                                                                 output.z = p1.z + mu * (p2.z - p1.z); }




						//////////////////////////////////////////////////////////////////////////////////////////////
						void MultiResolutionMarchingCubes2::interpolateEdge(Vector3d &p1,
							Vector3d &p2,
							float val_p1,
							float val_p2,
							Vector3d &output)
						{
							float mu = (_IsoThreshold - val_p1) / (val_p2 - val_p1);
							output.x = p1.x + mu * (p2.x - p1.x);
							output.y = p1.y + mu * (p2.y - p1.y);
							output.z = p1.z + mu * (p2.z - p1.z);
						}



						int MultiResolutionMarchingCubes2::interpolateEdgeX(Index3d& voxelPosition,
							float val_p1, float val_p2, Vector3d &output, int step)
						{
							int inc = step > 0 ? 1 : -1;

							//double mu = 0;

							output = voxelPosition * _VoxelStepX;

							if (inc == step)
							{
								Vector3d p1 = voxelPosition * _VoxelStepX;

								Index3d newVoxelPosition = voxelPosition;

								newVoxelPosition.x += step;

								Vector3d p2 = newVoxelPosition * _VoxelStepX;

								
								output = p1;
								INTERPOLATE_EDGE_X(p1, p2, val_p1, val_p2, output)

									if (IS_NAN(output))
									{
										std::cout << "nan value found " << std::endl;
									}

									//Vector3d output2 = p1;

								//interpolateEdge(p1, p2, val_p1, val_p2, output);
								//std::cout<< std::endl << p1.x << " " << p1.y << " " << p1.z << " | " 
								//	                  << p2.x << " " << p2.y << " " << p2.z << " | " 
								//	                  << output.x << " " << output.y << " " << output.z;

								//std::cout << " | "
								//	<< output2.x << " " << output2.y << " " << output2.z << std::endl;

								return 0;


							}
							else
							{
								float grayValue1 = val_p1;
								float grayValue2 = 0;
								bool intersectionFound = false;

								int xx = 0;

								int nCount = step / inc;
								int cc = 0; 

								std::vector<float> grayValues;

								grayValues.push_back(grayValue1);

								for ( xx = inc ; xx != step; xx += inc  )//
								{
									//now check for transition
									grayValue2 = grayValueAtCorner(voxelPosition.x + xx, voxelPosition.y, voxelPosition.z);

									grayValues.push_back(grayValue2);

									intersectionFound = (_IsoThreshold - grayValue1) * (_IsoThreshold - grayValue2) <= (-FLT_EPSILON);

									if (intersectionFound)
									{
										break;
									}

									grayValue1 = grayValue2;

								}

								if (!intersectionFound)
								{
									grayValue2 = val_p2;

									intersectionFound = (_IsoThreshold - grayValue1) * (_IsoThreshold - grayValue2) <= (-FLT_EPSILON);

									grayValues.push_back(grayValue2);
								}


								if (intersectionFound)
								{
									auto newVoxelPosition = voxelPosition;

									newVoxelPosition.x += (xx - inc);

									Vector3d p1 = newVoxelPosition * _VoxelStepX;

									newVoxelPosition.x += inc;

									Vector3d p2 = newVoxelPosition * _VoxelStepX;

									//interpolateEdge(p1, p2, grayValue1, grayValue2, output);
									output = p1;
									INTERPOLATE_EDGE_X(p1, p2, grayValue1, grayValue2, output)

										//interpolateEdge(p1, p2, val_p1, val_p2, output);

										if (IS_NAN(output))
										{
											std::cout << "nan value found " << std::endl;
										}

										//Vector3d output2 = p1;

										
										//std::cout<< std::endl << p1.x << " " << p1.y << " " << p1.z << " | " 
										//	                  << p2.x << " " << p2.y << " " << p2.z << " | " 
										//	                  << output.x << " " << output.y << " " << output.z;

										//std::cout << " | "
										//	<< output2.x << " " << output2.y << " " << output2.z << std::endl;
								}
								else
								{
									for (int vv = 0; vv < grayValues.size(); vv++)
									{
										std::cout << grayValues[vv] << " ";
									}

									std::cout << std::endl;

									std::cout << "intersection not found "<<val_p1<<" "<<val_p2 << std::endl;
								}

								return ( xx - inc);

							}




						}

						int MultiResolutionMarchingCubes2::interpolateEdgeY(Index3d& voxelPosition,
							float val_p1, float val_p2, Vector3d &output, int step)
						{
							int inc = step > 0 ? 1 : -1;

							double mu = 0;

							output = voxelPosition * _VoxelStepY;

							if (inc == step)
							{
								Vector3d p1 = voxelPosition * _VoxelStepY;

								Index3d newVoxelPosition = voxelPosition;

								newVoxelPosition.y += step;

								Vector3d p2 = newVoxelPosition * _VoxelStepY;

								//interpolateEdge(p1, p2, val_p1, val_p2, output);
								output = p1;
								INTERPOLATE_EDGE_Y(p1, p2, val_p1, val_p2, output)

									return 0;
							}
							else
							{
								float grayValue1 = val_p1;
								float grayValue2 = 0;

								bool intersectionFound = false;

								int yy = 0;

								//int nCount = step / inc;
								//int cc = 0;

								for (yy = inc ; yy != step; yy += inc )
								{
									//now check for transition
									grayValue2 = grayValueAtCorner(voxelPosition.x, voxelPosition.y + yy, voxelPosition.z);

									intersectionFound = (_IsoThreshold - grayValue1) * (_IsoThreshold - grayValue2) <= (-FLT_EPSILON);

									if (intersectionFound)
									{
										break;
									}

									grayValue1 = grayValue2;
								}

								if (!intersectionFound)
								{
									grayValue2 = val_p2;

									intersectionFound = (_IsoThreshold - grayValue1) * (_IsoThreshold - grayValue2) <= (-FLT_EPSILON);
								}


								if (intersectionFound)
								{
									auto newVoxelPosition = voxelPosition;

									newVoxelPosition.y += (yy - inc);

									Vector3d p1 = newVoxelPosition * _VoxelStepY;

									newVoxelPosition.y += inc;

									Vector3d p2 = newVoxelPosition * _VoxelStepY;

									//interpolateEdge(p1, p2, grayValue1, grayValue2, output);
									output = p1;
									INTERPOLATE_EDGE_Y(p1, p2, grayValue1, grayValue2, output)

										//interpolateEdge(p1, p2, val_p1, val_p2, output);
								}
								else
								{
									std::cout << "intersection not found " << std::endl;
								}

								if (IS_NAN(output))
								{
									std::cout << "nan value found " << std::endl;
								}

								return (yy - inc);
							}



						}

						int MultiResolutionMarchingCubes2::interpolateEdgeZ(Index3d& voxelPosition,
							float val_p1, float val_p2, Vector3d &output, int step)
						{
							int inc = step > 0 ? 1 : -1;

							double mu = 0;

							output = voxelPosition * _VoxelStepZ;

							if (inc == step)
							{
								Vector3d p1 = voxelPosition * _VoxelStepZ;

								Index3d newVoxelPosition = voxelPosition;

								newVoxelPosition.z += step;

								Vector3d p2 = newVoxelPosition * _VoxelStepZ;

								output = p1;
								//interpolateEdge(p1, p2, val_p1, val_p2, output);
								
								INTERPOLATE_EDGE_Z(p1, p2, val_p1, val_p2, output)

									return 0;
							}
							else
							{
								float grayValue1 = val_p1;
								float grayValue2 = 0;

								bool intersectionFound = false;

								int zz = 0;
								int nCount = step / inc;
								int cc = 0;

								for (zz = inc; zz != step; zz += inc)
								{
									//now check for transition
									grayValue2 = grayValueAtCorner(voxelPosition.x, voxelPosition.y, voxelPosition.z + zz);

									intersectionFound = (_IsoThreshold - grayValue1) * (_IsoThreshold - grayValue2) <= (-FLT_EPSILON);

									if (intersectionFound)
									{
										break;
									}

									grayValue1 = grayValue2;

									cc++;
								}

								if (!intersectionFound)
								{
									grayValue2 = val_p2;

									intersectionFound = (_IsoThreshold - grayValue1) * (_IsoThreshold - grayValue2) <= (-FLT_EPSILON);
								}


								if (intersectionFound)
								{
									auto newVoxelPosition = voxelPosition;

									newVoxelPosition.z += (zz - inc);

									Vector3d p1 = newVoxelPosition * _VoxelStepZ;

									newVoxelPosition.z += inc;

									Vector3d p2 = newVoxelPosition * _VoxelStepZ;

									//interpolateEdge(p1, p2, grayValue1, grayValue2, output);
									output = p1;
								    INTERPOLATE_EDGE_Z(p1, p2, grayValue1, grayValue2, output)

									//interpolateEdge(p1, p2, grayValue1, grayValue2, output);
								}
								else
								{
									std::cout << "intersection not found " << std::endl;
								}

								if (IS_NAN(output))
								{
									std::cout << "nan value found " << std::endl;
								}

								return (zz - inc);

							}



						}


						MultiResolutionMarchingCubes2::~MultiResolutionMarchingCubes2()
						{

						}
					}
				}
			}
		}
	}
}
