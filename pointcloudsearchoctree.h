#ifndef __IMT_POINTCLOUDSEARCHOCTREE_H__
#define __IMT_POINTCLOUDSEARCHOCTREE_H__

#include "eigenincludes.h"

namespace imt{
	
	
	namespace volume{

		typedef Eigen::Vector3d PointT;

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/** \brief @b Octree key class
		*  \note Octree keys contain integer indices for each coordinate axis in order to address an octree leaf node.
		*  \author Julius Kammerl (julius@kammerl.de)
		*/
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		class OctreeKey
		{
		public:

			/** \brief Empty constructor. */
			OctreeKey() :
				x(0), y(0), z(0)
			{
			}

			/** \brief Constructor for key initialization. */
			OctreeKey(unsigned int keyX, unsigned int keyY, unsigned int keyZ) :
				x(keyX), y(keyY), z(keyZ)
			{
			}

			/** \brief Copy constructor. */
			OctreeKey(const OctreeKey& source)
			{
				memcpy(key_, source.key_, sizeof(key_));
			}

			/** \brief Operator== for comparing octree keys with each other.
			*  \return "true" if leaf node indices are identical; "false" otherwise.
			* */
			bool
				operator == (const OctreeKey& b) const
			{
				return ((b.x == this->x) && (b.y == this->y) && (b.z == this->z));
			}

			/** \brief Operator<= for comparing octree keys with each other.
			*  \return "true" if key indices are not greater than the key indices of b  ; "false" otherwise.
			* */
			bool
				operator <= (const OctreeKey& b) const
			{
				return ((b.x >= this->x) && (b.y >= this->y) && (b.z >= this->z));
			}

			/** \brief Operator>= for comparing octree keys with each other.
			*  \return "true" if key indices are not smaller than the key indices of b  ; "false" otherwise.
			* */
			bool
				operator >= (const OctreeKey& b) const
			{
				return ((b.x <= this->x) && (b.y <= this->y) && (b.z <= this->z));
			}

			/** \brief push a child node to the octree key
			*  \param[in] childIndex index of child node to be added (0-7)
			* */
			inline void
				pushBranch(unsigned char childIndex)
			{
				this->x = (this->x << 1) | (!!(childIndex & (1 << 2)));
				this->y = (this->y << 1) | (!!(childIndex & (1 << 1)));
				this->z = (this->z << 1) | (!!(childIndex & (1 << 0)));
			}

			/** \brief pop child node from octree key
			* */
			inline void
				popBranch()
			{
				this->x >>= 1;
				this->y >>= 1;
				this->z >>= 1;
			}

			/** \brief get child node index using depthMask
			*  \param[in] depthMask bit mask with single bit set at query depth
			*  \return child node index
			* */
			inline unsigned char
				getChildIdxWithDepthMask(unsigned int depthMask) const
			{
				return static_cast<unsigned char> (((!!(this->x & depthMask)) << 2)
					| ((!!(this->y & depthMask)) << 1)
					| (!!(this->z & depthMask)));
			}

			/* \brief maximum depth that can be addressed */
			static const unsigned char maxDepth = static_cast<const unsigned char>(sizeof(unsigned int) * 8);

			// Indices addressing a voxel at (X, Y, Z)

			union
			{
				struct
				{
					unsigned int x;
					unsigned int y;
					unsigned int z;
				};
				unsigned int key_[3];
			};


		};




		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/** \brief @b Priority queue entry for point candidates
		* \note This class defines priority queue entries for the nearest neighbor point candidates.
		* \author Julius Kammerl (julius@kammerl.de)
		*/
		class prioPointQueueEntry
		{
		public:

			/** \brief Empty constructor  */
			prioPointQueueEntry() :
				point_idx_(0), point_distance_(0)
			{
			}

			/** \brief Constructor for initializing priority queue entry.
			* \param[in] point_idx an index representing a point in the dataset given by \a setInputCloud
			* \param[in] point_distance distance of query point to voxel center
			*/
			prioPointQueueEntry(unsigned int& point_idx, float point_distance) :
				point_idx_(point_idx), point_distance_(point_distance)
			{
			}

			/** \brief Operator< for comparing priority queue entries with each other.
			* \param[in] rhs priority queue to compare this against
			*/
			bool
				operator< (const prioPointQueueEntry& rhs) const
			{
				return (this->point_distance_ < rhs.point_distance_);
			}

			/** \brief Index representing a point in the dataset given by \a setInputCloud. */
			int point_idx_;

			/** \brief Distance to query point. */
			float point_distance_;
		};
		
		
	class PointCloudSearchOctree
	{
		
		public:


			struct OctreeNode
			{


				OctreeNode();

				inline PointCloudSearchOctree::OctreeNode* getChildPtr(unsigned char buffer_arg, unsigned char index_arg) const;

				void getPointIndices(std::vector<int>& data_vector_arg) const;


				inline void setChildPtr(unsigned char buffer_arg, unsigned char index_arg,
					PointCloudSearchOctree::OctreeNode* newNode_arg);

				bool hasChild(unsigned char buffer_selector_arg, unsigned char child_idx_arg) const;

				OctreeNode* child_node_array_[2][8];

				std::vector< int > _LeafIndices;

				int _NodeType;


				int getNodeType();

				size_t getSize();

			};


