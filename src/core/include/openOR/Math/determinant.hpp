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

#ifndef openOR_core_Math_determinant_hpp
#define openOR_core_Math_determinant_hpp

#include <boost/mpl/equal_to.hpp>
#include <boost/mpl/assert.hpp>

#include <openOR/Utility/conceptcheck.hpp>

#include <openOR/Math/detail/determinant_impl.hpp>

namespace openOR {
   namespace Math {

      //----------------------------------------------------------------------------
      //! @brief 
      //! @ingroup openOR_core_math
      //----------------------------------------------------------------------------
      template<class Type>
         inline
         OPENOR_CONCEPT_REQUIRES( ((Concept::ConstMatrix<Type>)),
      (typename MatrixTraits<Type>::ValueType)) determinant(const Type& mat) {
            
         BOOST_MPL_ASSERT((boost::mpl::equal_to<typename MatrixTraits<Type>::RowDimension, typename MatrixTraits<Type>::ColDimension>));
         return Impl::Determinant<Type, MatrixTraits<Type>::RowDimension::value>()(mat);
      }
   
   }
}


#endif
