#include "stdafx.h"
#include "MultiResolutionMarchingCubes.h"
#include "MarchingCubesLookupTable.h"
#include "SobelGradientOperator.h"
#include <algorithm>
#include <iostream>
#include <unordered_set>
#include "display3droutines.h"
#include "opencvincludes.h"

//#include "SphereCalculator.h"

namespace Zeiss {
	namespace IMT {

		namespace NG {

			namespace NeoInsights {

				namespace Volume {

					namespace MeshGenerator {


						//using namespace Algo::DefectAnalysis::Interop;


	

#define grayValueAt(x ,y, z) _VolumeData[ z * _ZStep + y * _YStep + x ]

						MultiResolutionMarchingCubes::MultiResolutionMarchingCubes(unsigned short* volumeData, int32_t volumeWidth, int32_t volumeHeight, int32_t volumeDepth,
							double voxelStepX, double voxelStepY, double voxelStepZ) :
							_VolumeData(volumeData), _VolumeWidth(volumeWidth), _VolumeHeight(volumeHeight), _VolumeDepth(volumeDepth),
							_VoxelStepX(voxelStepX), _VoxelStepY(voxelStepY), _VoxelStepZ(voxelStepZ)
						{
							_ZStep = (int64_t)_VolumeWidth * (int64_t)_VolumeHeight;
							_YStep = (int64_t)_VolumeWidth;
						}


						void MultiResolutionMarchingCubes::reinterpolateSlices(int z, int strip, float *reinterpolatedSlices)
						{
							for (int zz = 0; zz < strip; zz++)
								for (int yy = 0; yy < _VolumeHeight; yy++)
									for (int xx = 0; xx < _VolumeWidth; xx++)
									{
										reinterpolatedSlices[zz * _ZStep + yy * _YStep + xx] = (float)grayValueAtCorner(xx, yy, z + zz);
									}
						}


