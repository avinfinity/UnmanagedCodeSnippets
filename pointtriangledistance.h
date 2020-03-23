#ifndef __IMT_POINTTRAINGLEDISTANCE__
#define __IMT_POINTTRAINGLEDISTANCE__


#pragma once


#include "cmath"

namespace gte
{


	struct Triangle
	{
		float v0[3], v1[3], v2[3];
	};


//template <int N, typename float>
class DCPQuery //<float, Vector<N, float>, Triangle<N, float>>
{
public:
    struct Result
    {
        float distance, sqrDistance;
        float parameter[3];  // barycentric coordinates for triangle.v[3]
        float closest[3];//Vector<N, float> closest;
    };

    Result operator()(float const* point , Triangle const& triangle);

private:
    inline void GetMinEdge02(float const& a11, float const& b1, float p[2] ) const;

    inline void GetMinEdge12(float const& a01, float const& a11, float const& b1,
                             float const& f10, float const& f01, float p[2]) const; //Vector<2, float>&

	inline void GetMinInterior(const float p0[2], float const& h0, // Vector<2, float> const&
		const float p1[2], float const& h1, float p[2]) const;
};


void DCPQuery::GetMinEdge02( float const& a11, float const& b1, float p[2]) const //Vector<2, float>&
{
    p[0] = (float)0;
    if (b1 >= (float)0)
    {
        p[1] = (float)0;
    }
    else if (a11 + b1 <= (float)0)
    {
        p[1] = (float)1;
    }
    else
    {
        p[1] = -b1 / a11;
    }
}

void DCPQuery::GetMinEdge12(
    float const& a01, float const& a11, float const& b1, float const& f10,
    float const& f01, float p[2]) const
{
    float h0 = a01 + b1 - f10;
    if (h0 >= (float)0)
    {
        p[1] = (float)0;
    }
    else
    {
        float h1 = a11 + b1 - f01;
        if (h1 <= (float)0)
        {
            p[1] = (float)1;
        }
        else
        {
            p[1] = h0 / (h0 - h1);
        }
    }
    p[0] = (float)1 - p[1];
}


void DCPQuery::GetMinInterior(const float p0[2], float const& h0, // Vector<2, float> const&
	const float p1[2], float const& h1, float p[2]) const
{
    float z = h0 / (h0 - h1);
    p[0] = ((float)1 - z) * p0[0] + z * p1[0];
	p[1] = ((float)1 - z) * p0[1] + z * p1[1];
}


inline float Dot(float a[3], float b[3])
{
	return (a[0] * b[0] + a[1] * b[1] + a[2] * b[2]);
}

inline void Subtract( const float p1[3], const float p2[3], float p[3])
{
	p[0] = p1[0] - p2[0];
	p[1] = p1[1] - p2[1];
	p[2] = p1[2] - p2[2];
}

DCPQuery::Result
DCPQuery::operator()( const float  point[3], Triangle const& triangle ) //Vector<N, float> const&
{
    //Vector<N, float> diff = point - triangle.v[0];
    //Vector<N, float> edge0 = triangle.v[1] - triangle.v[0];
    //Vector<N, float> edge1 = triangle.v[2] - triangle.v[0];

	float edge0[3], edge1[3], diff[3];

	Subtract(point, triangle.v0, diff);
	Subtract(triangle.v1, triangle.v0, edge0);
	Subtract(triangle.v2, triangle.v0, edge1);

    float a00 = Dot(edge0, edge0);
    float a01 = Dot(edge0, edge1);
    float a11 = Dot(edge1, edge1);
    float b0 = -Dot(diff, edge0);
    float b1 = -Dot(diff, edge1);

    float f00 = b0;
    float f10 = b0 + a00;
    float f01 = b0 + a01;

    //Vector<2, float> p0, p1, p;
    
	float p0[2], p1[2], p[2];
	
	float dt1, h0, h1;

    // Compute the endpoints p0 and p1 of the segment.  The segment is
    // parameterized by L(z) = (1-z)*p0 + z*p1 for z in [0,1] and the
    // directional derivative of half the quadratic on the segment is
    // H(z) = Dot(p1-p0,gradient[Q](L(z))/2), where gradient[Q]/2 = (F,G).
    // By design, F(L(z)) = 0 for cases (2), (4), (5), and (6).  Cases (1) and
    // (3) can correspond to no-intersection or intersection of F = 0 with the
    // triangle.
    if (f00 >= (float)0)
    {
        if (f01 >= (float)0)
        {
            // (1) p0 = (0,0), p1 = (0,1), H(z) = G(L(z))
            GetMinEdge02(a11, b1, p);
        }
        else
        {
            // (2) p0 = (0,t10), p1 = (t01,1-t01), H(z) = (t11 - t10)*G(L(z))
            p0[0] = (float)0;
            p0[1] = f00 / (f00 - f01);
            p1[0] = f01 / (f01 - f10);
            p1[1] = (float)1 - p1[0];
            dt1 = p1[1] - p0[1];
            h0 = dt1 * (a11 * p0[1] + b1);
            if (h0 >= (float)0)
            {
                GetMinEdge02(a11, b1, p);
            }
            else
            {
                h1 = dt1 * (a01 * p1[0] + a11 * p1[1] + b1);
                if (h1 <= (float)0)
                {
                    GetMinEdge12(a01, a11, b1, f10, f01, p);
                }
                else
                {
                    GetMinInterior(p0, h0, p1, h1, p);
                }
            }
        }
    }
    else if (f01 <= (float)0)
    {
        if (f10 <= (float)0)
        {
            // (3) p0 = (1,0), p1 = (0,1), H(z) = G(L(z)) - F(L(z))
            GetMinEdge12(a01, a11, b1, f10, f01, p);
        }
        else
        {
            // (4) p0 = (t00,0), p1 = (t01,1-t01), H(z) = t11*G(L(z))
            p0[0] = f00 / (f00 - f10);
            p0[1] = (float)0;
            p1[0] = f01 / (f01 - f10);
            p1[1] = (float)1 - p1[0];
            h0 = p1[1] * (a01 * p0[0] + b1);
            if (h0 >= (float)0)
            {
                //p = p0;  // GetMinEdge01

				p[0] = p0[0];
				p[1] = p0[1];
            }
            else
            {
                h1 = p1[1] * (a01 * p1[0] + a11 * p1[1] + b1);
                if (h1 <= (float)0)
                {
                    GetMinEdge12(a01, a11, b1, f10, f01, p);
                }
                else
                {
                    GetMinInterior(p0, h0, p1, h1, p);
                }
            }
        }
    }
    else if (f10 <= (float)0)
    {
        // (5) p0 = (0,t10), p1 = (t01,1-t01), H(z) = (t11 - t10)*G(L(z))
        p0[0] = (float)0;
        p0[1] = f00 / (f00 - f01);
        p1[0] = f01 / (f01 - f10);
        p1[1] = (float)1 - p1[0];
        dt1 = p1[1] - p0[1];
        h0 = dt1 * (a11 * p0[1] + b1);
        if (h0 >= (float)0)
        {
            GetMinEdge02(a11, b1, p);
        }
        else
        {
            h1 = dt1 * (a01 * p1[0] + a11 * p1[1] + b1);
            if (h1 <= (float)0)
            {
                GetMinEdge12(a01, a11, b1, f10, f01, p);
            }
            else
            {
                GetMinInterior(p0, h0, p1, h1, p);
            }
        }
    }
    else
    {
        // (6) p0 = (t00,0), p1 = (0,t11), H(z) = t11*G(L(z))
        p0[0] = f00 / (f00 - f10);
        p0[1] = (float)0;
        p1[0] = (float)0;
        p1[1] = f00 / (f00 - f01);
        h0 = p1[1] * (a01 * p0[0] + b1);
        if (h0 >= (float)0)
        {
            //p = p0;  // GetMinEdge01

			p[0] = p0[0];
			p[1] = p0[1];
        }
        else
        {
            h1 = p1[1] * (a11 * p1[1] + b1);
            if (h1 <= (float)0)
            {
                GetMinEdge02(a11, b1, p);
            }
            else
            {
                GetMinInterior(p0, h0, p1, h1, p);
            }
        }
    }

    Result result;
    result.parameter[0] = (float)1 - p[0] - p[1];
    result.parameter[1] = p[0];
    result.parameter[2] = p[1];

	for (int ii = 0; ii < 3; ii++)
	{
		result.closest[ii] = triangle.v0[ii] + p[0] * edge0[ii] + p[1] * edge1[ii];
		diff[ii] = point[ii] - result.closest[ii];
	}

    result.sqrDistance = Dot(diff, diff);
    result.distance = sqrt(result.sqrDistance);
    return result;
}


}





#endif