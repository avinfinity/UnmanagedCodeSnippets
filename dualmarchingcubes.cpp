#include "dualmarchingcubes.h"
#include "display3droutines.h"
#include "volumeutility.h"
#include "dmc/marching_cubes.hpp"
#include "volumetopooctree.h"

//#include "sobel"

namespace imt {

	namespace volume {

		void enumerate_impl_v(
			const DMCOctree::DMCOctreeNode* n1,
			const DMCOctree::DMCOctreeNode* n2,
			const DMCOctree::DMCOctreeNode* n3,
			const DMCOctree::DMCOctreeNode* n4,
			const DMCOctree::DMCOctreeNode* n5,
			const DMCOctree::DMCOctreeNode* n6,
			const DMCOctree::DMCOctreeNode* n7,
			const DMCOctree::DMCOctreeNode* n8,
			std::vector<Eigen::Vector3f>& receiver)
		{
			auto b1 = !n1->_IsLeaf;//dynamic_cast<const branch_node_type*>(&n1);
			auto b2 = !n2->_IsLeaf;//dynamic_cast<const branch_node_type*>(&n2);
			auto b3 = !n3->_IsLeaf;// dynamic_cast<const branch_node_type*>(&n3);
			auto b4 = !n4->_IsLeaf;// dynamic_cast<const branch_node_type*>(&n4);
			auto b5 = !n5->_IsLeaf;//dynamic_cast<const branch_node_type*>(&n5);
			auto b6 = !n6->_IsLeaf;// dynamic_cast<const branch_node_type*>(&n6);
			auto b7 = !n7->_IsLeaf;// dynamic_cast<const branch_node_type*>(&n7);
			auto b8 = !n8->_IsLeaf;// dynamic_cast<const branch_node_type*>(&n8);

			if (b1 || b2 || b3 || b4 || b5 || b6 || b7 || b8)
			{
				enumerate_impl_v(
					b1 ? n1->children[7] : n1,
					b2 ? n2->children[6] : n2,
					b3 ? n3->children[5] : n3,
					b4 ? n4->children[4] : n4,
					b5 ? n5->children[3] : n5,
					b6 ? n6->children[2] : n6,
					b7 ? n7->children[1] : n7,
					b8 ? n8->children[0] : n8,
					receiver);
			}
			else
			{
				//auto l1 = static_cast<const leaf_node_type*>(&n1);
				//auto l2 = static_cast<const leaf_node_type*>(&n2);
				//auto l3 = static_cast<const leaf_node_type*>(&n3);
				//auto l4 = static_cast<const leaf_node_type*>(&n4);
				//auto l5 = static_cast<const leaf_node_type*>(&n5);
				//auto l6 = static_cast<const leaf_node_type*>(&n6);
				//auto l7 = static_cast<const leaf_node_type*>(&n7);
				//auto l8 = static_cast<const leaf_node_type*>(&n8);

				std::vector<DMCVertex> vertices;

				vertices.push_back(n1->vertex());
				vertices.push_back(n2->vertex());
				vertices.push_back(n3->vertex());
				vertices.push_back(n4->vertex());
				vertices.push_back(n5->vertex());
				vertices.push_back(n6->vertex());
				vertices.push_back(n7->vertex());
				vertices.push_back(n8->vertex());

				//{ {
				//		n1->vertex(),
				//		n2->vertex(),
				//		n3->vertex(),
				//		n4->vertex(),
				//		n5->vertex(),
				//		n6->vertex(),
				//		n7->vertex(),
				//		n8->vertex(),
				//	} };

				//dmc::marching_cubes<float>(vertices, receiver);

				runMarchingCubes(vertices, receiver);
			}
		}




		void enumerate_impl_e_xy(
			const DMCOctree::DMCOctreeNode* n1,
			const DMCOctree::DMCOctreeNode* n2,
			const DMCOctree::DMCOctreeNode* n3,
			const DMCOctree::DMCOctreeNode* n4,
			std::vector<Eigen::Vector3f>& receiver)
		{
			auto b1 = !n1->_IsLeaf;//dynamic_cast<const branch_node_type*>(&n1);
			auto b2 = !n2->_IsLeaf;//dynamic_cast<const branch_node_type*>(&n2);
			auto b3 = !n3->_IsLeaf;//dynamic_cast<const branch_node_type*>(&n3);
			auto b4 = !n4->_IsLeaf;//dynamic_cast<const branch_node_type*>(&n4);

			if (b1 || b2 || b3 || b4)
			{
				enumerate_impl_e_xy(
					b1 ? n1->children[3] : n1,
					b2 ? n2->children[2] : n2,
					b3 ? n3->children[1] : n3,
					b4 ? n4->children[0] : n4,
					receiver);

				enumerate_impl_e_xy(
					b1 ? n1->children[7] : n1,
					b2 ? n2->children[6] : n2,
					b3 ? n3->children[5] : n3,
					b4 ? n4->children[4] : n4,
					receiver);

				enumerate_impl_v(
					b1 ? n1->children[3] : n1,
					b2 ? n2->children[2] : n2,
					b3 ? n3->children[1] : n3,
					b4 ? n4->children[0] : n4,
					b1 ? n1->children[7] : n1,
					b2 ? n2->children[6] : n2,
					b3 ? n3->children[5] : n3,
					b4 ? n4->children[4] : n4,
					receiver);
			}
		}


