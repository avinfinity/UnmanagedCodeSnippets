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

#ifndef openOR_core_Math_vectorconcept_hpp
#define openOR_core_Math_vectorconcept_hpp

#include <boost/mpl/assert.hpp>
#include <boost/mpl/less_equal.hpp>
#include <boost/concept/usage.hpp>
#include <boost/concept_check.hpp>

#include <openOR/Utility/conceptcheck.hpp>
#include <openOR/Math/traits.hpp>

namespace openOR {
   namespace Math {


      template <int I, typename P>
         typename if_const < P,
                     typename VectorTraits<typename boost::remove_const<P>::type>::Access::ConstAccessType,
                     typename VectorTraits<typename boost::remove_const<P>::type>::Access::AccessType >
      ::type get(P& p) {
         return VectorTraits<typename boost::remove_const<P>::type>::Access::template get<I>(p);
      }



      namespace Impl {

         template < class Vec,
         int Begin = 0,
         int End = VectorTraits<Vec>::Dimension::value >
         class VectorCompileTimeIterator {

            public:

               template<template <class, int> class Op>
               inline void apply() const {
                  BOOST_MPL_ASSERT((boost::mpl::less_equal<boost::mpl::int_<Begin>, boost::mpl::int_<End> >));
                  Apply0<Begin, End, Op>()();
               }

               template<template <class, class, int> class Op, class Vec2>
               inline void apply(Vec& vec, const Vec2& vec2) const {
                  BOOST_MPL_ASSERT((boost::mpl::less_equal<boost::mpl::int_<Begin>, boost::mpl::int_<End> >));
                  Apply2<Begin, End, Op, Vec2>()(vec, vec2);
               }

               template<template <class, class, class, int> class Op, class Vec2, class Vec3>
               inline void apply(Vec& vec, const Vec2& vec2, const Vec3& vec3) const {
                  BOOST_MPL_ASSERT((boost::mpl::less_equal<boost::mpl::int_<Begin>, boost::mpl::int_<End> >));
                  Apply3<Begin, End, Op, Vec2, Vec3>()(vec, vec2, vec3);
               }

               template<template <class, int> class Op, typename Result, template<class> class Reducer>
               inline Result reduce(const Vec& vec) const {
                 BOOST_MPL_ASSERT((boost::mpl::less<boost::mpl::int_<Begin>, boost::mpl::int_<End> >));
                 return Reduce1 < Begin, End - 1, Op, Result, Reducer > ()(vec);
               }

               template<template <class, class, int> class Op, class Vec2, typename Result, template<class> class Reducer>
               inline Result reduce(const Vec& vec, const Vec2& vec2) const {
                  BOOST_MPL_ASSERT((boost::mpl::less<boost::mpl::int_<Begin>, boost::mpl::int_<End> >));
                  return Reduce2 < Begin, End - 1, Op, Vec2, Result, Reducer > ()(vec, vec2);
               }


            private:

               template<int I, int Count, template <class, int> class Op>
               struct Apply0 {
                  inline void operator()() const {
                     Op<Vec, I>()();
                     Apply0 < I + 1, Count, Op > ()();
                  }
               };

               template<int Count, template <class, int> class Op>
               struct Apply0<Count, Count, Op> {
                  inline void operator()() const {}
               };

               template<int I, int Count, template <class, class, int> class Op, class Vec2>
               struct Apply2 {
                  inline void operator()(Vec& vec, const Vec2& vec2) const {
                     Op<Vec, Vec2, I>()(vec, vec2);
                     Apply2 < I + 1, Count, Op, Vec2 > ()(vec, vec2);
                  }
               };

               template<int Count, template <class, class, int> class Op, class Vec2>
               struct Apply2<Count, Count, Op, Vec2> {
                  inline void operator()(Vec& vec, const Vec2& vec2) const {}
               };

               template<int I, int Count, template <class, class, class, int> class Op, class Vec2, class Vec3>
               struct Apply3 {
                  inline void operator()(Vec& vec, const Vec2& vec2, const Vec3& vec3) const {
                     Op<Vec, Vec2, Vec3, I>()(vec, vec2, vec3);
                     Apply3 < I + 1, Count, Op, Vec2, Vec3> ()(vec, vec2, vec3);
                  }
               };

