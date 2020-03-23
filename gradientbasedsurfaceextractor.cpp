#include "gradientbasedsurfaceextractor.h"
#include <iostream>
#include <opencvincludes.h>
#include "SobelGradientOperator.h"
#include "MarchingCubesLookupTable.h"
#include "eigenincludes.h"
#include "display3droutines.h"


namespace imt {

	namespace volume {

#define grayValueAt(x ,y, z) _VolumeData[ (z) * _ZStep + (y) * _YStep + (x) ]

		GradientBasedSurfaceExtractor::GradientBasedSurfaceExtractor(unsigned short* volumeData, int32_t volumeWidth, int32_t volumeHeight, int32_t volumeDepth,
			double voxelStepX, double voxelStepY, double voxelStepZ) : MultiResolutionMarchingCubes(volumeData, volumeWidth , volumeHeight , volumeDepth , 
				voxelStepX , voxelStepY , voxelStepZ )
		{

		}


		void GradientBasedSurfaceExtractor::compute( int grayValue1 , int grayValue2, std::vector<double>& vertices, std::vector<double>& vetexNormals,
			std::vector<unsigned int>& surfaceIndices )
		{

			_StartThreshold = grayValue1;
			_EndThreshold = grayValue2;

			_MaxLevel = 2;

			if (!_VolumeData)
				return;


			if (!_VolumeWidth || !_VolumeHeight || !_VolumeDepth)
				return;

			_IsoThreshold = ((int)( (grayValue1 + grayValue2) / 2 ) ) + 0.5;

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


			int pointCount = 0;

			std::vector<Eigen::Vector3f> points;

//#pragma omp parallel for
			for (int32_t rz = 2; rz < reducedDepth - 2; rz++)
			{
				for (int32_t ry = 2; ry < reducedHeight - 2; ry++)
					for (int32_t rx = 2; rx < reducedWidth - 2; rx++)
					{
						std::vector<float> leafValues(8);

						//sample the eight corners of the cube and check their gray values
						//sampleCorners(rx * reductionBand, ry * reductionBand, rz * reductionBand, leafValues, reductionBand);

						std::vector<bool> isInsideMaterial(8 , false);

						std::vector<Eigen::Vector3f> edgePoints(12 , Eigen::Vector3f(rx * reductionBand, ry * reductionBand, rz * reductionBand));

						if ( !sampleCornersBasedOnGradient(rx * reductionBand, ry * reductionBand, rz * reductionBand, isInsideMaterial, edgePoints ,  reductionBand))
							 continue;


						int cubeindex = 0;
						Vector3d vertex_list[12];


						if (!isInsideMaterial[0]) cubeindex |= 1;
						if (!isInsideMaterial[1]) cubeindex |= 2;
						if (!isInsideMaterial[2]) cubeindex |= 4;
						if (!isInsideMaterial[3]) cubeindex |= 8;
						if (!isInsideMaterial[4]) cubeindex |= 16;
						if (!isInsideMaterial[5]) cubeindex |= 32;
						if (!isInsideMaterial[6]) cubeindex |= 64;
						if (!isInsideMaterial[7]) cubeindex |= 128;


						// Cube is entirely in/out of the surface
						if (MarchingCubesLookupTable::edgeTable[cubeindex] == 0)
							continue;

						//Index3d index3d(rx * reductionBand, ry * reductionBand, rz * reductionBand);


						//points.push_back(Eigen::Vector3f(rx * reductionBand, ry * reductionBand, rz * reductionBand));



						//createSurface(leafValues, index3d, edgeInfos, reductionBand);

						for (int i = 0; MarchingCubesLookupTable::triTable[cubeindex][i] != -1; i += 3)
						{

							//EdgeInfo e1, e2, e3;
							Eigen::Vector3f p1, p2, p3;
							p1 = edgePoints[MarchingCubesLookupTable::triTable[cubeindex][i]];

							//e1 = eif[MarchingCubesLookupTable::triTable[cubeindex][i]];
							//e1._Coords = p1;
							//e1._TrianglePointId = edgeInfos.size();
							//edgeInfos.push_back(e1);

							p2 = edgePoints[MarchingCubesLookupTable::triTable[cubeindex][i + 1]];

							//e2 = eif[MarchingCubesLookupTable::triTable[cubeindex][i + 1]];
							//e2._Coords = p2;
							//e2._TrianglePointId = edgeInfos.size();

							//edgeInfos.push_back(e2);

							p3 = edgePoints[MarchingCubesLookupTable::triTable[cubeindex][i + 2]];

							//e3 = eif[MarchingCubesLookupTable::triTable[cubeindex][i + 2]];
							//e3._Coords = p3;
							//e3._TrianglePointId = edgeInfos.size();

							//edgeInfos.push_back(e3);

							points.push_back(p1);
							points.push_back(p2);
							points.push_back(p3);
						}



					}
			}


			std::vector<unsigned int> ids(points.size());

			for (int ii = 0; ii < ids.size(); ii++)
			{
				ids[ii] = ii;
			}


			tr::Display3DRoutines::displayPointSet(points, std::vector< Eigen::Vector3f >(points.size(), Eigen::Vector3f(1, 0, 0)));

			tr::Display3DRoutines::displayMesh(points, ids);


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

		}

