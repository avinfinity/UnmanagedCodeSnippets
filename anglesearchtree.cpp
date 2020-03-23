#include "anglesearchtree.h"
#include "unordered_map"
#include "unordered_set"
#include "iostream"
#include "display3droutines.h"
#include "wallthicknessestimator.h"

namespace imt{
	
	namespace volume{



		AngleSearchTree::SearchTreeNode::SearchTreeNode()
		{
			_IsOctreeLeaf = true;
			_NumChildren = 0;
			_NodeId = 0;

			
		}


		
		AngleSearchTree::AngleSearchTree(const std::vector< Eigen::Vector3d >& points, const std::vector< unsigned int >& triangleIndices, Eigen::Vector3d& voxelSize) :
			mPoints(points), mTriangleIndices(triangleIndices), mVoxelSize( voxelSize )
		{

			_LeafDimThreshold = 10;

			_LeafLocalDimThreshold = 2;

			//first we need to build incidence information 
			buildIncidenceInfo();

			buildOctree();

			//unsigned int totalPoints = 0;
			//unsigned int totalLeaves = 0;

			//std::cout << " number of nodes after coarse octree " << mTreeNodes.size() << std::endl;

			//totalPoints = 0;
			//totalLeaves = 0;

			//for (auto node : mTreeNodes)
			//{
			//	if (node->_IsOctreeLeaf)
			//	{
			//		totalPoints += node->mPointIndices.size();
			//		totalLeaves++;
			//	}
			//}

			//std::cout << " points per leaf node : " << totalPoints / totalLeaves << " " << totalLeaves << " " << mPoints.size() << " " << totalPoints << std::endl;

			buildAngleSearchTree();

			buildConnectionAlongTheSurface();

			//std::cout << " number of nodes after angle search tree " << mTreeNodes.size() << std::endl;

			//for (auto node : mTreeNodes)
			//{
			//	if (node->_IsOctreeLeaf)
			//	{
			//		totalPoints += node->mPointIndices.size();
			//		totalLeaves++;
			//	}
			//}

			//std::cout << " points per leaf node : " << totalPoints / totalLeaves << " " << totalLeaves << " " << mPoints.size() << " " << totalPoints << std::endl;

			buildFineTree();

			//std::cout << " nodes after fine tree construction : " << mTreeNodes.size() << std::endl;

			//totalPoints = 0;
			//totalLeaves = 0;

			//for (auto node : mTreeNodes)
			//{
			//	if (node->_IsOctreeLeaf)
			//	{
			//		totalPoints += node->mPointIndices.size();
			//		totalLeaves++;
			//	}
			//}

			//std::cout << " points per leaf node : " << totalPoints / totalLeaves << " " << totalLeaves << " " << mPoints.size() << " " << totalPoints << std::endl;

		}


		bool AngleSearchTree::isNodeWithInRange( const Eigen::Vector3d& refPoint , double r2, const SearchTreeNode* node)
		{
			bool isInRange = false;

			double dmin = 0;
			
			for ( int i = 0; i < 3; i++) 
			{
				if (refPoint(i) < node->_BoundingBox._MinValues[i])
				{
					double d = refPoint(i) - node->_BoundingBox._MinValues[i];

					dmin += d * d;
				}					
				else if (refPoint(i) > node->_BoundingBox._MaxValues[i])
				{
					double d = refPoint(i) - node->_BoundingBox._MaxValues[i];

					dmin += d * d;
				}
					
			}


			return dmin <= r2;
		}



		unsigned char  AngleSearchTree::isNodeInsideCone( const Eigen::Vector3d& refPoint , const Eigen::Vector3d& coneAxis , const double& coneAngle, const int& nodeId )
		{
			bool insideCone = false;

			auto node = mTreeNodes[nodeId];

			double w = node->_BoundingBox._MaxValues[0] - node->_BoundingBox._MinValues[0];
			double h = node->_BoundingBox._MaxValues[1] - node->_BoundingBox._MinValues[1];
			double d = node->_BoundingBox._MaxValues[2] - node->_BoundingBox._MinValues[2];

			double th = cos(coneAngle);

			int inclusionCount = 0;

			for ( int xx = 0; xx < 2; xx++)
				for ( int yy = 0; yy < 2; yy++)
					for ( int zz = 0; zz < 2; zz++)
					{
						Eigen::Vector3d corner(0,0,0);

						corner(0) = node->_BoundingBox._MinValues[0] + w * xx;
						corner(1) = node->_BoundingBox._MinValues[1] + h * yy;
						corner(2) = node->_BoundingBox._MinValues[2] + d * zz;

						Eigen::Vector3d vec = corner - refPoint;

						vec.normalize();

						double dotP = coneAxis.dot(vec);
						
						if ( dotP > th )
						{
							inclusionCount++;
						}
					}

			if ( inclusionCount == 8 )
			{
				insideCone = 2;
			}
			else if ( inclusionCount > 0 )
			{
				insideCone = 1;
			}

			return insideCone;		
		}


