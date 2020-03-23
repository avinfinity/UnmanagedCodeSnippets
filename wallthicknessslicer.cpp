#include "wallthicknessslicer.h"


namespace imt{

	namespace volume{


		WallThicknessSlicer::WallThicknessSlicer()
		{

		}

		void WallThicknessSlicer::computeSlice( float* vertices, unsigned int numTriangles   )
		{

		


		}


		void WallThicknessSlicer::initOptixPrime()
		{
			//RTPcontexttype contextType = RTP_CONTEXT_TYPE_CUDA;

			//rtpContextCreate( contextType , &_Context );


			//// Create buffers for geometry data 
			//rtpBufferDescCreate( _Context ,
			//	                 RTP_BUFFER_FORMAT_INDICES_INT3,
			//	                 RTP_BUFFER_TYPE_CUDA_LINEAR,
			//	                 _Faces, &_IndicesDesc );


			//rtpBufferDescCreate( _Context,
			//	                 RTP_BUFFER_FORMAT_VERTEX_FLOAT3,
			//	                 RTP_BUFFER_TYPE_CUDA_LINEAR,
			//	                _Vertices , &_VerticesDesc );


			//rtpModelCreate( _Context, &_Model);
		}


		void WallThicknessSlicer::computeOppositeEnds()
		{
			//initialize optix
			//RTPcontexttype contextType = RTP_CONTEXT_TYPE_CUDA;
			//RTPbuffertype bufferType = RTP_BUFFER_TYPE_CUDA_LINEAR;

			//rtpBufferDescSetRange( _IndicesDesc, 0, _NumFaces );
			//rtpBufferDescSetRange( _VerticesDesc , 0 , _NumVertices );

			//rtpModelSetTriangles( _Model, _IndicesDesc, _VerticesDesc );
			//rtpModelUpdate( _Model , 0 );


			//compute the opposite ends


		}




	}




}