		void GradientBasedSurfaceExtractor::sampleCorners( int x, int y, int z, std::vector<float>& grayValues, int step)
		{
			grayValues[0] = grayValueAt(x, y, z);
			grayValues[1] = grayValueAt((x + step), y, z);
			grayValues[2] = grayValueAt((x + step), y, (z + step));
			grayValues[3] = grayValueAt(x, y, (z + step));
			grayValues[4] = grayValueAt(x, (y + step), z);
			grayValues[5] = grayValueAt((x + step), (y + step), z);
			grayValues[6] = grayValueAt((x + step), (y + step), (z + step));
			grayValues[7] = grayValueAt(x, (y + step), (z + step));
		}


		void GradientBasedSurfaceExtractor::processEdgeX(int x, int y, int z, int step, int cornerId1,
			int cornerId2, int edgeId, std::vector<int>& cornerFlag,
			std::vector<int>& edgeFlag, Eigen::Vector3f& edgePoint )
		{
			//x , y , z  ---  x + 1 , y , z
			float g1 = maxGradientX(x - step, y, z, step), g2 = maxGradientX(x, y, z, step), g3 = maxGradientX(x + step, y, z, step);

			if (std::abs(g2) > std::abs(g1) && std::abs(g2) > std::abs(g3))
			{
				edgeFlag[edgeId] = 1;

				if (g2 > 0)
				{
					cornerFlag[cornerId1] = 1;
					cornerFlag[cornerId2] = 0;
				}
				else
				{
					cornerFlag[cornerId2] = 1;
					cornerFlag[cornerId1] = 0;
				}


				gradientBasedInterpolateX(x, y, z, step, edgePoint);
			}
			else
			{
				edgeFlag[edgeId] = 0;
			}
		}

		void GradientBasedSurfaceExtractor::processEdgeY(int x, int y, int z, int step, int cornerId1,
			int cornerId2, int edgeId, std::vector<int>& cornerFlag,
			std::vector<int>& edgeFlag, Eigen::Vector3f& edgePoint)
		{
			//x , y , z  ---  x + 1 , y , z
			float g1 = maxGradientY(x , y - step, z, step), g2 = maxGradientY(x, y, z, step), g3 = maxGradientX(x, y + step, z, step);

			if (std::abs(g2) > std::abs(g1) && std::abs(g2) > std::abs(g3))
			{
				edgeFlag[edgeId] = 1;

				if (g2 > 0)
				{
					cornerFlag[cornerId1] = 1;
					cornerFlag[cornerId2] = 0;
				}
				else
				{
					cornerFlag[cornerId2] = 1;
					cornerFlag[cornerId1] = 0;
				}


				gradientBasedInterpolateY(x, y, z, step, edgePoint);
			}
			else
			{
				edgeFlag[edgeId] = 0;
			}
		}

