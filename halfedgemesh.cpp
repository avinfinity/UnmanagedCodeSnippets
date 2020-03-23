#include "halfedgemesh.h"
#include <iostream>


namespace imt {

	namespace volume {


		HalfEdgeMesh::HalfEdgeMesh( std::vector<unsigned int>& surfaceIndices , std::vector<double>& vertices , 
			                        std::vector<double>& vertexNormals  ) : mSurfaceIndices( surfaceIndices )  , 

			mVertices(vertices), mVertexNormals(vertexNormals)
		{
			mNumVertices = vertices.size() / 3;
		
		}

#define subtract3(v1 , v2 , v) { v[0] = v2[0] - v1[0];\
		v[1] = v2[1] - v1[1];\
			v[2] = v2[2] - v1[2];}

#define norm3(v) sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2])


#define cross3(v1, v2 , v) { v[0] = v1[1] * v2[2] - v1[2] * v2[1];\
                             v[1] = v1[2] * v2[0] - v1[0] * v2[2];\
                             v[2] = v1[0] * v2[1] - v1[1] * v2[0];} 


#define dot3(v1 , v2) v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]


		void HalfEdgeMesh::fixFaceOrientations()
		{
			int32_t numFaces = mSurfaceIndices.size() / 3;

			double *vData = mVertices.data();
			double *nData = mVertexNormals.data();

			for (int32_t ff = 0; ff < numFaces; ff++)
			{
				int32_t vId1 = mSurfaceIndices[3 * ff];
				int32_t vId2 = mSurfaceIndices[3 * ff + 1];
				int32_t vId3 = mSurfaceIndices[3 * ff + 2];

				double vec1[3], vec2[3];

				subtract3((vData + vId2 * 3), (vData + vId1 * 3), vec1);
				subtract3((vData + vId3 * 3), (vData + vId2 * 3), vec2);

				//double v1Norm = norm3(vec1);
				//double v2Norm = norm3(vec2);

				double triangleNormal[3];

				cross3(vec1, vec2, triangleNormal);

				double d1 = dot3(triangleNormal, (nData + vId1 * 3));
				//double d2 = dot3(triangleNormal, (nData + vId2 * 3));
				//double d3 = dot3(triangleNormal, (nData + vId3 * 3));
				
				if (d1 < 0)
				{
					//reverse the triangle orientation
					std::swap(mSurfaceIndices[3 * ff], mSurfaceIndices[3 * ff + 2]);
				}
			}
		}



		HalfEdgeMesh::HalfEdge::HalfEdge()
		{
			mSource = -1;
			mTarget = -1;
			mOpposite = -1;
			mNext = -1;
		}


		void HalfEdgeMesh::initialize()
		{
			int32_t numTriangles = mSurfaceIndices.size() / 3;

			int32_t numHalfEdges = numTriangles * 3;

			int32_t* vertexEdgeIncidenceSource = new int[mNumVertices * 16];

			int32_t *vertexEdgeIncideneCountsSource = new int32_t[mNumVertices];

			int32_t* vertexEdgeIncidenceTarget = new int[mNumVertices * 16];

			int32_t *vertexEdgeIncideneCountsTarget = new int32_t[mNumVertices];

			for (int ii = 0; ii < mNumVertices ; ii++)
			{
				vertexEdgeIncideneCountsSource[ii] = 0;
				vertexEdgeIncideneCountsTarget[ii] = 0;
			}

			mHalfEdges.resize(numTriangles * 3);

			std::vector<int32_t> incidence;

			//fixFaceOrientations();

			//first generate all the half edges from the triangles
			for (int32_t tt = 0; tt < numTriangles; tt++)
			{
				int32_t vId1 = mSurfaceIndices[3 * tt];
				int32_t vId2 = mSurfaceIndices[3 * tt + 1];
				int32_t vId3 = mSurfaceIndices[3 * tt + 2];

				mHalfEdges[3 * tt].mSource = vId3;
				mHalfEdges[3 * tt].mTarget = vId1;
				
				mHalfEdges[3 * tt + 1].mSource = vId1;
				mHalfEdges[3 * tt + 1].mTarget = vId2;

				mHalfEdges[3 * tt + 2].mSource = vId2;
				mHalfEdges[3 * tt + 2].mTarget = vId3;

				mHalfEdges[3 * tt].mNext = 3 * tt + 1;
				mHalfEdges[3 * tt + 1].mNext = 3 * tt + 2;
				mHalfEdges[3 * tt + 2 ].mNext = 3 * tt;		

				vertexEdgeIncidenceSource[ vId1 * 16 + vertexEdgeIncideneCountsSource[vId1]++ ] = 3 * tt + 1;
				vertexEdgeIncidenceSource[ vId2 * 16 + vertexEdgeIncideneCountsSource[vId2]++ ] = 3 * tt + 2;
				vertexEdgeIncidenceSource[ vId3 * 16 + vertexEdgeIncideneCountsSource[vId3]++ ] = 3 * tt ;

				vertexEdgeIncidenceTarget[vId1 * 16 + vertexEdgeIncideneCountsTarget[vId1]++] = 3 * tt;
				vertexEdgeIncidenceTarget[vId2 * 16 + vertexEdgeIncideneCountsTarget[vId2]++] = 3 * tt + 1;
				vertexEdgeIncidenceTarget[vId3 * 16 + vertexEdgeIncideneCountsTarget[vId3]++] = 3 * tt + 2;
			}



			//now build the opposites
			for (int32_t hh = 0; hh < numHalfEdges; hh++)
			{
				auto& edge = mHalfEdges[hh];

				int32_t targetVertex = edge.mTarget;
				int32_t sourceVertex = edge.mSource;

				//int32_t *targetIncidence = vertexEdgeIncidenceTarget + targetVertex * 16;
				int32_t *sourceTargetIncidences = vertexEdgeIncidenceTarget + sourceVertex * 16;

				int32_t sourceTargetIncidenceCount = vertexEdgeIncideneCountsTarget[sourceVertex];

				if (sourceTargetIncidenceCount > 16)
				{
					std::cout << "number of incidence exceeds 16 " << std::endl;
				}


				for ( int ii = 0; ii < sourceTargetIncidenceCount; ii++ )
				{
					int32_t edgeId = sourceTargetIncidences[ii];

					auto& currentEdge = mHalfEdges[edgeId];

					if (currentEdge.mSource == edge.mTarget)
					{
						if (currentEdge.mOpposite >= 0)
						{
							if (currentEdge.mOpposite != hh)
							{
								std::cout << "found inconsistent edge " << std::endl;
							}
						}

						//found opposite half edge
						edge.mOpposite = edgeId;

						currentEdge.mOpposite = hh;
					}
				}


			}

			//now find all the holes
			std::vector<unsigned char> usedEdgeMask(numHalfEdges, 0);

			int numFoundHoles = 0;

			for ( int hh = 0; hh < numHalfEdges; hh++ )
			{
				if (usedEdgeMask[hh])
					continue;

				auto& edge = mHalfEdges[hh];
				
				if ( edge.mOpposite == -1 ) //detected an edge of a hole
				{
					numFoundHoles++;

					std::vector<int> holeVertexIds;

					auto currentEdge = edge;

					holeVertexIds.push_back(currentEdge.mTarget);

					usedEdgeMask[hh] = numFoundHoles;

					bool loopCompleted = false;

					while (1)
					{
						int* sourceIncidences = vertexEdgeIncidenceSource + currentEdge.mTarget * 16;
						int sourceIncidenceCount = vertexEdgeIncideneCountsSource[currentEdge.mTarget];

						for ( int ii = 0; ii < sourceIncidenceCount; ii++ )
						{
							int nextEdgeId = sourceIncidences[ ii ];

							if (nextEdgeId == hh)
							{
								loopCompleted = true;
								break;
							}
								

							auto& nextEdge = mHalfEdges[nextEdgeId];

							if (nextEdge.mOpposite == -1)
							{
								currentEdge = nextEdge;

								holeVertexIds.push_back(currentEdge.mTarget);

								usedEdgeMask[nextEdgeId] = 1;
							}
						}

						if (loopCompleted)
							break;

					}
				}
			}


			std::cout << " number of found holes : " << numFoundHoles << std::endl;

		
		}


	}



}