		void AngleSearchTree::computeOppositeEnds( std::vector< Eigen::Vector3d >& inputPoints, std::vector< unsigned int >& oppositeEndIndices, 
			                                       std::vector< Eigen::Vector3d >& oppositeEnds )
		{

			//initialize a ray tracer
			
			
			//compute opposite ends 
			
			
			//destroy the ray tracer

		}


		unsigned int AngleSearchTree::buildTree(AngleSearchTree::SearchTreeNode *node)
		{
			unsigned int numPointsInNode = 0;

			double minDim = mVoxelSize(0) * _LeafDimThreshold;

			double currDim = node->_BoundingBox._MaxValues[0] - node->_BoundingBox._MinValues[0];

			double halfDim = currDim / 2;

			if (currDim > minDim * 1.1)
			{
				node->_NumChildren = 8;

				node->mChildren.resize(8);

				for (int cc = 0; cc < 8; cc++)
				{
					node->mChildren[cc] = new AngleSearchTree::SearchTreeNode;

					node->mChildren[cc]->_NodeId = mTreeNodes.size();

					mTreeNodes.push_back(node->mChildren[cc]);
				}

				//split the current node
				for (int ii = 0; ii < 2; ii++)
					for (int jj = 0; jj < 2; jj++)
						for (int kk = 0; kk < 2; kk++)
						{
							int id = ii * 4 + 2 * jj + kk;

							node->mChildren[id]->_BoundingBox._MinValues[0] = node->_BoundingBox._MinValues[0] + ii * halfDim;
							node->mChildren[id]->_BoundingBox._MaxValues[0] = node->mChildren[id]->_BoundingBox._MinValues[0] + halfDim;

							node->mChildren[id]->_BoundingBox._MinValues[1] = node->_BoundingBox._MinValues[1] + jj * halfDim;
							node->mChildren[id]->_BoundingBox._MaxValues[1] = node->mChildren[id]->_BoundingBox._MinValues[1] + halfDim;

							node->mChildren[id]->_BoundingBox._MinValues[2] = node->_BoundingBox._MinValues[2] + kk * halfDim;
							node->mChildren[id]->_BoundingBox._MaxValues[2] = node->mChildren[id]->_BoundingBox._MinValues[2] + halfDim;

							node->mChildren[id]->_NumChildren = 0;

							numPointsInNode += buildTree(node->mChildren[id]);
						}


				node->_IsOctreeLeaf = false;
			}


			return numPointsInNode;

		}


		void AngleSearchTree::insertPointToLocalTree( AngleSearchTree::SearchTreeNode *node, const unsigned int& pointId )
		{
			unsigned int foundNodeId = 0;

			double currDim = node->_BoundingBox._MaxValues[0] - node->_BoundingBox._MinValues[0];

			auto point = mPoints[pointId];

			double minDim = mVoxelSize(0) * _LeafLocalDimThreshold;

			double halfDim = currDim / 2;

			if ( currDim > minDim * 1.1 )
			{
				int xx = ( point.x() - node->_BoundingBox._MinValues[0] ) < currDim / 2 ? 0 : 1;
				int yy = ( point.y() - node->_BoundingBox._MinValues[1] ) < currDim / 2 ? 0 : 1;
				int zz = ( point.z() - node->_BoundingBox._MinValues[2] ) < currDim / 2 ? 0 : 1;

				int id = xx * 4 + yy * 2 + zz;

				if (node->mChildren.size() == 0)
				{
					node->mChildren.resize(8);

					for (int nn = 0; nn < 8; nn++)
					{
						node->mChildren[nn] = 0;
					}
				}

				if ( !node->mChildren[id] )
				{
					node->mChildren[id] = new AngleSearchTree::SearchTreeNode;

					node->mChildren[id]->_BoundingBox._MinValues[0] = node->_BoundingBox._MinValues[0] + xx * halfDim;
					node->mChildren[id]->_BoundingBox._MaxValues[0] = node->mChildren[id]->_BoundingBox._MinValues[0] + halfDim;

					node->mChildren[id]->_BoundingBox._MinValues[1] = node->_BoundingBox._MinValues[1] + yy * halfDim;
					node->mChildren[id]->_BoundingBox._MaxValues[1] = node->mChildren[id]->_BoundingBox._MinValues[1] + halfDim;

					node->mChildren[id]->_BoundingBox._MinValues[2] = node->_BoundingBox._MinValues[2] + zz * halfDim;
					node->mChildren[id]->_BoundingBox._MaxValues[2] = node->mChildren[id]->_BoundingBox._MinValues[2] + halfDim;

					node->mChildren[id]->_NumChildren = 0;

					node->mChildren[id]->_NodeId = mTreeNodes.size();

					mTreeNodes.push_back(node->mChildren[id]);
				}


				insertPointToLocalTree(node->mChildren[id], pointId);

				node->_IsOctreeLeaf = false;
			}
			else
			{
				node->mPointIndices.push_back(pointId);
			}

		}


