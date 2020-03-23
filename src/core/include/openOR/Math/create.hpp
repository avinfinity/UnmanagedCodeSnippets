//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
/**
* @file
* @author Christian Winne
* @ingroup openOR_core
*/

#ifndef openOR_core_math_create_hpp
#define openOR_core_math_create_hpp

#include <openOR/Math/vector.hpp>
#include <openOR/Math/matrix.hpp>
#include <openOR/Math/matrixsetaxis.hpp>

#include <boost/mpl/assert.hpp>

namespace openOR {
	namespace Math {

		namespace Impl {
			/**
			* Functor for Impl::VectorCompileIterator in order to copy the content of source vector into destination vector
			* \internal
			*/
			template <class Vec, class VecSrc, int I>
			struct VectorCopy {
				void operator()(Vec& vec, const VecSrc& vecSrc) {
					get<I>(vec) = get<I>(vecSrc);
				}
			};

			template <class Mat, class MatSrc, int I, int J>
			struct MatrixCopy {
				void operator()(Mat& mat, const MatSrc& matSrc) {
					get<I, J>(mat) = get<I, J>(matSrc);
				}
			};


			/**
			* v0 is an vector because the destination vector is larger than Dimension 2.
			* \internal
			*/
			template<class Vec_type, class Type2, int Dim>
			struct CreateVec2 {
				static Vec_type create(const Type2& v0,
					typename VectorTraits<Vec_type>::ValueType v1) 
				{
					BOOST_MPL_ASSERT((typename VectorTraits<Vec_type>::IsVector));
					Vec_type t;
					Impl::VectorCompileTimeIterator < Vec_type, 0, VectorTraits<Vec_type>::Dimension::value - 1 > ()
						.template apply<Impl::VectorCopy, Type2>(t, v0);
					get < VectorTraits<Vec_type>::Dimension::value - 1 > (t) = v1;
					return t;
				}
			};

			/**
			* v0 is a ValueType because the vector to create has dimension 2.
			* \internal
			*/
			template<class Type, class Type2>
			struct CreateVec2<Type, Type2, 2> {
				static Type create(const Type2& v0,
					typename VectorTraits<Type>::ValueType v1) {
						Type t;
						get<0>(t) = static_cast<typename VectorTraits<Type>::ValueType>(v0);
						get<1>(t) = static_cast<typename VectorTraits<Type>::ValueType>(v1);
						return t;
				}
			};

			template<class Mat, class MatSrc, class Vec>
			struct CreateMat2 {
				static Mat create(
					const MatSrc& matSrc,
					const Vec& v) 
				{
					BOOST_MPL_ASSERT((typename MatrixTraits<Mat>::IsMatrix));
					BOOST_MPL_ASSERT((typename VectorTraits<Vec>::IsVector));
					Mat mat = MatrixTraits<Mat>::IDENTITY;
					Impl::MatrixCompileTimeIterator <Mat, 0, MatrixTraits<Mat>::RowDimension::value - 1, 0, MatrixTraits<Mat>::ColDimension::value - 1> ()
						.template apply<Impl::MatrixCopy, MatSrc>(mat, matSrc);

					for (int i = 0; i < VectorTraits<Vec>::Dimension::value; ++i)
					{
						mat(i, MatrixTraits<Mat>::ColDimension::value - 1) = v(i);
					}
					return mat;
				}
			};

			/**
			* 
			* \internal
			*/
			template<class Type, class Type2, class Type3, class IsVector>
			struct Create2 {
				static Type create(const Type2& v0, const Type3& v1) 
				{
					return CreateVec2<Type, Type2, VectorTraits<Type>::Dimension::value>::create(v0, v1);
				}
			};


			/**
			* 
			* \internal
			*/
			template<class Type, class Type2, class Type3>
			struct Create2<Type, Type2, Type3, boost::mpl::false_> {
				static Type create(const Type2& v0, const Type3& v1) 
				{
					return CreateMat2<Type, Type2, Type3>::create(v0, v1);
				}
			};


			/**
			* Type is a vector
			* \internal
			*/
			template<class Type, class Type2, class Type3, class Type4, class IsVector>
			struct Create3 {
				static Type create(const Type2& v0, 
					const Type3& v1, 
					const Type4& v2) {
						BOOST_MPL_ASSERT((typename VectorTraits<Type>::IsVector));
						Type t;
						get<0>(t) = static_cast<typename VectorTraits<Type>::ValueType>(v0);
						get<1>(t) = static_cast<typename VectorTraits<Type>::ValueType>(v1);
						get<2>(t) = static_cast<typename VectorTraits<Type>::ValueType>(v2);
						return t;
				}
			};