		void enumerate_impl_e_yz(
			const DMCOctree::DMCOctreeNode* n1,
			const DMCOctree::DMCOctreeNode* n2,
			const DMCOctree::DMCOctreeNode* n3,
			const DMCOctree::DMCOctreeNode* n4,
			std::vector<Eigen::Vector3f>& receiver)
		{
			auto b1 = !n1->_IsLeaf;//dynamic_cast<const branch_node_type*>(&n1);
			auto b2 = !n2->_IsLeaf;//dynamic_cast<const branch_node_type*>(&n2);
			auto b3 = !n3->_IsLeaf;//dynamic_cast<const branch_node_type*>(&n3);
			auto b4 = !n4->_IsLeaf;//dynamic_cast<const branch_node_type*>(&n4);

			if (b1 || b2 || b3 || b4)
			{
				enumerate_impl_e_yz(
					b1 ? n1->children[6] : n1,
					b2 ? n2->children[4] : n2,
					b3 ? n3->children[2] : n3,
					b4 ? n4->children[0] : n4,
					receiver);

				enumerate_impl_e_yz(
					b1 ? n1->children[7] : n1,
					b2 ? n2->children[5] : n2,
					b3 ? n3->children[3] : n3,
					b4 ? n4->children[1] : n4,
					receiver);

				enumerate_impl_v(
					b1 ? n1->children[6] : n1,
					b1 ? n1->children[7] : n1,
					b2 ? n2->children[4] : n2,
					b2 ? n2->children[5] : n2,
					b3 ? n3->children[2] : n3,
					b3 ? n3->children[3] : n3,
					b4 ? n4->children[0] : n4,
					b4 ? n4->children[1] : n4,
					receiver);
			}
		}


		void enumerate_impl_e_xz(
			const DMCOctree::DMCOctreeNode* n1,
			const DMCOctree::DMCOctreeNode* n2,
			const DMCOctree::DMCOctreeNode* n3,
			const DMCOctree::DMCOctreeNode* n4,
			std::vector<Eigen::Vector3f>& receiver)
		{
			auto b1 = !n1->_IsLeaf;//dynamic_cast<const branch_node_type*>(&n1);
			auto b2 = !n2->_IsLeaf;//dynamic_cast<const branch_node_type*>(&n2);
			auto b3 = !n3->_IsLeaf;//dynamic_cast<const branch_node_type*>(&n3);
			auto b4 = !n4->_IsLeaf;//dynamic_cast<const branch_node_type*>(&n4);

			if (b1 || b2 || b3 || b4)
			{
				enumerate_impl_e_xz(
					b1 ? n1->children[5] : n1,
					b2 ? n2->children[4] : n2,
					b3 ? n3->children[1] : n3,
					b4 ? n4->children[0] : n4,
					receiver);

				enumerate_impl_e_xz(
					b1 ? n1->children[7] : n1,
					b2 ? n2->children[6] : n2,
					b3 ? n3->children[3] : n3,
					b4 ? n4->children[2] : n4,
					receiver);

				enumerate_impl_v(
					b1 ? n1->children[5] : n1,
					b2 ? n2->children[4] : n2,
					b1 ? n1->children[7] : n1,
					b2 ? n2->children[6] : n2,
					b3 ? n3->children[1] : n3,
					b4 ? n4->children[0] : n4,
					b3 ? n3->children[3] : n3,
					b4 ? n4->children[2] : n4,
					receiver);
			}
		}


		void enumerate_impl_f_x(const DMCOctree::DMCOctreeNode* n1, const DMCOctree::DMCOctreeNode* n2, std::vector<Eigen::Vector3f>& receiver)
		{
			auto b1 = !n1->_IsLeaf;//dynamic_cast<const branch_node_type*>(&n1);
			auto b2 = !n2->_IsLeaf;//dynamic_cast<const branch_node_type*>(&n2);



			if (b1 || b2)
			{
				enumerate_impl_f_x(b1 ? n1->children[1] : n1, b2 ? n2->children[0] : n2, receiver);
				enumerate_impl_f_x(b1 ? n1->children[3] : n1, b2 ? n2->children[2] : n2, receiver);
				enumerate_impl_f_x(b1 ? n1->children[5] : n1, b2 ? n2->children[4] : n2, receiver);
				enumerate_impl_f_x(b1 ? n1->children[7] : n1, b2 ? n2->children[6] : n2, receiver);

				enumerate_impl_e_xy(
					b1 ? n1->children[1] : n1,
					b2 ? n2->children[0] : n2,
					b1 ? n1->children[3] : n1,
					b2 ? n2->children[2] : n2,
					receiver);

				enumerate_impl_e_xy(
					b1 ? n1->children[5] : n1,
					b2 ? n2->children[4] : n2,
					b1 ? n1->children[7] : n1,
					b2 ? n2->children[6] : n2,
					receiver);

				enumerate_impl_e_xz(
					b1 ? n1->children[1] : n1,
					b2 ? n2->children[0] : n2,
					b1 ? n1->children[5] : n1,
					b2 ? n2->children[4] : n2,
					receiver);

				enumerate_impl_e_xz(
					b1 ? n1->children[3] : n1,
					b2 ? n2->children[2] : n2,
					b1 ? n1->children[7] : n1,
					b2 ? n2->children[6] : n2,
					receiver);

				enumerate_impl_v(
					b1 ? n1->children[1] : n1,
					b2 ? n2->children[0] : n2,
					b1 ? n1->children[3] : n1,
					b2 ? n2->children[2] : n2,
					b1 ? n1->children[5] : n1,
					b2 ? n2->children[4] : n2,
					b1 ? n1->children[7] : n1,
					b2 ? n2->children[6] : n2,
					receiver);
			}
		}