						float MultiResolutionMarchingCubes::grayValueAtCorner(int x, int y, int z)
						{
							float grayValue = _VolumeData[z * _ZStep + y * _YStep + x];

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
						void MultiResolutionMarchingCubes::sampleCorners2(int x, int y, int z, std::vector<float>& grayValues)
						{
							int xx = 4 * x;
							int yy = 4 * y;
							int zz = 4 * z;

							//grayValues.resize(8);

							sampleCorners(xx, yy, zz, grayValues, 4);
						}

						//sample corners at resolution level 1
						void MultiResolutionMarchingCubes::sampleCorners1(int x, int y, int z, std::vector<float>& grayValues)
						{
							int xx = 2 * x;
							int yy = 2 * y;
							int zz = 2 * z;

							sampleCorners(x, y, z, grayValues, 2);
						}


						//sample corners at resolution level 0
						void MultiResolutionMarchingCubes::sampleCorners(int x, int y, int z, std::vector<float>& grayValues, int step)
						{
							//grayValues.resize(8);

#if 1
							grayValues[0] = grayValueAt(x, y, z); //grayValueAtCorner(x, y, z);
							grayValues[1] = grayValueAt((x + step), y, z); //grayValueAtCorner(x + step, y, z);
							grayValues[2] = grayValueAt((x + step), y, (z + step));// grayValueAtCorner(x + step, y, z + step);
							grayValues[3] = grayValueAt(x, y, (z + step));// grayValueAtCorner(x, y, z + step);
							grayValues[4] = grayValueAt(x, (y + step), z); //grayValueAtCorner(x, y + step, z);
							grayValues[5] = grayValueAt((x + step), (y + step), z); //grayValueAtCorner(x + step, y + step, z);
							grayValues[6] = grayValueAt((x + step), (y + step), (z + step)); //grayValueAtCorner(x + step, y + step, z + step);
							grayValues[7] = grayValueAt(x, (y + step), (z + step));// grayValueAtCorner(x, y + step, z + step);
#else
							grayValues[0] = grayValueAtCorner(x, y, z);
							grayValues[1] = grayValueAtCorner(x + step, y, z);
							grayValues[2] = grayValueAtCorner(x + step, y, z + step);
							grayValues[3] = grayValueAtCorner(x, y, z + step);
							grayValues[4] = grayValueAtCorner(x, y + step, z);
							grayValues[5] = grayValueAtCorner(x + step, y + step, z);
							grayValues[6] = grayValueAtCorner(x + step, y + step, z + step);
							grayValues[7] = grayValueAtCorner(x, y + step, z + step);
#endif
						}


						void MultiResolutionMarchingCubes::createSurface(std::vector< float > &leaf_node,
							MultiResolutionMarchingCubes::Index3d &index_3d, std::vector<MultiResolutionMarchingCubes::EdgeInfo>& edgeInfos,
							int step)
						{
							int cubeindex = 0;
							Vector3d vertex_list[12];


							if (leaf_node[0] < _IsoThreshold) cubeindex |= 1;
							if (leaf_node[1] < _IsoThreshold) cubeindex |= 2;
							if (leaf_node[2] < _IsoThreshold) cubeindex |= 4;
							if (leaf_node[3] < _IsoThreshold) cubeindex |= 8;
							if (leaf_node[4] < _IsoThreshold) cubeindex |= 16;
							if (leaf_node[5] < _IsoThreshold) cubeindex |= 32;
							if (leaf_node[6] < _IsoThreshold) cubeindex |= 64;
							if (leaf_node[7] < _IsoThreshold) cubeindex |= 128;

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

							// Cube is entirely in/out of the surface
							if (MarchingCubesLookupTable::edgeTable[cubeindex] == 0)
								return;

							//return;

							//Vector3d center;
							//center.x = index_3d.x * _VoxelStepX;
							//center.y = index_3d.y * _VoxelStepY;
							//center.z = index_3d.z * _VoxelStepZ;


							//// Find the vertices where the surface intersects the cube
							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 1) //x , y , z  ---  x + 1 , y , z
							{
								Index3d voxelPosition(x, y, z);

								interpolateEdgeX(voxelPosition, leaf_node[0], leaf_node[1], vertex_list[0], step);
							}


							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 2) //x + 1 , y , z ---  x + 1 , y , z + 1
							{
								Index3d voxelPosition(x + step, y, z);

								interpolateEdgeZ(voxelPosition, leaf_node[1], leaf_node[2], vertex_list[1], step);
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 4)//x + 1 , y , z + 1 --- x , y , z + 1
							{
								Index3d voxelPosition(x + step, y, z + step);

								interpolateEdgeX(voxelPosition, leaf_node[2], leaf_node[3], vertex_list[2], -step);
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 8)// x,y, z + 1  --- x , y , z
							{
								Index3d voxelPosition(x, y, z + step);

								interpolateEdgeZ(voxelPosition, leaf_node[3], leaf_node[0], vertex_list[3], -step);
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 16)//  x, y + 1 , z -- x + 1 , y + 1 , z
							{
								Index3d voxelPosition(x, y + step, z);

								interpolateEdgeX(voxelPosition, leaf_node[4], leaf_node[5], vertex_list[4], step);
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 32)//  x + 1 , y + 1 , z --- x + 1 , y + 1 , z + 1
							{
								Index3d voxelPosition(x + step, y + step, z);

								interpolateEdgeZ(voxelPosition, leaf_node[5], leaf_node[6], vertex_list[5], step);
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 64)//  x + 1 , y + 1 , z + 1 --- x , y + 1 , z + 1
							{
								Index3d voxelPosition(x + step, y + step, z + step);

								interpolateEdgeX(voxelPosition, leaf_node[6], leaf_node[7], vertex_list[6], -step);
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 128)// x , y + 1, z + 1  --- x , y + 1 , z
							{
								Index3d voxelPosition(x, y + step, z + step);

								interpolateEdgeZ(voxelPosition, leaf_node[7], leaf_node[4], vertex_list[7], -step);
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 256)// x , y , z --- x , y + 1 , z
							{
								Index3d voxelPosition(x, y, z);

								interpolateEdgeY(voxelPosition, leaf_node[0], leaf_node[4], vertex_list[8], step);
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 512)// x + 1 , y , z --- x + 1 , y + 1 , z
							{
								Index3d voxelPosition(x + step, y, z);

								interpolateEdgeY(voxelPosition, leaf_node[1], leaf_node[5], vertex_list[9], step);
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 1024)// x + 1 , y , z + 1 --- x + 1 , y + 1 , z + 1
							{

								Index3d voxelPosition(x + step, y, z + step);

								interpolateEdgeY(voxelPosition, leaf_node[2], leaf_node[6], vertex_list[10], step);
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 2048)// x , y , z + 1 --- x , y + 1 , z + 1
							{
								Index3d voxelPosition(x, y, z + step);

								interpolateEdgeY(voxelPosition, leaf_node[3], leaf_node[7], vertex_list[11], step);
							}


#pragma omp critical
							{
								// Create the triangle
								for (int i = 0; MarchingCubesLookupTable::triTable[cubeindex][i] != -1; i += 3)
								{

									EdgeInfo e1, e2, e3;
									Vector3d p1, p2, p3;
									p1 = vertex_list[MarchingCubesLookupTable::triTable[cubeindex][i]];

									e1 = eif[MarchingCubesLookupTable::triTable[cubeindex][i]];
									e1._Coords = p1;
									e1._TrianglePointId = edgeInfos.size();
									edgeInfos.push_back(e1);

									p2 = vertex_list[MarchingCubesLookupTable::triTable[cubeindex][i + 1]];

									e2 = eif[MarchingCubesLookupTable::triTable[cubeindex][i + 1]];
									e2._Coords = p2;
									e2._TrianglePointId = edgeInfos.size();

									edgeInfos.push_back(e2);

									p3 = vertex_list[MarchingCubesLookupTable::triTable[cubeindex][i + 2]];

									e3 = eif[MarchingCubesLookupTable::triTable[cubeindex][i + 2]];
									e3._Coords = p3;
									e3._TrianglePointId = edgeInfos.size();

									edgeInfos.push_back(e3);
								}
							}

							//now compute gradient at each point and compare if the voxel lies at corner
						}


