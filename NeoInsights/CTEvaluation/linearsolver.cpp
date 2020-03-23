// LinearSolver.cpp
//	Gehört zur VMClasses-Bibliothek für Vector/Matrix-Rechnung
//	Ursprünglicher Autor D.Görsch - goersch.at.zeiss-izm.de

// include "LinearSolver.h"

namespace Izm
{
namespace Numerics
{
namespace Solvers
{

template <class _LinearType, class _DomainType, class _RangeType>
LinearSolver<_LinearType, _DomainType, _RangeType>::LinearSolver():
	_Error(0),
	kMax(30),
	_Tol(1e-6),
	_Res(0.0)
{
}

template <class _LinearType, class _DomainType, class _RangeType>
LinearSolver<_LinearType, _DomainType, _RangeType>::~LinearSolver()
{
}

}
}
}

// LinearSolver.cpp