		void GradientBasedSurfaceExtractor::processEdgeZ(int x, int y, int z, int step, int cornerId1,
			int cornerId2, int edgeId, std::vector<int>& cornerFlag,
			std::vector<int>& edgeFlag, Eigen::Vector3f& edgePoint)
		{
			//x , y , z  ---  x + 1 , y , z
			float g1 = maxGradientX(x , y, z - step, step), g2 = maxGradientX(x, y, z, step), g3 = maxGradientX(x , y, z + step, step);

			if (std::abs(g2) > std::abs(g1) && std::abs(g2) > std::abs(g3))
			{
				edgeFlag[edgeId] = 1;

				if (g2 > 0)
				{
					cornerFlag[cornerId1] = 1;
					cornerFlag[cornerId2] = 0;
				}
				else
				{
					cornerFlag[cornerId2] = 1;
					cornerFlag[cornerId1] = 0;
				}


				gradientBasedInterpolateZ(x, y, z, step, edgePoint);
			}
			else
			{
				edgeFlag[edgeId] = 0;
			}
		}



		bool GradientBasedSurfaceExtractor::sampleCornersBasedOnGradient( int x, int y, int z, std::vector<bool>& grayValues, std::vector<Eigen::Vector3f>& edgePoints, int step )
		{
			bool isOutlier = true;

			std::vector<unsigned short> gVals(8);

			gVals[0] = grayValueAt(x, y, z);
			gVals[1] = grayValueAt((x + step), y, z);
			gVals[2] = grayValueAt((x + step), y, (z + step));
			gVals[3] = grayValueAt(x, y, (z + step));
			gVals[4] = grayValueAt(x, (y + step), z);
			gVals[5] = grayValueAt((x + step), (y + step), z);
			gVals[7] = grayValueAt((x + step), (y + step), (z + step));
			gVals[8] = grayValueAt(x, (y + step), (z + step));

			//grayValues.resize(8);

			int inlierCount = 0;

			int outside = 0, inside = 0 , middle = 0;

			for (int ii = 0; ii < 8; ii++)
			{
				//inlierCount += grayValues[ii] > _StartThreshold && grayValues[ii] < _EndThreshold;
			
				if ( gVals[ii] < _StartThreshold)
				{
					outside++;
				}

				if ( gVals[ii] >= _StartThreshold && gVals[ii] < _EndThreshold)
				{
					middle++;
				}

				if ( gVals[ii] > _EndThreshold )
				{
					inside++;
				}

			}

		 
			isOutlier = ( inside == 8 || outside == 8 ); //( middle == 0 ) && 

			if (isOutlier)
				return false;

			std::vector<int> edgeFlag(12, -1);

			std::vector<int> cornerFlag(8, -1);

			bool returnFlag = true;

			float gradientTh = 1000;

			if ( !isOutlier )
			{

				//we need to process each edge
	          
				//x , y , z  ---  x + 1 , y , z
				float g1 = maxGradientX(x - 1, y, z, step) , g2 = maxGradientX(x , y, z, step) , g3 = maxGradientX(x + 1, y, z, step);

				if ( std::abs(g2) > gradientTh && std::abs(g2) > std::abs(g1) && std::abs(g2) > std::abs(g3))
				{
					edgeFlag[0] = 1;
					//000 --- 100
					if ( g2 > 0 )
					{
						cornerFlag[1] = 1;
						cornerFlag[0] = 0;
					}
					else
					{
						cornerFlag[0] = 1;
						cornerFlag[1] = 0;
					}


					gradientBasedInterpolateX(x, y, z, step, edgePoints[0]);
				}
				else
				{
					edgeFlag[0] = 0;
				}

				//  x + 1 , y , z ---  x + 1 , y , z + 1
				g1 = maxGradientZ(x + 1, y, z - 1, step), g2 = maxGradientZ(x + 1, y, z, step), g3 = maxGradientZ(x + 1, y, z + 1, step);

				if (std::abs(g2) > gradientTh &&  std::abs(g2) > std::abs(g1) && std::abs(g2) > std::abs(g3) )
				{
					edgeFlag[1] = 1;

					//100 --- 101
					if (g2 > 0)
					{
						cornerFlag[4 + 1] = 1;
						
						if (cornerFlag[1] == 1)
							returnFlag = false;

						cornerFlag[1] = 0;
					}
					else
					{
						cornerFlag[4 + 1] = 0;
						
						if (cornerFlag[1] == 0)
							returnFlag = false;
						
						cornerFlag[1] = 1;
					}

					gradientBasedInterpolateZ(x + 1, y, z, step, edgePoints[1]);
				}
				else
				{
					edgeFlag[1] = 0;
				}
				
                //  x + 1 , y , z + 1 --- x , y , z + 1
				g1 = maxGradientX(x - 1, y, z + 1, step), g2 = maxGradientX(x , y, z + 1 , step), g3 = maxGradientX( x + 1, y, z + 1, step);

				if (std::abs(g2) > gradientTh && std::abs(g2) > std::abs(g1) && std::abs(g2) > std::abs(g3))
				{
					edgeFlag[2] = 1;

					//101 -- 001
					if (g2 > 0)
					{
						if (cornerFlag[4 + 1] == 0)
							returnFlag = false;

						

						if (cornerFlag[4] == 1)
							returnFlag = false;

						cornerFlag[4 + 1] = 1;

						cornerFlag[4] = 0;
					}
					else
					{
						if ( cornerFlag[4 + 1] == 1 )
							returnFlag = false;

						if (cornerFlag[4] == 0)
							returnFlag = false;

						cornerFlag[4 + 1] = 0;

						cornerFlag[4] = 1;
					}

					gradientBasedInterpolateX(x , y, z + 1, step, edgePoints[2]);
				}
				else
				{
					edgeFlag[2] = 0;
				}
				
				//  x, y, z + 1  --- x , y , z
				g1 = maxGradientZ(x , y, z + 1, step), g2 = maxGradientZ(x , y, z, step), g3 = maxGradientZ(x , y, z - 1, step);

				if (std::abs(g2) > gradientTh && std::abs(g2) > std::abs(g1) && std::abs(g2) > std::abs(g3))
				{
					edgeFlag[3] = 1;

					// 001 --- 000
					if (g2 > 0)
					{
						if (cornerFlag[4] == 0)
							returnFlag = false;

						if (cornerFlag[0] == 1)
							returnFlag = false;

						cornerFlag[4] = 1;

						cornerFlag[0] = 0;
					}
					else
					{
						if (cornerFlag[4] == 1)
							returnFlag = false;

						if (cornerFlag[0] == 0)
							returnFlag = false;

						cornerFlag[4] = 0;

						cornerFlag[0] = 1;
					}

					gradientBasedInterpolateZ(x, y, z  , step, edgePoints[3]);
				}
				else
				{
					edgeFlag[3] = 0;
				}


                //  x, y + 1 , z -- x + 1 , y + 1 , z
				g1 = maxGradientX(x - 1, y + 1, z , step), g2 = maxGradientX(x, y + 1, z, step), g3 = maxGradientX(x + 1, y + 1, z, step);

				if (std::abs(g2) > gradientTh && std::abs(g2) > std::abs(g1) && std::abs(g2) > std::abs(g3))
				{
					edgeFlag[4] = 1;

					//010 --- 110
					if (g2 > 0)
					{
						if (cornerFlag[3] == 0)
							returnFlag = false;

						if (cornerFlag[2] == 1)
							returnFlag = false;

						cornerFlag[3] = 1;

						cornerFlag[2] = 0;
					}
					else
					{
						if (cornerFlag[3] == 1)
							returnFlag = false;

						if (cornerFlag[2] == 0)
							returnFlag = false;

						cornerFlag[3] = 0;

						cornerFlag[2] = 1;
					}

					gradientBasedInterpolateX(x, y + 1, z, step, edgePoints[4]);
				}
				else
				{
					edgeFlag[4] = 0;
				}

                //  x + 1 , y + 1 , z --- x + 1 , y + 1 , z + 1
				g1 = maxGradientZ(x + 1, y + 1, z - 1, step), g2 = maxGradientZ(x + 1, y + 1, z, step), g3 = maxGradientZ(x + 1, y + 1, z + 1, step);

				if (std::abs(g2) > gradientTh && std::abs(g2) > std::abs(g1) && std::abs(g2) > std::abs(g3))
				{
					edgeFlag[5] = 1;

					//110 -- 111
					if (g2 > 0)
					{
						if (cornerFlag[7] == 0)
							returnFlag = false;


						if (cornerFlag[3] == 1)
							returnFlag = false;


						cornerFlag[7] = 1;

						cornerFlag[3] = 0;
					}
					else
					{
						if (cornerFlag[7] == 1)
							returnFlag = false;


						if (cornerFlag[3] == 0)
							returnFlag = false;


						cornerFlag[7] = 0;

						cornerFlag[3] = 1;
					}

					gradientBasedInterpolateZ(x + 1, y + 1, z, step, edgePoints[5]);
				}
				else
				{
					edgeFlag[5] = 0;
				}


                //  x + 1 , y + 1 , z + 1 --- x , y + 1 , z + 1
				g1 = maxGradientX(x + 1, y + 1, z + 1, step), g2 = maxGradientX(x, y + 1, z + 1, step), g3 = maxGradientX(x - 1, y + 1, z + 1, step);

				if (std::abs(g2) > gradientTh && std::abs(g2) > std::abs(g1) && std::abs(g2) > std::abs(g3) )
				{
					edgeFlag[6] = 1;

					//111 --- 011
					if (g2 > 0)
					{
						if (cornerFlag[7] == 0)
							returnFlag = false;


						if (cornerFlag[6] == 1)
							returnFlag = false;


						cornerFlag[7] = 1;

						cornerFlag[6] = 0;
					}
					else
					{
						if (cornerFlag[7] == 1)
							returnFlag = false;


						if (cornerFlag[6] == 0)
							returnFlag = false;


						cornerFlag[7] = 0;

						cornerFlag[6] = 1;
					}

					gradientBasedInterpolateX(x , y + 1, z + 1, step, edgePoints[6]);
				}
				else
				{
					edgeFlag[6] = 0;
				}

                //  x , y + 1, z + 1  --- x , y + 1 , z
				g1 = maxGradientZ(x, y + 1, z + 1, step), g2 = maxGradientZ(x, y + 1, z , step), g3 = maxGradientZ(x, y + 1, z - 1, step);

				if (std::abs(g2) > gradientTh && std::abs(g2) > std::abs(g1) && std::abs(g2) > std::abs(g3))
				{
					edgeFlag[7] = 1;

					//011 --- 010
					if (g2 > 0)
					{
						if (cornerFlag[6] == 0)
							returnFlag = false;


						if (cornerFlag[2] == 1)
							returnFlag = false;


						cornerFlag[6] = 1;

						cornerFlag[2] = 0;
					}
					else
					{
						if (cornerFlag[6] == 1)
							returnFlag = false;


						if (cornerFlag[2] == 0)
							returnFlag = false;


						cornerFlag[6] = 0;

						cornerFlag[2] = 1;
					}

					gradientBasedInterpolateZ(x, y + 1, z , step, edgePoints[7]);
				}
				else
				{
					edgeFlag[7] = 0;
				}


                //  x , y , z --- x , y + 1 , z
				g1 = maxGradientY( x, y - 1 , z, step), g2 = maxGradientY(x, y , z, step), g3 = maxGradientY( x , y + 1, z , step);

				if (std::abs(g2) > gradientTh && std::abs(g2) > std::abs(g1) && std::abs(g2) > std::abs(g3))
				{
					edgeFlag[8] = 1;

					//000 --- 010
					if (g2 > 0)
					{
						if (cornerFlag[2] == 0)
							returnFlag = false;


						if (cornerFlag[0] == 1)
							returnFlag = false;


						cornerFlag[2] = 1;

						cornerFlag[0] = 0;
					}
					else
					{
						if (cornerFlag[2] == 1)
							returnFlag = false;


						if (cornerFlag[0] == 0)
							returnFlag = false;


						cornerFlag[2] = 0;

						cornerFlag[0] = 1;
					}


					gradientBasedInterpolateY(x, y , z, step, edgePoints[8]);
				}
				else
				{
					edgeFlag[8] = 0;
				}


                //  x + 1 , y , z --- x + 1 , y + 1 , z
				g1 = maxGradientY(x + 1, y - 1, z, step), g2 = maxGradientY(x + 1, y, z, step), g3 = maxGradientY(x + 1, y + 1, z , step);

				if (std::abs(g2) > gradientTh && std::abs(g2) > std::abs(g1) && std::abs(g2) > std::abs(g3))
				{
					edgeFlag[9] = 1;

					//100 --- 110
					if (g2 > 0)
					{
						if (cornerFlag[3] == 0)
							returnFlag = false;

						if (cornerFlag[1] == 1)
							returnFlag = false;


						cornerFlag[3] = 1;

						cornerFlag[1] = 0;
					}
					else
					{
						if (cornerFlag[3] == 1)
							returnFlag = false;


						if (cornerFlag[1] == 0)
							returnFlag = false;


						cornerFlag[3] = 0;

						cornerFlag[1] = 1;
					}

					gradientBasedInterpolateY(x + 1, y, z, step, edgePoints[9]);
				}
				else
				{
					edgeFlag[9] = 0;
				}


                //  x + 1 , y , z + 1 --- x + 1 , y + 1 , z + 1
				g1 = maxGradientY(x + 1, y - 1, z + 1, step), g2 = maxGradientY(x + 1, y, z + 1, step), g3 = maxGradientY(x + 1, y + 1, z + 1, step);

				if (std::abs(g2) > gradientTh && std::abs(g2) > std::abs(g1) && std::abs(g2) > std::abs(g3))
				{
					edgeFlag[10] = 1;

					//101 --- 111
					if (g2 > 0)
					{
						if (cornerFlag[7] == 0)
							returnFlag = false;


						if (cornerFlag[5] == 1)
							returnFlag = false;


						cornerFlag[7] = 1;

						cornerFlag[5] = 0;
					}
					else
					{
						if (cornerFlag[7] == 1)
							returnFlag = false;


						if (cornerFlag[5] == 0)
							returnFlag = false;


						cornerFlag[7] = 0;

						cornerFlag[5] = 1;
					}

					gradientBasedInterpolateY(x + 1, y, z + 1, step, edgePoints[10]);
				}
				else
				{
					edgeFlag[10] = 0;
				}

                //  x , y , z + 1 --- x , y + 1 , z + 1	
				g1 = maxGradientY(x , y - 1, z + 1, step), g2 = maxGradientY(x , y, z + 1, step), g3 = maxGradientY(x , y + 1, z + 1, step);

				if (std::abs(g2) > gradientTh && std::abs(g2) > std::abs(g1) && std::abs(g2) > std::abs(g3))
				{
					edgeFlag[11] = 1;

					//001 --- 011
					if (g2 > 0)
					{
						if (cornerFlag[6] == 0)
							returnFlag = false;


						if (cornerFlag[4] == 1)
							returnFlag = false;


						cornerFlag[6] = 1;

						cornerFlag[4] = 0;
					}
					else
					{
						if (cornerFlag[6] == 1)
							returnFlag = false;


						if (cornerFlag[4] == 0)
							returnFlag = false;


						cornerFlag[6] = 0;

						cornerFlag[4] = 1;
					}

					gradientBasedInterpolateY(x , y, z + 1, step, edgePoints[11]);
				}
				else
				{
					edgeFlag[11] = 0;
				}

			}


			for ( int ii = 0; ii < 8; ii++ )
			{
				if (cornerFlag[ii] < 0)
					return false;
			
				grayValues[ii] = cornerFlag[ii];
			}
            

			if (!updateCornerFlags(edgeFlag, cornerFlag))
				return false;



			grayValues[0] = cornerFlag[0]; //grayValueAt(x, y, z); //grayValueAtCorner(x, y, z);
			grayValues[1] = cornerFlag[1]; //grayValueAt((x + step), y, z); //grayValueAtCorner(x + step, y, z);
			grayValues[2] = cornerFlag[5]; //grayValueAt((x + step), y, (z + step));// grayValueAtCorner(x + step, y, z + step);
			grayValues[3] = cornerFlag[4]; //grayValueAt(x, y, (z + step));// grayValueAtCorner(x, y, z + step);
			grayValues[4] = cornerFlag[2]; //grayValueAt(x, (y + step), z); //grayValueAtCorner(x, y + step, z);
			grayValues[5] = cornerFlag[3]; //grayValueAt((x + step), (y + step), z); //grayValueAtCorner(x + step, y + step, z);
			grayValues[6] = cornerFlag[7]; //grayValueAt((x + step), (y + step), (z + step)); //grayValueAtCorner(x + step, y + step, z + step);
			grayValues[7] = cornerFlag[6]; //grayValueAt(x, (y + step), (z + step));// grayValueAtCorner(x, y + step, z + step);


			return true;// returnFlag; //true;// isOutlier;
		}


