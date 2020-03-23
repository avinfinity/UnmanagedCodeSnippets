//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//!
//! @file
//! @ingroup openOR_core_math

#ifndef openOR_core_Math_matrix_types_hpp
#define openOR_core_Math_matrix_types_hpp

#include <boost/numeric/ublas/fwd.hpp>

namespace openOR {
   namespace Math {
      
      //----------------------------------------------------------------------------
      // convenience typedefs
      //----------------------------------------------------------------------------    

      typedef boost::numeric::ublas::bounded_matrix<int, 2, 2, boost::numeric::ublas::column_major>          Matrix22i;    //!< @ingroup openOR_core_math
      typedef boost::numeric::ublas::bounded_matrix<unsigned int, 2, 2, boost::numeric::ublas::column_major> Matrix22ui;   //!< @ingroup openOR_core_math
      typedef boost::numeric::ublas::bounded_matrix<float, 2, 2, boost::numeric::ublas::column_major>        Matrix22f;    //!< @ingroup openOR_core_math
      typedef boost::numeric::ublas::bounded_matrix<double, 2, 2, boost::numeric::ublas::column_major>       Matrix22d;    //!< @ingroup openOR_core_math

      typedef boost::numeric::ublas::bounded_matrix<int, 3, 3, boost::numeric::ublas::column_major>          Matrix33i;    //!< @ingroup openOR_core_math
      typedef boost::numeric::ublas::bounded_matrix<unsigned int, 3, 3, boost::numeric::ublas::column_major> Matrix33ui;   //!< @ingroup openOR_core_math
      typedef boost::numeric::ublas::bounded_matrix<float, 3, 3, boost::numeric::ublas::column_major>        Matrix33f;    //!< @ingroup openOR_core_math
      typedef boost::numeric::ublas::bounded_matrix<double, 3, 3, boost::numeric::ublas::column_major>       Matrix33d;    //!< @ingroup openOR_core_math

      typedef boost::numeric::ublas::bounded_matrix<int, 4, 4, boost::numeric::ublas::column_major>          Matrix44i;    //!< @ingroup openOR_core_math
      typedef boost::numeric::ublas::bounded_matrix<unsigned int, 4, 4, boost::numeric::ublas::column_major> Matrix44ui;   //!< @ingroup openOR_core_math
      typedef boost::numeric::ublas::bounded_matrix<float, 4, 4, boost::numeric::ublas::column_major>        Matrix44f;    //!< @ingroup openOR_core_math
      typedef boost::numeric::ublas::bounded_matrix<double, 4, 4, boost::numeric::ublas::column_major>       Matrix44d;    //!< @ingroup openOR_core_math

   }
}

#endif
