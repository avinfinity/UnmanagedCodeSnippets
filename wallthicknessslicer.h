#ifndef __IMT_WALLTHICKNESSSLICER_H__
#define __IMT_WALLTHICKNESSSLICER_H__

//#include "optixu/optixu.h"
//#include "optixu/optixpp_namespace.h"
//#include "optix_prime/optix_prime.h"

namespace imt{
	
	namespace volume{
		
		
		class WallThicknessSlicer
		{
			
			public:
			
			  WallThicknessSlicer();
			

			  void computeSlice( float* vertices , unsigned int numTriangles );
			

		protected:

			void computeOppositeEnds( );

			void initOptixPrime();


		protected:

			float *_Vertices;

			float *_IntersectedVertexBuffer;

			float *_IntersectedTriangles;

			unsigned int *_Faces;

			int _NumFaces , _NumVertices;

			//RTPcontext _Context;
			//RTPbufferdesc _IndicesDesc, _VerticesDesc;
			//RTPmodel _Model;
		};
		
		
		
		
		
		
	}
	
	
	
	
	
}





#endif