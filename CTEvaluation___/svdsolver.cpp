// svdsolver.cpp
//	Gehört zur VMClasses-Bibliothek für Vector/Matrix-Rechnung
//	Ursprünglicher Autor D.Görsch - goersch.at.zeiss-izm.de
//  Singular Value Decomposition
//  zur Berechnung Eigenwertaufgaben


#include "svdsolver.h"
#include "utils.h"
#include <limits>
#include <stdio.h>
#include <stdarg.h>

#undef max
#undef min

//extern void ZiTrace( LPCWSTR format, ...);

//
//
namespace Izm 
{
namespace Numerics
{
namespace Solvers
{

namespace 
{
	int _svd(int nm, int m, int n, Matrix& A, Vector& w, bool compute_U, Matrix& U, bool compute_V, Matrix& V, int& error, Vector& temp);
}


void SVDSolver::Solve(Matrix& A, const Vector& b, Vector& x, bool doSVD)
{
	const int m = A.xDim();
	const int n = A.yDim();

	Vector s(n), y(n);
	Matrix V(n,n), U(m,m);

	SVD(A,U,s,V);
	if (_Error != 0) return;
	
	// Compute y = U^T b / s
	for (int i = 1; i <= n; ++i)
	{
		if (s(i) > _RegTolerance)
		{
			double h = 0.0;

			for (int j = 1; j <= m; ++j)
			{
				h += U(j,i)*b(j);
			}

			y(i) = h / s(i);
		}
	}

	x = V*y;
}

//void SVDSolver::Solve(Matrix& A, Vector& x)
//{
//	const int m = A.xDim();
//	const int n = A.yDim();
//
//	Matrix S(n,n);
//	Matrix V(n,n);
//
//	SVD(A,S,V);
//
//	if (_Error == 0) 
//	{
//		int iMin = 0;
//		double eMin = std::numeric_limits<double>::max();
//
//		for (int i = 1; i <= n; ++i) 
//		{
//			if (eMin > Numerics::abs(S(i,i))) 
//			{
//				iMin = i;
//				eMin = Numerics::abs(S(i,i));
//			}
//		}
//
//		for (int i = 1; i <= n; ++i) x(i) = V(i,iMin);
//	}
//}

void SVDSolver::SVD(Matrix& A, Vector& s)
{
	_Error = 0;

	const int m = A.xDim();
	const int n = A.yDim();
	const int nm = Numerics::max(m,n);

	Vector temp(nm);
	Matrix U(m,m), V(n,n);

	_svd(nm,m,n,A,s,false,U,false,V,_Error,temp);
}

void SVDSolver::SVD(Matrix& A, Matrix& S)
{
	_Error = 0;

	const int m = A.xDim();
	const int n = A.yDim();
	const int nm = Numerics::max(m,n);

	Vector temp(nm), w(n);
	Matrix U(m,m), V(n,n);

	_svd(nm,m,n,A,w,false,U,false,V,_Error,temp);

	S.clear();
	for (int i = 1; i <= n; ++i) S(i,i) = w(i);
}

void SVDSolver::SVD(Matrix& A, Matrix& S, Matrix& V)
{
	_Error = 0;

	const int m = A.xDim();
	const int n = A.yDim();
	const int nm = Numerics::max(m,n);

	Vector temp(nm), w(n);
	Matrix U(m,m);

	_svd(nm,m,n,A,w,false,U,true,V,_Error,temp);

	S.clear();
	for (int i = 1; i <= n; ++i) S(i,i) = w(i);
}

void SVDSolver::SVD(Matrix& A, Vector& s, Matrix& V)
{
	_Error = 0;

	const int m = A.xDim();
	const int n = A.yDim();
	const int nm = Numerics::max(m,n);

	Vector temp(nm);
	Matrix U(m,m);
	_svd(nm,m,n,A,s,false,U,true,V,_Error,temp);
}

void SVDSolver::SVD(Matrix& A, Matrix& U, Matrix& S, Matrix& V)
{
	_Error = 0;

	const int m = A.xDim();
	const int n = A.yDim();
	const int nm = Numerics::max(m,n);

	Vector temp(nm), w(n);

	_svd(nm,m,n,A,w,true,U,true,V,_Error,temp);

	S.clear();
	for (int i = 1; i <= n; ++i) S(i,i) = w(i);
}

void SVDSolver::SVD(Matrix& A, Matrix& U, Vector& s, Matrix& V)
{
	_Error = 0;

	const int m = A.xDim();
	const int n = A.yDim();
	const int nm = Numerics::max(m,n);

	Vector temp(nm);

	_svd(nm,m,n,A,s,true,U,true,V,_Error,temp);
}

namespace 
{

double dsign(double a, double b)
{
    double x = (a >= 0.0 ? a : -a);
    return( b >= 0.0 ? x : -x);
}

double pythag(double a, double b)
{
    double result, d1, d2, d3;
    double p, r, s, t, u;

// finds dsqrt(a**2+b**2) without overflow or destructive underflow

// Computing MAX 

    d1 = Numerics::abs(a);
	d2 = Numerics::abs(b);

	p = Numerics::max(d1,d2);

	if( Numerics::lessEqual(p, 0.0)) goto L20;

// Computing MIN

    d2 = Numerics::abs(a);
	d3 = Numerics::abs(b);

// Computing 2nd power

    d1 = Numerics::min(d2,d3) / p;
    r = d1 * d1;

L10:

    t = r + 4.0;

	if (Numerics::equal(t,4.0)) goto L20;

    s = r / t;
    u = s * 2.0 + 1.0;
    p = u * p;

// Computing 2nd power 

    d1 = s / u;
    r  = d1 * d1 * r;

    goto L10;

L20:

    result = p;

    return result;
}

int _svd(int nm, int m, int n, Matrix& A, Vector& w, bool matu, Matrix& U, bool matv, Matrix& V, int& ierr, Vector& rv1)
{

/*
     This subroutine is a translation of the algol procedure svd,
     num. math. 14, 403-420(1970) by golub and reinsch.
     handbook for auto. comp., vol ii-linear algebra, 134-151(1971).

     This subroutine determines the singular value decomposition

     A = U*S*V'  of a real m by n rectangular matrix.  Householder
     bidiagonalization and a variant of the qr algorithm are used.
     on input.

		nm must be set to the row dimension of two-dimensional
          array parameters as declared in the calling program
          dimension statement.  note that nm must be at least
          as large as the maximum of m and n.

        m is the number of rows of A (and U).

        n is the number of columns of A (and U) and the order of V.

        A contains the rectangular input matrix to be decomposed.

        matu should be set to .true. if the u matrix in the
          decomposition is desired, and to .false. otherwise.

        matv should be set to .true. if the v matrix in the
          decomposition is desired, and to .false. otherwise.

     on output

        A is unaltered (unless overwritten by U or V).

        w contains the n (non-negative) singular values of a (the
          diagonal elements of s).  they are unordered.  if an
          error exit is made, the singular values should be correct
          for indices ierr+1,ierr+2,...,n.

        U contains the matrix u (orthogonal column vectors) of the
          decomposition if matu has been set to true.  Otherwise
          U is used as a temporary array.  U may coincide with A.
          If an error exit is made, the columns of U corresponding
          to indices of correct singular values should be correct.

        V contains the matrix v (orthogonal) of the decomposition if
          matv has been set to true.  Otherwise V is not referenced.
          V may also coincide with A if U is not needed.  If an error
          exit is made, the columns of V corresponding to indices of
          correct singular values should be correct.

        ierr is set to
          zero       for normal return,
          k          if the k-th singular value has not been
                     determined after 30 iterations.

        rv1 is a temporary storage array.

     calls pythag for  dsqrt(a*a + b*b) .
     
	 questions and comments should be directed to burton s. garbow,
     mathematics and computer science div, argonne national laboratory

     this version dated august 1983.

*/

	double d1, d2, d3, d4;

    double c, f, g, h;
    int i, j, k, l;
    double s, x, y, z, scale;
    int i1, k1, l1, ii, kk, ll, mn;
    int its;
    double tst1, tst2;

    // Function Body

    ierr = 0;

    for (i = 1; i <= m; ++i) 
	{
		for (j = 1; j <= n; ++j)
		{
	    	U(i,j) = A(i,j);
		}
    }

//     .......... householder reduction to bidiagonal form .......... 

    g = 0.0;
    scale = 0.0;
    x = 0.0;

    for (i = 1; i <= n; ++i) 
	{
		l = i + 1;

		rv1(i) = scale * g;
		g = 0.0;
		s = 0.0;
		scale = 0.0;

		if (i > m) goto L210;

		for (k = i; k <= m; ++k) 
		{
			d1 = U(k,i);
		    scale += Numerics::abs(d1);
		}

		if(Numerics::equal(scale, 0.0,1e-12)) goto L210;

		for (k = i; k <= m; ++k) 
		{
		    U(k,i) = U(k,i) / scale;

	  		d1 = U(k,i);
	    	s += d1 * d1;
		}

		f = U(i,i);
		d1 = sqrt(s);
		g = -dsign(d1,f);
		h = f * g - s;
		U(i,i) = f - g;

		if (i == n) goto L190;

		for (j = l; j <= n; ++j) 
		{
		    s = 0.0;

		    for (k = i; k <= m; ++k)
			{
				s += U(k,i) * U(k,j);
	    	}

	    	f = s / h;

		    for (k = i; k <= m; ++k) 
			{
				U(k, j) = U(k, j) + f * U(k, i);
		    }
		}

L190:

		for (k = i; k <= m; ++k)
		{
		    U(k,i) = scale * U(k,i);
		}

L210:

		w(i) = scale * g;
		
		g = 0.0;
		s = 0.0;
		scale = 0.0;
	
		if (i > m || i == n) goto L290;

		for (k = l; k <= n; ++k) 
		{
			d1 = U(i,k);
		    scale += Numerics::abs(d1);
		}

		if(Numerics::equal(scale, 0.0,1e-12)) goto L290;

		for (k = l; k <= n; ++k)
		{
		    U(i,k) = U(i,k) / scale;

	    	d1 = U(i,k);
	    	s += d1 * d1;
		}

		f = U(i,l);
		d1 = sqrt(s);
		g = -dsign(d1,f);
		h = f * g - s;
		U(i, l) = f - g;

		for (k = l; k <= n; ++k) 
		{
		    rv1(k) = U(i,k) / h;
		}

		if (i == m) goto L270;


		for (j = l; j <= m; ++j) 
		{
		    s = 0.0;

		    for (k = l; k <= n; ++k) 
			{
				s += U(j,k) * U(i,k);
		    }

		    for (k = l; k <= n; ++k)
			{
				U(j,k) = U(j,k) + s * rv1(k);
	    	}
		}

L270:

		for (k = l; k <= n; ++k) 
		{
		    U(i,k) = scale * U(i,k);
		}

L290:

		// Computing MAX

		d3 = x;
		d1 = w(i);
		d2 = rv1(i);
		d4 = Numerics::abs(d1) + Numerics::abs(d2);
		x = Numerics::max(d3,d4);
    }

//     .......... accumulation of right-hand transformations .......... 

    if (! matv) goto L410;

//     .......... for i=n step -1 until 1 do -- ..........

    for (ii = 1; ii <= n; ++ii)
	{
		i = n + 1 - ii;
	
		if (i == n  ) goto L390;
		if(Numerics::equal(g, 0.0,1e-12)) goto L360;


		for (j = l; j <= n; ++j)
		{
			V(j,i) = U(i,j) / U(i,l) / g;
		}

		for (j = l; j <= n; ++j) 
		{
	    	s = 0.0;

	    	for (k = l; k <= n; ++k)
			{
				s += U(i,k) * V(k,j);
		    }

		    for (k = l; k <= n; ++k)
			{
				V(k,j) = V(k,j) + s * V(k,i);
	    	}
		}

L360:

		for (j = l; j <= n; ++j) 
		{
	    	V(i,j) = 0.0;
	    	V(j,i) = 0.0;
		}

L390:

		V(i,i) = 1.0;
		g = rv1(i);
		l = i;
    }

//     .......... accumulation of left-hand transformations ..........

L410:

	if (! matu) goto L510;

//     ..........for i=Numerics::min(m,n) step -1 until 1 do -- ..........

    mn = n;
    if (m < n) mn = m;

    for (ii = 1; ii <= mn; ++ii) 
	{
		i = mn + 1 - ii;
		l = i + 1;
		g = w(i);

		if (i == n) goto L430;

		for (j = l; j <= n; ++j) {
		    U(i,j) = 0.0;
		}

L430:

		if(Numerics::equal(g, 0.0,1e-12))  goto L475;

		if (i == mn) goto L460;

		for (j = l; j <= n; ++j) {
	    	s = 0.0;

		    for (k = l; k <= m; ++k)
			{
				s += U(k,i) * U(k,j);
	    	}
	    
			f = s / U(i,i) / g;

		    for (k = i; k <= m; ++k) 
			{
				U(k,j) = U(k,j) + f * U(k,i);
		    }
		}

L460:

		for (j = i; j <= m; ++j) 
		{
		    U(j,i) = U(j,i) / g;
		}

		goto L490;

L475:

		for (j = i; j <= m; ++j) 
		{
		    U(j,i) = 0.0;
		}

L490:

		U(i,i) = U(i,i) + 1.0;
	}

//     .......... diagonalization of the bidiagonal form ..........

L510:

    tst1 = x;

//     .......... for k=n step -1 until 1 do -- .......... 

    for (kk = 1; kk <= n; ++kk) 
	{
		k1 = n - kk;
		k = k1 + 1;
		its = 0;

//     .......... test for splitting.
//                for l=k step -1 until 1 do -- ..........

L520:

		for (ll = 1; ll <= k; ++ll) 
		{
	    	l1 = k - ll;
	    	l = l1 + 1;

			d1 = rv1(l);

	    	tst2 = tst1 + Numerics::abs(d1);

	    	if (tst2 == tst1) goto L565;

//     .......... rv1(1) is always zero, so there is no exit
//                through the bottom of the loop ..........

			d1 = w(l1);

	    	tst2 = tst1 + Numerics::abs(d1);

	    	if (tst2 == tst1) goto L540;
		}

//     .......... cancellation of rv1(l) if l greater than 1 ..........

L540:

		c = 0.0;
		s = 1.0;

		for (i = l; i <= k; ++i) 
		{
	   		f = s * rv1(i);
	    	rv1(i) = c * rv1(i);
	    	tst2 = tst1 + Numerics::abs(f);

	    	if (tst2 == tst1) goto L565;

	    	g = w(i);
	    	h = pythag(f,g);

	    	w(i) = h;
	    	
			c = g / h;
	    	s = -f / h;

	    	if (!matu) goto L560;

		    for (j = 1; j <= m; ++j)
			{
				y = U(j, l1);
				z = U(j, i);
				U(j, l1) =  y * c + z * s;
				U(j, i ) = -y * s + z * c;
		    }

L560:	
	    	;
		}

//     .......... test for convergence .......... 

L565:

		z = w(k);

		if (l == k) goto L650;

//     .......... shift from bottom 2 by 2 minor ..........

		if (its == 30) goto L1000;

		++its;

		x = w(l);
		y = w(k1);
		g = rv1(k1);
		h = rv1(k);

		f = ((g + z) / h * ((g - z) / y) + y / h - h / y) * 0.5;
		g = pythag(f,1.0);

		f = x - z / x * z + h / x * (y / (f + dsign(g,f)) - h);

//     .......... next qr transformation ..........

		c = 1.0;
		s = 1.0;

		for (i1 = l; i1 <= k1; ++i1) 
		{
		    i = i1 + 1;

	    	g = rv1(i);
	    	y = w(i);
	    	h = s * g;
	    	g = c * g;
	    	z = pythag(f,h);
	    	rv1(i1) = z;
	    	c = f / z;
	    	s = h / z;
	    	f = x * c + g * s;
	    	g = -x * s + g * c;
	    	h = y * s;
	    	y *= c;
			if (!matv) goto L575;


		    for (j = 1; j <= n; ++j)
			{
				x = V(j,i1);
				z = V(j,i);

				V(j,i1) =  x * c + z * s;
				V(j,i ) = -x * s + z * c;
		    }

L575:

		    z = pythag(f,h);
	    	w(i1) = z;

//     .......... rotation can be arbitrary if z is zero ..........

			if(Numerics::equal(z, 0.0,1e-12))  goto L580;

	    	c = f / z;
	    	s = h / z;

L580:

		    f =  c * g + s * y;
	    	x = -s * g + c * y;

	    	if (!matu) goto L600;

	    	for (j = 1; j <= m; ++j)
			{
				y = U(j, i1);
				z = U(j, i);

				U(j,i1) =  y * c + z * s;
				U(j,i ) = -y * s + z * c;
	    	}

L600:
	;
		}

		rv1(l) = 0.0;
		rv1(k) = f;
		w(k) = x;

		goto L520;

//     .......... convergence .......... 

L650:

		if (z >= 0.0) goto L700;

//     .......... w(k) is made non-negative .......... 

		w(k) = -z;

		if (!matv) goto L700;

		for (j = 1; j <= n; ++j) 
		{
		    V(j,k) = -V(j,k);
		}

L700:
		;
    }

    goto L1001;

//     .......... set error -- no convergence to A
//                singular value after 30 iterations .......... 

L1000:

    ierr = k;

L1001:

    return 0;
}

}

}
}
}

