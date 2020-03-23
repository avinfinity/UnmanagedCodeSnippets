#ifndef __IMT_VOLUME_HALFEDGEMESH_H__
#define __IMT_VOLUME_HALFEDGEMESH_H__

#include <vector>

namespace imt {

	namespace volume {


		class HalfEdgeMesh {


		public:

			struct HalfEdge 
			{
				int32_t  mSource , mTarget, mOpposite, mNext;

				HalfEdge();
			};


			HalfEdgeMesh(std::vector<unsigned int>& surfaceIndices, std::vector<double>& vertices,
				std::vector<double>& vertexNormals);

			void initialize();



		protected:

			void fixFaceOrientations();



		protected:



			std::vector<unsigned int>& mSurfaceIndices;
			int64_t mNumVertices;

			std::vector<double> &mVertices, &mVertexNormals;

			std::vector< HalfEdge > mHalfEdges;

		};



	}



}





#endif