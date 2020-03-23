#include "implicitsurfacegenerator.h"

namespace imt{
	
	namespace volume{


	ImplicitFunction::ImplicitFunction()
	{


	}


	short ImplicitFunction::value(int x, int y, int z)
	{
		int coeff = 1;

		mWidth = 512 * coeff;
		mHeight = 512 * coeff;
		mDepth = 512 * coeff;


	   int center = mWidth / 2;

	   int r = (128 + 64 + 32) * coeff, r1 = 64 * coeff, r2 = 64 * coeff;

	   short v = 0;

	   if (z > mDepth - 20 * coeff || z < 20 * coeff)
	   {
		   return v;
	   }

	   int val = 0 ;
	   int iso = 5000;

	   float x1 = 128 * coeff, y1 = 256 * coeff, x2 = (256 + 128) * coeff, y2 = 256 * coeff;

	   Eigen::Vector2f centerPoint(256 * coeff, 256 * coeff);

	   Eigen::Vector2f centerPoint1(x1, y1), centerPoint2(x2, y2);

	   Eigen::Vector2f pt( x , y );

	   float d = (centerPoint - pt).norm();
       float d1 = (centerPoint1 - centerPoint).norm() - r1;

	   float d2 = (centerPoint1 - pt).norm();
	   float d3 = (centerPoint2 - pt).norm();

	   float rd = (d1 + 2 * r1 + +r) / 2;

	   if ( d <= d1 && d2 < r1 )
	   {
		   val = iso + 4 * (d1 - d);
	   }
	   else if (  d2 > r1 && d3 > r1  && d < rd )
	   {
		   val = iso + 4 * std::min( d2 , d3 );
	   }
	   else if ( d >= rd && d <= r )
	   {
		   val = iso + 4 * (r - d);
	   }
	   else
	   {
		   val = 0;
	   }

	   v = val;


	   return v;
	}


	Eigen::Vector3f ImplicitFunction::getNormal( const Eigen::Vector3f& point )
	{
		Eigen::Vector3f n;

		float x1 = 128, y1 = 256, x2 = 256 + 128, y2 = 256;

		Eigen::Vector2f centerPoint(256, 256);

		Eigen::Vector2f centerPoint1(x1, y1), centerPoint2(x2, y2);

		float zDiff1 = std::abs( point.z() - mDepth + 20 );
		float zDiff2 = std::abs( point.z() - 20 );

		Eigen::Vector2f pt(point(0), point(1));

		int r = 128 + 64 + 32, r1 = 64, r2 = 64;

		float d = (centerPoint - pt).norm();
		float d1 = (centerPoint1 - centerPoint).norm() - r1;

		float d2 = (centerPoint1 - pt).norm();
		float d3 = (centerPoint2 - pt).norm();

		if ( zDiff1 < 2 || zDiff1 < 2 )
		{
           
		}


		return n;

	}


	short ImplicitFunction::isoValue()
	{
	   short iv = 0;



	   return iv;
	}

		
		
	ImplicitSurfaceGenerator::ImplicitSurfaceGenerator()
	{
		
	}	



	void ImplicitSurfaceGenerator::setImplicitFunction( ImplicitFunction *imf )
	{
		mImplicitFunction = imf;
	}

	void ImplicitSurfaceGenerator::generate( int sizeX, int sizeY, int sizeZ )
	{
		ImplicitFunction imf;



	}

	void ImplicitSurfaceGenerator::getSurface( std::vector< Eigen::Vector3f >& vertices, std::vector< unsigned int >& indices )
	{

	}


		
		
	}
	
}