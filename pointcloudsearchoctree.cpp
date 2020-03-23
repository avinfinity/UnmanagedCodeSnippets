#include "pointcloudsearchoctree.h"
#include "vector"
#include "iostream"



namespace imt{
	
	
	namespace volume{

#define BRANCH_NODE 0
#define LEAF_NODE 1



		PointCloudSearchOctree::OctreeNode::OctreeNode()
		{

			for (int ii = 0; ii < 8; ii++)
			{
				child_node_array_[0][ii] = 0;
				child_node_array_[1][ii] = 0;
			}

			_NodeType = 0;
			
		}

		void
			PointCloudSearchOctree::genVoxelCenterFromOctreeKey(
			const OctreeKey & key_arg,
			unsigned int tree_depth_arg,
			PointT& point_arg) const
		{
			// generate point for voxel center defined by treedepth (bitLen) and key
			point_arg.x() = static_cast<float> ((static_cast <double> (key_arg.x) + 0.5f) * (this->resolution_ * static_cast<double> (1 << (this->octree_depth_ - tree_depth_arg))) + this->min_x_);
			point_arg.y() = static_cast<float> ((static_cast <double> (key_arg.y) + 0.5f) * (this->resolution_ * static_cast<double> (1 << (this->octree_depth_ - tree_depth_arg))) + this->min_y_);
			point_arg.z() = static_cast<float> ((static_cast <double> (key_arg.z) + 0.5f) * (this->resolution_ * static_cast<double> (1 << (this->octree_depth_ - tree_depth_arg))) + this->min_z_);
		}





		PointCloudSearchOctree::PointCloudSearchOctree(const std::vector< Eigen::Vector3d >& points, double resolution) : mPoints(points), resolution_( resolution )
		{
			/** \brief Epsilon precision (error bound) for nearest neighbors searches. */
			double epsilon_ = 0.1 * resolution_;

			/** \brief Octree resolution. */
			//double resolution_;

			// Octree bounding box coordinates
			min_x_ = DBL_MAX;
			max_x_ = -DBL_MAX;

			min_y_ = DBL_MAX;
			max_y_ = -DBL_MAX;

			min_z_ = DBL_MAX;
			max_z_ = -DBL_MAX;

			/** \brief Flag indicating if octree has defined bounding box. */
			bounding_box_defined_ = false; //bool 

			/** \brief Amount of DataT objects per leafNode before expanding branch
			*  \note zero indicates a fixed/maximum depth octree structure
			* **/
			max_objs_per_leaf_ = 8;

			/** \brief Amount of leaf nodes   **/
			leaf_count_ = 0;

			/** \brief Amount of branch nodes   **/
			branch_count_ = 0;

			/** \brief Pointer to root branch node of octree   **/
			root_node_ = new OctreeNode;

			root_node_->_NodeType = 1;

			/** \brief Depth mask based on octree depth   **/
			unsigned int depth_mask_ =0;

			/** \brief key range */
			//OctreeKey max_key_;

			/** \brief Currently active octree buffer  **/
			buffer_selector_ = 0;

			// flags indicating if unused branches and leafs might exist in previous buffer
			tree_dirty_flag_ = false;

			/** \brief Octree depth */
			octree_depth_ = 0;

			/** \brief Enable dynamic_depth
			*  \note Note that this parameter is ignored in octree2buf! */
			dynamic_depth_enabled_ = true;

			depth_mask_ = 0;
		}


		void PointCloudSearchOctree::addPointsFromInputCloud()
		{
			size_t i;

			//if (indices_)
			//{
			//	for (std::vector<int>::const_iterator current = indices_->begin(); current != indices_->end(); ++current)
			//	{
			//		assert((*current >= 0) && (*current < static_cast<int> (input_->points.size())));

			//		if (isFinite(input_->points[*current]))
			//		{
			//			// add points to octree
			//			addPointIdx(*current);
			//		}
			//	}
			//}
			//else
			//{
				for (i = 0; i < mPoints.size(); i++)
				{
					//if (isFinite(mPoints[i]))
					{
						// add points to octree
						addPointIdx(static_cast<unsigned int> (i));
					}
				}
			//}
		}