			/** \brief @b Priority queue entry for branch nodes
			*  \note This class defines priority queue entries for the nearest neighbor search.
			*  \author Julius Kammerl (julius@kammerl.de)
			*/
			class prioBranchQueueEntry
			{
			public:
				/** \brief Empty constructor  */
				prioBranchQueueEntry() :
					node(), point_distance(0), key()
				{
				}

				/** \brief Constructor for initializing priority queue entry.
				* \param _node pointer to octree node
				* \param _key octree key addressing voxel in octree structure
				* \param[in] _point_distance distance of query point to voxel center
				*/
				prioBranchQueueEntry(OctreeNode* _node, OctreeKey& _key, float _point_distance) :
					node(_node), point_distance(_point_distance), key(_key)
				{
				}

				/** \brief Operator< for comparing priority queue entries with each other.
				* \param[in] rhs the priority queue to compare this against
				*/
				bool
					operator < (const prioBranchQueueEntry rhs) const
				{
					return (this->point_distance > rhs.point_distance);
				}

				/** \brief Pointer to octree node. */
				const OctreeNode* node;

				/** \brief Distance to query point. */
				float point_distance;

				/** \brief Octree key. */
				OctreeKey key;
			};


		
			PointCloudSearchOctree(const std::vector< Eigen::Vector3d >& points , double resolution );


			void addPointsFromInputCloud();





		int nearestKSearch( const PointT &p_q , int k , std::vector<int> &k_indices,
			                std::vector<float> &k_sqr_distances );



	protected:

		double getKNearestNeighborRecursive(const Eigen::Vector3d & point, unsigned int K, const OctreeNode* node, const OctreeKey& key, unsigned int tree_depth,
			const double squared_search_radius, std::vector<prioPointQueueEntry>& point_candidates) const;

		double	getVoxelSquaredSideLen( unsigned int tree_depth_arg ) const;

		double  getVoxelSquaredDiameter(unsigned int tree_depth_arg) const;

		bool branchHasChild( const PointCloudSearchOctree::OctreeNode* node, unsigned char child_idx) const;

		void genVoxelCenterFromOctreeKey(const OctreeKey & key_arg, unsigned int tree_depth_arg, PointT& point_arg) const;

		inline PointCloudSearchOctree::OctreeNode* getBranchChildPtr(const PointCloudSearchOctree::OctreeNode& node , unsigned char buffer_arg) const;

		double	pointSquaredDist(const PointT & point_a, const PointT & point_b) const;

		const PointT& getPointByIndex(const unsigned int index_arg) const;

		void addPointIdx(const int point_idx_arg);
		
		void adoptBoundingBoxToPoint(const PointT& point_idx_arg);

		inline void setBranchChildPtr(OctreeNode& branch_arg, unsigned char child_idx_arg, OctreeNode* new_child_arg);

		void setTreeDepth(unsigned int depth_arg);

		void getKeyBitSize();

		void genOctreeKeyforPoint(const PointT& point_arg,
			OctreeKey & key_arg) const;


		unsigned int createLeafRecursive(const OctreeKey& key_arg,
			unsigned int depth_mask_arg,
			OctreeNode* branch_arg,
			OctreeNode*& return_leaf_arg,
			OctreeNode*& parent_of_leaf_arg);

		inline  PointCloudSearchOctree::OctreeNode* createBranchChild(PointCloudSearchOctree::OctreeNode& branch_arg, //BranchNode& branch_arg,
			unsigned char child_idx_arg);
		
		void expandLeafNode(PointCloudSearchOctree::OctreeNode* leaf_node, PointCloudSearchOctree::OctreeNode* parent_branch,
			unsigned char child_idx, unsigned int depth_mask);

		void deleteBranchChild(PointCloudSearchOctree::OctreeNode& branch_arg, //BranchNode& branch_arg,
			unsigned char buffer_selector_arg,
			unsigned char child_idx_arg);

		inline void deleteBranch(PointCloudSearchOctree::OctreeNode& branch_arg);

		inline void deleteBranchChild(PointCloudSearchOctree::OctreeNode& branch_arg, unsigned char child_idx_arg);

	protected:



		/** \brief Epsilon precision (error bound) for nearest neighbors searches. */
		double epsilon_;

		/** \brief Octree resolution. */
		double resolution_;

		// Octree bounding box coordinates
		double min_x_;
		double max_x_;

		double min_y_;
		double max_y_;

		double min_z_;
		double max_z_;

		/** \brief Flag indicating if octree has defined bounding box. */
		bool bounding_box_defined_;

		/** \brief Amount of DataT objects per leafNode before expanding branch
		*  \note zero indicates a fixed/maximum depth octree structure
		* **/
		std::size_t max_objs_per_leaf_;

		/** \brief Amount of leaf nodes   **/
		std::size_t leaf_count_;

		/** \brief Amount of branch nodes   **/
		std::size_t branch_count_;

		/** \brief Pointer to root branch node of octree   **/
		OctreeNode* root_node_;

		/** \brief Depth mask based on octree depth   **/
		unsigned int depth_mask_;

		/** \brief key range */
		OctreeKey max_key_;

		/** \brief Currently active octree buffer  **/
		unsigned char buffer_selector_;

		// flags indicating if unused branches and leafs might exist in previous buffer
		bool tree_dirty_flag_;

		/** \brief Octree depth */
		unsigned int octree_depth_;

		/** \brief Enable dynamic_depth
		*  \note Note that this parameter is ignored in octree2buf! */
		bool dynamic_depth_enabled_;



		const std::vector< Eigen::Vector3d >& mPoints;

		
	};	
		
		
		
	}
	
	
	
}






#endif