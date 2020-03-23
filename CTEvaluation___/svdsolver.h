// svdsolver.h

#ifndef __SVDSOLVER_H
#define __SVDSOLVER_H

#include "linearsolver.h"
#include "vmclasses.h"
#include "utils.h"

namespace Izm 
{
namespace Numerics
{
namespace Solvers
{

class SVDSolver : public LinearSolver<Matrix,Vector,Vector>
{
private:
	double _RegTolerance;

public:
	SVDSolver(double regTolerance = 1e-3): LinearSolver<Matrix,Vector,Vector>(),
		_RegTolerance(regTolerance)
	{ 
	}

	~SVDSolver() 
	{ 
	}

	void SVD(Matrix& A, Vector& s);
	void SVD(Matrix& A, Matrix& S);
	void SVD(Matrix& A, Matrix& S, Matrix& V);
	void SVD(Matrix& A, Vector& S, Matrix& V);
	void SVD(Matrix& A, Matrix& U, Matrix& S, Matrix& V);
	void SVD(Matrix& A, Matrix& U, Vector& s, Matrix& V);

	void Solve(Matrix& A, Vector& x);
	void Solve(Matrix& A, const Vector& b, Vector& x, bool doSVD = true);
};

}
}
}

#endif // __SVDSOLVER_H
