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

#ifndef openOR_core_Math_matrixconcept_hpp
#define openOR_core_Math_matrixconcept_hpp

#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/int.hpp>
#include <boost/concept/usage.hpp>
#include <boost/concept_check.hpp>
#include <boost/math/quaternion.hpp>

#include <openOR/Math/utilities.hpp>
#include <openOR/Math/traits.hpp>
#include <openOR/Math/vector.hpp>
#include <openOR/Math/ublasmatrix.hpp>
#include <openOR/Math/ublasvector.hpp>


namespace openOR {
   namespace Math {

      template <int I, int J, typename P>
         typename if_const < P,
                     typename MatrixTraits<typename boost::remove_const<P>::type>::Access::ConstAccessType,
                     typename MatrixTraits<typename boost::remove_const<P>::type>::Access::AccessType > 
      ::type get(P& p) {
         return MatrixTraits<typename boost::remove_const<P>::type>::Access::template get<I, J>(p);
      }


      namespace Impl {


         template < class Mat,
         int BeginRow = 0,
         int EndRow = MatrixTraits<Mat>::RowDimension::value,
         int BeginCol = 0,
         int EndCol = MatrixTraits<Mat>::ColDimension::value >
         class MatrixCompileTimeIterator {
               static const int SizeRow = EndRow - BeginRow;
               static const int SizeCol = EndCol - BeginCol;

            public:

               template<template<class, int, int> class Op>
               inline void apply() const {
                  BOOST_MPL_ASSERT((boost::mpl::less_equal<boost::mpl::int_<BeginRow>, boost::mpl::int_<EndRow> >));
                  BOOST_MPL_ASSERT((boost::mpl::less_equal<boost::mpl::int_<BeginCol>, boost::mpl::int_<EndCol> >));
                  Apply0<0, SizeRow * SizeCol, Op>()();
               }

               template<template<class, class, int, int> class Op, class Mat2>
               inline void apply(Mat& mat, const Mat2& mat2) {
                  BOOST_MPL_ASSERT((boost::mpl::less_equal<boost::mpl::int_<BeginRow>, boost::mpl::int_<EndRow> >));
                  BOOST_MPL_ASSERT((boost::mpl::less_equal<boost::mpl::int_<BeginCol>, boost::mpl::int_<EndCol> >));
                  Apply2<0, SizeRow * SizeCol, Op, Mat2>()(mat, mat2);
               }

               template<template <class, int, int> class Op, typename Result, template<class> class Reducer>
               inline Result reduce(const Mat& mat) const {
                  BOOST_MPL_ASSERT((boost::mpl::less<boost::mpl::int_<BeginRow>, boost::mpl::int_<EndRow> >));
                  BOOST_MPL_ASSERT((boost::mpl::less<boost::mpl::int_<BeginCol>, boost::mpl::int_<EndCol> >));
                  return Reduce1 < 0, SizeRow * SizeCol - 1, Op, Result, Reducer > ()(mat);
               }

               template<template <class, class, int, int> class Op, class Mat2, typename Result, template<class> class Reducer>
               inline Result reduce(const Mat& mat, const Mat2& mat2) const {
                  BOOST_MPL_ASSERT((boost::mpl::less<boost::mpl::int_<BeginRow>, boost::mpl::int_<EndRow> >));
                  BOOST_MPL_ASSERT((boost::mpl::less<boost::mpl::int_<BeginCol>, boost::mpl::int_<EndCol> >));
                  return Reduce2 < 0, SizeRow * SizeCol - 1, Op, Mat2, Result, Reducer > ()(mat, mat2);
               }

            private:

               template<int I, int Count, template<class, int, int> class Op>
               struct Apply0 {
                  inline void operator()() const {
                     Op < Mat, BeginRow + (I % SizeRow), BeginCol + (I / SizeRow) > ()();
                     Apply0 < I + 1, Count, Op > ()();
                  }
               };


               template<int Count, template<class, int, int> class Op>
               struct Apply0<Count, Count, Op> {
                  inline void operator()() const {}
               };


               template<int I, int Count, template<class, class, int, int> class Op, class Mat2>
               struct Apply2 {
                  inline void operator()(Mat& mat, const Mat2& mat2) const {
                     Op < Mat, Mat2, BeginRow + (I % SizeRow), BeginCol + (I / SizeRow) > ()(mat, mat2);
                     Apply2 < I + 1, Count, Op, Mat2 > ()(mat, mat2);
                  }
               };


               template<int Count, template<class, class, int, int> class Op, class Mat2>
               struct Apply2<Count, Count, Op, Mat2> {
                  inline void operator()(Mat& mat, const Mat2& mat2) const {}
               };