		void enumerate_impl_f_y(const DMCOctree::DMCOctreeNode* n1, const DMCOctree::DMCOctreeNode* n2, std::vector<Eigen::Vector3f>& receiver)
		{
			auto b1 = !n1->_IsLeaf;//dynamic_cast<const branch_node_type*>(&n1);
			auto b2 = !n2->_IsLeaf;//dynamic_cast<const branch_node_type*>(&n2);

			if (b1 || b2)
			{
				enumerate_impl_f_y(b1 ? n1->children[2] : n1, b2 ? n2->children[0] : n2, receiver);
				enumerate_impl_f_y(b1 ? n1->children[3] : n1, b2 ? n2->children[1] : n2, receiver);
				enumerate_impl_f_y(b1 ? n1->children[6] : n1, b2 ? n2->children[4] : n2, receiver);
				enumerate_impl_f_y(b1 ? n1->children[7] : n1, b2 ? n2->children[5] : n2, receiver);

				enumerate_impl_e_xy(
					b1 ? n1->children[2] : n1,
					b1 ? n1->children[3] : n1,
					b2 ? n2->children[0] : n2,
					b2 ? n2->children[1] : n2,
					receiver);

				enumerate_impl_e_xy(
					b1 ? n1->children[6] : n1,
					b1 ? n1->children[7] : n1,
					b2 ? n2->children[4] : n2,
					b2 ? n2->children[5] : n2,
					receiver);

				enumerate_impl_e_yz(
					b1 ? n1->children[3] : n1,
					b2 ? n2->children[1] : n2,
					b1 ? n1->children[7] : n1,
					b2 ? n2->children[5] : n2,
					receiver);

				enumerate_impl_e_yz(
					b1 ? n1->children[2] : n1,
					b2 ? n2->children[0] : n2,
					b1 ? n1->children[6] : n1,
					b2 ? n2->children[4] : n2,
					receiver);

				enumerate_impl_v(
					b1 ? n1->children[2] : n1,
					b1 ? n1->children[3] : n1,
					b2 ? n2->children[0] : n2,
					b2 ? n2->children[1] : n2,
					b1 ? n1->children[6] : n1,
					b1 ? n1->children[7] : n1,
					b2 ? n2->children[4] : n2,
					b2 ? n2->children[5] : n2,
					receiver);
			}
		}


		void enumerate_impl_f_z(const DMCOctree::DMCOctreeNode* n1, const DMCOctree::DMCOctreeNode* n2, std::vector<Eigen::Vector3f>& receiver)
		{
			auto b1 = !n1->_IsLeaf;//dynamic_cast<const branch_node_type*>(&n1);
			auto b2 = !n2->_IsLeaf; //dynamic_cast<const branch_node_type*>(&n2);

			if (b1 || b2)
			{
				enumerate_impl_f_z(b1 ? n1->children[4] : n1, b2 ? n2->children[0] : n2, receiver);
				enumerate_impl_f_z(b1 ? n1->children[5] : n1, b2 ? n2->children[1] : n2, receiver);
				enumerate_impl_f_z(b1 ? n1->children[6] : n1, b2 ? n2->children[2] : n2, receiver);
				enumerate_impl_f_z(b1 ? n1->children[7] : n1, b2 ? n2->children[3] : n2, receiver);

				enumerate_impl_e_xz(
					b1 ? n1->children[4] : n1,
					b1 ? n1->children[5] : n1,
					b2 ? n2->children[0] : n2,
					b2 ? n2->children[1] : n2,
					receiver);

				enumerate_impl_e_xz(
					b1 ? n1->children[6] : n1,
					b1 ? n1->children[7] : n1,
					b2 ? n2->children[2] : n2,
					b2 ? n2->children[3] : n2,
					receiver);

				enumerate_impl_e_yz(
					b1 ? n1->children[4] : n1,
					b1 ? n1->children[6] : n1,
					b2 ? n2->children[0] : n2,
					b2 ? n2->children[2] : n2,
					receiver);

				enumerate_impl_e_yz(
					b1 ? n1->children[5] : n1,
					b1 ? n1->children[7] : n1,
					b2 ? n2->children[1] : n2,
					b2 ? n2->children[3] : n2,
					receiver);

				enumerate_impl_v(
					b1 ? n1->children[4] : n1,
					b1 ? n1->children[5] : n1,
					b1 ? n1->children[6] : n1,
					b1 ? n1->children[7] : n1,
					b2 ? n2->children[0] : n2,
					b2 ? n2->children[1] : n2,
					b2 ? n2->children[2] : n2,
					b2 ? n2->children[3] : n2,
					receiver);
			}
		}


		
		void enumerate_impl_c(const DMCOctree::DMCOctreeNode* n, std::vector<Eigen::Vector3f>& receiver)
		{
			if (!n->_IsLeaf)//auto b = dynamic_cast<const branch_node_type*>(&n)
			{

				enumerate_impl_c(n->children[0], receiver);
				enumerate_impl_c(n->children[1], receiver);
				enumerate_impl_c(n->children[2], receiver);
				enumerate_impl_c(n->children[3], receiver);
				enumerate_impl_c(n->children[4], receiver);
				enumerate_impl_c(n->children[5], receiver);
				enumerate_impl_c(n->children[6], receiver);
				enumerate_impl_c(n->children[7], receiver);

				enumerate_impl_f_x(n->children[0], n->children[1], receiver);
				enumerate_impl_f_x(n->children[2], n->children[3], receiver);
				enumerate_impl_f_x(n->children[4], n->children[5], receiver);
				enumerate_impl_f_x(n->children[6], n->children[7], receiver);

				enumerate_impl_f_y(n->children[0], n->children[2], receiver);
				enumerate_impl_f_y(n->children[1], n->children[3], receiver);
				enumerate_impl_f_y(n->children[4], n->children[6], receiver);
				enumerate_impl_f_y(n->children[5], n->children[7], receiver);

				enumerate_impl_f_z(n->children[0], n->children[4], receiver);
				enumerate_impl_f_z(n->children[1], n->children[5], receiver);
				enumerate_impl_f_z(n->children[2], n->children[6], receiver);
				enumerate_impl_f_z(n->children[3], n->children[7], receiver);

				enumerate_impl_e_xy(n->children[0], n->children[1], n->children[2], n->children[3], receiver);
				enumerate_impl_e_xy(n->children[4], n->children[5], n->children[6], n->children[7], receiver);

				enumerate_impl_e_yz(n->children[0], n->children[2], n->children[4], n->children[6], receiver);
				enumerate_impl_e_yz(n->children[1], n->children[3], n->children[5], n->children[7], receiver);

				enumerate_impl_e_xz(n->children[0], n->children[1], n->children[4], n->children[5], receiver);
				enumerate_impl_e_xz(n->children[2], n->children[3], n->children[6], n->children[7], receiver);

				enumerate_impl_v(
					n->children[0],
					n->children[1],
					n->children[2],
					n->children[3],
					n->children[4],
					n->children[5],
					n->children[6],
					n->children[7],
					receiver);
			}
		}

		





