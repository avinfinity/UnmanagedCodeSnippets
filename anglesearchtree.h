#ifndef __IMT_ANGLESEARCHTREE_H__
#define __IMT_ANGLESEARCHTREE_H__

#include "eigenincludes.h"
#include "queue"
#include "unordered_map"

namespace imt{
	
	namespace volume{

    class AngleSearchTree
	{



	protected:

		struct BoundingBox
		{
			double _MinValues[3], _MaxValues[3];
		};

		struct SearchTreeNode
		{

		public:

			SearchTreeNode();

			std::vector< unsigned int > mPointIndices;

			int _NumChildren;
			std::vector< SearchTreeNode* > mChildren , mSurfaceConnectionNodes;
			
			std::vector< BoundingBox > _ChildrenBoundingBoxes;
			BoundingBox _BoundingBox;

			unsigned int _NodeId;
			bool _IsOctreeLeaf;

			

		};

		struct QElem
		{

			SearchTreeNode *_Node;

			double _Dist;

			unsigned char _InclusionFlag;
		};


		struct LessThanByDistance
		{
			bool operator()(const QElem& lhs, const QElem& rhs) const
			{
				return lhs._Dist > rhs._Dist;
			}
		};

		bool ElemCompare(const QElem& elem1, const QElem& elem2) const;

		typedef std::priority_queue< QElem, std::vector< QElem >, LessThanByDistance > DistanceNodeQ;

		bool isNodeWithInRange(const Eigen::Vector3d& refPoint , double r2 , const SearchTreeNode* node);


		// 0 -- complete outside cone , 1 -- partially inside cone , 2 -- completely inside the cone
		unsigned char isNodeInsideCone(const Eigen::Vector3d& refPoint, const Eigen::Vector3d& coneAxis, const double& coneAngle, const int& nodeId);
		
		
		
		public:
		
		AngleSearchTree( const std::vector< Eigen::Vector3d >& points , const std::vector< unsigned int >& triangleIndices , Eigen::Vector3d& voxelSize  );

		void computeOppositeEnds( std::vector< Eigen::Vector3d >& inputPoints , std::vector< unsigned int >& oppositeEndIndices ,
			                      std::vector< Eigen::Vector3d >& oppositeEnds );

		void findClosestPoint(const Eigen::Vector3d& inputPoint, const unsigned int& oppEndFaceIndex, Eigen::Vector3d& closestPoint);

		void findClosestPoint( const Eigen::Vector3d& inputPoint , const Eigen::Vector3d& coneAxis , const unsigned int& oppEndFaceIndex , 
			                   const double& searchAngle , Eigen::Vector3d& closestPoint , unsigned int& closestFaceIndex );

		Eigen::Vector3d geometricCenter(const unsigned int& faceIndex) const;

	protected:

		//first a simple octree is initiated over the octree
		void buildOctree();

		
		//angle search tree is built on top of the octree , by modifying and further 
		//splitting it based on surface connectivity
		void buildAngleSearchTree();

		//in the coarse tree we simply divide the space and generate nodes for all the scpace divisions.
		//but as it happens , many of the space divisions are simply empty , so in pruning stage we delete 
		// the nodes which are empty. ( It's actually more efficient than directly entering the points while 
		//building the tree and allocating only non emty nodes )
		void pruneTree();


		//In the first step we build a surface connection tree , which is hierarchically a coarse octree , but the leaf nodes of the tree
		// are further connected to each other along surface. In this below method , we further build a fine octree hierarchy.
		void buildFineTree();

		//builds a simple octree hierarchy with treating input node as root node 
		unsigned int buildTree( AngleSearchTree::SearchTreeNode *node );

		//This method builds insert points into local node with , and simultaneously creates new hierarchy nodes during the process if needed
		// based on fine octree node size contraint.
		void insertPointToLocalTree( AngleSearchTree::SearchTreeNode *node , const unsigned int& pointId );

		//This method builds insert points into local node with , and simultaneously creates new hierarchy nodes during the process if needed
		// based on coarse octree node size contraint.
		void insertPointToNode(AngleSearchTree::SearchTreeNode *node, const unsigned int& pointId, double dimThreshold);

		//finds nodeId where the input point would actually fall inside the build tree structure
		unsigned int findNodeId(const Eigen::Vector3d& point);

		//finds nodeId where the input point would actually fall inside the built loacl tree structure , treating node as root 
		unsigned int findNodeId(const Eigen::Vector3d& point, AngleSearchTree::SearchTreeNode *node);

		//finds number connected components inside a surface node ( here connected component all points lies on same triangualted surface 
		//and can be reached from one to another  )
		bool findConnectedComponents( AngleSearchTree::SearchTreeNode *node , std::vector< std::vector<unsigned int> >& connectedComponents );

		//builds connected component given one of its point ids
		void buildConnectedComponent(unsigned int pointId , std::vector< unsigned int >& connectedComponent );

		//retrieves all the leaf nodes of the tree
		std::vector< AngleSearchTree::SearchTreeNode* > getLeafNodes();

		void buildConnectionAlongTheSurface();

		void buildIncidenceInfo();

		void visualizeLeafNodes();

		void collectCandidateNodes(const unsigned int& oppEndFaceIndex, std::vector< SearchTreeNode* >& candidateNodes);

		void collectCandidateNodes(const unsigned int& oppEndFaceIndex, const Eigen::Vector3d& coneOrigin , const Eigen::Vector3d& coneAxis,
			const double& searchAngle, std::unordered_map< unsigned int, unsigned char >& candidateNodes);

		

	protected:

		const std::vector< Eigen::Vector3d >& mPoints;

		const std::vector< unsigned int >& mTriangleIndices;

		const std::vector< std::vector< unsigned int > > _PointToPointIncidenceList;

		std::vector< SearchTreeNode* > mTreeNodes;

		std::vector< unsigned int > _PointNodeIds , _SurfaceConnectionNodeIds , _TriangleNodeIds;

		std::vector< unsigned char > _IsPointUsed;

		std::vector< std::vector< unsigned int > > _VertexTriangleIncidence;

		std::vector< std::vector<unsigned int> > _VertexVertexIncidence;

		Eigen::Vector3d mVoxelSize;
		
		unsigned int _LeafDimThreshold , _LeafLocalDimThreshold;

		DistanceNodeQ _SearchQ;

	};	
		
		
		
		
		
		
	}
	
	
}




#endif