		void AngleSearchTree::insertPointToNode( AngleSearchTree::SearchTreeNode *node, const unsigned int& pointId, double dimThreshold )
		{
			unsigned int foundNodeId = 0;

			double currDim = node->_BoundingBox._MaxValues[0] - node->_BoundingBox._MinValues[0];

			auto point = mPoints[pointId];

			double minDim = mVoxelSize(0) * dimThreshold;

			double halfDim = currDim / 2;

			if (currDim > minDim * 1.1)
			{
				int xx = (point.x() - node->_BoundingBox._MinValues[0]) < currDim / 2 ? 0 : 1;
				int yy = (point.y() - node->_BoundingBox._MinValues[1]) < currDim / 2 ? 0 : 1;
				int zz = (point.z() - node->_BoundingBox._MinValues[2]) < currDim / 2 ? 0 : 1;

				int id = xx * 4 + yy * 2 + zz;

				if (node->mChildren.size() == 0)
				{
					node->mChildren.resize(8);

					for (int nn = 0; nn < 8; nn++)
					{
						node->mChildren[nn] = 0;
					}
				}

				if (!node->mChildren[id])
				{
					node->mChildren[id] = new AngleSearchTree::SearchTreeNode;

					node->mChildren[id]->_BoundingBox._MinValues[0] = node->_BoundingBox._MinValues[0] + xx * halfDim;
					node->mChildren[id]->_BoundingBox._MaxValues[0] = node->mChildren[id]->_BoundingBox._MinValues[0] + halfDim;

					node->mChildren[id]->_BoundingBox._MinValues[1] = node->_BoundingBox._MinValues[1] + yy * halfDim;
					node->mChildren[id]->_BoundingBox._MaxValues[1] = node->mChildren[id]->_BoundingBox._MinValues[1] + halfDim;

					node->mChildren[id]->_BoundingBox._MinValues[2] = node->_BoundingBox._MinValues[2] + zz * halfDim;
					node->mChildren[id]->_BoundingBox._MaxValues[2] = node->mChildren[id]->_BoundingBox._MinValues[2] + halfDim;

					node->mChildren[id]->_NumChildren = 0;

					node->mChildren[id]->_NodeId = mTreeNodes.size();

					mTreeNodes.push_back(node->mChildren[id]);
				}

				insertPointToNode(node->mChildren[id], pointId, dimThreshold);

				node->_IsOctreeLeaf = false;
			}
			else
			{
				node->mPointIndices.push_back(pointId);

				_PointNodeIds[pointId] = node->_NodeId;
			}

		}


		unsigned int AngleSearchTree::findNodeId(const Eigen::Vector3d& point)
		{

			SearchTreeNode *rootNode = mTreeNodes[0];

			if (rootNode->_NumChildren == 0)
				return rootNode->_NodeId;

			double currDim = rootNode->_BoundingBox._MaxValues[0] - rootNode->_BoundingBox._MinValues[0];

		    //start with root node
			int xx = (point.x() - rootNode->_BoundingBox._MinValues[0]) < currDim / 2 ? 0 : 1;
			int yy = (point.y() - rootNode->_BoundingBox._MinValues[1]) < currDim / 2 ? 0 : 1;
			int zz = (point.z() - rootNode->_BoundingBox._MinValues[2]) < currDim / 2 ? 0 : 1;

			int id = xx * 4 + yy * 2 + zz;

			return findNodeId(point, rootNode->mChildren[id]);
		}


		unsigned int AngleSearchTree::findNodeId(const Eigen::Vector3d& point, AngleSearchTree::SearchTreeNode *node)
		{
			unsigned int foundNodeId = 0;


			if (node->_NumChildren == 0)
				return node->_NodeId;

			double currDim = node->_BoundingBox._MaxValues[0] - node->_BoundingBox._MinValues[0];

			//start with root node

			int xx = (point.x() - node->_BoundingBox._MinValues[0]) < currDim / 2 ? 0 : 1;
			int yy = (point.y() - node->_BoundingBox._MinValues[1]) < currDim / 2 ? 0 : 1;
			int zz = (point.z() - node->_BoundingBox._MinValues[2]) < currDim / 2 ? 0 : 1;

			int id = xx * 4 + yy * 2 + zz;

			return findNodeId(point, node->mChildren[id]);
		}


		bool AngleSearchTree::findConnectedComponents( AngleSearchTree::SearchTreeNode *node, std::vector< std::vector<unsigned int> >& connectedComponents )
		{
			bool multipleComponentsFound = false;

			unsigned int numNodePoints = node->mPointIndices.size();

			for (int pp = 0; pp < numNodePoints; pp++)
			{
				int pId = node->mPointIndices[pp];

				if (_IsPointUsed[pId])
					continue;

				std::vector< unsigned int > connectedComponent;

				buildConnectedComponent(pId , connectedComponent );

				connectedComponents.push_back(connectedComponent);
			}

			multipleComponentsFound = connectedComponents.size() > 1;
			
			return multipleComponentsFound;
		}

