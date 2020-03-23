#include "planeslicercpu.h"
#include "eigenincludes.h"
#include "display3droutines.h"
#include "omp.h"
#include <vtkIdList.h>

void PlaneSlicerCPU::setMeshData(vtkSmartPointer< vtkPolyData > mesh)
{

	int nTris = mesh->GetNumberOfCells();

	mTriangles.resize(9 * nTris);

	vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();

	for (int tt = 0; tt < nTris; tt++)
	{
		mesh->GetCellPoints(tt, triangle);

		double p1[3], p2[3], p3[3];

		mesh->GetPoint(triangle->GetId(0), p1);
		mesh->GetPoint(triangle->GetId(1), p2);
		mesh->GetPoint(triangle->GetId(2), p3);


		mTriangles[9 * tt] = p1[0];
		mTriangles[9 * tt + 1] = p1[1];
		mTriangles[9 * tt + 2] = p1[2];

		mTriangles[9 * tt + 3] = p2[0];
		mTriangles[9 * tt + 4] = p2[1];
		mTriangles[9 * tt + 5] = p2[2];

		mTriangles[9 * tt + 6] = p3[0];
		mTriangles[9 * tt + 7] = p3[1];
		mTriangles[9 * tt + 8] = p3[2];

	}

}


float dot3( float* a1, float* a2)
{
	return ( a1[0] * a2[0] + a1[1] * a2[1] + a1[2] * a2[2] );
}


void PlaneSlicerCPU::computeSlice(float *position, float *normal, std::vector< float >& edges)
{

	int nTris = mTriangles.size() / 9;

	int nIntersections = 0;

	float *t3 = mTriangles.data();

	edges.reserve( nTris );

#pragma omp parallel for
	for ( int tt = 0; tt < nTris; tt++ )
	{
		int p = 0, n = 0;

		float *t3 = mTriangles.data() + 9 * tt ;

		float d1[3], d2[3], d3[3];

		d1[0] = position[0] - t3[0];
		d1[1] = position[1] - t3[1];
		d1[2] = position[2] - t3[2];

		d2[0] = position[0] - t3[3];
		d2[1] = position[1] - t3[4];
		d2[2] = position[2] - t3[5];

		d3[0] = position[0] - t3[6];
		d3[1] = position[1] - t3[7];
		d3[2] = position[2] - t3[8];
		
			
	    float f1 = dot3( d1 , normal );
		float f2 = dot3( d2 , normal );
		float f3 = dot3( d3 , normal );

		p += f1 > 0;
		p += f2 > 0;
		p += f3 > 0;

		n += f1 < 0;
		n += f2 < 0;
		n += f3 < 0;

		float l1[3], l2[3], l3[3];

		l1[0] = t3[3] - t3[0];
		l1[1] = t3[4] - t3[1];
		l1[2] = t3[5] - t3[2];

		l2[0] = t3[6] - t3[3];
		l2[1] = t3[7] - t3[4];
		l2[2] = t3[8] - t3[5];

		l3[0] = t3[0] - t3[6];
		l3[1] = t3[1] - t3[7];
		l3[2] = t3[2] - t3[8];



		if (p < 3 && p > 0)
		{
			float lambda1 = 1 - f2 / dot3(l1, normal);
			float lambda2 =  1 - f3 / dot3(l2, normal);
			float lambda3 = 1 - f1 / dot3(l3, normal);

			bool p1Found = false, p2Found = false;

			float p1[3], p2[3];

			if ( lambda1 >= 0 && lambda1 <= 1 )
			{
				p1[0] = lambda1 * t3[0] + (1 - lambda1) * t3[3];
				p1[1] = lambda1 * t3[1] + (1 - lambda1) * t3[4];
				p1[2] = lambda1 * t3[2] + (1 - lambda1) * t3[5];

				p1Found = true;
			}

			if (lambda2 >= 0 && lambda2 <= 1)
			{
				if (!p1Found)
				{
					p1[0] = lambda2 * t3[3] + (1 - lambda2) * t3[6];
					p1[1] = lambda2 * t3[4] + (1 - lambda2) * t3[7];
					p1[2] = lambda2 * t3[5] + (1 - lambda2) * t3[8];

					p1Found = true;

				}
				else
				{
					p2[0] = lambda2 * t3[3] + (1 - lambda2) * t3[6];
					p2[1] = lambda2 * t3[4] + (1 - lambda2) * t3[7];
					p2[2] = lambda2 * t3[5] + (1 - lambda2) * t3[8];

					p2Found = true;
				}
			}

			if (lambda3 >= 0 && lambda3 <= 1)
			{
				if ( !p2Found )
				{
					p2[0] = lambda3 * t3[6] + (1 - lambda3) * t3[0];
					p2[1] = lambda3 * t3[7] + (1 - lambda3) * t3[1];
					p2[2] = lambda3 * t3[8] + (1 - lambda3) * t3[2];

					p2Found = true;
				}
			}

#pragma omp critical
			{
				edges.push_back( p1[0] );
				edges.push_back( p1[1] );
				edges.push_back( p1[2] );

				edges.push_back( p2[0] );
				edges.push_back( p2[1] );
				edges.push_back( p2[2] );
			}

		}


	}


}