						void MultiResolutionMarchingCubes::compute(int32_t isoThreshold, std::vector<double>& vertices, std::vector<double>& vetexNormals,
							std::vector<unsigned int>& surfaceIndices)
						{

							_MaxLevel = 2;

							if (!_VolumeData)
								return;


							if (!_VolumeWidth || !_VolumeHeight || !_VolumeDepth)
								return;


							_IsoThreshold = isoThreshold + 0.5;

							double volumeSizeInMB = (double)_VolumeWidth * (double)_VolumeHeight * (double)_VolumeDepth / (1024.0 * 1024.0);

							int reductionBand = 4;

							if (volumeSizeInMB < 1600)
							{
								reductionBand = 1;
							}
							else if (volumeSizeInMB < 16000)
							{
								reductionBand = 2;
							}
							else if (volumeSizeInMB < 32000)
							{
								reductionBand = 3;
							}
							else
							{
								reductionBand = 4;
							}

							//reductionBand = 1;



							int32_t reducedWidth = _VolumeWidth / reductionBand; //_VolumeWidth >> _MaxLevel;
							int32_t reducedHeight = _VolumeHeight / reductionBand; //_VolumeHeight >> _MaxLevel;
							int32_t reducedDepth = _VolumeDepth / reductionBand;  //>> _MaxLevel;

							std::vector<double> surface;
							std::vector<EdgeInfo> edgeInfos;

							int reserveSize = 25e6;

							edgeInfos.reserve(reserveSize);
							surface.reserve(reserveSize * 3);

#pragma omp parallel for
							for (int32_t rz = 2; rz < reducedDepth - 2; rz++)
							{
								for (int32_t ry = 2; ry < reducedHeight - 2; ry++)
									for (int32_t rx = 2; rx < reducedWidth - 2; rx++)
									{
										std::vector<float> leafValues(8);

										//sample the eight corners of the cube and check their gray values
										sampleCorners(rx * reductionBand, ry * reductionBand, rz * reductionBand, leafValues, reductionBand);

										Index3d index3d(rx * reductionBand, ry * reductionBand, rz * reductionBand);

										createSurface(leafValues, index3d, edgeInfos, reductionBand);

									}
							}


							std::cout << "number of surface indices : " << edgeInfos.size() << std::endl;

							//std::vector<double> vertices;
							surfaceIndices.resize(edgeInfos.size(), 0);

							mergeDuplicateVertices(edgeInfos, vertices, surfaceIndices);


							double initT = cv::getTickCount();

							removeSmallFragments(vertices, surfaceIndices);

							std::cout << " time spent in removing small fragments : " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;

							//compute the vertex gradients now using sobel operator
							SobelGradientOperator gradientOperator(_VolumeData, _VolumeWidth, _VolumeHeight, _VolumeDepth);

							int numVertices = (int)vertices.size() / 3;

							vetexNormals.resize(vertices.size());

							double *vertexData = vertices.data();
							double *normalData = vetexNormals.data();

#pragma omp parallel for
							for (int vv = 0; vv < numVertices; vv++)
							{
								double voxelCoordiantes[3] = { vertexData[3 * vv] / _VoxelStepX , vertexData[3 * vv + 1] / _VoxelStepY , vertexData[3 * vv + 2] / _VoxelStepZ };

								gradientOperator.apply(voxelCoordiantes, normalData + vv * 3, true);
							}

							//delete[] reinterpolatedSlices;
						}






						void MultiResolutionMarchingCubes::generateConnectedComponents(int32_t isoThreshold, std::vector<ConnectedComponent>& connectedComponents)
						{
							_MaxLevel = 2;

							if (!_VolumeData)
								return;


							if (!_VolumeWidth || !_VolumeHeight || !_VolumeDepth)
								return;


							_IsoThreshold = isoThreshold + 0.5;

							double volumeSizeInMB = (double)_VolumeWidth * (double)_VolumeHeight * (double)_VolumeDepth / (1024.0 * 1024.0);

							int reductionBand = 4;

							if (volumeSizeInMB < 100)
							{
								reductionBand = 1;
							}
							else if (volumeSizeInMB < 5000)
							{
								reductionBand = 2;
							}
							else if (volumeSizeInMB < 12000)
							{
								reductionBand = 3;
							}
							else
							{
								reductionBand = 4;
							}



							int32_t reducedWidth = _VolumeWidth / reductionBand; //_VolumeWidth >> _MaxLevel;
							int32_t reducedHeight = _VolumeHeight / reductionBand; //_VolumeHeight >> _MaxLevel;
							int32_t reducedDepth = _VolumeDepth / reductionBand;  //>> _MaxLevel;

							std::vector<double> surface;
							std::vector<EdgeInfo> edgeInfos;

							int reserveSize = 25e6;

							edgeInfos.reserve(reserveSize);
							surface.reserve(reserveSize * 3);

#pragma omp parallel for
							for (int32_t rz = 2; rz < reducedDepth - 2; rz++)
							{
								for (int32_t ry = 2; ry < reducedHeight - 2; ry++)
									for (int32_t rx = 2; rx < reducedWidth - 2; rx++)
									{
										std::vector<float> leafValues(8);

										//sample the eight corners of the cube and check their gray values
										sampleCorners(rx * reductionBand, ry * reductionBand, rz * reductionBand, leafValues, reductionBand);

										Index3d index3d(rx * reductionBand, ry * reductionBand, rz * reductionBand);

										createSurface(leafValues, index3d, edgeInfos, reductionBand);

									}
							}


							std::vector<unsigned int> surfaceIndices;

							std::vector<double> vertices;

							//std::vector<double> vertices;
							surfaceIndices.resize(edgeInfos.size(), 0);

							mergeDuplicateVertices(edgeInfos, vertices, surfaceIndices);

							GenerateConnectedComponents(vertices, surfaceIndices, connectedComponents);


							int numComponents = connectedComponents.size();

							std::cout << "number of components : " << numComponents << std::endl;

							std::vector<vtkSmartPointer<vtkPolyData>> meshes;

							for ( int cc = 0; cc < numComponents; cc++ )
							{
								std::cout << connectedComponents[cc]._Vertices.size() / 3 << " " << connectedComponents[cc]._SurfaceIndices.size() / 3 << std::endl;


								Eigen::Vector3f color(0, 1, 0);

								//if (fitSphereToComponent(connectedComponents[cc]))
								//{

								//}
								//else
								//{
								//	color = Eigen::Vector3f(1, 0, 0);

								//	//std::cout << "failed to fit sphere to component" << std::endl;
								//}
								
								vtkSmartPointer<vtkPolyData> cage = tr::Display3DRoutines::generateCage(connectedComponents[cc]._StartX, connectedComponents[cc]._StartY, connectedComponents[cc]._StartZ,
									connectedComponents[cc]._Width, connectedComponents[cc]._Height, connectedComponents[cc]._Depth);

								vtkSmartPointer<vtkPolyData> mesh = tr::Display3DRoutines::generatePolyData(connectedComponents[cc]._Vertices, connectedComponents[cc]._SurfaceIndices , color);

								//tr::Display3DRoutines::displayMesh(connectedComponents[cc]._Vertices, connectedComponents[cc]._SurfaceIndices);


								

								//meshes.push_back(cage);
								meshes.push_back(mesh);

								
							}

							tr::Display3DRoutines::displayPolyData(meshes);

						}