		DMCOctree::DMCOctreeNode::DMCOctreeNode(float step)
		{
			_IsLeaf = false;

			_Step = step;

			for (int cc = 0; cc < 0; cc++)
			{
				children[cc] = 0;
			}
		}

		DMCVertex DMCOctree::DMCOctreeNode::vertex() const
		{
			return _SolvedVertex;
		}


		DualMarchingCubes::DualMarchingCubes(imt::volume::VolumeInfo& volume, int isoThreshold, std::vector<Eigen::Vector3f>& vertices) : _VolumeInfo(volume), _IsoThreshold(isoThreshold)
		{
			mMarchingCubesVertices = vertices;

			VolumeUtility utility;

			int64_t numMCVerties = vertices.size();

			utility.volumeGradient( vertices , volume , mVertexGradients );

			for (int64_t vv = 0; vv < numMCVerties; vv++)
			{
				mVertexGradients[vv].normalize();
			}


			_UniformSubdivisionDepth = 8;

			_QuadricEnergyThreshold = 0.25;
		}

		class TriangleCollector
		{


		public:

			void operator () (dmc::triangle<dmc::vector3f>& triangle)
			{
				_Triangles.push_back(triangle);
			}


		protected:

			std::vector<dmc::triangle<dmc::vector3f>> _Triangles;
		};