		void AngleSearchTree::buildConnectedComponent( unsigned int pointId, std::vector< unsigned int >& connectedComponent )
		{

			int nodeId = _PointNodeIds[pointId];

			std::vector< unsigned int > nextLayer, currentLayer;

			currentLayer.push_back(pointId);

			_IsPointUsed[pointId] = true;

			while (true)
			{
				for (auto candidateId : currentLayer)
				{
					//find all the neighbors
					auto& incidenceList = _VertexVertexIncidence[candidateId];

					connectedComponent.push_back(candidateId);

					for (auto neighborId : incidenceList)
					{
						if (_PointNodeIds[neighborId] == nodeId)
						{
							if (!_IsPointUsed[neighborId])
							{
								nextLayer.push_back(neighborId);

								_IsPointUsed[neighborId] = true;
							}
						}
					}
				}

				currentLayer = nextLayer;

				nextLayer.clear();

				if ( currentLayer.size() == 0 )
					break;
			}
		
		}


		void AngleSearchTree::buildConnectionAlongTheSurface()
		{
			int numNodes = mTreeNodes.size();

			_SurfaceConnectionNodeIds.resize( mPoints.size());

			for ( int nn = 0; nn < numNodes; nn++ )
			{
				auto node = mTreeNodes[nn];

				if ( !node->_IsOctreeLeaf )
					continue;

				for (int cc = 0; cc < node->_NumChildren; cc++)
				{
					auto childNode = node->mChildren[cc];

					std::unordered_map< unsigned int, unsigned char > collectedNeighbors;

					for ( unsigned int pointId : childNode->mPointIndices )
					{
						auto& incidentPoints = _VertexVertexIncidence[pointId];

						for ( auto neighborPointId : incidentPoints )
						{
							if ( _PointNodeIds[neighborPointId] != childNode->_NodeId )
							{
								collectedNeighbors[ _PointNodeIds[neighborPointId] ] = 1;
							}
						}
					}

					for (auto neighbor : collectedNeighbors)
					{
						childNode->mSurfaceConnectionNodes.push_back(mTreeNodes[neighbor.first]);
					}
				}
			}

			for (int nn = 0; nn < numNodes; nn++)
			{
				auto node = mTreeNodes[nn];

				if (!node->_IsOctreeLeaf)
					continue;

				for (auto index : node->mPointIndices)
				{
					_SurfaceConnectionNodeIds[index] = node->_NodeId;
				}
			}
		}



		void AngleSearchTree::buildIncidenceInfo()
		{
		    
			unsigned int nFaces = mTriangleIndices.size() / 3;

			unsigned int nVertices = mPoints.size();

			std::vector< std::unordered_set< unsigned int > > vertexIncidences(nVertices);

			_VertexTriangleIncidence.resize(nVertices);

			for (unsigned int tt = 0; tt < nFaces; tt++)
			{
				unsigned int vId1 = mTriangleIndices[3 * tt];
				unsigned int vId2 = mTriangleIndices[3 * tt + 1];
				unsigned int vId3 = mTriangleIndices[3 * tt + 2];

				vertexIncidences[vId1].insert(vId2);
				vertexIncidences[vId1].insert(vId3);

				vertexIncidences[vId2].insert(vId1);
				vertexIncidences[vId2].insert(vId3);

				vertexIncidences[vId3].insert(vId1);
				vertexIncidences[vId3].insert(vId2);

				_VertexTriangleIncidence[vId1].push_back(tt);
				_VertexTriangleIncidence[vId2].push_back(tt);
				_VertexTriangleIncidence[vId3].push_back(tt);

			}


			_VertexVertexIncidence.resize(nVertices);

			for (unsigned int vv = 0; vv < nVertices; vv++)
			{
				_VertexVertexIncidence[vv].insert(_VertexVertexIncidence[vv].end(), vertexIncidences[vv].begin(), vertexIncidences[vv].end());
			}

		}


		void AngleSearchTree::visualizeLeafNodes()
		{

			unsigned int numVertices = mPoints.size();

			int numNodes = mTreeNodes.size();

			std::vector< Eigen::Vector3f > points( numVertices ), colors( numVertices , Eigen::Vector3f( 1 , 1 , 1 ) );

			std::unordered_set< unsigned int > nonVacantNodes;

			for (int vv = 0; vv < numVertices; vv++)
			{
				points[vv] = mPoints[vv].cast<float>();

				nonVacantNodes.insert(_PointNodeIds[vv]);
			}




			//for (int vv = 0; vv < numVertices; vv++)
			//{
			//	points[vv] = mPoints[vv].cast<float>();

			//	if (_PointNodeIds[vv] == *( nonVacantNodes.begin() + 1 ))
			//	{
			//		colors[vv] = Eigen::Vector3f(1, 0, 0);
			//	}
			//}

			//tr::Display3DRoutines::displayPointSet( points , colors );

		}


