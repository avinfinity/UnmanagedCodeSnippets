#include "stdafx.h"
#include "MultiResolutionMarchingCubes.h"
#include "MarchingCubesLookupTable.h"
#include "SobelGradientOperator.h"
#include <algorithm>
#include <iostream>
#include "eigenincludes.h"
#include "display3droutines.h"

namespace Zeiss {
	namespace IMT {

		namespace NG {

			namespace NeoInsights {

				namespace Volume {

					namespace MeshGenerator {



#define IS_NAN(vec) (vec.x != vec.x || vec.y != vec.y || vec.z != vec.z)

						MultiResolutionMarchingCubes::MultiResolutionMarchingCubes(unsigned short* volumeData, int32_t volumeWidth, int32_t volumeHeight, int32_t volumeDepth,
							double voxelStepX, double voxelStepY, double voxelStepZ) :
							_VolumeData(volumeData), _VolumeWidth(volumeWidth), _VolumeHeight(volumeHeight), _VolumeDepth(volumeDepth),
							_VoxelStepX(voxelStepX), _VoxelStepY(voxelStepY), _VoxelStepZ(voxelStepZ)
						{
							_ZStep = (int64_t)_VolumeWidth * (int64_t)_VolumeHeight;
							_YStep = (int64_t)_VolumeWidth;

							_DeviationThreshold = cos(0.0 / 180 * M_PI);
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
							grayValues.resize(8);

							grayValues[0] = grayValueAtCorner(x, y, z);
							grayValues[1] = grayValueAtCorner(x + step, y, z);
							grayValues[2] = grayValueAtCorner(x + step, y, z + step);
							grayValues[3] = grayValueAtCorner(x, y, z + step);
							grayValues[4] = grayValueAtCorner(x, y + step, z);
							grayValues[5] = grayValueAtCorner(x + step, y + step, z);
							grayValues[6] = grayValueAtCorner(x + step, y + step, z + step);
							grayValues[7] = grayValueAtCorner(x, y + step, z + step);
						}



						MultiResolutionMarchingCubes::Vector3d::Vector3d()
						{
							x = 0;
							y = 0;
							z = 0;
						}

						MultiResolutionMarchingCubes::Vector3d::Vector3d(double xVal, double yVal, double zVal)
						{
							x = xVal;
							y = yVal;
							z = zVal;
						}



#define DOT(v1 , v2) v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]


						bool MultiResolutionMarchingCubes::createSurface(std::vector< float > &leaf_node,
							MultiResolutionMarchingCubes::Index3d &index_3d, std::vector<MultiResolutionMarchingCubes::EdgeInfo>& edgeInfos, SobelGradientOperator* gradientOperator,
							int step)
						{
							int cubeindex = 0;
							Vector3d vertex_list[12];

							double checkThreshold = _IsoThreshold;


							if (leaf_node[0] < checkThreshold) cubeindex |= 1;
							if (leaf_node[1] < checkThreshold) cubeindex |= 2;
							if (leaf_node[2] < checkThreshold) cubeindex |= 4;
							if (leaf_node[3] < checkThreshold) cubeindex |= 8;
							if (leaf_node[4] < checkThreshold) cubeindex |= 16;
							if (leaf_node[5] < checkThreshold) cubeindex |= 32;
							if (leaf_node[6] < checkThreshold) cubeindex |= 64;
							if (leaf_node[7] < checkThreshold) cubeindex |= 128;

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
								return false;

							Vector3d center;
							center.x = index_3d.x * _VoxelStepX;
							center.y = index_3d.y * _VoxelStepY;
							center.z = index_3d.z * _VoxelStepZ;


							//// Find the vertices where the surface intersects the cube
							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 1) //x , y , z  ---  x + 1 , y , z
							{
								Index3d voxelPosition(x, y, z);

								int xEdgeShift = interpolateEdgeX(voxelPosition, leaf_node[0], leaf_node[1], vertex_list[0], step);

								eif[0]._EdgeId += xEdgeShift;
							}


							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 2) //x + 1 , y , z ---  x + 1 , y , z + 1
							{
								Index3d voxelPosition(x + step, y, z);

								int zEdgeShift = interpolateEdgeZ(voxelPosition, leaf_node[1], leaf_node[2], vertex_list[1], step);
							
								eif[1]._EdgeId += zEdgeShift * _ZStep;
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 4)//x + 1 , y , z + 1 --- x , y , z + 1
							{
								Index3d voxelPosition(x + step, y, z + step);

								int xEdgeShift = interpolateEdgeX(voxelPosition, leaf_node[2], leaf_node[3], vertex_list[2], -step);

								eif[2]._EdgeId += xEdgeShift;
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 8)// x,y, z + 1  --- x , y , z
							{
								Index3d voxelPosition(x, y, z + step);

								int zEdgeShift = interpolateEdgeZ(voxelPosition, leaf_node[3], leaf_node[0], vertex_list[3], -step);

								eif[3]._EdgeId += zEdgeShift * _ZStep;
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 16)//  x, y + 1 , z -- x + 1 , y + 1 , z
							{
								Index3d voxelPosition(x, y + step, z);

								int xEdgeShift = interpolateEdgeX(voxelPosition, leaf_node[4], leaf_node[5], vertex_list[4], step);

								eif[4]._EdgeId += xEdgeShift;
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 32)//  x + 1 , y + 1 , z --- x + 1 , y + 1 , z + 1
							{
								Index3d voxelPosition(x + step, y + step, z);

								int zEdgeShift = interpolateEdgeZ(voxelPosition, leaf_node[5], leaf_node[6], vertex_list[5], step);

								eif[5]._EdgeId += zEdgeShift * _ZStep;
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 64)//  x + 1 , y + 1 , z + 1 --- x , y + 1 , z + 1
							{
								Index3d voxelPosition(x + step, y + step, z + step);

								int xEdgeShift = interpolateEdgeX(voxelPosition, leaf_node[6], leaf_node[7], vertex_list[6], -step);

								eif[6]._EdgeId += xEdgeShift;
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 128)// x , y + 1, z + 1  --- x , y + 1 , z
							{
								Index3d voxelPosition(x, y + step, z + step);

								int zEdgeShift = interpolateEdgeZ(voxelPosition, leaf_node[7], leaf_node[4], vertex_list[7], -step);

								eif[7]._EdgeId += zEdgeShift * _ZStep;
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 256)// x , y , z --- x , y + 1 , z
							{
								Index3d voxelPosition(x, y, z);

								int yEdgeShift = interpolateEdgeY(voxelPosition, leaf_node[0], leaf_node[4], vertex_list[8], step);

								eif[8]._EdgeId += yEdgeShift * _YStep;
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 512)// x + 1 , y , z --- x + 1 , y + 1 , z
							{
								Index3d voxelPosition(x + step, y, z);

								int yEdgeShift = interpolateEdgeY(voxelPosition, leaf_node[1], leaf_node[5], vertex_list[9], step);

								eif[9]._EdgeId += yEdgeShift * _YStep;
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 1024)// x + 1 , y , z + 1 --- x + 1 , y + 1 , z + 1
							{

								Index3d voxelPosition(x + step, y, z + step);

								int yEdgeShift = interpolateEdgeY(voxelPosition, leaf_node[2], leaf_node[6], vertex_list[10], step);

								eif[10]._EdgeId += yEdgeShift * _YStep;
							}

