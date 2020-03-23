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
//! \file
//! \ingroup openOR_core
//****************************************************************************

#ifndef openOR_core_math_access_hpp
#define openOR_core_math_access_hpp

#include <boost/mpl/bool.hpp>
#include <openOR/Math/traits.hpp>
#include <openOR/Math/vector.hpp>
#include <openOR/Math/matrix.hpp>

namespace openOR {
   namespace Math {

      namespace Impl {
      

         /**
          * \internal
          */
         template<class Type, class IsVector>
         struct GetData {
          
          typename MathTraits<Type>::ConstPointer operator()(const Type& vec) 
          {
            BOOST_MPL_ASSERT((typename VectorTraits<Type>::IsVector));
            return &get<0>(vec);
          }
        };


         /**
          * \internal
          */
         template<class Type>
         struct GetData<Type, boost::mpl::false_> {

          typename MathTraits<Type>::ConstPointer operator()(const Type& mat) 
          {
            BOOST_MPL_ASSERT((typename MatrixTraits<Type>::IsMatrix));
            return &get<0, 0>(mat);
          }
        };


         /**
          * \internal
          */
         template<class Type, class IsVector>
         struct GetMutableData {

          typename MathTraits<Type>::Pointer operator()(Type& vec) 
          {
            BOOST_MPL_ASSERT((typename VectorTraits<Type>::IsVector));
            return &get<0>(vec);
          }
        };


         /**
          * \internal
          */
         template<class Type>
         struct GetMutableData<Type, boost::mpl::false_> {

          typename MathTraits<Type>::Pointer operator()(Type& mat) 
          {
            BOOST_MPL_ASSERT((typename MatrixTraits<Type>::IsMatrix));
            return &get<0, 0>(mat);
          }
        };
            
      }


      /**
       * @brief Read-only access to memory of math types like vectors or matrices.
       * @param var matrix or vector object
       * @return const pointer to the first element of the vector or the matrix
       * @ingroup openOR_core
       */
      template <class Type>
      inline
      typename MathTraits<Type>::ConstPointer
      data(const Type& var) {
        return Impl::GetData<Type, typename VectorTraits<Type>::IsVector>()(var);
      }

      /**
      * @brief Access to memory of math types like vectors or matrices.
      * @param vec matrix or vector object
      * @return pointer to the first element of the vector or the matrix
      * @ingroup openOR_core
      */
      template <class Type>
      inline
      typename MathTraits<Type>::Pointer
      mutableData(Type& vec) {
          return Impl::GetMutableData<Type, typename VectorTraits<Type>::IsVector>()(vec);
      }


   }
}


#endif