		void AngleSearchTree::findClosestPoint( const Eigen::Vector3d& inputPoint, const unsigned int& oppEndFaceIndex,
			Eigen::Vector3d& closestPoint )
		{
			std::vector< SearchTreeNode* > candidateNodes;

			std::cout << " total number of nodes : " << mTreeNodes.size() << std::endl;

		    //first collect all the candidate nodes 
			collectCandidateNodes(oppEndFaceIndex, candidateNodes);

			DistanceNodeQ distQ;

			double initMinDistance = FLT_MAX;

			double minDistance = initMinDistance;

			//build the priority queue
			for (auto candidateNode : candidateNodes)
			{
				QElem elem;

				elem._Node = candidateNode;

				Eigen::Vector3d avgPoint;

				avgPoint(0) = candidateNode->_BoundingBox._MaxValues[0] - candidateNode->_BoundingBox._MinValues[0];
				avgPoint(1) = candidateNode->_BoundingBox._MaxValues[1] - candidateNode->_BoundingBox._MinValues[1];
				avgPoint(2) = candidateNode->_BoundingBox._MaxValues[2] - candidateNode->_BoundingBox._MinValues[2];

				elem._Dist = (inputPoint - avgPoint ).squaredNorm();

				distQ.push(elem);
			}

			unsigned int numComparisons = 0 , numLeavesChecked = 0;

			int minDistPointId = -1;

			while (!distQ.empty())
			{
				auto elem = distQ.top();

				distQ.pop();

				//first check if node is a leaf node
				if ( elem._Node->_IsOctreeLeaf )
				{
					for (auto pointId : elem._Node->mPointIndices)
					{
					    double dist = (mPoints[pointId] - inputPoint).squaredNorm();

						if ( dist < minDistance)
						{
							closestPoint = mPoints[pointId];

							minDistPointId = pointId;

							minDistance = dist;
						}

						numComparisons++;
					}

					numLeavesChecked++;

					//std::cout << " leaf node size : " << elem._Node->mPointIndices.size() << " " << elem._Node->_NodeId << std::endl;

				}
				else
				{
					//if it's a branch node we need to push its valid children to the queue
					int nChildren = elem._Node->mChildren.size();

					for ( int cc = 0; cc < nChildren; cc++ )
					{
						auto childNode = elem._Node->mChildren[cc];

						if (!childNode || !isNodeWithInRange(inputPoint, minDistance, childNode))
							continue;

						Eigen::Vector3d nodeCenter;

						nodeCenter(0) = (childNode->_BoundingBox._MaxValues[0] + childNode->_BoundingBox._MinValues[0]) * 0.5;
						nodeCenter(1) = (childNode->_BoundingBox._MaxValues[1] + childNode->_BoundingBox._MinValues[1]) * 0.5;
						nodeCenter(2) = (childNode->_BoundingBox._MaxValues[2] + childNode->_BoundingBox._MinValues[2]) * 0.5;

						double avgDist = (inputPoint - nodeCenter).squaredNorm();

						QElem elem;

						elem._Dist = avgDist;
						elem._Node = childNode;

						distQ.push(elem);
					}
				}

				

			}

			std::cout << " total number of comparisons : " << numComparisons << std::endl;
			std::cout << " number of leaves checked " << numLeavesChecked << std::endl;
			std::cout << " minimum distance : " << minDistance << std::endl;

			unsigned int nPoints = mPoints.size();

			minDistance = FLT_MAX;

			Eigen::Vector3d linearSearchPoint;

			for (unsigned int pp = 0; pp < nPoints; pp++)
			{
				double dist = (inputPoint - mPoints[pp]).squaredNorm();

				if (dist < minDistance)
				{
					linearSearchPoint = mPoints[pp];

					minDistance = dist;
				}
			}


			std::cout << " linear search min distance : " << minDistance << std::endl;
			std::cout << " linear search closest point : " << linearSearchPoint.transpose() << std::endl;
			 
		
		}