		//////////////////////////////////////////////////////////////////////////////////////////////
		void PointCloudSearchOctree::genOctreeKeyforPoint(const PointT& point_arg,
			OctreeKey & key_arg) const
		{
			// calculate integer key for point coordinates
			key_arg.x = static_cast<unsigned int> ((point_arg.x() - this->min_x_) / this->resolution_);
			key_arg.y = static_cast<unsigned int> ((point_arg.y() - this->min_y_) / this->resolution_);
			key_arg.z = static_cast<unsigned int> ((point_arg.z() - this->min_z_) / this->resolution_);
		}



		/** \brief Fetch and add a new branch child to a branch class in current buffer
		*  \param branch_arg: reference to octree branch class
		*  \param child_idx_arg: index to child node
		*  \return pointer of new branch child to this reference
		* */
		inline  PointCloudSearchOctree::OctreeNode* PointCloudSearchOctree::createBranchChild(PointCloudSearchOctree::OctreeNode& branch_arg, //BranchNode& branch_arg,
			unsigned char child_idx_arg)
		{
			//BranchNode* new_branch_child = new BranchNode();

			PointCloudSearchOctree::OctreeNode* new_branch_child = new PointCloudSearchOctree::OctreeNode();

			branch_arg.setChildPtr( buffer_selector_, child_idx_arg,
				static_cast<OctreeNode*> (new_branch_child));

			return new_branch_child;
		}


		/** \brief Create and add a new branch child to a branch class
		*  \param branch_arg: reference to octree branch class
		*  \param child_idx_arg: index to child node
		*  \return pointer of new branch child to this reference
		* */
		PointCloudSearchOctree::OctreeNode* createBranchChild(PointCloudSearchOctree::OctreeNode& branch_arg,
			unsigned char child_idx_arg)
		{
			//BranchNode* new_branch_child = new BranchNode();

			PointCloudSearchOctree::OctreeNode* new_branch_child = new PointCloudSearchOctree::OctreeNode();

			branch_arg.child_node_array_[0][child_idx_arg] = static_cast<PointCloudSearchOctree::OctreeNode*> (new_branch_child);

			return new_branch_child;
		}

		/** \brief Create and add a new leaf child to a branch class
		*  \param branch_arg: reference to octree branch class
		*  \param child_idx_arg: index to child node
		*  \return pointer of new leaf child to this reference
		* */
		PointCloudSearchOctree::OctreeNode*
			createLeafChild(PointCloudSearchOctree::OctreeNode& branch_arg, unsigned char child_idx_arg)
		{
			//LeafNode* new_leaf_child = new LeafNode();

			PointCloudSearchOctree::OctreeNode* new_leaf_child = new PointCloudSearchOctree::OctreeNode();
			
			branch_arg.child_node_array_[1][child_idx_arg] = static_cast<PointCloudSearchOctree::OctreeNode*> (new_leaf_child);

			return new_leaf_child;
		}



