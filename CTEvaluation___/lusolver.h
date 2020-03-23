// lusolver.h

#ifndef __LUSOLVER_H
#define __LUSOLVER_H

#include "linearsolver.h"
#include "array.h"

namespace Izm 
{
namespace Numerics
{
namespace Solvers
{

template <class LinearType, class DomainType, class RangeType>
class LUSolver : public LinearSolver<LinearType,DomainType,RangeType>
{
protected:
   Array<int> _Swap;
   int _Swaps;

public:
   LUSolver(): LinearSolver<LinearType,DomainType,RangeType>() 
   { 
   }

   ~LUSolver() 
   { 
   }

   void LU(LinearType& A);
   void LU(LinearType& A, LinearType& L, LinearType& U);
   void LU(LinearType& A, LinearType& L, LinearType& U, LinearType& P);

   void Solve(LinearType& A, const RangeType& b, DomainType& x, bool doLU = true);
   void Solve(LinearType& A, const LinearType& B, LinearType& C, bool doLU);

   void Invert(LinearType& A, LinearType& B, bool doLU = true);
   double Det (LinearType& A, bool doLU = true);
};

}
}
}

#include "lusolver.cpp"

#endif // __LUSOLVER_H