		bool GradientBasedSurfaceExtractor::updateCornerFlags( std::vector<int>& edgeFlags, std::vector<int>& cornerFlag)
		{
			if (!edgeFlags[0])
			{
				if (cornerFlag[0] < 0)
					cornerFlag[0] = cornerFlag[1];

				if (cornerFlag[1] < 0)
					cornerFlag[1] = cornerFlag[0];

				if (cornerFlag[0] != cornerFlag[1])
					return false;
			}
		
			if (!edgeFlags[1])
			{
				if (cornerFlag[4 + 1] < 0)
					cornerFlag[4 + 1] = cornerFlag[1];

				if (cornerFlag[1] < 0)
					cornerFlag[1] = cornerFlag[4 + 1];
			
				if (cornerFlag[1] != cornerFlag[5])
					return false;
			}

			
			if (!edgeFlags[2])
			{
				if (cornerFlag[4 + 1] < 0)
					cornerFlag[4 + 1] = cornerFlag[4];

				if (cornerFlag[4] < 0)
					cornerFlag[4] = cornerFlag[4 + 1];
			
				if (cornerFlag[4] != cornerFlag[5])
					return false;
			}

			if (!edgeFlags[3])
			{
				if (cornerFlag[4] < 0)
					cornerFlag[4] = cornerFlag[0];

				if (cornerFlag[0] < 0)
					cornerFlag[0] = cornerFlag[4];
			
				if (cornerFlag[4] != cornerFlag[0])
					return false;
			}

			
			if (!edgeFlags[4])
			{
				if (cornerFlag[3] < 0)
					cornerFlag[3] = cornerFlag[2];

				if (cornerFlag[2] < 0)
					cornerFlag[2] = cornerFlag[3];
			
				if (cornerFlag[2] != cornerFlag[3])
					return false;
			
			}

			
			if (!edgeFlags[5])
			{
				if (cornerFlag[7] < 0)
					cornerFlag[7] = cornerFlag[3];

				if (cornerFlag[3] < 0)
					cornerFlag[3] = cornerFlag[7];
			

				if (cornerFlag[3] != cornerFlag[7])
					return false;
			}
			
			if (!edgeFlags[6])
			{
				if (cornerFlag[7] < 0)
					cornerFlag[7] = cornerFlag[6];

				if (cornerFlag[6] < 0)
					cornerFlag[6] = cornerFlag[7];


				if (cornerFlag[6] != cornerFlag[7])
					return false;
			}
			

			if (!edgeFlags[7])
			{
				if (cornerFlag[6] < 0)
					cornerFlag[6] = cornerFlag[2];

				if (cornerFlag[2] < 0)
					cornerFlag[2] = cornerFlag[6];
			

				if (cornerFlag[2] != cornerFlag[6])
					return false;
			}

			
			if (!edgeFlags[8])
			{
				if (cornerFlag[2] < 0)
					cornerFlag[2] = cornerFlag[0];

				if (cornerFlag[0] < 0)
					cornerFlag[0] = cornerFlag[2];

				if (cornerFlag[0] != cornerFlag[2])
					return false;

			}
			
			if (!edgeFlags[9])
			{
				if (cornerFlag[3] < 0)
					cornerFlag[3] = cornerFlag[1];

				if (cornerFlag[1] < 0)
					cornerFlag[1] = cornerFlag[3];

				if (cornerFlag[1] != cornerFlag[3])
					return false;
			}

			
			if (!edgeFlags[10])
			{
				if (cornerFlag[7] < 0)
					cornerFlag[7] = cornerFlag[5];

				if (cornerFlag[5] < 0)
					cornerFlag[5] = cornerFlag[7];

				if (cornerFlag[5] != cornerFlag[7])
					return false;
			}
			
			if (!edgeFlags[11])
			{
				if (cornerFlag[6] < 0)
					cornerFlag[6] = cornerFlag[4];

				if (cornerFlag[4] < 0)
					cornerFlag[4] = cornerFlag[6];


				if (cornerFlag[4] != cornerFlag[6])
					return false;
			}


			return true;
		}