		void DualMarchingCubes::compute()
		{

			buildOctree(3);

			std::vector<Eigen::Vector3f> receiver;

			enumerate_impl_c( _AllNodes[0] , receiver);

			std::vector<unsigned int> indices(receiver.size());

			for (unsigned int id = 0; id < indices.size(); id++)
			{
				indices[id] = id;
			}

			tr::Display3DRoutines::displayPointSet(receiver, std::vector<Eigen::Vector3f>(receiver.size(), Eigen::Vector3f(1, 0, 0)));

			tr::Display3DRoutines::displayMesh(receiver, indices);

			int w = _VolumeInfo.mWidth;
			int h = _VolumeInfo.mHeight;
			int d = _VolumeInfo.mDepth;


			int32_t maxDim = std::max(w, h);
			maxDim = std::max(d, maxDim);

			int division = 4;

			int initLevelW = w % division == 0 ? w / division  : w / division + 1;
			int initLevelH = h % division == 0 ? h / division : h / division + 1;
			int initLevelD = d % division == 0 ? d / division : d / division + 1;

			int initLevelDim = maxDim % division == 0 ? maxDim / division : maxDim / division + 1;

			std::vector<DMCOctree::DMCOctreeNode*> initNodes( initLevelW * initLevelD * initLevelH , 0);

			std::vector<std::vector<int64_t>> nodePointIds(initLevelW * initLevelD * initLevelH);

#if 0
			float step = _VolumeInfo.mVoxelStep(0) * division;

			unsigned short *volumeData = (unsigned short*)_VolumeInfo.mVolumeData;

			for(int zz = 0 ; zz < initLevelD - 1; zz++)
				for(int yy = 0; yy < initLevelH - 1; yy++)
					for (int xx = 0; xx < initLevelW - 1; xx++)
					{
						int64_t id = initLevelW * initLevelH * zz + initLevelW * yy + xx;

						//check if each cell contains the surface

						bool outSideSurface = false, insideSurface = false;

						float xOffset = 0, yOffset = 0, zOffset = 0;

						for(int zt = 0; zt < 2; zt++)
						  for(int yt = 0; yt < 2; yt++)
							  for (int xt = 0; xt < 2; xt++)
							  {
								  double zCoords = ( (zz + zt) * step + zOffset ) / _VolumeInfo.mVoxelStep(0);
								  double yCoords = ( (yy + yt) * step + yOffset ) / _VolumeInfo.mVoxelStep(0);
								  double xCoords = ( (xx + xt) * step + xOffset ) / _VolumeInfo.mVoxelStep(0);

								  xCoords = std::max(0.1, xCoords);
								  xCoords = std::min(xCoords, w - 1.1);

								  yCoords = std::max(0.1, yCoords);
								  yCoords = std::min(yCoords, h - 1.1);

								  zCoords = std::max(0.1, zCoords);
								  zCoords = std::min(zCoords, d - 1.1);

								   double grayValue = valueAt(xCoords , yCoords , zCoords , _VolumeInfo.mWidth, _VolumeInfo.mHeight, volumeData);
							  
								   if (grayValue < _IsoThreshold)
								   {
									   outSideSurface = true;
								   }

								   if (grayValue >= _IsoThreshold)
								   {
									   insideSurface = true;
								   }
							  }
						
						if (outSideSurface && insideSurface)
						{
							initNodes[id] = new DMCOctree::DMCOctreeNode;

							double zCoords = (zz + 0.5) * step;
							double yCoords = (yy + 0.5) * step;
							double xCoords = (xx + 0.5) * step;

							initNodes[id]->_Step = division;
							initNodes[id]->_Center = Eigen::Vector3f(xCoords, yCoords, zCoords);
						}
					}



			int64_t numTotalNodes = initNodes.size();

			std::vector<Eigen::Vector3f> displayCenters;

			for ( int64_t nn = 0; nn < numTotalNodes; nn++ )
			{
				if (initNodes[nn])
				{
					displayCenters.push_back(initNodes[nn]->_Center);

					buildTree(initNodes[nn]);
				}
			}

			std::cout << "number of non empty nodes : " << displayCenters.size() << std::endl;

			tr::Display3DRoutines::displayPointSet(mOptimalPositions, std::vector<Eigen::Vector3f>(displayCenters.size(), Eigen::Vector3f(1, 0, 0)));

#else

			int64_t numSurfacePoints = mMarchingCubesVertices.size();

			std::vector<Eigen::Vector3f> pointNormals;

			float step = _VolumeInfo.mVoxelStep(0) * division;

			Eigen::Vector3f startPoint(_VolumeInfo.mVoxelStep(0) * 0.5, _VolumeInfo.mVoxelStep(1) * 0.5, _VolumeInfo.mVoxelStep(2) * 0.5);

			std::vector<int> initNodeIds(numSurfacePoints, -1);

			std::vector<Eigen::Vector3f> optimalPositions;


			for ( int64_t pp = 0; pp < numSurfacePoints; pp++ )
			{
				Eigen::Vector3f point = mMarchingCubesVertices[pp];

				Eigen::Vector3f cellCoords = ( point - startPoint) / step;

				int xi = cellCoords(0);
				int yi = cellCoords(1);
				int zi = cellCoords(2);

				int initNodeId = zi * initLevelW * initLevelH + yi * initLevelW + xi;
				
				initNodeIds[pp] = initNodeId;

				nodePointIds[initNodeId].push_back(pp);

				if (!initNodes[initNodeId])
				{
					initNodes[initNodeId] = new DMCOctree::DMCOctreeNode;
				}
			}

			//return;


			int64_t maxNumNodes = initLevelW * initLevelD * initLevelH;

			for ( int64_t nn = 0; nn < maxNumNodes; nn++ )
			{
				if ( initNodes[nn] )
				{
					int numNodePoints = nodePointIds[nn].size();

					std::vector<Eigen::Vector3f> nodePoints(numNodePoints);
					std::vector<Eigen::Vector3f> nodePointGradients(numNodePoints);

					for (int np = 0; np < numNodePoints; np++)
					{
						//if (nodePointIds[nn][np] >= mMarchingCubesVertices.size())
						//{
						//	std::cout << "bad point index " << std::endl;
						//}

						//mMarchingCubesVertices[nodePointIds[nn][np]];
						nodePoints[np] = mMarchingCubesVertices[nodePointIds[nn][np]];
						nodePointGradients[np] = mVertexGradients[nodePointIds[nn][np]];
					}


					//std::cout << " number of points : " << nodePoints.size() << std::endl;

					//tr::Display3DRoutines::displayPointSet(nodePoints, std::vector<Eigen::Vector3f>(nodePoints.size(), Eigen::Vector3f(1, 0, 0)));

					Eigen::Vector3f optimalPosition;

					double distance = 0;
					
					if ( computeOptimalPosition(nodePointIds[nn], optimalPosition , distance ) )
					{
						optimalPositions.push_back(optimalPosition);
					}

				}
			}


			std::vector<Eigen::Vector3f> surfaceVertices;

			enumerate_impl_c(initNodes[0], surfaceVertices);

			std::cout << "number of optimal positions : " << optimalPositions.size() << std::endl;

			tr::Display3DRoutines::displayPointSet(optimalPositions, std::vector<Eigen::Vector3f>(optimalPositions.size(), Eigen::Vector3f(1, 0, 0)));


#endif

		}


#define grayValue(x , y , z)  volumeData[ zStep * z + yStep * y + x ] 
		double DualMarchingCubes::valueAt(double x, double y, double z, unsigned int width, unsigned int height, unsigned short *volumeData)
		{
			unsigned short interpolatedValue = 0;

			size_t zStep = width * height;
			size_t yStep = width;

			int lx = (int)x;
			int ly = (int)y;
			int lz = (int)z;

			int ux = (int)std::ceil(x);
			int uy = (int)std::ceil(y);
			int uz = (int)std::ceil(z);

			double xV = x - lx;
			double yV = y - ly;
			double zV = z - lz;

			double c000 = grayValue(lx, ly, lz);
			double c100 = grayValue(ux, ly, lz);
			double c010 = grayValue(lx, uy, lz);
			double c110 = grayValue(ux, uy, lz);
			double c001 = grayValue(lx, ly, uz);
			double c101 = grayValue(ux, ly, uz);
			double c011 = grayValue(lx, uy, uz);
			double c111 = grayValue(ux, uy, uz);

			double interpolatedValF = c000 * (1.0 - xV) * (1.0 - yV) * (1.0 - zV) +
				c100 * xV * (1.0 - yV) * (1.0 - zV) +
				c010 * (1.0 - xV) * yV * (1.0 - zV) +
				c110 * xV * yV * (1.0 - zV) +
				c001 * (1.0 - xV) * (1.0 - yV) * zV +
				c101 * xV * (1.0 - yV) * zV +
				c011 * (1.0 - xV) * yV * zV +
				c111 * xV * yV * zV;

			return interpolatedValF;
		}



