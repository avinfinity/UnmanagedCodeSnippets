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

#ifndef openOR_core_Math_vector_types_hpp
#define openOR_core_Math_vector_types_hpp

#include <boost/numeric/ublas/fwd.hpp>

namespace openOR {
   namespace Math {
      
      //----------------------------------------------------------------------------
      // convenience typedefs
      //----------------------------------------------------------------------------     
      
      typedef boost::numeric::ublas::bounded_vector<int, 2>          Vector2i;     //!< @ingroup openOR_core_math
      typedef boost::numeric::ublas::bounded_vector<unsigned int, 2> Vector2ui;    //!< @ingroup openOR_core_math
      typedef boost::numeric::ublas::bounded_vector<float, 2>        Vector2f;     //!< @ingroup openOR_core_math
      typedef boost::numeric::ublas::bounded_vector<double, 2>       Vector2d;     //!< @ingroup openOR_core_math
      typedef boost::numeric::ublas::bounded_vector<long long, 2>    Vector2ll;    //!< @ingroup openOR_core_math
      
      typedef boost::numeric::ublas::bounded_vector<int, 3>          Vector3i;     //!< @ingroup openOR_core_math
      typedef boost::numeric::ublas::bounded_vector<unsigned int, 3> Vector3ui;    //!< @ingroup openOR_core_math
      typedef boost::numeric::ublas::bounded_vector<float, 3>        Vector3f;     //!< @ingroup openOR_core_math
      typedef boost::numeric::ublas::bounded_vector<double, 3>       Vector3d;     //!< @ingroup openOR_core_math
      typedef boost::numeric::ublas::bounded_vector<long long, 3>    Vector3ll;    //!< @ingroup openOR_core_math
      
      typedef boost::numeric::ublas::bounded_vector<int, 4>          Vector4i;     //!< @ingroup openOR_core_math
      typedef boost::numeric::ublas::bounded_vector<unsigned int, 4> Vector4ui;    //!< @ingroup openOR_core_math
      typedef boost::numeric::ublas::bounded_vector<float, 4>        Vector4f;     //!< @ingroup openOR_core_math
      typedef boost::numeric::ublas::bounded_vector<double, 4>       Vector4d;     //!< @ingroup openOR_core_math
      typedef boost::numeric::ublas::bounded_vector<long long, 4>    Vector4ll;    //!< @ingroup openOR_core_math

      typedef boost::numeric::ublas::bounded_vector<int, 6>          Vector6i;     //!< @ingroup openOR_core_math
      
   }
}

#endif