			/**
			* Type is matrix
			* \internal
			*/
			template<class Type, class Type2, class Type3, class Type4>
			struct Create3<Type, Type2, Type3, Type4, boost::mpl::false_> {
				static Type create(const Type2& v0, const Type3& v1, const Type4& v2) {
					BOOST_MPL_ASSERT((typename MatrixTraits<Type>::IsMatrix));
					Type t = MatrixTraits<Type>::IDENTITY;
					setXAxis(t, v0);
					setYAxis(t, v1);
					setZAxis(t, v2);
					return t;
				}
			};


			/**
			* Type is a vector
			* \internal
			*/
			template<class Type, class Type2, class Type3, class Type4, class Type5, class IsVector>
			struct Create4 {
				static Type create(const Type2& v0, const Type3& v1, const Type4& v2, const Type5& v3) {
					BOOST_MPL_ASSERT((typename VectorTraits<Type>::IsVector));
					Type t;
					get<0>(t) = static_cast<typename VectorTraits<Type>::ValueType>(v0);
					get<1>(t) = static_cast<typename VectorTraits<Type>::ValueType>(v1);
					get<2>(t) = static_cast<typename VectorTraits<Type>::ValueType>(v2);
					get<3>(t) = static_cast<typename VectorTraits<Type>::ValueType>(v3);
					return t;
				}
			};


			/** 
			* Type is matrix
			* \internal
			*/
			template<class Type, class Type2, class Type3, class Type4, class Type5>
			struct Create4<Type, Type2, Type3, Type4, Type5, boost::mpl::false_> {
				static Type create(const Type2& v0, const Type3& v1, const Type4& v2, const Type5& v3) {
					Type t = MatrixTraits<Type>::IDENTITY;
					setXAxis(t, v0);
					setYAxis(t, v1);
					setZAxis(t, v2);
					setTranslation(t, v3);
					return t;
				}
			};


			template<class Type, class Type2, class Type3, class Type4, class Type5, class Type6, class Type7, class IsVector>
			struct Create6 {
				static Type create(const Type2& v0, const Type3& v1, const Type4& v2, const Type5& v3, const Type6& v4, const Type7& v5) {
					BOOST_MPL_ASSERT((typename VectorTraits<Type>::IsVector));
					Type t;
					get<0>(t) = static_cast<typename VectorTraits<Type>::ValueType>(v0);
					get<1>(t) = static_cast<typename VectorTraits<Type>::ValueType>(v1);
					get<2>(t) = static_cast<typename VectorTraits<Type>::ValueType>(v2);
					get<3>(t) = static_cast<typename VectorTraits<Type>::ValueType>(v3);
					get<4>(t) = static_cast<typename VectorTraits<Type>::ValueType>(v4);
					get<5>(t) = static_cast<typename VectorTraits<Type>::ValueType>(v5);
					return t;
				}
			};
		}



		/** 
		* \brief create
		* \ingroup openOR_core
		*/
		template <class Type, class Vec>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::Matrix<Type>))
			((Concept::ConstVector<Vec>)),
			(Type))
			create(const Vec& vec) {
				Type mat = MatrixTraits<Type>::IDENTITY;
				setTranslation(mat, vec);
				return mat;
		}


		/**
		* @brief Creation of a vector with two parameters.
		* @ingroup openOR_core
		*/
		template<class Type, class Type2, class Type3>
		inline Type create(const Type2& v0,
			const Type3& v1) {
				return Impl::Create2<Type, Type2, Type3, typename VectorTraits<Type>::IsVector>::create(v0, v1);
		}

		/**
		* @brief Creation of a vector with three parameters.
		* @ingroup openOR_core
		*/
		template<class Type, class Type2, class Type3, class Type4>
		inline Type create(const Type2& v0,
			const Type3& v1,
			const Type4& v2) {
				return Impl::Create3<Type, Type2, Type3, Type4, typename VectorTraits<Type>::IsVector>::create(v0, v1, v2);
		}


		/**
		* @brief Creation of a vector with four parameters.
		* @ingroup openOR_core
		*/
		template<class Type, class Type2, class Type3, class Type4, class Type5>
		inline Type create(const Type2& v0, const Type3& v1, const Type4& v2, const Type5& v3) {
			return Impl::Create4<Type, Type2, Type3, Type4, Type5, typename VectorTraits<Type>::IsVector>::create(v0, v1, v2, v3);
		}


		/**
		* @brief Creation of a vector with six parameters.
		* @ingroup openOR_core
		*/
		template<class Type, class Type2, class Type3, class Type4, class Type5, class Type6, class Type7>
		inline Type create(const Type2& v0, const Type3& v1, const Type4& v2, const Type5& v3, const Type6& v4, const Type7& v5) {
			return Impl::Create6<Type, Type2, Type3, Type4, Type5, Type6, Type7, typename VectorTraits<Type>::IsVector>::create(v0, v1, v2, v3, v4, v5);
		}



	}
}


#endif