		unsigned int PointCloudSearchOctree::createLeafRecursive(const OctreeKey& key_arg,
			unsigned int depth_mask_arg,
			OctreeNode* branch_arg,
			OctreeNode*& return_leaf_arg,
			OctreeNode*& parent_of_leaf_arg)
		{
			// index to branch child
			unsigned char child_idx;

			//std::cout << " depth mask : " << depth_mask_arg << std::endl;

			// find branch child from key
			child_idx = key_arg.getChildIdxWithDepthMask(depth_mask_arg);

			//std::cout << " child index id : " << (unsigned int)child_idx << std::endl;

			OctreeNode* child_node = (*branch_arg).child_node_array_[0][child_idx];

			if (!child_node)
			{
				if ((!dynamic_depth_enabled_) && (depth_mask_arg > 1))
				{
					// if required branch does not exist -> create it
					PointCloudSearchOctree::OctreeNode *childBranch = createBranchChild(*branch_arg, child_idx); //BranchNode*

					branch_count_++;
					// recursively proceed with indexed child branch
					return createLeafRecursive(key_arg, depth_mask_arg / 2, childBranch, return_leaf_arg, parent_of_leaf_arg);

				}
				else
				{
					// if leaf node at child_idx does not exist
					//LeafNode* leaf_node = createLeafChild(*branch_arg, child_idx);
					OctreeNode* leaf_node = createLeafChild(*branch_arg, child_idx);
					
					return_leaf_arg = leaf_node;
					parent_of_leaf_arg = branch_arg;
					this->leaf_count_++;
				}
			}
			else
			{
				// Node exists already
				switch (child_node->getNodeType())
				{

				case BRANCH_NODE:
					// recursively proceed with indexed child branch
					return createLeafRecursive( key_arg, depth_mask_arg / 2, static_cast<OctreeNode*> (child_node), //BranchNode*
						                        return_leaf_arg, parent_of_leaf_arg);
					break;

				case LEAF_NODE:
					return_leaf_arg = static_cast<OctreeNode*> (child_node); //LeafNode*
					parent_of_leaf_arg = branch_arg;
					break;
				}

			}

			return (depth_mask_arg >> 1);
		}



		//////////////////////////////////////////////////////////////////////////////////////////////
		void PointCloudSearchOctree::addPointIdx(const int point_idx_arg)
		{
			OctreeKey key;

			assert(point_idx_arg < static_cast<int> ( mPoints.size() )); //input_->points.size()

			const PointT& point = mPoints[point_idx_arg];

			// make sure bounding box is big enough
			adoptBoundingBoxToPoint(point);

			//return;

			// generate key
			genOctreeKeyforPoint(point, key);

			//LeafNode* leaf_node;
			OctreeNode *leaf_node;
			
			//BranchNode* parent_branch_of_leaf_node;
			OctreeNode *parent_branch_of_leaf_node;

			unsigned int depth_mask = this->createLeafRecursive(key, this->depth_mask_, this->root_node_, leaf_node, parent_branch_of_leaf_node);

			//return;

			if (this->dynamic_depth_enabled_ && depth_mask)
			{
				// get amount of objects in leaf container
				size_t leaf_obj_count = leaf_node->getSize(); //(*leaf_node)->getSize();

				while (leaf_obj_count >= max_objs_per_leaf_ && depth_mask)
				{
					// index to branch child
					unsigned char child_idx = key.getChildIdxWithDepthMask(depth_mask * 2);

					expandLeafNode( leaf_node , parent_branch_of_leaf_node,
						            child_idx ,  depth_mask );

					depth_mask = this->createLeafRecursive(key, this->depth_mask_, this->root_node_, leaf_node, parent_branch_of_leaf_node);
					leaf_obj_count = leaf_node->getSize(); //(*leaf_node)->getSize();
				}

			}

			//(*leaf_node)->addPointIndex(point_idx_arg);
			//leaf_node->_LeafIndices.push_back(point_idx_arg);
			
		}