		bool GradientBasedSurfaceExtractor::checkGradientMaximaOnEdgeX( int x, int y, int z, int step )
		{
			bool maximaFound = false;



			return maximaFound;
		}


		float GradientBasedSurfaceExtractor::maxGradientX(int x, int y, int z, int step)
		{
			float gPrev = grayValueAt(x , y, z);
			float gCurrent = grayValueAt(x + 1 , y, z);
			float gNext = grayValueAt(x + 2, y, z);


			float maxGradient = (gCurrent - gPrev);

			for ( int ss = 0; ss < step; ss++ )
			{
				float gradient = ( gCurrent - gPrev );

				if ( std::abs(maxGradient) < std::abs(gradient))
					maxGradient = gradient;
			
				gPrev = gCurrent;
				gCurrent = gNext;
				gNext = grayValueAt(x + 2 + ss , y , z);
			}

			return maxGradient;
		}

		float GradientBasedSurfaceExtractor::maxGradientY(int x, int y, int z, int step)
		{
			float gPrev = grayValueAt(x , y , z);
			float gCurrent = grayValueAt(x, y + 1, z);
			float gNext = grayValueAt(x , y + 2, z);

			float maxGradient = (gCurrent - gPrev);

			for (int ss = 0; ss < step; ss++)
			{
				float gradient = (gCurrent - gPrev);

				if (std::abs(maxGradient) < std::abs(gradient))
					maxGradient = gradient;

				gPrev = gCurrent;
				gCurrent = gNext;
				gNext = grayValueAt(x, y + 2 + ss, z);
			
			}

			return maxGradient;
		}