		void AngleSearchTree::findClosestPoint( const Eigen::Vector3d& inputPoint, const Eigen::Vector3d& coneAxis, const unsigned int& oppEndFaceIndex,
			                                    const double& searchAngle, Eigen::Vector3d& closestPoint, unsigned int& closestFaceIndex )
		{
			DistanceNodeQ distQ;

			Eigen::Vector3d initClosestPoint = geometricCenter(oppEndFaceIndex);

			std::cout << " init closest point : " << initClosestPoint.transpose() << std::endl;

			double initMinDistance = ( initClosestPoint - inputPoint ).squaredNorm();

			double minDistance = initMinDistance;

			std::unordered_map< unsigned int, unsigned char > candidateNodes;

			//first collect all the candidate nodes 
			collectCandidateNodes( oppEndFaceIndex , inputPoint , coneAxis , searchAngle , candidateNodes );

			//push the nodes in search priority queue
			for (auto candidateNodeId : candidateNodes)
			{
				QElem elem;

				auto candidateNode = mTreeNodes[candidateNodeId.first];

				Eigen::Vector3d avgPoint;

				avgPoint(0) = candidateNode->_BoundingBox._MaxValues[0] - candidateNode->_BoundingBox._MinValues[0];
				avgPoint(1) = candidateNode->_BoundingBox._MaxValues[1] - candidateNode->_BoundingBox._MinValues[1];
				avgPoint(2) = candidateNode->_BoundingBox._MaxValues[2] - candidateNode->_BoundingBox._MinValues[2];

				elem._Dist = (inputPoint - avgPoint).squaredNorm();

				elem._InclusionFlag = candidateNodeId.second;

				distQ.push(elem);
			}


			//now perform the angle search
			unsigned int numComparisons = 0, numLeavesChecked = 0;

			int minDistPointId = -1;

			while (!distQ.empty())
			{
				auto elem = distQ.top();

				distQ.pop();

				//first check if node is a leaf node
				if (elem._Node->_IsOctreeLeaf)
				{
					for (auto pointId : elem._Node->mPointIndices)
					{
						double dist = (mPoints[pointId] - inputPoint).squaredNorm();

						if (dist < minDistance)
						{
							closestPoint = mPoints[pointId];

							minDistPointId = pointId;

							minDistance = dist;
						}

						numComparisons++;
					}

					numLeavesChecked++;

				}
				else
				{
					//if it's a branch node we need to push its valid children to the queue
					int nChildren = elem._Node->mChildren.size();

					for (int cc = 0; cc < nChildren; cc++)
					{
						auto childNode = elem._Node->mChildren[cc];

						if (!childNode || !isNodeWithInRange(inputPoint, minDistance, childNode))
							continue;

						Eigen::Vector3d nodeCenter;

						nodeCenter(0) = (childNode->_BoundingBox._MaxValues[0] + childNode->_BoundingBox._MinValues[0]) * 0.5;
						nodeCenter(1) = (childNode->_BoundingBox._MaxValues[1] + childNode->_BoundingBox._MinValues[1]) * 0.5;
						nodeCenter(2) = (childNode->_BoundingBox._MaxValues[2] + childNode->_BoundingBox._MinValues[2]) * 0.5;

						double avgDist = (inputPoint - nodeCenter).squaredNorm();

						QElem elem;

						elem._Dist = avgDist;
						elem._Node = childNode;

						distQ.push(elem);
					}
				}
			}

		}



		void AngleSearchTree::collectCandidateNodes( const unsigned int& oppEndFaceIndex , std::vector< SearchTreeNode* >& candidateNodes )
		{
		   
			candidateNodes.push_back(mTreeNodes[0]);
		
		}


		void AngleSearchTree::collectCandidateNodes( const unsigned int& oppEndFaceIndex, const Eigen::Vector3d& coneOrigin, const Eigen::Vector3d& coneAxis,
			const double& searchAngle, std::unordered_map< unsigned int, unsigned char >& candidateNodes)
		{

			typedef std::unordered_map< unsigned int, unsigned char >::iterator NodeIter;

			unsigned int vId1 = mTriangleIndices[3 * oppEndFaceIndex];
			unsigned int vId2 = mTriangleIndices[3 * oppEndFaceIndex + 1];
			unsigned int vId3 = mTriangleIndices[3 * oppEndFaceIndex + 2];

			for (int ii = 0; ii < 3; ii++)
			{
				int vId = mTriangleIndices[3 * oppEndFaceIndex + ii];

				unsigned int surfaceNodeId = _SurfaceConnectionNodeIds[vId];

				auto nodeIt = candidateNodes.find(surfaceNodeId);

				if (nodeIt == candidateNodes.end())
				{
					unsigned char inclusionFlag = isNodeInsideCone(coneOrigin, coneAxis, searchAngle, surfaceNodeId);

					if (inclusionFlag)
					{
						candidateNodes[surfaceNodeId] = inclusionFlag;
					}
				}


			}
			candidateNodes[_SurfaceConnectionNodeIds[vId1]] = 1;
			candidateNodes[_SurfaceConnectionNodeIds[vId2]] = 1;
			candidateNodes[_SurfaceConnectionNodeIds[vId3]] = 1;
			
			std::vector< unsigned int > searchNodes;

			for (auto candidate : candidateNodes)
			{
				int nodeId = candidate.first;

				auto node = mTreeNodes[nodeId];

				int nConnectionNodes = node->mSurfaceConnectionNodes.size();

				for (int nn = 0; nn < nConnectionNodes; nn++)
				{
					unsigned int connectionNodeId = node->mSurfaceConnectionNodes[nn]->_NodeId;

					auto nodeIt = candidateNodes.find(connectionNodeId);

					if (nodeIt == candidateNodes.end())
					{
						searchNodes.push_back(connectionNodeId);
					}
				}
			}

			while (searchNodes.size() > 0)
			{
				std::vector< unsigned int > newSearchNodes;

				for (auto searchNodeId : searchNodes)
				{
					auto node = mTreeNodes[searchNodeId];

					int nConnectionNodes = node->mSurfaceConnectionNodes.size();

					for (int nn = 0; nn < nConnectionNodes; nn++)
					{
						unsigned int connectionNodeId = node->mSurfaceConnectionNodes[nn]->_NodeId;

						auto nodeIt = candidateNodes.find(connectionNodeId);

						if (nodeIt == candidateNodes.end())
						{
							unsigned char inclusionFlag = isNodeInsideCone(coneOrigin, coneAxis, searchAngle, connectionNodeId);

							//first check if node lies inside search cone
							if (inclusionFlag)
							{
								newSearchNodes.push_back(connectionNodeId);

								candidateNodes[connectionNodeId] = inclusionFlag;
							}
							
						}
					}
				}

				searchNodes = newSearchNodes;
			}

		}



