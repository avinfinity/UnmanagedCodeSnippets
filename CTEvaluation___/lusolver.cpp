// lusolver.cpp

//#include "lusolver.h"
#include "utils.h"

namespace Izm
{
namespace Numerics
{
namespace Solvers
{

template <class LinearType, class DomainType, class RangeType> 
void LUSolver<LinearType,DomainType,RangeType>::LU(LinearType& A)
{
	const int m = A.xDim();
	const int n = A.yDim();

	_Error = 0;

	_Swap.setSize(m);
	for (int i = 1; i <= m; ++i) _Swap(i) = i;
	_Swaps = 0;

	for (int k = 1; k < n; ++k)
	{
		// Pivot-Suche
		int piv = k;
		for (int i = k+1; i <= m; ++i)
		{
			if (Numerics::abs(A(_Swap(i),k)) > Numerics::abs(A(_Swap(piv),k))) piv = i;
		}
		// Zeilentausch in swap
		if (piv > k) 
		{
			std::swap(_Swap(piv),_Swap(k));
			_Swaps++;
		}

		// Berechnung der Gauﬂ-Faktoren
		for (int i = k+1; i <= m; ++i)
		{
			if (Numerics::abs(A(_Swap(k),k)) > 10.0*eps)
			{
				A(_Swap(i),k) = A(_Swap(i),k)/A(_Swap(k),k);
			}
			else 
			{
				_Error = -1;
				return;
			}
		}
		// Transformation der Zeilen i:=k+1,...,m
		for (int i = k+1; i <= m; ++i)
		{
			for (int j = k+1; j <= n; ++j)
			{
				A(_Swap(i),j) = A(_Swap(i),j) - A(_Swap(i),k) * A(_Swap(k),j);
			}
		}
	}
}

template <class LinearType, class DomainType, class RangeType>
void LUSolver<LinearType,DomainType,RangeType>::LU(LinearType& A, LinearType& L, LinearType& U)
{
	const int m = A.xDim();
	const int n = A.yDim();

	LU(A);
	if (_Error != 0) return;

	// Aufbau von U
	U.clear();
	for (int i = 1; i <= m; ++i)
	{
		for (int j = i; j <= n; ++j)
		{
			U(i,j) = A(_Swap(i),j);
		}
	}

	// Aufbau von L
	L.clear();
	for (int i = 1; i <= m; ++i)
	{
		for (int j = 1; j <= i-1; ++j)
		{
			L(i,j) = A(_Swap(i),j);
		}
	}

	for (int i = 1; i <= m; ++i) L(i,i) = 1.0;
}

template <class LinearType, class DomainType, class RangeType>
void LUSolver<LinearType,DomainType,RangeType>::LU(LinearType& A, LinearType& L, LinearType& U, LinearType& P)
{
	LU(A,L,U);
	if (_Error != 0) return;

	P.clear();
	for (int i = 1; i <= _Swap.Size(); ++i)
	{
		P(i,_Swap(i)) = 1.0;
	}
}

template <class LinearType, class DomainType, class RangeType>
void LUSolver<LinearType,DomainType,RangeType>::Solve(LinearType& A, const RangeType& b, DomainType& c, bool doLU)
{
	const int m = A.xDim();
	const int n = A.yDim();

	RangeType y(m);

	_Error = 0;

	if (doLU) LU(A);
	if (_Error != 0) return;

	// Initialisierung von y mit b entsprechend den vongenommenen Zeilenvertauschungen
	for (int i = 1; i <= m; ++i) y(i) = b(_Swap(i));

	// Berechnung der Lˆsung von L * y = b
	for (int k = 1; k <= n; ++k)
	{
		for (int j = 1; j <= k-1; ++j)
		{
			y(k) -= A(_Swap(k),j) * y(j);
		}
	}

	for (int k = 1; k <= n; ++k) c(k) = y(k);

	// Lˆsung von U * c := y
	for (int k = n; k >= 1; --k)
	{
		for (int j = k+1; j <= n; ++j) c(k) -= A(_Swap(k),j) * c(j);

		if (Numerics::abs(A(_Swap(k),k)) < 10.0*eps) 
		{
			_Error = -1;
			return;
		}
		else 
		{
			c(k) /= A(_Swap(k),k);
		}
	}
}

template <class LinearType, class DomainType, class RangeType>
void LUSolver<LinearType,DomainType,RangeType>::Solve(LinearType& A, const LinearType& B, LinearType& C, bool doLU)
{
	const int n = A.yDim();
	const int m = B.yDim();

	DomainType b(n), x(n);

	_Error = 0;

	if (doLU) LU(A);
	if (_Error != 0) return;

	for (int i = 1; i <= m; ++i)
	{
		for (int k = 1; k <= n; ++k) b(k) = B(k,i);
	
		Solve(A,b,x,false);
		if (_Error != 0) return;

		for (int k = 1; k <= n; ++k) C(k,i) = x(k);
	}
}

template <class LinearType, class DomainType, class RangeType>
void LUSolver<LinearType,DomainType,RangeType>::Invert(LinearType& A, LinearType& B, bool doLU)
{
	LinearType E(A.xDim(),A.yDim());

	for (int i = 1; i <= A.xDim(); ++i) E(i,i) = 1.0;

	Solve(A,E,B,doLU);
}

template <class LinearType, class DomainType, class RangeType>
double LUSolver<LinearType,DomainType,RangeType>::Det(LinearType& A, bool doLU)
{
	double result = 1.0;
	_Error = 0;

	if (doLU) LU(A);
	if (_Error != 0) return 0.0;

	for (int i = 1; i <= A.xDim(); ++i) result *= A(_Swap(i),i);

	if ((_Swaps % 2)!=0) return -result; else return result;
}

}
}
}

// lusolver.cpp