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
 * @author Christian Winne
 * @ingroup openOR_core
 */

#ifndef openOR_core_vectorfunctions_hpp
#define openOR_core_vectorfunctions_hpp

#include <functional>

#include <boost/mpl/bool.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/equal_to.hpp>

#include <openOR/Utility/conceptcheck.hpp>
#include <openOR/Math/traits.hpp>
#include <openOR/Math/vector.hpp>
#include <openOR/Math/constants.hpp>


namespace openOR {
   namespace Math {
      
      /**
       * @brief 
       * @ingroup openOR_core
       */
      template <class Type>
      inline
      OPENOR_CONCEPT_REQUIRES(
         ((Concept::ConstVector<Type>)),
         (typename VectorTraits<Type>::ValueType))
      squaredNorm(const Type& vec) {
         return dot(vec, vec);
      }

   
      namespace Impl {
      
        template <class V, int I>
        struct VectorAccess {
           typename VectorTraits<V>::ValueType operator()(const V& vec) const { return get<I>(vec); }
        };


        template<class _Type>
        struct Min : public std::binary_function<_Type, _Type, _Type> {	
           _Type operator()(const _Type& left, const _Type& right) const {	
              return std::min<_Type>(left, right);
           }
        };


        template<class _Type>
        struct Max : public std::binary_function<_Type, _Type, _Type> {
           _Type operator()(const _Type& left, const _Type& right) const {	
              return std::max<_Type>(left, right);
           }
        };


      }

      /**
       * @brief Returns the minimal element of a vector.
       * @ingroup openOR_core
       */
      template <class Type>
      inline
      OPENOR_CONCEPT_REQUIRES(
        ((Concept::ConstVector<Type>)),
        (typename VectorTraits<Type>::ValueType))
      minimalElement(const Type& vec) 
      {
         typedef typename VectorTraits<Type>::ValueType ValueType;
         return Impl::VectorCompileTimeIterator<Type>().template reduce<Impl::VectorAccess, ValueType, Impl::Min>(vec);
      }


      /**
       * @brief 
       * @ingroup openOR_core
       */
      template <class Type>
      inline
      OPENOR_CONCEPT_REQUIRES(
        ((Concept::ConstVector<Type>)),
        (typename VectorTraits<Type>::ValueType))
      maximalElement(const Type& vec) 
      {
        typedef typename VectorTraits<Type>::ValueType ValueType;
        return Impl::VectorCompileTimeIterator<Type>().template reduce<Impl::VectorAccess, ValueType, Impl::Max>(vec);
      }


      /**
       * @brief 
       * @ingroup openOR_core
       */
      template <class Type>
      inline
      OPENOR_CONCEPT_REQUIRES(
        ((Concept::ConstVector<Type>)),
        (typename VectorTraits<Type>::ValueType))
      summarizedElements(const Type& vec) 
      {
        typedef typename VectorTraits<Type>::ValueType ValueType;
        return Impl::VectorCompileTimeIterator<Type>().template reduce<Impl::VectorAccess, ValueType, std::plus>(vec);
      }


      /**
       * @brief 
       * @ingroup openOR_core
       */
      template <class Type>
      inline
      OPENOR_CONCEPT_REQUIRES(
        ((Concept::ConstVector<Type>)),
        (typename VectorTraits<Type>::ValueType))
      multipliedElements(const Type& vec) 
      {
        typedef typename VectorTraits<Type>::ValueType ValueType;
        return Impl::VectorCompileTimeIterator<Type>().template reduce<Impl::VectorAccess, ValueType, std::multiplies>(vec);
      }


      /**
       * @brief 
       * @ingroup openOR_core
       */
      template <class Type>
      inline
      OPENOR_CONCEPT_REQUIRES(
         ((Concept::Vector<Type>)),
         (Type&))
      normalize(Type& vec) {
         BOOST_MPL_ASSERT((boost::is_floating_point<typename VectorTraits<Type>::ValueType>));
         typedef typename ScalarTraits<typename VectorTraits<Type>::ValueType>::RealType RealType;
         RealType invNorm = (RealType) 1.0 / norm(vec);
         vec *= invNorm;
         return vec;
      }


      namespace Impl
      {
         template <class Vec, class Vec2, class Vec3, int I>
         struct ElementwiseMult {
            inline void operator()(Vec& p1, const Vec2& p2 , const Vec3& p3) const 
            {
               typedef typename VectorTraits<Vec>::ValueType ValueType;
               ValueType v2 = static_cast<ValueType>(get<I>(p2));
               ValueType v3 = static_cast<ValueType>(get<I>(p3));
               get<I>(p1) = v2 * v3;
            }
         };
      }