		//LeafNode* leaf_node, BranchNode* parent_branch, unsigned char child_idx, unsigned int depth_mask
		//////////////////////////////////////////////////////////////////////////////////////////////
		void PointCloudSearchOctree::expandLeafNode(PointCloudSearchOctree::OctreeNode* leaf_node, PointCloudSearchOctree::OctreeNode* parent_branch,
			unsigned char child_idx, unsigned int depth_mask)
		{

			if (depth_mask)
			{
				// get amount of objects in leaf container
				size_t leaf_obj_count = leaf_node->getSize(); //(*leaf_node)->getSize();

				// copy leaf data
				std::vector<int> leafIndices;
				leafIndices.reserve(leaf_obj_count);

				//(*leaf_node)->getPointIndices(leafIndices);
				leaf_node->getPointIndices(leafIndices);


				// delete current leaf node
				this->deleteBranchChild(*parent_branch, child_idx);
				this->leaf_count_--;

				// create new branch node
				//BranchNode* childBranch = this->createBranchChild(*parent_branch, child_idx);
				OctreeNode* childBranch = this->createBranchChild(*parent_branch, child_idx);

				childBranch->_NodeType = 1;
				
				this->branch_count_++;

				std::vector<int>::iterator it = leafIndices.begin();
				std::vector<int>::const_iterator it_end = leafIndices.end();

				// add data to new branch
				OctreeKey new_index_key;

				for (it = leafIndices.begin(); it != it_end; ++it)
				{

					const PointT& point_from_index = mPoints[*it];
					// generate key
					genOctreeKeyforPoint(point_from_index, new_index_key);

					//LeafNode* newLeaf;
					//BranchNode* newBranchParent;
					OctreeNode* newLeaf;
					OctreeNode* newBranchParent;

					this->createLeafRecursive(new_index_key, depth_mask, childBranch, newLeaf, newBranchParent);

					//(newLeaf)->addPointIndex(*it); //* TODO

					newLeaf->_LeafIndices.push_back(*it);
				}
			}


		}





		/** \brief Assign new child node to branch
		*  \param branch_arg: reference to octree branch class
		*  \param child_idx_arg: index to child node
		*  \param new_child_arg: pointer to new child node
		* */
		inline void PointCloudSearchOctree::setBranchChildPtr(OctreeNode& branch_arg, unsigned char child_idx_arg, OctreeNode* new_child_arg)
		{
			branch_arg.setChildPtr(buffer_selector_, child_idx_arg, new_child_arg);
		}



		void PointCloudSearchOctree::setTreeDepth(unsigned int depth_arg)
		{
			assert(depth_arg>0);

			// set octree depth
			octree_depth_ = depth_arg;

			// define depthMask_ by setting a single bit to 1 at bit position == tree depth
			depth_mask_ = (1 << (depth_arg - 1));

			// define max. keys
			max_key_.x = max_key_.y = max_key_.z = (1 << depth_arg) - 1;
		}


		/** \brief Helper function to calculate the binary logarithm
		* \param n_arg: some value
		* \return binary logarithm (log2) of argument n_arg
		*/
		inline double Log2(double n_arg)
		{
			return log(n_arg) / log(2.0);
		}



		//////////////////////////////////////////////////////////////////////////////////////////////
		void PointCloudSearchOctree::getKeyBitSize()
		{
			unsigned int max_voxels;

			unsigned int max_key_x;
			unsigned int max_key_y;
			unsigned int max_key_z;

			double octree_side_len;

			const float minValue = std::numeric_limits<float>::epsilon();

			// find maximum key values for x, y, z
			max_key_x = static_cast<unsigned int> ((max_x_ - min_x_) / resolution_);
			max_key_y = static_cast<unsigned int> ((max_y_ - min_y_) / resolution_);
			max_key_z = static_cast<unsigned int> ((max_z_ - min_z_) / resolution_);

			// find maximum amount of keys
			max_voxels = std::max(std::max(std::max(max_key_x, max_key_y), max_key_z), static_cast<unsigned int> (2));


			// tree depth == amount of bits of max_voxels
			this->octree_depth_ = std::max((std::min(static_cast<unsigned int> (OctreeKey::maxDepth), static_cast<unsigned int> (ceil( Log2(max_voxels) - minValue)))),
				static_cast<unsigned int> (0));

			octree_side_len = static_cast<double> (1 << this->octree_depth_) * resolution_ - minValue;

			if (this->leaf_count_ == 0)
			{
				double octree_oversize_x;
				double octree_oversize_y;
				double octree_oversize_z;

				octree_oversize_x = (octree_side_len - (max_x_ - min_x_)) / 2.0;
				octree_oversize_y = (octree_side_len - (max_y_ - min_y_)) / 2.0;
				octree_oversize_z = (octree_side_len - (max_z_ - min_z_)) / 2.0;

				min_x_ -= octree_oversize_x;
				min_y_ -= octree_oversize_y;
				min_z_ -= octree_oversize_z;

				max_x_ += octree_oversize_x;
				max_y_ += octree_oversize_y;
				max_z_ += octree_oversize_z;
			}
			else
			{
				max_x_ = min_x_ + octree_side_len;
				max_y_ = min_y_ + octree_side_len;
				max_z_ = min_z_ + octree_side_len;
			}

			// configure tree depth of octree
			this->setTreeDepth(this->octree_depth_);

		}