		float GradientBasedSurfaceExtractor::maxGradientZ(int x, int y, int z, int step)
		{
			float gPrev = grayValueAt(x, y , z );
			float gCurrent = grayValueAt(x, y, z + 1);
			float gNext = grayValueAt(x, y , z + 2);

			float maxGradient = (gCurrent - gPrev);

			for (int ss = 0; ss < step; ss++)
			{
				float gradient = (gCurrent - gPrev);

				if (std::abs(maxGradient) < std::abs(gradient))
					maxGradient = gradient;

				gPrev = gCurrent;
				gCurrent = gNext;
				gNext = grayValueAt(x, y , z + 2 + ss);

			}

			return maxGradient;
		}




		void GradientBasedSurfaceExtractor::gradientBasedInterpolateX( int x , int y , int z , int step , Eigen::Vector3f& interpolatedPoint )
		{
			interpolatedPoint.x() = x + step * 0.5;
			interpolatedPoint.y() = y;
			interpolatedPoint.z() = z;
		}

		void GradientBasedSurfaceExtractor::gradientBasedInterpolateY(int x, int y, int z, int step, Eigen::Vector3f& interpolatedPoint)
		{
			interpolatedPoint.x() = x;
			interpolatedPoint.y() = y + 0.5 * step;
			interpolatedPoint.z() = z;
		}

		void GradientBasedSurfaceExtractor::gradientBasedInterpolateZ(int x, int y, int z, int step, Eigen::Vector3f& interpolatedPoint)
		{
			interpolatedPoint.x() = x;
			interpolatedPoint.y() = y;
			interpolatedPoint.z() = z + step * 0.5;
		}



	}



}