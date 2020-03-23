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
//!
//! @file
//! @ingroup openOR_core

#ifndef openOR_core_Math_ublasmatrix_hpp
#define openOR_core_Math_ublasmatrix_hpp


#if defined(_MSC_VER)
#  pragma warning(disable:4172)
#endif 

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/math/quaternion.hpp>

#if defined(_MSC_VER)
#  pragma warning(default:4172)
#endif 

#include <openOR/Math/traits.hpp>
#include <openOR/Math/matrixconcept.hpp>
#include <openOR/Math/matrix_types.hpp>


namespace openOR {
   namespace Math {

      //----------------------------------------------------------------------------
      // Adapting the corresponding traits
      //---------------------------------------------------------------------------- 
      
      template<typename MatrixType>
      struct UBLASMatrixAccess {
         typedef typename MatrixType::reference AccessType;
         typedef typename MatrixType::const_reference ConstAccessType;

         template <int I, int J, typename PP>
            static typename if_const<PP, ConstAccessType, AccessType>::
         type get(PP& p) {
            assert(I < p.size1() && J < p.size2());
            return p(I, J);
         }
      };



      template<typename Value, std::size_t Row, std::size_t Col, typename Layout>
      struct MatrixTraits<boost::numeric::ublas::bounded_matrix<Value, Row, Col, Layout> > {
         typedef boost::numeric::ublas::bounded_matrix<Value, Row, Col, Layout> MatrixType;
         typedef boost::numeric::ublas::bounded_matrix<typename ScalarTraits<Value>::RealType, Row, Col, Layout> RealMatrixType;
         typedef typename boost::numeric::ublas::bounded_matrix<Value, Row, Col, Layout>::value_type ValueType;
         typedef boost::mpl::int_<Row> RowDimension;
         typedef boost::mpl::int_<Col> ColDimension;
         typedef boost::numeric::ublas::bounded_vector<Value, Col> RowVectorType;
         typedef boost::numeric::ublas::bounded_vector<Value, Row> ColVectorType;
         typedef boost::numeric::ublas::bounded_vector<Value, 3> Vector3Type;
		 typedef boost::numeric::ublas::bounded_vector<Value, 2> Vector2Type;
         typedef boost::numeric::ublas::bounded_matrix<typename ScalarTraits<Value>::RealType, 3, 3, Layout> Matrix33Type;
         typedef UBLASMatrixAccess<MatrixType> Access;
         typedef boost::mpl::true_ IsMatrix;

         static MatrixType ZEROS;
         static MatrixType IDENTITY;
      };


      template<typename Value, std::size_t Row, std::size_t Col, typename Layout>
      typename MatrixTraits<boost::numeric::ublas::bounded_matrix<Value, Row, Col, Layout> >::MatrixType
      MatrixTraits<boost::numeric::ublas::bounded_matrix<Value, Row, Col, Layout> >::ZEROS = boost::numeric::ublas::zero_matrix<Value>(Row, Col);

      template<typename Value, std::size_t Row, std::size_t Col, typename Layout>
      typename MatrixTraits<boost::numeric::ublas::bounded_matrix<Value, Row, Col, Layout> >::MatrixType
      MatrixTraits<boost::numeric::ublas::bounded_matrix<Value, Row, Col, Layout> >::IDENTITY = boost::numeric::ublas::identity_matrix<Value>(Row, Col);

      //----------------------------------------------------------------------------
      //! @brief 
      //! @ingroup openOR_core
      //---------------------------------------------------------------------------- 
      template<class E1, class E2>
         typename boost::numeric::ublas::matrix_vector_binary1_traits < 
            typename E1::value_type, E1,
            typename E2::value_type, E2 >::
      result_type prod( const boost::numeric::ublas::matrix_expression<E1> &e1,
                        const boost::numeric::ublas::vector_expression<E2> &e2) 
      {
         return boost::numeric::ublas::prod(e1, e2);
      }

      //----------------------------------------------------------------------------
      //! @brief 
      //! @ingroup openOR_core
      //---------------------------------------------------------------------------- 
      template<class E1, class E2>
         typename boost::numeric::ublas::matrix_matrix_binary_traits < 
            typename E1::value_type, E1,
            typename E2::value_type, E2 >::
      result_type prod( const boost::numeric::ublas::matrix_expression<E1> &e1,
                        const boost::numeric::ublas::matrix_expression<E2> &e2) 
      {
         return boost::numeric::ublas::prod(e1, e2);
      }

   }
}

namespace boost {
   namespace numeric {
      namespace ublas {
         
         // TODO: boost ublas is in the process of defining the usual operaters more sensibly
         // (probably part of 1.42.0) we should get rid of these then

         template<class V, class E1, class E2>
         inline V operator% (const matrix_expression<E1>& e1, const vector_expression<E2>& e2) {
            return prod(e1, e2);
         }


         template<class M, class E1, class E2>
         inline M operator% (const matrix_expression<E1>& e1, const matrix_expression<E2>& e2) {
            return prod(e1, e2);
         }

      }
   }
}


#endif