							if (MarchingCubesLookupTable::edgeTable[cubeindex] & 2048)// x , y , z + 1 --- x , y + 1 , z + 1
							{
								Index3d voxelPosition(x, y, z + step);

								int yEdgeShift = interpolateEdgeY(voxelPosition, leaf_node[3], leaf_node[7], vertex_list[11], step);

								eif[11]._EdgeId += yEdgeShift * _YStep;
							}

							bool foundIntersection = false;
							bool needFurtherSubdivision = false;


							if (step > 1)
							{
								// Create the triangle
								for (int i = 0; MarchingCubesLookupTable::triTable[cubeindex][i] != -1; i += 3)
								{

									EdgeInfo e1, e2, e3;
									Vector3d p1, p2, p3;

									p1 = vertex_list[MarchingCubesLookupTable::triTable[cubeindex][i]];
									
									p2 = vertex_list[MarchingCubesLookupTable::triTable[cubeindex][i + 1]];

									p3 = vertex_list[MarchingCubesLookupTable::triTable[cubeindex][i + 2]];

									if (IS_NAN(p1) || IS_NAN(p2) || IS_NAN(p3))
									{
										std::cout << "nan value found" << std::endl;
									}

									

									double g1[3], g2[3], g3[3];


										Vector3d p1Voxel( p1.x / _VoxelStepX , p1.y / _VoxelStepY , p1.z / _VoxelStepZ );
										Vector3d p2Voxel(p2.x / _VoxelStepX, p2.y / _VoxelStepY, p2.z / _VoxelStepZ);
										Vector3d p3Voxel(p3.x / _VoxelStepX, p3.y / _VoxelStepY, p3.z / _VoxelStepZ);

									//	//compute gradient at p1 , p2 , p3
										gradientOperator->apply((double*)&p1Voxel, g1, true);
										gradientOperator->apply((double*)&p2Voxel, g2, true);
										gradientOperator->apply((double*)&p3Voxel, g3, true);

										//check for angle deviation between three direction
										double diff1 = DOT(g1, g2);
										double diff2 = DOT(g2, g3);
										double diff3 = DOT(g3, g1);

										if (diff1 < _DeviationThreshold || diff2 < _DeviationThreshold || diff3 < _DeviationThreshold)
										{
											//the intersection resides at the edge or corner of the object , we need to further subdivide the 
											//the voxel at higher resolution(if possible)

											needFurtherSubdivision = true;

											break;
										}
									
								}
							}

								if (false)//(step > 1)//(needFurtherSubdivision)
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

