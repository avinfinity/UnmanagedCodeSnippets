// utils.h

#ifndef __UTILS_H
#define __UTILS_H

#include "math.h"

// Makrodefinitionen aus, da sonst Schwierigkeiten 
// mit unten stehenden Templates
#undef max
#undef min

namespace Izm
{
namespace Numerics
{

const double Pi = 3.141592653589793238462643;
const double eps = 1e-15;
const double sqrtEps = sqrt(eps);

inline double abs(double x)
{
	return (x >= 0.0) ? x : -x;
}

inline bool equal(double x, double y, double currentEps = 1e-8)
{
	return (abs(x - y) < currentEps);
}

inline bool less(double x, double y, double currentEps = 1e-8)
{
	return (abs(x - y) < currentEps) ? false : x < y;
}

inline bool lessEqual(double x, double y, double currentEps = 1e-8)
{
	return (abs(x - y) < currentEps) ? true : x < y;
}

inline bool greater(double x, double y, double currentEps = 1e-8)
{
	return (abs(x - y) < currentEps) ? false : x > y;
}

inline bool greaterEqual(double x, double y, double currentEps = 1e-8)
{
	return (abs(x - y) < currentEps) ? true : x > y;
}

inline double sqr(double x)
{
	return x*x;
}

inline double sign(double x)
{
	return (x >= 0.0) ? 1.0 : -1.0;
}

template <class T>
T max(T x, T y)
{
	return x < y ? y : x;
}

template <class T>
T max(T x, T y, T z)
{
	return max(max(x,y),z);
}

template <class T>
T min(T x, T y)
{
	return x < y ? x : y;
}

template <class T>
T min(T x, T y, T z)
{
	return min(min(x,y),z);
}

template <class S, class T>
struct ordered_pair
{
	S first;
	T second;

	ordered_pair(): first(S()), second(T()) { }
	ordered_pair(const S& _first, const T& _second): first(_first), second(_second) { }

	friend bool operator ==(const ordered_pair& x, const ordered_pair& y) { return (x.first == y.first); }
	friend bool operator <(const ordered_pair& x, const ordered_pair& y) { return (x.first < y.first); }
};

template <class S, class T>
ordered_pair<S,T> make_ordered_pair(const S& s, const T& t)
{
	return ordered_pair<S,T>(s,t);
}

}
}

#endif // __UTILS_H








