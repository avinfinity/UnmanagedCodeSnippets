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

#ifndef openOR_core_Math_ublasvector_hpp
#define openOR_core_Math_ublasvector_hpp

#if defined(_MSC_VER)
#  pragma warning(disable:4172)
#endif

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>

#if defined(_MSC_VER)
#  pragma warning(default:4172)
#endif 

#include <openOR/Math/traits.hpp>
#include <openOR/Math/vectorconcept.hpp>
#include <openOR/Math/vector_types.hpp>


namespace openOR {
   namespace Math {

      //----------------------------------------------------------------------------
      // Adapting the corresponding traits
      //---------------------------------------------------------------------------- 
      
      template<typename VectorType>
      struct UBLASVectorAccess {
         typedef typename VectorType::reference AccessType;
         typedef typename VectorType::const_reference ConstAccessType;

         template <int I, typename PP> static 
            typename if_const<PP, ConstAccessType, AccessType>::
         type get(PP& p) {
            assert(I < p.size());
            return p(I);
         }
      };

      template<typename Value, std::size_t d>
      struct VectorTraits<boost::numeric::ublas::bounded_vector<Value, d> > {
         typedef boost::numeric::ublas::bounded_vector<Value, d> VectorType;
         typedef boost::numeric::ublas::bounded_vector<typename ScalarTraits<Value>::RealType, d>  RealVectorType;
         typedef Value ValueType;
         typedef boost::numeric::ublas::bounded_vector<Value, 3> Vector3Type;
         typedef boost::numeric::ublas::bounded_vector<Value, 2> Vector2Type;
         typedef boost::mpl::int_<d> Dimension;
         typedef UBLASVectorAccess<VectorType> Access;
         typedef boost::mpl::true_ IsVector;

         static VectorType ZEROS;
      };


      template<typename Value, std::size_t d>
      typename VectorTraits< boost::numeric::ublas::bounded_vector<Value, d> >::VectorType
      VectorTraits< boost::numeric::ublas::bounded_vector<Value, d> >::ZEROS = boost::numeric::ublas::zero_vector<Value>(d);


      template<typename E>
      struct UBLASVectorExpressionTraits {
         typedef E VectorType;
         typedef typename E::value_type ValueType;
         typedef boost::numeric::ublas::bounded_vector<typename E::value_type, 3> Vector3Type;
         typedef boost::numeric::ublas::bounded_vector<typename E::value_type, 2> Vector2Type;
         typedef boost::mpl::int_<0> Dimension;
         typedef UBLASVectorAccess<E> Access;
         typedef boost::mpl::true_ IsVector;
      };

      template<class E>
      struct VectorTraits<boost::numeric::ublas::vector_expression<E> > : VectorTraits<E> {};

      template<class C>
      struct VectorTraits<boost::numeric::ublas::vector_container<C> > : UBLASVectorExpressionTraits<boost::numeric::ublas::vector_container<C> > {};

      template<class T, class A>
      struct VectorTraits<boost::numeric::ublas::vector<T, A> > : UBLASVectorExpressionTraits<boost::numeric::ublas::vector<T, A> > {};

      template<class E>
      struct VectorTraits<boost::numeric::ublas::vector_reference<E> > : UBLASVectorExpressionTraits<boost::numeric::ublas::vector_reference<E> > {};

      template<typename E, typename F>
      struct VectorTraits<boost::numeric::ublas::vector_unary<E, F> > : UBLASVectorExpressionTraits<boost::numeric::ublas::vector_unary<E, F> > {};

      template<typename E1, typename E2, typename F>
      struct VectorTraits<boost::numeric::ublas::vector_binary<E1, E2, F> > : UBLASVectorExpressionTraits<boost::numeric::ublas::vector_binary<E1, E2, F> > {};

      template<typename E1, typename E2, typename F>
      struct VectorTraits<boost::numeric::ublas::vector_binary_scalar1<E1, E2, F> > : UBLASVectorExpressionTraits<boost::numeric::ublas::vector_binary_scalar1<E1, E2, F> > {};

      template<typename E1, typename E2, typename F>
      struct VectorTraits<boost::numeric::ublas::vector_binary_scalar2<E1, E2, F> > : UBLASVectorExpressionTraits<boost::numeric::ublas::vector_binary_scalar2<E1, E2, F> > {};


      //----------------------------------------------------------------------------
      //! @brief 
      //! @ingroup openOR_core
      //---------------------------------------------------------------------------- 
      template <class E>
         inline typename boost::numeric::ublas::vector_scalar_unary_traits<E, boost::numeric::ublas::vector_norm_2<E> >::
      result_type norm(const boost::numeric::ublas::vector_expression<E>& e) {
         return boost::numeric::ublas::norm_2(e);
      }


      //----------------------------------------------------------------------------
      //! @brief 
      //! @ingroup openOR_core
      //---------------------------------------------------------------------------- 
      template <class E1, class E2>
         inline typename boost::numeric::ublas::vector_scalar_binary_traits < E1, E2, 
                  boost::numeric::ublas::vector_inner_prod < E1, E2,
                  typename boost::numeric::ublas::promote_traits < typename E1::value_type,
                  typename E2::value_type >::promote_type > >::
      result_type dot(  const boost::numeric::ublas::vector_expression<E1> &e1,
                        const boost::numeric::ublas::vector_expression<E2> &e2) 
      {
         return boost::numeric::ublas::inner_prod(e1, e2);
      }

   }
}

//namespace boost {
//   namespace numeric {
//      namespace ublas {
//
//         template<class E1, class E2>
//         BOOST_UBLAS_INLINE
//            typename vector_scalar_binary_traits < E1, E2, vector_inner_prod < E1, E2,
//                        typename promote_traits < typename E1::value_type,
//                        typename E2::value_type >::promote_type > >::
//         result_type operator% ( const vector_expression<E1> &e1,
//                                 const vector_expression<E2> &e2) 
//         {
//            typedef typename vector_scalar_binary_traits < E1, E2, vector_inner_prod < E1, E2,
//                                 typename promote_traits < typename E1::value_type,
//                                 typename E2::value_type >::promote_type > >::expression_type expression_type;
//            return expression_type(e1(), e2());
//         }
//
//      }
//   }
//}

#endif