						void MultiResolutionMarchingCubes::generatePoreInfo( std::string poreFilePath , std::vector<ConnectedComponent>& connectedComponents )
						{

						}




						void MultiResolutionMarchingCubes::mergeDuplicateVertices(std::vector< MultiResolutionMarchingCubes::EdgeInfo >& edgeInfos, std::vector<double>& surfaceVertices,
							std::vector<unsigned int>& surfaceIndices)
						{
							std::vector<EdgeInfo> axisEdgeInfos[3];

							int64_t nAllPoints = edgeInfos.size();

							for (int64_t pp = 0; pp < nAllPoints; pp++)
							{
								axisEdgeInfos[edgeInfos[pp]._EdgeType].push_back(edgeInfos[pp]);
							}


							std::fill(surfaceIndices.begin(), surfaceIndices.end(), 0);

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


								for (int64_t ee = 0; ee < nEdges - 1; ee++)
								{
									if (axisEdgeInfos[ii][ee]._EdgeId != axisEdgeInfos[ii][ee + 1]._EdgeId)
									{
										surfaceVertices.push_back(axisEdgeInfos[ii][ee + 1]._Coords.x);
										surfaceVertices.push_back(axisEdgeInfos[ii][ee + 1]._Coords.y);
										surfaceVertices.push_back(axisEdgeInfos[ii][ee + 1]._Coords.z);
									}

									surfaceIndices[axisEdgeInfos[ii][ee + 1]._TrianglePointId] = (unsigned int)surfaceVertices.size() / 3 - 1;
								}

							}
						}



