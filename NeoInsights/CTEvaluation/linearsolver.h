// LinearSolver.h

#ifndef __LINEARSOLVER_H
#define __LINEARSOLVER_H

namespace Izm
{
namespace Numerics
{
namespace Solvers
{

template <class _LinearType, class _DomainType, class _RangeType>
class LinearSolver
{
protected:
	int _Error, kMax;
	double _Tol, _Res;

public:
	typedef _LinearType LinearType;
	typedef _DomainType DomainType;
	typedef _RangeType RangeType;

	LinearSolver();
	virtual ~LinearSolver();

	virtual void Solve(LinearType& A, const RangeType& b, DomainType& x, bool = true) = 0;

	int    Error()    const { return _Error; }
	double Residual() const { return _Res; }
	double Accuracy() const { return _Tol; }
	int    MaxSteps() const { return kMax; }

	void SetAccuracy(double value) { _Tol = value; }
	void SetMaxSteps(int value) { kMax = value; }
};

}
}
}

#include "LinearSolver.cpp"

#endif // __LINEARSOLVER_H