												createSurface(newLeafValues , newIndex, edgeInfos, gradientOperator, newStep);
											}
									
								}
								else
								{
									// Create the triangle
									for (int i = 0; MarchingCubesLookupTable::triTable[cubeindex][i] != -1; i += 3)
									{

										EdgeInfo e1, e2, e3;
										Vector3d p1, p2, p3;

										p1 = vertex_list[MarchingCubesLookupTable::triTable[cubeindex][i]];

										p2 = vertex_list[MarchingCubesLookupTable::triTable[cubeindex][i + 1]];

										p3 = vertex_list[MarchingCubesLookupTable::triTable[cubeindex][i + 2]];

										foundIntersection = true;

										double g1[3], g2[3], g3[3];

#pragma omp critical (edgeInsertion)
										{

											e1 = eif[MarchingCubesLookupTable::triTable[cubeindex][i]];
											e1._Coords = p1;
											e1._TrianglePointId = edgeInfos.size();

											e2 = eif[MarchingCubesLookupTable::triTable[cubeindex][i + 1]];
											e2._Coords = p2;
											e2._TrianglePointId = edgeInfos.size() + 1;

											e3 = eif[MarchingCubesLookupTable::triTable[cubeindex][i + 2]];
											e3._Coords = p3;
											e3._TrianglePointId = edgeInfos.size() + 2;

											edgeInfos.push_back(e1);
											edgeInfos.push_back(e2);
											edgeInfos.push_back(e3);
										}
									}
								}
							

							//now compute gradient at each point and compare if the voxel lies at corner
							return foundIntersection;
						}


						bool MultiResolutionMarchingCubes::isCorner(Index3d index , int res)
						{
							return false;
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

							int reductionFactor = 4;

							int32_t reducedWidth = _VolumeWidth / reductionFactor; // >> _MaxLevel;
							int32_t reducedHeight = _VolumeHeight / reductionFactor; // >> _MaxLevel;
							int32_t reducedDepth = _VolumeDepth / reductionFactor; // >> _MaxLevel;

							std::vector<double> surface;
							std::vector<EdgeInfo> edgeInfos;

							int32_t bandWidth = 3 * reductionFactor;//(1 << _MaxLevel);

							int32_t strip = 1 << _MaxLevel;

							//float *reinterpolatedSlices = new float[_ZStep * bandWidth];

							//compute the vertex gradients now using sobel operator
							SobelGradientOperator gradientOperator(_VolumeData, _VolumeWidth, _VolumeHeight, _VolumeDepth);


//#pragma omp parallel for
							for (int32_t rz = 2; rz < reducedDepth - 2; rz++)
							{
								int32_t zz = (rz + 1) * strip;

								for (int32_t ry = 2; ry < reducedHeight - 2; ry++)
									for (int32_t rx = 2; rx < reducedWidth - 2; rx++)
									{
										std::vector<float> leafValues;

										//sample the eight corners of the cube and check their gray values
										//sampleCorners2(rx, ry, rz, leafValues);
										sampleCorners(rx * reductionFactor, ry * reductionFactor, rz * reductionFactor, leafValues ,reductionFactor);

										Index3d index3d(rx * reductionFactor, ry * reductionFactor, rz * reductionFactor);

										if ( createSurface(leafValues, index3d, edgeInfos, &gradientOperator, reductionFactor) )
										{
											//intersection found , we need to check if this is a corner

										}


									}
							}

							//std::vector<double> vertices;
							surfaceIndices.resize(edgeInfos.size(), 0);

							mergeDuplicateVertices(edgeInfos, vertices, surfaceIndices);



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

							std::cout << "display duplicate mesh" << std::endl;
							tr::Display3DRoutines::displayMesh(duplicateVertices, duplicateIndices);
						}




#if 1

#define INTERPOLATE_EDGE_X( p1 , p2 , valP1 , valP2 , output ) { double mu = (_IsoThreshold - valP1) / (valP2 - valP1); \
                                                                 output.x = p1.x + mu * (p2.x - p1.x); \
                                                                 }
						//std::cout<<"value of mu : "<<mu<<std::endl;
#define INTERPOLATE_EDGE_Y( p1 , p2 , valP1 , valP2 , output ) { double mu = (_IsoThreshold - valP1) / (valP2 - valP1); \
                                                                 output.y = p1.y + mu * (p2.y - p1.y); }


#define INTERPOLATE_EDGE_Z( p1 , p2 , valP1 , valP2 , output ) { double mu = (_IsoThreshold - valP1) / (valP2 - valP1); \
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



						int MultiResolutionMarchingCubes::interpolateEdgeX(Index3d& voxelPosition,
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

						int MultiResolutionMarchingCubes::interpolateEdgeY(Index3d& voxelPosition,
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

						int MultiResolutionMarchingCubes::interpolateEdgeZ(Index3d& voxelPosition,
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


						MultiResolutionMarchingCubes::~MultiResolutionMarchingCubes()
						{

						}
					}
				}
			}
		}
	}
}