		Eigen::Vector3f DualMarchingCubes::gradientAt(double x, double y, double z, unsigned int width, unsigned int height, unsigned short *volumeData , float scale )
		{
			double xm = ( x - 0.5 * scale ) ;
			double xp = x +  0.5 * scale;

			double ym = y -  0.5 * scale;
			double yp = y +  0.5 * scale;

			double zm = z -  0.5 * scale;
			double zp = z + 0.5 * scale;

			double dx = valueAt(xp, y, z, width, height, volumeData) - valueAt(xm, y, z, width, height, volumeData);
			double dy = valueAt(x, yp, z, width, height, volumeData) - valueAt(x, ym, z, width, height, volumeData);
			double dz = valueAt(x, y, zp, width, height, volumeData) - valueAt(x, y, zm, width, height, volumeData);

			Eigen::Vector3f gradient(dx , dy , dz);

			return gradient;
		}


		void DualMarchingCubes::buildTree( DMCOctree::DMCOctreeNode *node )
		{

			unsigned short *volumeData = (unsigned short*)_VolumeInfo.mVolumeData;

			Eigen::Vector3f cellCenter = node->_Center;

			//check if the node needs subdivision
			std::vector<Eigen::Vector3f> corners(8);

			std::vector<Eigen::Vector3f> gradients(8);// we need gradients at each corner
			std::vector<float> grayValues(8);//gray values at each corner

			for( int zz = 0; zz < 2; zz++ )
				for( int yy = 0; yy < 2; yy++)
					for ( int xx = 0; xx < 2; xx++ )
					{
						float xCoord = (node->_Center(0) / _VolumeInfo.mVoxelStep(0) + node->_Step * xx);
						float yCoord = (node->_Center(1) / _VolumeInfo.mVoxelStep(1) + node->_Step * yy);
						float zCoord = (node->_Center(2) / _VolumeInfo.mVoxelStep(2) + node->_Step * zz);

						grayValues[zz * 4 + yy * 2 + xx] = valueAt(  xCoord , yCoord, zCoord, _VolumeInfo.mWidth ,
							                                         _VolumeInfo.mHeight , volumeData );

						gradients[zz * 4 + yy * 2 + xx] = gradientAt( xCoord, yCoord, zCoord, _VolumeInfo.mWidth, 
							                                          _VolumeInfo.mHeight, volumeData , node->_Step) / ( node->_Step * _VolumeInfo.mVoxelStep(0) );

						corners[zz * 4 + yy * 2 + xx] = Eigen::Vector3f(xCoord * _VolumeInfo.mVoxelStep(0), 
							                                            yCoord * _VolumeInfo.mVoxelStep(1), 
							                                            zCoord * _VolumeInfo.mVoxelStep(2));
					}

			double w = 0;// _IsoThreshold;// -grayValues;// _IsoThreshold;

			//we need to find the point inside the cell which minimizes the quadric energy
			std::vector<Eigen::Vector3f> li(8);
			std::vector<float> mi(8, 0) , si(8 , 0);

			for (int ii = 0; ii < 8; ii++)
			{
				//std::cout << grayValues[ii] << " ";

				double di = 1 + gradients[ii].squaredNorm();

				double ri = sqrt(di);

				double ni = w + gradients[ii].dot(corners[ii]);

				mi[ii] = ni / ri;

				li[ii] = gradients[ii] / ri;

				si[ii] = li[ii].dot(cellCenter);
			}

			//std::cout << std::endl;

			Eigen::Matrix3d A;
			Eigen::Vector3d b;

			A.setZero();
			b.setZero();

			for ( int ii = 0; ii < 3; ii++ )
			{
				for (int cc = 0; cc < 8; cc++)
				{
					A(ii, 0) += li[cc](0) * li[cc](ii);
					A(ii, 1) += li[cc](1) * li[cc](ii);
					A(ii, 2) += li[cc](2) * li[cc](ii);

					b(ii) += mi[cc] * li[cc](ii);
				}
			}

			
			Eigen::Vector3d optimalPosition = A.llt().solve(b);

			Eigen::Vector3f optimalPositionF = optimalPosition.cast<float>();

			//distance from center to optimal postion
			float dist = (cellCenter - optimalPositionF).norm();

			if ( dist > node->_Step * _VolumeInfo.mVoxelStep(0) * 2 || std::abs(A.determinant()) < 0.000001)
			{
				//unstable solution , just assign cell center as the solution
				optimalPositionF = cellCenter;
			}

			

			Eigen::Vector3f optimalPositionVoxel = optimalPositionF * _VolumeInfo.mVoxelStep(0);
			
			////gray value at the center
			auto offset = _IsoThreshold;//valueAt( optimalPositionVoxel.x() , optimalPositionVoxel.y() , optimalPositionVoxel.z() ,
				          //         _VolumeInfo.mWidth , _VolumeInfo.mHeight , volumeData );

			float error = 0;

			for (int i = 0; i < 8; ++i)
			{
				float val = gradients[i].dot(optimalPositionF - corners[i]);

			   error += val * val / (1 + gradients[i].squaredNorm() );
			}

			double errorTolerance = 0.02;

			if ( error > errorTolerance )
			{
				//we need to split the node

			}
			else
			{
				mOptimalPositions.push_back(optimalPositionF);
			}

			//std::cout << " error : " << error << std::endl;

		}


