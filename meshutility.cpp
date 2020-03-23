#include "meshutility.h"
#include <vtkIdList.h>

namespace imt{

	namespace volume{


		void MeshUtility::computeOppositeEnds(std::vector< Eigen::Vector3f >& edgeEnds, std::vector< Eigen::Vector3f >& edgeNormals,
			std::vector< Eigen::Vector3f >& oppositeEnds, std::vector< Eigen::Vector3f >& oppositeEndNormals,
			std::vector< bool >& oppositeEndFound, std::vector< float >& closestSurfaceDistances, RTCDevice& device, RTCScene& scene, float vstep)
		{

#if 0

				std::vector< Eigen::Vector3f > colors(edgeEnds.size(), Eigen::Vector3f(1, 0, 0));

				oppositeEndFound.resize(edgeEnds.size(), false);

				std::fill(oppositeEndFound.begin(), oppositeEndFound.end(), false);

				//tr::Display3DRoutines::displayPointSet(edgeEnds , colors);

				float maxLength = FLT_MAX;

				double initT = cv::getTickCount();

				float near = 0;

				std::cout << " vstep : " << vstep << std::endl;

				int numVertices = edgeEnds.size();

				std::atomic<int > nValidIntersection;

				nValidIntersection = 0;

				std::fill(closestSurfaceDistances.begin(), closestSurfaceDistances.end(), 1000);

#pragma omp parallel for
				for (int vv = 0; vv < numVertices + 8; vv += 8)
				{
					RTCRay8 ray;

					for (int ii = 0; ii < 8; ii++)
					{
						int id;

						if ((vv + ii) >= numVertices)
						{
							id = numVertices - 1;
						}
						else
						{
							id = vv + ii;
						}

						ray.orgx[ii] = edgeEnds[id](0);
						ray.orgy[ii] = edgeEnds[id](1);
						ray.orgz[ii] = edgeEnds[id](2);
					}


					for (int ii = 0; ii < 8; ii++)
					{
						Eigen::Vector3f dir;

						int id;

						if ((vv + ii) >= numVertices)
						{
							id = numVertices - 1;
						}
						else
						{
							id = vv + ii;
						}

						//std::cout << edgeNormals[id].transpose() << std::endl;

						ray.dirx[ii] = edgeNormals[id](0);
						ray.diry[ii] = edgeNormals[id](1);
						ray.dirz[ii] = edgeNormals[id](2);

						ray.geomID[ii] = RTC_INVALID_GEOMETRY_ID;
						ray.instID[ii] = RTC_INVALID_GEOMETRY_ID;
						ray.primID[ii] = RTC_INVALID_GEOMETRY_ID;

						ray.tnear[ii] = near;
						ray.tfar[ii] = maxLength;

						ray.mask[ii] = 0xFFFFFFFF;
						ray.time[ii] = 0.f;
					}

					__aligned(32) int valid8[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

					rtcIntersect8(valid8, scene, ray);

					for (int ii = 0; ii < 8; ii++)
					{
						if ((vv + ii) >= numVertices)
						{
							continue;
						}

						if (ray.primID[ii] != RTC_INVALID_GEOMETRY_ID)
						{
							oppositeEnds[vv + ii](0) = ray.orgx[ii] + ray.tfar[ii] * ray.dirx[ii];
							oppositeEnds[vv + ii](1) = ray.orgy[ii] + ray.tfar[ii] * ray.diry[ii];
							oppositeEnds[vv + ii](2) = ray.orgz[ii] + ray.tfar[ii] * ray.dirz[ii];

							closestSurfaceDistances[vv + ii] = ray.tfar[ii];

							oppositeEndFound[vv + ii] = true;

							nValidIntersection++;
						}
					}

				}


				std::cout << " number of valid intersections : " << nValidIntersection << std::endl;


#pragma omp parallel for
				for (int vv = 0; vv < numVertices + 8; vv += 8)
				{
					RTCRay8 ray;

					for (int ii = 0; ii < 8; ii++)
					{
						int id;

						if ((vv + ii) >= numVertices)
						{
							id = numVertices - 1;
						}
						else
						{
							id = vv + ii;
						}

						ray.orgx[ii] = edgeEnds[id](0);
						ray.orgy[ii] = edgeEnds[id](1);
						ray.orgz[ii] = edgeEnds[id](2);
					}


					for (int ii = 0; ii < 8; ii++)
					{
						Eigen::Vector3f dir;

						int id;

						if ((vv + ii) >= numVertices)
						{
							id = numVertices - 1;
						}
						else
						{
							id = vv + ii;
						}

						//std::cout << edgeNormals[id].transpose() << std::endl;

						ray.dirx[ii] = -edgeNormals[id](0);
						ray.diry[ii] = -edgeNormals[id](1);
						ray.dirz[ii] = -edgeNormals[id](2);

						ray.geomID[ii] = RTC_INVALID_GEOMETRY_ID;
						ray.instID[ii] = RTC_INVALID_GEOMETRY_ID;
						ray.primID[ii] = RTC_INVALID_GEOMETRY_ID;

						ray.tnear[ii] = near;
						ray.tfar[ii] = maxLength;

						ray.mask[ii] = 0xFFFFFFFF;
						ray.time[ii] = 0.f;
					}

					__aligned(32) int valid8[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

					rtcIntersect8(valid8, scene, ray);

					for (int ii = 0; ii < 8; ii++)
					{
						if ((vv + ii) >= numVertices)
						{
							continue;
						}

						if (ray.primID[ii] != RTC_INVALID_GEOMETRY_ID)
						{
							oppositeEnds[vv + ii](0) = ray.orgx[ii] + ray.tfar[ii] * ray.dirx[ii];
							oppositeEnds[vv + ii](1) = ray.orgy[ii] + ray.tfar[ii] * ray.diry[ii];
							oppositeEnds[vv + ii](2) = ray.orgz[ii] + ray.tfar[ii] * ray.dirz[ii];

							closestSurfaceDistances[vv + ii] = std::min(closestSurfaceDistances[vv + ii], ray.tfar[ii]);

							oppositeEndFound[vv + ii] = true;

							nValidIntersection++;
						}
					}

				}
#endif

		}

		void MeshUtility::convertToVertexAndIndices(vtkSmartPointer< vtkPolyData > mesh, std::vector< Eigen::Vector3f >& vertices, std::vector< unsigned int >& indices)
		{
			vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();

			vertices.resize(mesh->GetNumberOfPoints());
			indices.resize(mesh->GetNumberOfCells() * 3);

			int nPoints = mesh->GetNumberOfPoints();



			for (int pp = 0; pp < nPoints; pp++)
			{

				double pt[3];

				mesh->GetPoint(pp, pt);

				vertices[pp](0) = pt[0];
				vertices[pp](1) = pt[1];
				vertices[pp](2) = pt[2];


			}


			int numTriangles = mesh->GetNumberOfCells();

			for (int cc = 0; cc < numTriangles; cc++)
			{
				mesh->GetCellPoints(cc, triangle);

				indices[3 * cc] = triangle->GetId(0);
				indices[3 * cc + 1] = triangle->GetId(1);
				indices[3 * cc + 2] = triangle->GetId(2);
			}



		}


		void MeshUtility::convertToVertexAndIndices(vtkSmartPointer< vtkPolyData > mesh, std::vector< Eigen::Vector3f >& vertices, std::vector< Eigen::Vector3f >& normals, std::vector< unsigned int >& indices)
		{
			vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();

			vertices.resize(mesh->GetNumberOfPoints());
			normals.resize(mesh->GetNumberOfPoints());
			indices.resize(mesh->GetNumberOfCells() * 3);

			int nPoints = mesh->GetNumberOfPoints();

			vtkSmartPointer< vtkDataArray > vtknormals = mesh->GetPointData()->GetNormals();

			for (int pp = 0; pp < nPoints; pp++)
			{

				double pt[3];

				mesh->GetPoint(pp, pt);

				vertices[pp](0) = pt[0];
				vertices[pp](1) = pt[1];
				vertices[pp](2) = pt[2];


				double n[3];

				vtknormals->GetTuple(pp, n);

				normals[pp](0) = n[0];
				normals[pp](1) = n[1];
				normals[pp](2) = n[2];

			}


			int numTriangles = mesh->GetNumberOfCells();

			for (int cc = 0; cc < numTriangles; cc++)
			{
				mesh->GetCellPoints(cc, triangle);

				indices[3 * cc] = triangle->GetId(0);
				indices[3 * cc + 1] = triangle->GetId(1);
				indices[3 * cc + 2] = triangle->GetId(2);
			}



		}






	}





}