		void PointCloudSearchOctree::adoptBoundingBoxToPoint(const PointT& point_idx_arg)
		{

			const float minValue = std::numeric_limits<float>::epsilon();

			// increase octree size until point fits into bounding box
			while (true)
			{
				bool bLowerBoundViolationX = (point_idx_arg.x() < min_x_);
				bool bLowerBoundViolationY = (point_idx_arg.y() < min_y_);
				bool bLowerBoundViolationZ = (point_idx_arg.z() < min_z_);

				bool bUpperBoundViolationX = (point_idx_arg.x() >= max_x_);
				bool bUpperBoundViolationY = (point_idx_arg.y() >= max_y_);
				bool bUpperBoundViolationZ = (point_idx_arg.z() >= max_z_);

				// do we violate any bounds?
				if (bLowerBoundViolationX || bLowerBoundViolationY || bLowerBoundViolationZ || bUpperBoundViolationX
					|| bUpperBoundViolationY || bUpperBoundViolationZ || (!bounding_box_defined_))
				{

					if (bounding_box_defined_)
					{

						double octreeSideLen;
						unsigned char child_idx;

						// octree not empty - we add another tree level and thus increase its size by a factor of 2*2*2
						child_idx = static_cast<unsigned char> (((!bUpperBoundViolationX) << 2) | ((!bUpperBoundViolationY) << 1)
							| ((!bUpperBoundViolationZ)));

						//BranchNode* newRootBranch;

						OctreeNode* newRootBranch;

						newRootBranch = new OctreeNode();//BranchNode();

						this->branch_count_++;

						this->setBranchChildPtr(*newRootBranch, child_idx, this->root_node_);

						this->root_node_ = newRootBranch;

						octreeSideLen = static_cast<double> (1 << this->octree_depth_) * resolution_;

						if (!bUpperBoundViolationX)
							min_x_ -= octreeSideLen;

						if (!bUpperBoundViolationY)
							min_y_ -= octreeSideLen;

						if (!bUpperBoundViolationZ)
							min_z_ -= octreeSideLen;

						// configure tree depth of octree
						this->octree_depth_++;
						this->setTreeDepth(this->octree_depth_);

						// recalculate bounding box width
						octreeSideLen = static_cast<double> (1 << this->octree_depth_) * resolution_ - minValue;

						// increase octree bounding box
						max_x_ = min_x_ + octreeSideLen;
						max_y_ = min_y_ + octreeSideLen;
						max_z_ = min_z_ + octreeSideLen;

					}
					// bounding box is not defined - set it to point position
					else
					{
						// octree is empty - we set the center of the bounding box to our first pixel
						this->min_x_ = point_idx_arg.x() - this->resolution_ / 2;
						this->min_y_ = point_idx_arg.y() - this->resolution_ / 2;
						this->min_z_ = point_idx_arg.z() - this->resolution_ / 2;

						this->max_x_ = point_idx_arg.x() + this->resolution_ / 2;
						this->max_y_ = point_idx_arg.y() + this->resolution_ / 2;
						this->max_z_ = point_idx_arg.z() + this->resolution_ / 2;

						getKeyBitSize();

						bounding_box_defined_ = true;
					}

				}
				else
					// no bound violations anymore - leave while loop
					break;
			}
		}