               template<int I, int Count, template <class, int, int> class Op, typename Result, template<class> class Reducer>
               struct Reduce1 {
                  inline Result operator()(const Mat& mat) const {
                     return Reducer<Result>()(Op < Mat, BeginRow + (I % SizeRow), BeginCol + (I / SizeRow) > ()(mat),
                                              Reduce1 < I + 1, Count, Op, Result, Reducer > ()(mat));
                  }
               };


               template<int Count, template <class, int, int> class Op, typename Result, template<class> class Reducer>
               struct Reduce1<Count, Count, Op, Result, Reducer> {
                  inline Result operator()(const Mat& mat) {
                     return Op < Mat, BeginRow + (Count % SizeRow), BeginCol + (Count / SizeRow) > ()(mat);
                  }
               };


               template<int I, int Count, template <class, class, int, int> class Op, class Mat2, typename Result, template<class> class Reducer>
               struct Reduce2 {
                  inline Result operator()(const Mat& mat, const Mat2& mat2) {
                     return Reducer<Result>()(Op < Mat, Mat2, BeginRow + (I % SizeRow), BeginCol + (I / SizeRow) > ()(mat, mat2),
                                              Reduce2 < I + 1, Count, Op, Mat2, Result, Reducer > ()(mat, mat2));
                  }
               };

               template<int Count, template <class, class, int, int> class Op, class Mat2, typename Result, template<class> class Reducer>
               struct Reduce2<Count, Count, Op, Mat2, Result, Reducer> {
                  inline Result operator()(const Mat& mat, const Mat2& mat2) {
                     return Op < Mat, Mat2, BeginRow + (Count % SizeRow), BeginCol + (Count / SizeRow) > ()(mat, mat2);
                  }
               };

         };

      }


      namespace Concept {

         template <typename Type>
         class ConstMatrix {
            private:

               BOOST_MPL_ASSERT((typename MatrixTraits<Type>::IsMatrix));

               typedef typename MatrixTraits<Type>::ValueType ValueType;
               typedef typename MatrixTraits<Type>::RowVectorType RowVectorType;
               typedef typename MatrixTraits<Type>::ColVectorType ColVectorType;
               enum { ROW_DIMENSION = MatrixTraits<Type>::RowDimension::value,
                      COL_DIMENSION = MatrixTraits<Type>::ColDimension::value
                 };


               template <class M, int I, int J>
               struct AccessCheck {
                  inline void operator()() const {
                     const M* p = 0;
                     typename MatrixTraits<M>::ValueType val(Math::get<I, J>(*p));
                     (void)sizeof(val); // To avoid "unused variable" warnings
                  }
               };

            public :
               /// BCCL macro to check the ConstPoint concept
               BOOST_CONCEPT_USAGE(ConstMatrix) {
                  Impl::MatrixCompileTimeIterator<Type, 0, ROW_DIMENSION, 0, COL_DIMENSION>().template apply<AccessCheck>();
                  Type* pMat = NULL;
                  const RowVectorType* pRowVector = NULL;
                  ColVectorType colvec(Math::prod(*pMat, *pRowVector));
               }
         };


         template<class Type>
         class Matrix : public boost::DefaultConstructible<Type>, public boost::CopyConstructible<Type>, public boost::Assignable<Type> {
            private:

               BOOST_CONCEPT_ASSERT((Concept::ConstMatrix<Type>));
               BOOST_MPL_ASSERT((typename MatrixTraits<Type>::IsMatrix));

               typedef typename MatrixTraits<Type>::ValueType ValueType;
               enum { ROW_DIMENSION = MatrixTraits<Type>::RowDimension::value,
                      COL_DIMENSION = MatrixTraits<Type>::ColDimension::value
                 };


               template <class M, int I, int J>
               struct AccessCheck {
                  void operator()() const {
                     M* p = NULL;
                     get<I, J>(*p) = typename MatrixTraits<M>::ValueType();
                  }
               };


               template<typename P, int RowCount, int ColCount>
               struct MatrixProductChecker {
                  static void check() {}
               };

               template<typename P, int Count>
               struct MatrixProductChecker<P, Count, Count> {
                  static void check() {
                     P* p = 0;
                     *p = Math::prod(*p, *p);
                  }
               };

            public:
               /// BCCL macro to check the Point concept
               BOOST_CONCEPT_USAGE(Matrix) {
                  Impl::MatrixCompileTimeIterator<Type, 0, ROW_DIMENSION, 0, COL_DIMENSION>().template apply<AccessCheck>();
                  MatrixProductChecker<Type, ROW_DIMENSION, COL_DIMENSION>::check();
                  ValueType v;
                  Type t;
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
