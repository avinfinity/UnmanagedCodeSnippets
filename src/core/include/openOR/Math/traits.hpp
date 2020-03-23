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

#ifndef openOR_core_math_traits_hpp
#define openOR_core_math_traits_hpp

#include <openOR/Utility/if_const.hpp>
#include <openOR/Utility/Types.hpp>

#include <boost/mpl/if.hpp>
#include <boost/mpl/int.hpp>

#include <boost/numeric/ublas/traits.hpp>

// boost ublas scalar traits adjustments
namespace boost {
   namespace numeric {
      namespace ublas {

         template<>
         struct type_traits<char> : scalar_traits<char> {
            typedef type_traits<char> self_type;
            typedef int value_type;
            typedef const value_type &const_reference;
            typedef value_type &reference;
            typedef double real_type;
            typedef real_type precision_type;
         };

         template<>
         struct type_traits<unsigned char> : scalar_traits<unsigned char> {
            typedef type_traits<unsigned char> self_type;
            typedef int value_type;
            typedef const value_type &const_reference;
            typedef value_type &reference;
            typedef double real_type;
            typedef real_type precision_type;
         };


         template<>
         struct type_traits<short> : scalar_traits<short> {
            typedef type_traits<short> self_type;
            typedef int value_type;
            typedef const value_type &const_reference;
            typedef value_type &reference;
            typedef double real_type;
            typedef real_type precision_type;
         };


         template<>
         struct type_traits<unsigned short> : scalar_traits<unsigned short> {
            typedef type_traits<unsigned short> self_type;
            typedef int value_type;
            typedef const value_type &const_reference;
            typedef value_type &reference;
            typedef double real_type;
            typedef real_type precision_type;
         };


         template<>
         struct type_traits<int> : scalar_traits<int> {
            typedef type_traits<int> self_type;
            typedef int value_type;
            typedef const value_type &const_reference;
            typedef value_type &reference;
            typedef double real_type;
            typedef real_type precision_type;
         };


         template<>
         struct type_traits<unsigned int> : scalar_traits<unsigned int> {
            typedef type_traits<unsigned int> self_type;
            typedef int value_type;
            typedef const value_type &const_reference;
            typedef value_type &reference;
            typedef double real_type;
            typedef real_type precision_type;
         };

      }
   }
}



namespace openOR {
   namespace Math {

      /**
       * @brief traits for scalar types
       * @ingroup openOR_core
       */
      template<class Type>
      struct ScalarTraits {
         typedef typename boost::numeric::ublas::type_traits<Type>::value_type ValueType;
         typedef typename boost::numeric::ublas::type_traits<Type>::real_type RealType;
         typedef typename boost::numeric::ublas::type_traits<Type>::precision_type PrecisionType;
      };




      /**
       * @brief Functor for Vector element access
       * @ingroup openOR_core
       */
      template <typename Type>
      struct DefaultVectorAccess {
         typedef invalid_type AccessType;
         typedef invalid_type ConstAccessType;

         template <int I, typename PP>
            static typename if_const<PP, ConstAccessType, AccessType>::
         type get(PP& p) {
            return p.template get<I>();
         }
      };

      /**
      * \brief Default traits for non-vector types. 
      * @ingroup openOR_core
      * 
      * This traits class has to be specialized for concrete vector implementations.
      */
      template<class Type>
      struct VectorTraits {
         typedef Type VectorType;                     //!< self type
         typedef Type RealVectorType;                 //!< vector type with a floating point value type
         typedef invalid_type ValueType;                   //!< type of a vector element
         typedef invalid_type Vector3Type;                 //!< type of a vector with dimension 3 with same value type
		 typedef invalid_type Vector2Type;                 //!< type of a vector with dimension 2 with same value type
         typedef boost::mpl::int_<0> Dimension;       //!< number of elements (boost::mpl::int_<n>)
         typedef DefaultVectorAccess<Type> Access;    //!< functor type for element access
         typedef boost::mpl::false_ IsVector;         //!< flag to indicate that is is a valid vector type (must be set to boost::mpl::true_ if specialized for vector implementations.

         static VectorType ZEROS;                     //!< vector filled with zeros
      };


      /**
       * @brief Functor for matrix element access
       * @ingroup openOR_core
       */
      template <typename Type>
      struct DefaultMatrixAccess {
         typedef invalid_type AccessType;
         typedef invalid_type ConstAccessType;

         template <int I, int J, typename PP>
            static typename if_const<PP, ConstAccessType, AccessType>::type
         get(PP& p) {
            return p.template get<I, J>();
         }
      };


      /**
       * \brief Default traits for non-matrix types. 
       * @ingroup openOR_core
       * 
       * This traits class has to be specialized for concrete matrix implementations.
       */
      template<class Type>
      struct MatrixTraits {
         typedef Type MatrixType;                     //!< self type
         typedef Type RealMatrixType;                 //!< matrix type with a floating point value type
         typedef invalid_type ValueType;                   //!< type of a matrix element
         typedef boost::mpl::int_<0> RowDimension;    //!< number of rows (boost::mpl::int_<n>)
         typedef boost::mpl::int_<0> ColDimension;    //!< number of columns (boost::mpl::int_<n>)
         typedef invalid_type RowVectorType;               //!< type of a row vector
         typedef invalid_type ColVectorType;               //!< type of a column vector
         typedef invalid_type Vector3Type;                 //!< type of a vector with dimension 3 with same value type
		 typedef invalid_type Vector2Type;                 //!< type of a vector with dimension 2 with same value type
         typedef invalid_type Matrix33Type;                //!< type of a 3x3 matrix with same value type
         typedef DefaultMatrixAccess<Type> Access;    //!< functor type for element access
         typedef boost::mpl::false_ IsMatrix;         //!< flag to indicate that is is a valid matrix type (must be set to boost::mpl::true_ if specialized for matrix implementations.

         static MatrixType ZEROS;                     //!< matrix filled with zeros
         static MatrixType IDENTITY;                  //!< identify matrix
      };


      /**
       * @brief Traits for math type
       * @ingroup openOR_core
       */
      template<class Type>
      struct MathTraits {
        typedef typename boost::mpl::if_<typename MatrixTraits<Type>::IsMatrix, 
                                           typename MatrixTraits<Type>::ValueType, 
                                           typename boost::mpl::if_<typename VectorTraits<Type>::IsVector, 
                                             typename VectorTraits<Type>::ValueType, 
                                             typename ScalarTraits<Type>::ValueType>::type>::type ValueType;
        typedef ValueType *Pointer;
        typedef const ValueType *ConstPointer;
      };
   }
}

#endif