		/** \brief Get child pointer in current branch node
		*  \param buffer_arg: buffer selector
		*  \param index_arg: index of child in node
		*  \return pointer to child node
		* */
		inline PointCloudSearchOctree::OctreeNode*
			PointCloudSearchOctree::OctreeNode::getChildPtr(unsigned char buffer_arg, unsigned char index_arg) const
		{
			assert((buffer_arg<2) && (index_arg<8));
			return child_node_array_[buffer_arg][index_arg];
		}


		/** \brief Set child pointer in current branch node
		*  \param buffer_arg: buffer selector
		*  \param index_arg: index of child in node
		*  \param newNode_arg: pointer to new child node
		* */
		inline void PointCloudSearchOctree::OctreeNode::setChildPtr(unsigned char buffer_arg, unsigned char index_arg,
			PointCloudSearchOctree::OctreeNode* newNode_arg)
		{
			assert((buffer_arg<2) && (index_arg<8));
			child_node_array_[buffer_arg][index_arg] = newNode_arg;
		}


		bool PointCloudSearchOctree::OctreeNode::hasChild(unsigned char buffer_selector_arg, unsigned char child_idx_arg) const
		{

			if (child_node_array_[buffer_selector_arg][child_idx_arg])
			{
				return true;
			}
			else
			{
				return false;
			}

		}


		int PointCloudSearchOctree::OctreeNode::getNodeType()
		{
			return _NodeType;
		}


		size_t PointCloudSearchOctree::OctreeNode::getSize()
		{
			return _LeafIndices.size();
		}



		/** \brief Retrieve point indices from container. This container stores a vector of point indices.
		* \param[out] data_vector_arg vector of point indices to be stored within data vector
		*/
		void
			PointCloudSearchOctree::OctreeNode::getPointIndices(std::vector<int>& data_vector_arg) const
		{
			data_vector_arg.insert(data_vector_arg.end(), _LeafIndices.begin(), _LeafIndices.end());
		}



		inline PointCloudSearchOctree::OctreeNode*
			PointCloudSearchOctree::getBranchChildPtr( const PointCloudSearchOctree::OctreeNode& branch_arg,
			unsigned char child_idx_arg) const
		{
			return branch_arg.getChildPtr( buffer_selector_ , child_idx_arg );
		}



		//////////////////////////////////////////////////////////////////////////////////////////////
		double	PointCloudSearchOctree::pointSquaredDist( const PointT & point_a, const PointT & point_b) const
		{
			return (point_a - point_b).squaredNorm();
		}

		
		
		//////////////////////////////////////////////////////////////////////////////////////////////
	   int
			PointCloudSearchOctree::nearestKSearch(const PointT &p_q, int k,
			std::vector<int> &k_indices,
			std::vector<float> &k_sqr_distances)
		{
			assert(this->leaf_count_>0);
			assert(isFinite(p_q) && "Invalid (NaN, Inf) point coordinates given to nearestKSearch!");

			k_indices.clear();
			k_sqr_distances.clear();

			if (k < 1)
				return 0;

			unsigned int i;
			unsigned int result_count;

			prioPointQueueEntry point_entry;
			std::vector<prioPointQueueEntry> point_candidates;

			OctreeKey key;
			key.x = key.y = key.z = 0;

			// initalize smallest point distance in search with high value
			double smallest_dist = std::numeric_limits<double>::max();

			getKNearestNeighborRecursive(p_q, k, this->root_node_, key, 1, smallest_dist, point_candidates);

			result_count = static_cast<unsigned int> (point_candidates.size());

			k_indices.resize(result_count);
			k_sqr_distances.resize(result_count);

			for (i = 0; i < result_count; ++i)
			{
				k_indices[i] = point_candidates[i].point_idx_;
				k_sqr_distances[i] = point_candidates[i].point_distance_;
			}

			return static_cast<int> (k_indices.size());
		}