		bool DualMarchingCubes::computeOptimalPosition(std::vector<int64_t>& pointIndices, Eigen::Vector3f& optimalPosition, double &distance)
		{
			unsigned short *volumeData = (unsigned short*)_VolumeInfo.mVolumeData;

			Eigen::Vector3f cellCenter(0,0,0);

			double w = 0;// _IsoThreshold;// -grayValues;// _IsoThreshold;

			//we need to find the point inside the cell which minimizes the quadric energy
			int numVertices = pointIndices.size();
			std::vector<Eigen::Vector3f> li(numVertices);
			std::vector<float> mi(numVertices, 0), si(numVertices, 0);

			for (int ii = 0; ii < numVertices; ii++)
			{
				int vId = pointIndices[ii];
				cellCenter += mMarchingCubesVertices[vId];
			}

			cellCenter /= numVertices;

			for (int ii = 0; ii < numVertices; ii++)
			{
				//std::cout << grayValues[ii] << " ";

				int vId = pointIndices[ii];

				double di = 1 + mVertexGradients[vId].squaredNorm();

				double ri = sqrt(di);

				double ni = w + mVertexGradients[vId].dot(mMarchingCubesVertices[vId]);

				mi[ii] = ni / ri;

				li[ii] = mVertexGradients[vId] / ri;

				si[ii] = li[ii].dot(cellCenter);
			}

			//std::cout << std::endl;

			Eigen::Matrix3d A;
			Eigen::Vector3d b;

			A.setZero();
			b.setZero();

			for (int ii = 0; ii < 3; ii++)
			{
				for (int cc = 0; cc < 8; cc++)
				{
					A(ii, 0) += li[cc](0) * li[cc](ii);
					A(ii, 1) += li[cc](1) * li[cc](ii);
					A(ii, 2) += li[cc](2) * li[cc](ii);

					b(ii) += mi[cc] * li[cc](ii);
				}
			}


			Eigen::Vector3d sol = A.llt().solve(b);

			optimalPosition = sol.cast<float>();

			

			if ((cellCenter - optimalPosition).norm() > _VolumeInfo.mVoxelStep(0))
			{
				//std::cout << "number of vertices : " << numVertices << std::endl;

				optimalPosition = cellCenter;

				return false;
			}

			double energy = 0;

			distance = 0;

			Eigen::Vector3f averageDirection(0, 0, 0);

			double denom = 0;

			double d = 0;

			for (int vv = 0; vv < numVertices; vv++)
			{
				energy = std::max( (double)std::abs((optimalPosition - mMarchingCubesVertices[pointIndices[vv]]).dot(mVertexGradients[pointIndices[vv]])) , energy );

				d = (optimalPosition - mMarchingCubesVertices[pointIndices[vv]]).dot(mVertexGradients[pointIndices[vv]]);

				averageDirection += mVertexGradients[pointIndices[vv]] / d;
			
				denom += 1 / d;

				distance += d;
			}

			if (denom > std::numeric_limits<double>::epsilon())
			{
				averageDirection /= denom;
			}

			distance /= numVertices;

			if (distance > 0)
			{
				optimalPosition -= (averageDirection * distance * 1.01);

				distance = -0.01 * distance;
			}

			
			energy /= _VolumeInfo.mVoxelStep(0);

			return energy < _QuadricEnergyThreshold;
		}

		void DualMarchingCubes::buildOctree(int division)
		{
			DMCOctree::DMCOctreeNode *rootNode = new DMCOctree::DMCOctreeNode();

			_AllNodes.push_back(rootNode);

			size_t maxDim = std::max(_VolumeInfo.mWidth, _VolumeInfo.mHeight);
			maxDim = std::max(maxDim, _VolumeInfo.mDepth);

			Eigen::Vector3f min = Eigen::Vector3f(_VolumeInfo.mVoxelStep(0) * 0.5, _VolumeInfo.mVoxelStep(1) * 0.5, _VolumeInfo.mVoxelStep(2) * 0.5);
			Eigen::Vector3f max = min + _VolumeInfo.mVoxelStep * maxDim;

			int64_t numMarchingCubePoints = mMarchingCubesVertices.size();

			std::vector<int64_t> pointIds(numMarchingCubePoints);

			for (int64_t pp = 0; pp < numMarchingCubePoints; pp++)
			{
				pointIds[pp] = pp;
			}

			rootNode->sign = 0;
			rootNode->_SolvedVertex.sign = 0;
			rootNode->_SolvedVertex.offset = 0;
			rootNode->_IsLeaf = false;

			buildSubtree(rootNode, pointIds, min, max , 1 );

			std::cout << "number of total nodes : " << _AllNodes.size() << std::endl;

			std::vector<Eigen::Vector3f> reducedPoints;

			int64_t numTotalNodes = _AllNodes.size();

			int nReducedPoints = 0;

			for (int64_t nn = 0; nn < numTotalNodes; nn++)
			{
				if (_AllNodes[nn]->sign == 1)
				{
					reducedPoints.push_back(_AllNodes[nn]->_Center);
				
					nReducedPoints++;
				}
			}

			std::cout << "number of reduced points : " << nReducedPoints << std::endl;


			tr::Display3DRoutines::displayPointSet(reducedPoints, std::vector<Eigen::Vector3f>(reducedPoints.size(), Eigen::Vector3f(1, 0, 0)));

		}