						void MultiResolutionMarchingCubes::removeSmallFragments(std::vector<double>& surfaceVertices,
							std::vector<unsigned int>& surfaceIndices)
						{

							int nVertices = surfaceVertices.size() / 3;
							int nTriangles = surfaceIndices.size() / 3;

							//std::cout << "number of vertices and triangles : " << nVertices << " " << nTriangles << std::endl;

							std::vector<unsigned char> vertexMask(nVertices, 0);

							const int maxDegree = 20;

							std::unordered_set< int > *incidentVerticesUS = new std::unordered_set< int >[nVertices];

							for ( int tt = 0; tt < nTriangles; tt++ )
							{

								int vId1 = surfaceIndices[3 * tt];
								int vId2 = surfaceIndices[3 * tt + 1];
								int vId3 = surfaceIndices[3 * tt + 2];

								incidentVerticesUS[vId1].insert(vId2);
								incidentVerticesUS[vId1].insert(vId3);

								incidentVerticesUS[vId2].insert(vId1);
								incidentVerticesUS[vId2].insert(vId3);
								
								incidentVerticesUS[vId3].insert(vId1);
								incidentVerticesUS[vId3].insert(vId2);
							}

							std::vector<int> incidenceDegree(nVertices, 0);
							std::vector<int> incidentVertices(nVertices * maxDegree, -1);

							for ( int vv = 0; vv < nVertices; vv++ )
							{
								incidenceDegree[vv] = incidentVerticesUS[vv].size();

								int cc = 0;

								for (auto vId : incidentVerticesUS[vv])
								{
									incidentVertices[vv * maxDegree + cc] = vId;

									cc++;
								}
							}

							delete[] incidentVerticesUS;

							int vId = 0;

							std::vector<int> componentIds(nVertices , -1);

							int currentComonentId = 0;

							for (int vv = 0; vv < nVertices; vv++)
							{
								if ( vertexMask[vv] )
									continue;

							   std::vector<int> vertexInterface;

							   vertexMask[vv] = 1;
							   componentIds[vv] = currentComonentId;
							   vertexInterface.push_back(vv);

								while (1)
								{
									int vertexInterfaceSize = vertexInterface.size();

									if (vertexInterfaceSize == 0)
									{
										break;
									}

									for (int vi = 0; vi < vertexInterfaceSize; vi++)
									{
										componentIds[vertexInterface[vi]] = currentComonentId;
									}

									std::vector<int> nextVertexInterface;

									for (int vi = 0; vi < vertexInterfaceSize; vi++)
									{
										int cVid = vertexInterface[vi];

										int incidenceCount = incidenceDegree[cVid];

										for (int ii = 0; ii < incidenceCount; ii++)
										{
											int iVid = incidentVertices[cVid * maxDegree + ii];

											if (!vertexMask[iVid])
											{
												vertexMask[iVid] = 1;

												nextVertexInterface.push_back(iVid);
											}
										}
									}

									vertexInterface = nextVertexInterface;

								}

								currentComonentId++;
							}

							std::cout << "number of components : " << currentComonentId << std::endl;


							std::vector<unsigned int> *components = new std::vector<unsigned int>[currentComonentId];

							int numComponents = currentComonentId;

							std::vector < std::pair<int, int> > componentSizes(numComponents );

							for (int cc = 0; cc < numComponents; cc++)
							{
								componentSizes[cc].first = cc;
								componentSizes[cc].second = 0;
							}

							for ( int vv = 0; vv < nVertices; vv++ )
							{
								int vertexComponentId = componentIds[vv];

								componentSizes[vertexComponentId].second++;
							}

							//now lets sort the components
							std::sort( componentSizes.begin() , componentSizes.end() , [](const std::pair<int , int>& left , 
								                                                          const std::pair<int , int>& right)->bool{
							
								return left.second > right.second;
							});

							double minFragRatio = 0.05;

							std::fill(vertexMask.begin(), vertexMask.end(), 0);

							std::vector<bool> isValidComponent(numComponents, false);

							int numValidComponents = 0;

							for ( int cc = 0; cc < numComponents; cc++ )
							{

								double fr = (double)componentSizes[cc].second / (double)nVertices;
								

								if ( fr < minFragRatio )
									    break;

								numValidComponents++;

								isValidComponent[componentSizes[cc].first] = true;
							}

							std::cout << "number of valid components : " << numValidComponents << std::endl;

							int nValidVertices = 0;

							for (int vv = 0; vv < nVertices; vv++)
							{
								int vertexComponentId = componentIds[vv];

								if ( isValidComponent[vertexComponentId] )
								{
									vertexMask[vv] = 1;

									nValidVertices++;
								}
							}

							std::vector<int> filteredToAllVerticesMap(nVertices , -1);

							std::vector<double> filteredVertices(nValidVertices * 3);
							std::vector<unsigned int> filteredIndices;

							vId = 0;

							for (int vv = 0; vv < nVertices; vv++)
							{
								if (vertexMask[vv])
								{
									filteredToAllVerticesMap[vv] = vId;

									filteredVertices[3 * vId] = surfaceVertices[3 * vv];
									filteredVertices[3 * vId + 1] = surfaceVertices[3 * vv + 1];
									filteredVertices[3 * vId + 2] = surfaceVertices[3 * vv + 2];

									vId++;
								}
							}


							for (int tt = 0; tt < nTriangles; tt++)
							{
								int vId1 = surfaceIndices[3 * tt];
								int vId2 = surfaceIndices[3 * tt + 1];
								int vId3 = surfaceIndices[3 * tt + 2];


								if (!vertexMask[vId1])
									continue;

								int fVid1 = filteredToAllVerticesMap[vId1];
								int fVid2 = filteredToAllVerticesMap[vId2];
								int fVid3 = filteredToAllVerticesMap[vId3];

								filteredIndices.push_back(fVid1);
								filteredIndices.push_back(fVid2);
								filteredIndices.push_back(fVid3);

							}



							//now we need to remove all the small fragments

 
							/*for ( int tt = 0; tt < nTriangles; tt++ )
							{
								int vId1 = surfaceIndices[3 * tt];
								int vId2 = surfaceIndices[3 * tt + 1];
								int vId3 = surfaceIndices[3 * tt + 2];

								int compId = componentIds[vId1];

								components[compId].push_back(vId1);
								components[compId].push_back(vId2);
								components[compId].push_back(vId3);
							}*/




							//std::ve

							//for (int cc = 0; cc < currentComonentId; cc++)
							//{
								//tr::Display3DRoutines::displayMesh(filteredVertices, filteredIndices);
							//}


						}