		double	PointCloudSearchOctree::getKNearestNeighborRecursive( const Eigen::Vector3d & point, unsigned int K, const PointCloudSearchOctree::OctreeNode* node, const OctreeKey& key, unsigned int tree_depth,
			                                                          const double squared_search_radius, std::vector<prioPointQueueEntry>& point_candidates) const
		{
			std::vector<prioBranchQueueEntry> search_heap;
			search_heap.resize(8);

			unsigned char child_idx;

			OctreeKey new_key;

			double smallest_squared_dist = squared_search_radius;

			// get spatial voxel information
			double voxelSquaredDiameter = this->getVoxelSquaredDiameter(tree_depth);

			// iterate over all children
			for (child_idx = 0; child_idx < 8; child_idx++)
			{
				if ( branchHasChild( node, child_idx) )
				{
					Eigen::Vector3d voxel_center;

					search_heap[child_idx].key.x = (key.x << 1) + (!!(child_idx & (1 << 2)));
					search_heap[child_idx].key.y = (key.y << 1) + (!!(child_idx & (1 << 1)));
					search_heap[child_idx].key.z = (key.z << 1) + (!!(child_idx & (1 << 0)));

					// generate voxel center point for voxel at key
					genVoxelCenterFromOctreeKey(search_heap[child_idx].key, tree_depth, voxel_center);

					// generate new priority queue element
					search_heap[child_idx].node = this->getBranchChildPtr(*node, child_idx);
					search_heap[child_idx].point_distance = pointSquaredDist(voxel_center, point);
				}
				else
				{
					search_heap[child_idx].point_distance = std::numeric_limits<float>::infinity();
				}
			}

			std::sort(search_heap.begin(), search_heap.end());

			std::cout << " search heap size :  " << tree_depth << " " << this->octree_depth_ << " "
				<< smallest_squared_dist << "  " << voxelSquaredDiameter << " " 
				<< search_heap.size() << " " << search_heap.back().point_distance << std::endl;


			// iterate over all children in priority queue
			// check if the distance to search candidate is smaller than the best point distance (smallest_squared_dist)
			while ((!search_heap.empty()) && (search_heap.back().point_distance <
				smallest_squared_dist + voxelSquaredDiameter / 4.0 + sqrt(smallest_squared_dist * voxelSquaredDiameter) - this->epsilon_))
			{
				const OctreeNode* child_node;

				// read from priority queue element
				child_node = search_heap.back().node;
				new_key = search_heap.back().key;

				if (tree_depth < this->octree_depth_)
				{
					// we have not reached maximum tree depth
					smallest_squared_dist = getKNearestNeighborRecursive(point, K, static_cast<const PointCloudSearchOctree::OctreeNode*> (child_node), new_key, tree_depth + 1,
						smallest_squared_dist, point_candidates);
				}
				else
				{
					// we reached leaf node level

					float squared_dist;
					size_t i;
					std::vector<int> decoded_point_vector;

					const PointCloudSearchOctree::OctreeNode* child_leaf = static_cast<const PointCloudSearchOctree::OctreeNode*> (child_node);

					// decode leaf node into decoded_point_vector
					//(*child_leaf)->getPointIndices(decoded_point_vector);

					std::cout << child_leaf->_NodeType << " " << child_leaf->_LeafIndices.size() << std::endl;

					child_leaf->getPointIndices(decoded_point_vector);

					// Linearly iterate over all decoded (unsorted) points
					for (i = 0; i < decoded_point_vector.size(); i++)
					{

						const PointT& candidate_point = this->getPointByIndex(decoded_point_vector[i]);

						// calculate point distance to search point
						squared_dist = pointSquaredDist(candidate_point, point);

						// check if a closer match is found
						if (squared_dist < smallest_squared_dist)
						{
							prioPointQueueEntry point_entry;

							point_entry.point_distance_ = squared_dist;
							point_entry.point_idx_ = decoded_point_vector[i];
							point_candidates.push_back(point_entry);
						}
					}

					std::sort(point_candidates.begin(), point_candidates.end());

					if (point_candidates.size() > K)
						point_candidates.resize(K);

					if (point_candidates.size() == K)
						smallest_squared_dist = point_candidates.back().point_distance_;
				}
				// pop element from priority queue
				search_heap.pop_back();
			}

			return (smallest_squared_dist);
		}