		Eigen::Vector3d AngleSearchTree::geometricCenter(const unsigned int& faceIndex) const
		{
			std::cout << "size of triangle indices array : " << mTriangleIndices.size() << std::endl;

			Eigen::Vector3d geometricCenter( 0 , 0 , 0);

			unsigned int vId1 = mTriangleIndices[3 * faceIndex];
			unsigned int vId2 = mTriangleIndices[3 * faceIndex + 1];
			unsigned int vId3 = mTriangleIndices[3 * faceIndex + 2];

			geometricCenter = (mPoints[vId1] + mPoints[vId2] + mPoints[vId3]) / 3.0;

			return geometricCenter;
		}


		std::vector< AngleSearchTree::SearchTreeNode* > AngleSearchTree::getLeafNodes()
		{
			int numAllNodes = mTreeNodes.size();

			std::cout << " num all nodes : " << numAllNodes << std::endl;

			std::vector< AngleSearchTree::SearchTreeNode* > leafNodes;

			for ( int tt = 0; tt < numAllNodes; tt++ )
			{
				AngleSearchTree::SearchTreeNode* node = mTreeNodes[tt];

				if ( node->_IsOctreeLeaf ) //node->mChildren.size() == 0
				{
					//found a leaf node
					leafNodes.push_back(node);
				}
			}


			return leafNodes;

		}



		//first a simple octree is initiated over the octree
		void AngleSearchTree::buildOctree()
		{

			BoundingBox rootBoundingBox;

			rootBoundingBox._MinValues[0] = FLT_MAX;
			rootBoundingBox._MinValues[1] = FLT_MAX;
			rootBoundingBox._MinValues[2] = FLT_MAX;

			rootBoundingBox._MaxValues[0] = -FLT_MAX;
			rootBoundingBox._MaxValues[1] = -FLT_MAX;
			rootBoundingBox._MaxValues[2] = -FLT_MAX;

		    //first build the octree over point clouds and mark each point with corresponding node
			unsigned int nPoints = mPoints.size();

			for (int pp = 0; pp < nPoints; pp++)
			{
				rootBoundingBox._MinValues[0] = std::min( rootBoundingBox._MinValues[0] , mPoints[pp](0) );
				rootBoundingBox._MinValues[1] = std::min( rootBoundingBox._MinValues[1] , mPoints[pp](1) );
				rootBoundingBox._MinValues[2] = std::min( rootBoundingBox._MinValues[2] , mPoints[pp](2) );

				rootBoundingBox._MaxValues[0] = std::max( rootBoundingBox._MaxValues[0] , mPoints[pp](0) );
				rootBoundingBox._MaxValues[1] = std::max( rootBoundingBox._MaxValues[1] , mPoints[pp](1) );
				rootBoundingBox._MaxValues[2] = std::max( rootBoundingBox._MaxValues[2] , mPoints[pp](2) );
			}

			rootBoundingBox._MinValues[0] -= mVoxelSize(0);
			rootBoundingBox._MinValues[1] -= mVoxelSize(0);
			rootBoundingBox._MinValues[2] -= mVoxelSize(0);

			rootBoundingBox._MaxValues[0] += mVoxelSize(0);
			rootBoundingBox._MaxValues[1] += mVoxelSize(0);
			rootBoundingBox._MaxValues[2] += mVoxelSize(0);


			double maxDim = 0;

			maxDim = rootBoundingBox._MaxValues[0] - rootBoundingBox._MinValues[0];
			maxDim = std::max(rootBoundingBox._MaxValues[1] - rootBoundingBox._MinValues[1], maxDim);
			maxDim = std::max(rootBoundingBox._MaxValues[2] - rootBoundingBox._MinValues[2], maxDim);

			rootBoundingBox._MaxValues[0] = rootBoundingBox._MinValues[0] + maxDim;
			rootBoundingBox._MaxValues[1] = rootBoundingBox._MinValues[1] + maxDim;
			rootBoundingBox._MaxValues[2] = rootBoundingBox._MinValues[2] + maxDim;

			SearchTreeNode *treeNode = new SearchTreeNode;

			treeNode->_BoundingBox = rootBoundingBox;

			treeNode->_NodeId = mTreeNodes.size();

			mTreeNodes.push_back(treeNode);

#if 0
			//first create a theoretical bound tree , its dimensions should be decided with voxel size
			buildTree(treeNode);

			unsigned int numPoints = mPoints.size();
			unsigned int numTriangles = mTriangleIndices.size() / 3;


			_PointNodeIds.resize(numPoints , -1);
			_TriangleNodeIds.resize(numTriangles);


			//once tree is built , now we need to mark all the vertices with their corresponding container nodes
			for ( unsigned int pp = 0; pp < numPoints; pp++ )
			{
				unsigned int nodeId = -1;

				nodeId = findNodeId(mPoints[pp]);

				_PointNodeIds[pp] = nodeId;

				mTreeNodes[nodeId]->mPointIndices.push_back(pp);
			}
#else



			unsigned int numPoints = mPoints.size();
			unsigned int numTriangles = mTriangleIndices.size() / 3;

			_PointNodeIds.resize(numPoints, -1);

			for ( unsigned int pp = 0; pp < numPoints; pp++ )
			{
				insertPointToNode(treeNode, pp, _LeafDimThreshold);
			}


#endif
			
		}