		void DualMarchingCubes::buildSubtree(DMCOctree::DMCOctreeNode* node, std::vector<int64_t>& pointIds , const Eigen::Vector3f min, const Eigen::Vector3f& max , int depth)
		{
			//first the node should have eight children and if no point falls in any particular 
			//child node , we will terminate it by making it leaf
			std::vector< std::vector<int64_t> > nodePointIds(8);

			Eigen::Vector3f center = (min + max) * 0.5 - min;

			Eigen::Vector3f stepDiff = (max - min) * 0.5;

			for (int ii = 0; ii < 8; ii++)
			{
				node->children[ii] = new DMCOctree::DMCOctreeNode;

				node->children[ii]->_IsLeaf = false;

				_AllNodes.push_back(node->children[ii]);


			}

			unsigned short *volumeData = (unsigned short*)_VolumeInfo.mVolumeData;

			int64_t nPoints = pointIds.size();

			for (int64_t pp = 0; pp < nPoints; pp++)
			{
				Eigen::Vector3f diff = mMarchingCubesVertices[pointIds[pp]] - min;

				int xi = 0, yi = 0, zi = 0;

				if (diff(0) > center(0))
				{
					xi = 1;
				}

				if (diff(1) > center(1))
				{
					yi = 1;
				}

				if (diff(2) > center(2))
				{
					zi = 1;
				}

				int id = 4 * zi + 2 * yi + xi;

				nodePointIds[id].push_back(pointIds[pp]);
			}

			for(int zi = 0; zi < 2; zi++)
				for(int yi = 0; yi < 2; yi++)
					for (int xi = 0; xi < 2; xi++)
					{
						Eigen::Vector3f minPoint = min + Eigen::Vector3f( xi * stepDiff(0) , yi * stepDiff(1) , zi * stepDiff(2));
						Eigen::Vector3f maxPoint = min + Eigen::Vector3f( (xi + 1) * stepDiff(0), ( yi + 1) * stepDiff(1), ( zi + 1 ) * stepDiff(2));

						int nodeId = 4 * zi + 2 * yi + xi;

						//find an optimal distance from the points and assign the distance as sign dstance field
						double dist = FLT_MAX;

						Eigen::Vector3f center = (minPoint + maxPoint) * 0.5;

						if (nodePointIds[nodeId].size() > 0)
						{
							center = Eigen::Vector3f(0, 0, 0);

							for (auto pointId : nodePointIds[nodeId])
							{
								center += mMarchingCubesVertices[pointId];
							}

							center /= nodePointIds[nodeId].size();
						}
						

						for (int64_t pp = 0; pp < nPoints; pp++)
						{
							dist = std::min( (double)(center - mMarchingCubesVertices[pointIds[pp]]).norm() , dist);  //(center - mMarchingCubesVertices[pointIds[pp]]).dot(mVertexGradients[pointIds[pp]]);
						}

						//dist /= nPoints;

						Eigen::Vector3f gridCenter = center / _VolumeInfo.mVoxelStep(0);

						double grayValue = valueAt(gridCenter.x(), gridCenter.y(), gridCenter.z(), _VolumeInfo.mWidth, _VolumeInfo.mHeight, volumeData);

						if (grayValue > _IsoThreshold)
						{
							dist *= -1;
						}

						if ( nodePointIds[nodeId].size() == 0 )
						{
							node->children[nodeId]->_IsLeaf = true;
							node->children[nodeId]->sign = 0;//this node would be ignored while doing the triangulation
							node->children[nodeId]->_SolvedVertex.sign = 0;


							//if (depth > _UniformSubdivisionDepth - 2)
							//{
								node->children[nodeId]->_SolvedVertex.offset = dist;
								node->children[nodeId]->_SolvedVertex.position = center;
								node->children[nodeId]->_SolvedVertex.sign = dist;

							//}
							//else
							//{
							//	node->children[nodeId]->_SolvedVertex.offset = 0;
							//	node->children[nodeId]->_SolvedVertex.position = center;
							//	node->children[nodeId]->_SolvedVertex.sign = 0;

							//}
							
							continue;
						}

						//if we are already at target depth then we need check for quadric energy for further splitting or we make it as leaf
						// node
						Eigen::Vector3f optimalPosition = (minPoint + maxPoint) / 2;

						//Eigen::Vector3f center = optimalPosition;

						double distance = dist;

						/*if (depth >= _UniformSubdivisionDepth)
						{
							computeOptimalPosition(nodePointIds[nodeId], optimalPosition , distance);

							if (optimalPosition.hasNaN())
							{
								optimalPosition = center;
							}
						}*/
						
						// (center - mMarchingCubesVertices[pointIds[pp]]).
						
						double offset = distance;

						if (depth < _UniformSubdivisionDepth || std::abs(distance) > _VolumeInfo.mVoxelStep(0) * 0.1 )//|| !computeOptimalPosition(nodePointIds[nodeId], optimalPosition)
						{
							buildSubtree(node->children[nodeId], nodePointIds[nodeId], minPoint, maxPoint , depth + 1);
						}
						else 
						{
							node->children[nodeId]->_IsLeaf = true;
							node->children[nodeId]->_Center = optimalPosition;
							node->children[nodeId]->sign = offset;

							node->children[nodeId]->_SolvedVertex.position = optimalPosition;
							node->children[nodeId]->_SolvedVertex.sign = offset;
							node->children[nodeId]->_SolvedVertex.offset = offset;
						}
						
					}


		}



		DMCOctree::DMCOctree()
		{

		}

		void DMCOctree::build(VolumeInfo& volume, int divisions )
		{

		}




	}




}