						void MultiResolutionMarchingCubes::GenerateConnectedComponents(std::vector<double>& surfaceVertices,
							std::vector<unsigned int>& surfaceIndices,
							std::vector< ConnectedComponent >& connectedComponents)
						{

							int nVertices = surfaceVertices.size() / 3;
							int nTriangles = surfaceIndices.size() / 3;

#if 1

							

							std::vector<unsigned char> vertexMask(nVertices, 0);

							const int maxDegree = 20;

							std::unordered_set< int > *incidentVerticesUS = new std::unordered_set< int >[nVertices];

							for (int tt = 0; tt < nTriangles; tt++)
							{

								int vId1 = surfaceIndices[3 * tt];
								int vId2 = surfaceIndices[3 * tt + 1];
								int vId3 = surfaceIndices[3 * tt + 2];

								incidentVerticesUS[vId1].insert(vId2);
								incidentVerticesUS[vId1].insert(vId3);

								incidentVerticesUS[vId2].insert(vId1);
								incidentVerticesUS[vId2].insert(vId3);

								incidentVerticesUS[vId3].insert(vId1);
								incidentVerticesUS[vId3].insert(vId2);
							}

							std::vector<int> incidenceDegree(nVertices, 0);
							std::vector<int> incidentVertices(nVertices * maxDegree, -1);

							for (int vv = 0; vv < nVertices; vv++)
							{
								incidenceDegree[vv] = incidentVerticesUS[vv].size();

								int cc = 0;

								for (auto vId : incidentVerticesUS[vv])
								{
									incidentVertices[vv * maxDegree + cc] = vId;

									cc++;
								}
							}

							delete[] incidentVerticesUS;

							int vId = 0;

							std::vector<int> componentIds(nVertices, -1);



							int currentComonentId = 0;

							for (int vv = 0; vv < nVertices; vv++)
							{
								if (vertexMask[vv])
									continue;

								std::vector<int> vertexInterface;

								vertexMask[vv] = 1;
								componentIds[vv] = currentComonentId;
								vertexInterface.push_back(vv);

								while (1)
								{
									int vertexInterfaceSize = vertexInterface.size();

									if (vertexInterfaceSize == 0)
									{
										break;
									}

									for (int vi = 0; vi < vertexInterfaceSize; vi++)
									{
										componentIds[vertexInterface[vi]] = currentComonentId;
									}

									std::vector<int> nextVertexInterface;

									for (int vi = 0; vi < vertexInterfaceSize; vi++)
									{
										int cVid = vertexInterface[vi];

										int incidenceCount = incidenceDegree[cVid];

										for (int ii = 0; ii < incidenceCount; ii++)
										{
											int iVid = incidentVertices[cVid * maxDegree + ii];

											if (!vertexMask[iVid])
											{
												vertexMask[iVid] = 1;

												nextVertexInterface.push_back(iVid);
											}
										}
									}

									vertexInterface = nextVertexInterface;

								}

								currentComonentId++;
							}
#else
                             std::vector<int> componentIds(nVertices, -1);

							 std::vector< std::pair<int, int> > surfaceMinVertexIds(nTriangles);

							 for (int vv = 0; vv < nVertices; vv++)
							 {
								 componentIds[vv] = vv;
							 }

							 for (int ii = 0; ii < nTriangles; ii++)
							 {
								 
								 int vId1 = surfaceIndices[3 * ii];
								 int vId2 = surfaceIndices[3 * ii + 1];
								 int vId3 = surfaceIndices[3 * ii + 2];

								 int cId = std::min(vId1, vId2);
								 cId = std::min(cId, vId3);

								 surfaceMinVertexIds[ii].first = ii;
								 surfaceMinVertexIds[ii].second = cId;
							 }

							 std::sort(surfaceMinVertexIds.begin(), surfaceMinVertexIds.end(), [](const std::pair<int, int>& left, const std::pair<int, int>& right)->bool 
							 {							 
								 return left.second < right.second;
							 });

							 for (int ii = 0; ii < nTriangles; ii++)
							 {
								 int vId = surfaceMinVertexIds[ii].first;

								 int vId1 = surfaceIndices[3 * ii];
								 int vId2 = surfaceIndices[3 * ii + 1];
								 int vId3 = surfaceIndices[3 * ii + 2];
							 
								 int cId = componentIds[vId];
								 
								 componentIds[vId1] = cId;
								 componentIds[vId2] = cId;
								 componentIds[vId3] = cId;
							 }


#endif


							std::vector<unsigned int> *components = new std::vector<unsigned int>[currentComonentId];

							int numComponents = currentComonentId;

							connectedComponents.resize(numComponents);

							std::vector < std::pair<int, int> > componentSizes(numComponents);

							for (int cc = 0; cc < numComponents; cc++)
							{
								componentSizes[cc].first = cc;
								componentSizes[cc].second = 0;
							}

							for (int vv = 0; vv < nVertices; vv++)
							{
								int vertexComponentId = componentIds[vv];

								componentSizes[vertexComponentId].second++;
							}

							//std::sort(componentSizes.begin(), componentSizes.end(), [](const std::pair<int, int>& left,
							//	const std::pair<int, int>& right)->bool
							//{

							//	return left.second > right.second;
							//});


							double minFragRatio = 0.05;

							std::vector<int> validComponentIds, allToValidComponentMap(numComponents, -1);

							std::cout << "number of total components : " << numComponents << std::endl;

							for (int cc = 0; cc < numComponents; cc++)
							{
								double fr = (double)componentSizes[cc].second / (double)nVertices;

								if (fr > 0.05)
									continue;

								validComponentIds.push_back(cc);

								allToValidComponentMap[cc] = validComponentIds.size() - 1;
							}

							for (int cc = 0; cc < numComponents; cc++)
							{
								componentSizes[cc].first = cc;
								componentSizes[cc].second = 0;
							}

							std::vector<int> filteredToAllVerticesMap(nVertices, -1);

							connectedComponents.resize(validComponentIds.size());

							for (int vv = 0; vv < nVertices; vv++)
							{
								int vertexComponentId = componentIds[vv];

								int validComponentId = allToValidComponentMap[vertexComponentId];

								if (validComponentId < 0)
									continue;


								filteredToAllVerticesMap[vv] = componentSizes[validComponentId].second;

								componentSizes[validComponentId].second++;

								connectedComponents[validComponentId]._Vertices.push_back(surfaceVertices[3 * vv]);
								connectedComponents[validComponentId]._Vertices.push_back(surfaceVertices[3 * vv + 1]);
								connectedComponents[validComponentId]._Vertices.push_back(surfaceVertices[3 * vv + 2]);
							}





							for (int tt = 0; tt < nTriangles; tt++)
							{
								int vId1 = surfaceIndices[3 * tt];
								int vId2 = surfaceIndices[3 * tt + 1];
								int vId3 = surfaceIndices[3 * tt + 2];


								if (!vertexMask[vId1])
									continue;

								int fVid1 = filteredToAllVerticesMap[vId1];
								int fVid2 = filteredToAllVerticesMap[vId2];
								int fVid3 = filteredToAllVerticesMap[vId3];

								int componentId = componentIds[vId1];

								int validComponentId = allToValidComponentMap[componentId];

								if (validComponentId < 0)
									continue;

								connectedComponents[validComponentId]._SurfaceIndices.push_back(fVid1);
								connectedComponents[validComponentId]._SurfaceIndices.push_back(fVid2);
								connectedComponents[validComponentId]._SurfaceIndices.push_back(fVid3);

							}

							SobelGradientOperator gradientOperator(_VolumeData, _VolumeWidth, _VolumeHeight, _VolumeDepth);

							int nFilteredComponents = connectedComponents.size();
							
							for (int cc = 0; cc < nFilteredComponents; cc++)
							{

								double xmin = FLT_MAX, xmax = 0, ymin = FLT_MAX, ymax = 0, zmin = FLT_MAX, zmax = 0;

								auto& comp = connectedComponents[cc];

								int nComponentVertices = comp._Vertices.size() / 3;

								//tr::Display3DRoutines::displayMesh(comp._Vertices, comp._SurfaceIndices);

								comp._Normals.resize(comp._Vertices.size());

								double *normalData = comp._Normals.data();

								for (int vv = 0; vv < nComponentVertices; vv++)
								{
									double voxelCoordiantes[3] = { comp._Vertices[3 * vv] / _VoxelStepX , comp._Vertices[3 * vv + 1] / _VoxelStepY , comp._Vertices[3 * vv + 2] / _VoxelStepZ };

									gradientOperator.apply(voxelCoordiantes, normalData + vv * 3, true);

									xmin = std::min(xmin, voxelCoordiantes[0]);
									xmax = std::max(xmax, voxelCoordiantes[0]);

									ymin = std::min(ymin, voxelCoordiantes[1]);
									ymax = std::max(ymax, voxelCoordiantes[1]);

									zmin = std::min(zmin, voxelCoordiantes[2]);
									zmax = std::max(zmax, voxelCoordiantes[2]);
								}


								int sx = std::floor(xmin); // _VoxelStepX;

								sx = std::max(sx, 0);


								int sy = std::floor(ymin);// / _VoxelStepY;

								sy = std::max(sy, 0);


								int sz = std::floor(zmin);// / _VoxelStepZ;

								sz = std::max(sz, 0);

								int xd = std::ceil(xmax - sx);// / _VoxelStepX;
								int yd = std::ceil(ymax - sy);// / _VoxelStepY;
								int zd = std::ceil(zmax - sz);// / _VoxelStepZ;

								xd = std::min(xd + 1, _VolumeWidth - 2);
								yd = std::min(yd + 1, _VolumeHeight - 2);
								zd = std::min(zd + 1, _VolumeDepth - 2);


								comp._StartX = sx * _VoxelStepX;
								comp._StartY = sy * _VoxelStepY;
								comp._StartZ = sz * _VoxelStepZ;

								comp._Width = xd * _VoxelStepX;
								comp._Height = yd * _VoxelStepY;
								comp._Depth = zd * _VoxelStepZ;

								int64_t volumeVoxelCount = 0;

								std::cout<<" ( "<<sx<<" , "<<sy<<" , "<<sz<<" ) ---- "  << xd << " " << yd << " " << zd << std::endl;

								//comp._GrayValues = new unsigned short[xd * yd * zd];
								//comp._MaskValues = new unsigned char[xd * yd * zd];

								for (int zz = sz; zz < zd + sz; zz++)
									for (int yy = sy; yy < yd + sy; yy++)
										for (int xx = sx; xx < xd + sx; xx++)
										{
											if (xx < 0 || yy < 0 || zz < 0 || xx >= _VolumeWidth || yy >= _VolumeHeight || zz >= _VolumeDepth)
												continue;

											int64_t voxelId = _ZStep * zz + _YStep * yy + xx;

											if (_VolumeData[voxelId] < _IsoThreshold)
											{
												volumeVoxelCount++;
											}
										}


								comp._Volume = volumeVoxelCount * _VoxelStepX * _VoxelStepY * _VoxelStepZ;

								std::cout << "pore volume : " << comp._Volume <<" "<<volumeVoxelCount<< std::endl;





							}
						}




#if 1

#define INTERPOLATE_EDGE_X( p1 , p2 , valP1 , valP2 , output ) { mu = (_IsoThreshold - valP1) / (valP2 - valP1); \
                                                                 output.x = p1.x + mu * (p2.x - p1.x); }

#define INTERPOLATE_EDGE_Y( p1 , p2 , valP1 , valP2 , output ) { mu = (_IsoThreshold - valP1) / (valP2 - valP1); \
                                                                 output.y = p1.y + mu * (p2.y - p1.y); }


#define INTERPOLATE_EDGE_Z( p1 , p2 , valP1 , valP2 , output ) { mu = (_IsoThreshold - valP1) / (valP2 - valP1); \
                                                                 output.z = p1.z + mu * (p2.z - p1.z); }


#else

#define INTERPOLATE_EDGE_X( p1 , p2 , valP1 , valP2 , output ) interpolateEdge( p1 , p2 , valP1 , valP2 , output );

#define INTERPOLATE_EDGE_Y( p1 , p2 , valP1 , valP2 , output ) interpolateEdge( p1 , p2 , valP1 , valP2 , output );


#define INTERPOLATE_EDGE_Z( p1 , p2 , valP1 , valP2 , output ) interpolateEdge( p1 , p2 , valP1 , valP2 , output );

#endif



