#ifndef __IMT_IMPLICITSURFACEGENERATOR_H__
#define __IMT_IMPLICITSURFACEGENERATOR_H__

#include "eigenincludes.h"
#include "vector"

namespace imt
{
	namespace volume{


	class ImplicitFunction
	{

	   public:

		   ImplicitFunction();

		   short value( int x , int y , int z );

		   short isoValue();

		   void setDimensions(int w, int h, int d);

		   Eigen::Vector3f getNormal( const Eigen::Vector3f& point );

	protected:

		int mDepth, mHeight, mWidth;


	};
		
     
	 
	 class ImplicitSurfaceGenerator
	 {
		 
		public:

        ImplicitSurfaceGenerator();		

		void setImplicitFunction( ImplicitFunction *imf );

		void generate(int sizeX, int sizeY, int sizeZ);

		void getSurface( std::vector< Eigen::Vector3f >& vertices , std::vector< unsigned int >& indices );


	 protected:

		 ImplicitFunction *mImplicitFunction;


		 
		 
	 };
		
	}
	
}


#endif