               template<int Count, template <class, class, class, int> class Op, class Vec2, class Vec3>
               struct Apply3<Count, Count, Op, Vec2, Vec3> {
                  inline void operator()(Vec&, const Vec2&, const Vec3&) const {
                  }
               };

               template<int I, int Count, template <class, int> class Op, typename Result, template<class> class Reducer>
               struct Reduce1 {
                  inline Result operator()(const Vec& vec) {
                     Result r1 = Op<Vec, I>()(vec);
                     Result r2 = Reduce1 < I + 1, Count, Op, Result, Reducer > ()(vec);
                     return Reducer<Result>()(r1, r2);
                  }
               };

               template<int Count, template <class, int> class Op, typename Result, template<class> class Reducer>
               struct Reduce1<Count, Count, Op, Result, Reducer> {
                  inline Result operator()(const Vec& vec) {
                     return Op<Vec, Count>()(vec);
                  }
               };

               template<int I, int Count, template <class, class, int> class Op, class Vec2, typename Result, template<class> class Reducer>
               struct Reduce2 {
                  inline Result operator()(const Vec& vec, const Vec2& vec2) {
                     return Reducer<Result>()(Op<Vec, Vec2, I>()(vec, vec2), Reduce2 < I + 1, Count, Op, Vec2, Result, Reducer > ()(vec, vec2));
                  }
               };

               template<int Count, template <class, class, int> class Op, class Vec2, typename Result, template<class> class Reducer>
               struct Reduce2<Count, Count, Op, Vec2, Result, Reducer> {
                  inline Result operator()(const Vec& vec, const Vec2& vec2) {
                     return Op<Vec, Vec2, Count>()(vec, vec2);
                  }
               };
         };

      }




      namespace Concept {

         template <typename Type>
         class ConstVector {
            private:

               BOOST_MPL_ASSERT((typename VectorTraits<Type>::IsVector));
               // FIX THIS!!! TODO?
               //BOOST_MPL_ASSERT((boost::mpl::greater_equal<
               //                                    typename VectorTraits<Type>::Dimension,
               //                                    boost::mpl::int_<0> >
               //                 ));

               typedef typename VectorTraits<Type>::ValueType ValueType;
               enum { DIMENSION = VectorTraits<Type>::Dimension::value };

               template <class V, int I>
               struct AccessCheck {
                  void operator()() const {
                     const V* p = 0;
                     typename VectorTraits<V>::ValueType val(get<I>(*p));
                     (void)sizeof(val); // To avoid "unused variable" warnings
                  }
               };

            public :
               /// BCCL macro to check the ConstPoint concept
               BOOST_CONCEPT_USAGE(ConstVector) {
                  Impl::VectorCompileTimeIterator<Type>().template apply<AccessCheck>();
                  
                  // TODO: FIX THIS!!!
                  // Type* p = NULL;
                  //ValueType norm(Math::norm(*p));
                  //ValueType dotv(Math::dot(*p, *p));
               }
         };


         template<class Type>
         class Vector : public boost::DefaultConstructible<Type>, public boost::CopyConstructible<Type>, public boost::Assignable<Type> {
            private:

               BOOST_MPL_ASSERT((typename VectorTraits<Type>::IsVector)); // A valid vector trait has to be defined for "Type"
               BOOST_CONCEPT_ASSERT((Concept::ConstVector<Type>));        // "Type" has to fullfil the ConstVector concept
            //BOOST_MPL_ASSERT((boost::mpl::greater<typename VectorTraits<Type>::Dimension, boost::mpl::int_<0> >)); // "Type" must have a valid dimension

               typedef typename VectorTraits<Type>::ValueType ValueType;
               enum { DIMENSION = VectorTraits<Type>::Dimension::value };

               template <class V, int I>
               struct AccessCheck {
                  void operator()() const {
                     V* p = NULL;
                     get<I>(*p) = typename VectorTraits<V>::ValueType();
                  }
               };


            public:
               /// BCCL macro to check the Point concept
               BOOST_CONCEPT_USAGE(Vector) {
                  Impl::VectorCompileTimeIterator<Type>().template apply<AccessCheck>();
                  Type t;
                  ValueType v;
                  t *= v;
                  t = v * t;
                  t = t * v;
                  t = t + t;
                  t = t - t;
                  t += t;
                  t -= t;
               }
         };

      }

   }
}

#endif