						//////////////////////////////////////////////////////////////////////////////////////////////
						void MultiResolutionMarchingCubes::interpolateEdge(Vector3d &p1,
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



						void MultiResolutionMarchingCubes::interpolateEdgeXGradientMaxima(Index3d& voxelPosition,
							float val_p1, float val_p2, Vector3d &output, int step)
						{

							//create an array of gradients and find the local maxima of the grdients along the edge 
							float sampleStep = 0.25;

							int numSamples = step * 5 / sampleStep + 2;

							int start = -2 / sampleStep - 1;

							std::vector<float> grayValueSamples( numSamples );

							for (int ii = 0; ii < numSamples; ii++)
							{
								float sx = voxelPosition.x + start;
							}

							std::vector<float> gradientSamples(numSamples - 2);

							for (int ii = 1; ii < numSamples - 1; ii++)
							{
								gradientSamples[ii] = grayValueSamples[ii] - grayValueSamples[ii - 1];
							}

							//apply gaussian smoothing on the gradients and find the maxima


							

						}

						void MultiResolutionMarchingCubes::interpolateEdgeYGradientMaxima(Index3d& voxelPosition,
							float val_p1, float val_p2, Vector3d &output, int step)
						{
							//create an array of gradients and find the local maxima of the grdients along the edge
						}

						void MultiResolutionMarchingCubes::interpolateEdgeZGradientMaxima(Index3d& voxelPosition,
							float val_p1, float val_p2, Vector3d &output, int step)
						{
							//create an array of gradients and find the local maxima of the grdients along the edge
						}




						void MultiResolutionMarchingCubes::interpolateEdgeX(Index3d& voxelPosition,
							float val_p1, float val_p2, Vector3d &output, int step)
						{
							int inc = step > 0 ? 1 : -1;

							double mu = 0;

							if (inc == step)
							{
								Vector3d p1 = voxelPosition * _VoxelStepX;

								Index3d newVoxelPosition = voxelPosition;

								newVoxelPosition.x += step;

								Vector3d p2 = newVoxelPosition * _VoxelStepX;

								//interpolateEdge(p1, p2, val_p1, val_p2, output);
								output = p1;
								INTERPOLATE_EDGE_X(p1, p2, val_p1, val_p2, output)

							}
							else
							{
								float grayValue1 = val_p1;
								float grayValue2 = 0;
								bool intersectionFound = false;

								int xx = 0;

								for (xx = inc; xx != step; xx += inc)
								{
									//now check for transition
									grayValue2 = grayValueAtCorner(voxelPosition.x + xx, voxelPosition.y, voxelPosition.z);

									intersectionFound = (_IsoThreshold - grayValue1) * (_IsoThreshold - grayValue2) <= 0;

									if (intersectionFound)
									{
										break;
									}

									grayValue1 = grayValue2;
								}

								if (!intersectionFound)
								{
									grayValue2 = val_p2;

									intersectionFound = (_IsoThreshold - grayValue1) * (_IsoThreshold - grayValue2) <= 0;
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
								}

							}

						}

						void MultiResolutionMarchingCubes::interpolateEdgeY(Index3d& voxelPosition,
							float val_p1, float val_p2, Vector3d &output, int step)
						{
							int inc = step > 0 ? 1 : -1;

							double mu = 0;

							if (inc == step)
							{
								Vector3d p1 = voxelPosition * _VoxelStepY;

								Index3d newVoxelPosition = voxelPosition;

								newVoxelPosition.y += step;

								Vector3d p2 = newVoxelPosition * _VoxelStepY;

								//interpolateEdge(p1, p2, val_p1, val_p2, output);
								output = p1;
								INTERPOLATE_EDGE_Y(p1, p2, val_p1, val_p2, output)
							}
							else
							{
								float grayValue1 = val_p1;
								float grayValue2 = 0;

								bool intersectionFound = false;

								int yy = 0;

								for (yy = inc; yy != step; yy += inc)
								{
									//now check for transition
									grayValue2 = grayValueAtCorner(voxelPosition.x, voxelPosition.y + yy, voxelPosition.z);

									intersectionFound = (_IsoThreshold - grayValue1) * (_IsoThreshold - grayValue2) <= 0;

									if (intersectionFound)
									{
										break;
									}

									grayValue1 = grayValue2;
								}

								if (!intersectionFound)
								{
									grayValue2 = val_p2;

									intersectionFound = (_IsoThreshold - grayValue1) * (_IsoThreshold - grayValue2) <= 0;
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
								}
							}

						}

						void MultiResolutionMarchingCubes::interpolateEdgeZ(Index3d& voxelPosition,
							float val_p1, float val_p2, Vector3d &output, int step)
						{
							int inc = step > 0 ? 1 : -1;

							double mu = 0;

							if (inc == step)
							{
								Vector3d p1 = voxelPosition * _VoxelStepZ;

								Index3d newVoxelPosition = voxelPosition;

								newVoxelPosition.z += step;

								Vector3d p2 = newVoxelPosition * _VoxelStepZ;

								//interpolateEdge(p1, p2, val_p1, val_p2, output);
								output = p1;
								INTERPOLATE_EDGE_Z(p1, p2, val_p1, val_p2, output)
							}
							else
							{
								float grayValue1 = val_p1;
								float grayValue2 = 0;

								bool intersectionFound = false;

								int zz = 0;

								for (zz = inc; zz != step; zz += inc)
								{
									//now check for transition
									grayValue2 = grayValueAtCorner(voxelPosition.x, voxelPosition.y, voxelPosition.z + zz);

									intersectionFound = (_IsoThreshold - grayValue1) * (_IsoThreshold - grayValue2) <= 0;

									if (intersectionFound)
									{
										break;
									}

									grayValue1 = grayValue2;
								}

								if (!intersectionFound)
								{
									grayValue2 = val_p2;

									intersectionFound = (_IsoThreshold - grayValue1) * (_IsoThreshold - grayValue2) <= 0;
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
								}

							}

						}


						MultiResolutionMarchingCubes::~MultiResolutionMarchingCubes()
						{

						}
					}
				}
			}
		}
	}
}
