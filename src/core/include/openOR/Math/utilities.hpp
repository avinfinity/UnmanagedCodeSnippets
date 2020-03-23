//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//! OPENOR_INTERFACE_FILE(openOR_core)
//****************************************************************************
/**
 * @file
 * @ingroup openOR_core
 */

#ifndef openOR_core_math_utilities_hpp
#define openOR_core_math_utilities_hpp

#include <cmath>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/ublas/lu.hpp> 

namespace openOR {

   namespace Math {

      /**
       * \brief square
       * @ingroup openOR_core
       */
      template<typename T>
      T square(const T& arg) { return arg * arg; }


      /**
       * \brief clamp
       * @ingroup openOR_core
       */
      template<typename T>
      T clamp(const T& arg, const T& min, const T& max) { return std::min<T>(max, std::max<T>(min, arg)); }


      /**
       * \brief rounds a double to the next integer value
       */
      inline double round(const double& d)
      {
         return floor(d + 0.5);
      }


      /**
       * \brief rounds a float to the next integer value
       */
      inline float round(const float& f)
      {
         return floorf(f + 0.5f);
      }


      /**
       * \brief rounds a long double to the next integer value
       */

      inline long double round(const long double& d)
      {
         return floorl(d + 0.5);
      }


      /**
       * \brief nextPowerOfTwo
       * @ingroup openOR_core
       */
      inline static unsigned int nextPowerOfTwo(unsigned int v) {
         --v;
         v |= v >> 1;
         v |= v >> 2;
         v |= v >> 4;
         v |= v >> 8;
         v |= v >> 16;
         ++v;
         return v;
      }


      /**
       * \brief nextPowerOfTwo
       * @ingroup openOR_core
       */
      inline static unsigned short nextPowerOfTwo(unsigned short v) {
         --v;
         v |= v >> 1;
         v |= v >> 2;
         v |= v >> 4;
         v |= v >> 8;
         ++v;
         return v;
      }

      /**
       * \brief isPowerOfTwo
       * @ingroup openOR_core
       */
      inline bool isPowerOfTwo(unsigned int n) { return ((n & (n - 1)) == 0); }
      
      
      /**
       * \brief isPowerOfTwo
       * @ingroup openOR_core
       */
      inline bool isPowerOfTwo(unsigned short n) { return ((n & (n - 1)) == 0); }


      /**
       * \brief Solver for linear function A*x=y
       */
      template<typename Type>
      boost::numeric::ublas::vector<Type> solve(const boost::numeric::ublas::matrix<Type>& A, const boost::numeric::ublas::vector<Type>& y)
      {
         //create a permutation matrix for the LU-factorization
         boost::numeric::ublas::matrix<Type> matA(A);
         boost::numeric::ublas::permutation_matrix<std::size_t> pm(matA.size1());
         int res = boost::numeric::ublas::lu_factorize(matA, pm);
         if (res == 0)
            return boost::numeric::ublas::vector<Type>();

         boost::numeric::ublas::vector<Type> vecX(y);
         boost::numeric::ublas::lu_substitute(matA, pm, vecX);
         return vecX;
      }

   }
}
#endif