      /**
      * @brief 
      * @ingroup openOR_core
      */
      template <class Type, class Type2, class Type3>
      inline
         OPENOR_CONCEPT_REQUIRES(
         ((Concept::Vector<Type>))
         ((Concept::ConstVector<Type2))
         ((Concept::ConstVector<Type3)),
         (Type))
         elementProd(const Type2& left, const Type3& right) 
      {
         Type result;
         Impl::VectorCompileTimeIterator<Type>().template apply<Impl::ElementwiseMult, Type2, Type3>(result, left, right);
         return result;
      }

      template<class Type>
      inline Type fromString(const std::string& str) {
            Type retval;

            std::stringstream in;
            in.str(str);
            in >> retval;
            
            return retval;
      }


      ///**
      // * @brief 
      // * @ingroup openOR_core_math
      // */
      //template <class Type>
      //inline
      //OPENOR_CONCEPT_REQUIRES(
      //                        ((Concept::Vector<Type>)),
      //                        (Type&))
      //normalize(const Type& vec) {
      //   BOOST_MPL_ASSERT((boost::is_floating_point<typename VectorTraits<Type>::ValueType>));
      //   typename ScalarTraits<typename VectorTraits<Type>::ValueType>::RealType invNorm = 1.0 / norm(vec);
      //   Type res = vec * invNorm;
      //   return res;
      //}
      

      /**
       * @brief 
       * @ingroup openOR_core
       */
      template <class Type>
      inline
      OPENOR_CONCEPT_REQUIRES(
         ((Concept::ConstVector<Type>)),
         (typename VectorTraits<Type>::RealVectorType))
      normalized(const Type& vec) {
         typename VectorTraits<Type>::RealVectorType v(vec);
         normalize(v);
         return v;
      }


      namespace Impl {

         template <int I, class Type, class Type2>
         struct Cross {
            typedef invalid_type ResultType;
         };

         template<class Type, class Type2>
         struct Cross<2, Type, Type2> {
            typedef typename VectorTraits<Type>::ValueType ResultType;

            ResultType operator()(const Type& left, const Type2& right) const {
               return get<0>(left) * get<1>(right) - get<1>(left) * get<0>(right);
            }
         };


         template <class Type, class Type2>
         struct Cross<3, Type, Type2> {
            typedef typename VectorTraits<Type>::Vector3Type ResultType;

            ResultType operator()(const Type& left, const Type2& right) const {
               BOOST_MPL_ASSERT((boost::mpl::equal_to<typename VectorTraits<typename VectorTraits<Type>::Vector3Type>::Dimension, boost::mpl::int_<3> >));
               ResultType vec;
               get<0>(vec) = get<1>(left) * get<2>(right) - get<2>(left) * get<1>(right);
               get<1>(vec) = get<2>(left) * get<0>(right) - get<0>(left) * get<2>(right);
               get<2>(vec) = get<0>(left) * get<1>(right) - get<1>(left) * get<0>(right);
               return vec;
            }
         };

      }


       /**
       * @brief 
       * @ingroup openOR_core
       */
      template <class Type, class Type2>
      inline
      OPENOR_CONCEPT_REQUIRES(
         ((Concept::ConstVector<Type>))
         ((Concept::ConstVector<Type2>))
         ((Concept::Vector<typename VectorTraits<Type>::Vector3Type>)),
         (typename Impl::Cross<VectorTraits<Type>::Dimension::value, Type, Type2>::ResultType))
      cross(const Type& left, const Type2& right) {
         BOOST_MPL_ASSERT((boost::mpl::equal_to<typename VectorTraits<typename VectorTraits<Type>::Vector3Type>::Dimension, boost::mpl::int_<3> >));
         return typename Impl::Cross<VectorTraits<Type>::Dimension::value, Type, Type2>()(left, right);
      }

      
      /**
       * @brief 
       * @ingroup openOR_core
       */
      template <class Type>
      inline
      OPENOR_CONCEPT_REQUIRES(
                              ((Concept::ConstVector<Type>)),
                              (Type))
      rotate3(const Type& vec, const Type& vecAxis, const typename ScalarTraits<typename VectorTraits<Type>::ValueType>::RealType angle) {
         
         typedef typename VectorTraits<Type>::RealVectorType RealVectorType;
         typedef typename ScalarTraits<typename VectorTraits<Type>::ValueType>::RealType RealType;
         RealVectorType vecReal(vec);
         RealVectorType vecAxisReal(vecAxis);
         normalize(vecAxisReal);
         vecAxisReal *= static_cast<RealType>(dot(vec, vecAxis));
         
         
         // first radius vector
         RealVectorType vecRad1(vecReal - vecAxisReal);
         RealType length = norm(vecRad1);
         if (length < 0.0001) return vec;
         
         // second radius vector
         RealVectorType vecRad2 = cross(RealVectorType(vecAxis), vecRad1);
         normalize(vecRad1);
         normalize(vecRad2);
         
         RealType factor1 = length * cos(angle);
         RealType factor2 = length * sin(angle);
         
         RealVectorType result = vecAxisReal + vecRad1 * factor1 + vecRad2 * factor2;
         return Type(result);
      }
      

   }
}



#endif