		//angle search tree is built on top of the octree , by modifying and further 
		//splitting it based on surface connectivity
		void AngleSearchTree::buildAngleSearchTree()
		{
			//first collect all the leaf nodes and check if they are splittable
			auto leafNodes = getLeafNodes();

			_IsPointUsed.resize(mPoints.size());

			std::fill(_IsPointUsed.begin(), _IsPointUsed.end(), false);

			//now build angle search tree
			for (auto node : leafNodes)
			{
				std::vector< std::vector< unsigned int > > connectedComponents;

				if (node->mPointIndices.size() == 0)
					continue;

				if ( findConnectedComponents( node , connectedComponents ) )
				{
					node->_IsOctreeLeaf = false;

					//multiple connected components found , we need to split the nodes
					int numComponents = connectedComponents.size();

					node->_NumChildren = numComponents;

					node->mChildren.resize(node->_NumChildren);

					unsigned int componentSum = 0;

					//each component should go in separate node 
					for (int cc = 0; cc < numComponents; cc++)
					{
						node->mChildren[cc] = new SearchTreeNode;

						node->mChildren[cc]->_NodeId = mTreeNodes.size();

						mTreeNodes.push_back(node->mChildren[cc]);

						SearchTreeNode *newNode = node->mChildren[cc];

						newNode->mPointIndices = connectedComponents[cc];

						newNode->_NumChildren = 0;

						newNode->_NodeId = mTreeNodes.size();

						int nCurrentNodePoints = newNode->mPointIndices.size();

						componentSum += nCurrentNodePoints;

						BoundingBox boundingBox;

						boundingBox._MinValues[0] = FLT_MAX;
						boundingBox._MinValues[1] = FLT_MAX;
						boundingBox._MinValues[2] = FLT_MAX;

						boundingBox._MaxValues[0] = -FLT_MAX;
						boundingBox._MaxValues[1] = -FLT_MAX;
						boundingBox._MaxValues[2] = -FLT_MAX;

						//remark the point id nodes
						for (int pp = 0; pp < nCurrentNodePoints; pp++)
						{
							int pointId = newNode->mPointIndices[pp];

							_PointNodeIds[pointId] = newNode->_NodeId;

							boundingBox._MinValues[0] = std::min(boundingBox._MinValues[0], mPoints[pointId](0));
							boundingBox._MinValues[1] = std::min(boundingBox._MinValues[1], mPoints[pointId](1));
							boundingBox._MinValues[2] = std::min(boundingBox._MinValues[2], mPoints[pointId](2));

							boundingBox._MaxValues[0] = std::max(boundingBox._MaxValues[0], mPoints[pointId](0));
							boundingBox._MaxValues[1] = std::max(boundingBox._MaxValues[1], mPoints[pointId](1));
							boundingBox._MaxValues[2] = std::max(boundingBox._MaxValues[2], mPoints[pointId](2));
						}

						newNode->_BoundingBox = boundingBox;

					}

					node->mPointIndices.clear();
				}

			}




		}
		
		

		void AngleSearchTree::pruneTree()
		{
		   
		
		}


		void AngleSearchTree::buildFineTree()
		{
			unsigned int nTreeNodes = mTreeNodes.size();

			for (int tt = 0; tt < nTreeNodes; tt++)
			{
				auto node = mTreeNodes[tt];

				if (node && node->_IsOctreeLeaf)
				{
					auto containedIndices = node->mPointIndices;

					node->mPointIndices.clear();

					//now build the local tree
					for (auto index : containedIndices)
					{
						insertPointToLocalTree(node , index);
					}
				}

				if ( node->mPointIndices.size() == 0 )
				{
					node->_IsOctreeLeaf = false;
				}
			}
		
		}

		
	}
	
}