		//////////////////////////////////////////////////////////////////////////////////////////////
		const PointT& PointCloudSearchOctree::getPointByIndex(const unsigned int index_arg) const
		{
			// retrieve point from input cloud
			assert(index_arg < static_cast<unsigned int> (input_->points.size()));
			
			return mPoints[index_arg];  //( input_->points[index_arg] );
		}



		double	PointCloudSearchOctree::getVoxelSquaredSideLen(unsigned int tree_depth_arg) const
		{
			double side_len;

			// side length of the voxel cube increases exponentially with the octree depth
			side_len = this->resolution_ * static_cast<double>(1 << ( octree_depth_ - tree_depth_arg));

			// squared voxel side length
			side_len *= side_len;

			return (side_len);
		}

		//////////////////////////////////////////////////////////////////////////////////////////////
		double  PointCloudSearchOctree::getVoxelSquaredDiameter(unsigned int tree_depth_arg) const
		{
			// return the squared side length of the voxel cube as a function of the octree depth
			return (getVoxelSquaredSideLen(tree_depth_arg) * 3);
		}


		bool PointCloudSearchOctree::branchHasChild( const PointCloudSearchOctree::OctreeNode* node, unsigned char child_idx) const 
		{


			return (node->getChildPtr(0 , child_idx) != 0);
		}



		/** \brief Delete child node and all its subchilds from octree in specific buffer
		*  \param branch_arg: reference to octree branch class
		*  \param buffer_selector_arg: buffer selector
		*  \param child_idx_arg: index to child node
		* */
		void PointCloudSearchOctree::deleteBranchChild( PointCloudSearchOctree::OctreeNode& branch_arg, //BranchNode& branch_arg,
			unsigned char buffer_selector_arg , unsigned char child_idx_arg)
		{
			if (branch_arg.hasChild(buffer_selector_arg, child_idx_arg))
			{
				OctreeNode* branchChild = branch_arg.getChildPtr(buffer_selector_arg, child_idx_arg);

				switch (branchChild->getNodeType())
				{
				case BRANCH_NODE:
				{
					// free child branch recursively
					deleteBranch(*static_cast<OctreeNode*> (branchChild)); //BranchNode

					// delete unused branch
					delete (branchChild);
					break;
				}

				case LEAF_NODE:
				{
					// push unused leaf to branch pool
					delete (branchChild);
					break;
				}
				default:
					break;
				}

				// set branch child pointer to 0
				branch_arg.setChildPtr(buffer_selector_arg, child_idx_arg, 0);
			}
		}

		/** \brief Delete child node and all its subchilds from octree in current buffer
		*  \param branch_arg: reference to octree branch class
		*  \param child_idx_arg: index to child node
		* */
		inline void PointCloudSearchOctree::deleteBranchChild(PointCloudSearchOctree::OctreeNode& branch_arg, unsigned char child_idx_arg) //BranchNode& branch_arg,
		{
			deleteBranchChild(branch_arg, buffer_selector_, child_idx_arg);
		}

		/** \brief Delete branch and all its subchilds from octree (both buffers)
		*  \param branch_arg: reference to octree branch class
		* */
		inline void PointCloudSearchOctree::deleteBranch(PointCloudSearchOctree::OctreeNode& branch_arg) //BranchNode& branch_arg
		{
			char i;

			// delete all branch node children
			for (i = 0; i < 8; i++)
			{

				if (branch_arg.getChildPtr(0, i) == branch_arg.getChildPtr(1, i))
				{
					// reference was copied - there is only one child instance to be deleted
					deleteBranchChild(branch_arg, 0, i);

					// remove pointers from both buffers
					branch_arg.setChildPtr(0, i, 0);
					branch_arg.setChildPtr(1, i, 0);
				}
				else
				{
					deleteBranchChild(branch_arg, 0, i);
					deleteBranchChild(branch_arg, 1, i);
				}
			}
		}

		
		
	}
	
	
	
}