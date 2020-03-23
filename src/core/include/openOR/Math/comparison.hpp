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

#ifndef openOR_core_math_comparison_hpp
#define openOR_core_math_comparison_hpp

#include <openOR/Math/vector.hpp>
#include <openOR/Math/matrix.hpp>

namespace openOR {
	namespace Math {
		//biref implementation of comparison for different types for the following accuracy epsilon.
		static const int FLOATING_POINT_EPSILON_FACTOR = 10000;

		template<class T> struct EqualTo : std::equal_to<T> {};
		template<class T> struct NotEqualTo : std::not_equal_to<T> {};
		template<class T> struct Less : std::less<T> {};
		template<class T> struct LessEqual : std::less_equal<T> {};
		template<class T> struct Greater : std::greater<T> {};
		template<class T> struct GreaterEqual : std::greater_equal<T> {};


		template<class T>
		struct RealEqualTo : public std::binary_function<T, T, bool> {
			bool operator()(const T& left, const T& right) {

				BOOST_MPL_ASSERT((boost::is_floating_point<T>));
				T x = std::numeric_limits<T>::epsilon() * FLOATING_POINT_EPSILON_FACTOR * std::max<T>(std::abs(left), 1);
				T y = std::numeric_limits<T>::epsilon() * FLOATING_POINT_EPSILON_FACTOR * std::max<T>(std::abs(right), 1);
				T diff = std::abs(left - right);
				return (diff < x) || (diff < y);
			}
		};


		template<class T>
		struct RealNotEqualTo : public std::binary_function<T, T, bool> {
			bool operator()(const T& left, const T& right) {
				return !RealEqualTo<T>()(left, right);
			}
		};


		template<class T>
		struct RealLess : public std::binary_function<T, T, bool> {
			bool operator()(const T& left, const T& right) {
				return left < right && !RealEqualTo<T>()(left, right);
			}
		};


		template<class T>
		struct RealLessEqual : public std::binary_function<T, T, bool> {
			bool operator()(const T& left, const T& right) {
				return left < right || RealEqualTo<T>()(left, right);
			}
		};

		template<class T>
		struct RealGreater : public std::binary_function<T, T, bool> {
			bool operator()(const T& left, const T& right) {
				return left > right && !RealEqualTo<T>()(left, right);
			}
		};


		template<class T>
		struct RealGreaterEqual : public std::binary_function<T, T, bool> {
			bool operator()(const T& left, const T& right) {
				return left > right || RealEqualTo<T>()(left, right);
			}
		};


		template<> struct EqualTo<float>      : RealEqualTo<float> {};
		template<> struct NotEqualTo<float>   : RealNotEqualTo<float> {};
		template<> struct Less<float>         : RealLess<float> {};
		template<> struct LessEqual<float>    : RealLessEqual<float> {};
		template<> struct Greater<float>      : RealGreater<float> {};
		template<> struct GreaterEqual<float> : RealGreaterEqual<float> {};

		template<> struct EqualTo<double>      : RealEqualTo<double> {};
		template<> struct NotEqualTo<double>   : RealNotEqualTo<double> {};
		template<> struct Less<double>         : RealLess<double> {};
		template<> struct LessEqual<double>    : RealLessEqual<double> {};
		template<> struct Greater<double>      : RealGreater<double> {};
		template<> struct GreaterEqual<double> : RealGreaterEqual<double> {};

		template<> struct EqualTo<long double>      : RealEqualTo<long double> {};
		template<> struct NotEqualTo<long double>   : RealNotEqualTo<long double> {};
		template<> struct Less<long double>         : RealLess<long double> {};
		template<> struct LessEqual<long double>    : RealLessEqual<long double> {};
		template<> struct Greater<long double>      : RealGreater<long double> {};
		template<> struct GreaterEqual<long double> : RealGreaterEqual<long double> {};


		// fwd decl to use for [Vector|Matrix]EqualToCheck.
		template<typename Type>
		const bool isEqualTo(const Type& left, const Type& right);

		namespace Impl {
			template <class V, class V2, int I>
			struct VectorEqualToCheck {
				bool operator()(const V& vec, const V2& vec2) const {
					return isEqualTo(get<I>(vec), get<I>(vec2));
				}
			};
		}


		template<class Type>
		struct VectorEqualTo : public std::binary_function<Type, Type, bool> {
			bool operator()(const Type& left, const Type& right) {
				BOOST_MPL_ASSERT((typename VectorTraits<Type>::IsVector));
				BOOST_MPL_ASSERT((boost::mpl::less<boost::mpl::int_<0>, typename VectorTraits<Type>::Dimension>));
				return Impl::VectorCompileTimeIterator<Type>().template reduce<Impl::VectorEqualToCheck, Type, bool, std::logical_and>(left, right);
			}
		};


		template<class Type>
		struct VectorNotEqualTo : public std::binary_function<Type, Type, bool> {
			bool operator()(const Type& left, const Type& right) {
				return !VectorEqualTo<Type>()(left, right);
			}
		};


		template<class Type>
		struct VectorPerpendicularTo : public std::binary_function<Type, Type, bool> {
			bool operator()(const Type& left, const Type& right) {
				BOOST_MPL_ASSERT((typename VectorTraits<Type>::IsVector));
				if (boost::is_floating_point<typename VectorTraits<Type>::ValueType>::value) {
					return EqualTo<typename VectorTraits<Type>::ValueType>()(0, Math::dot(left, right) / std::max<typename VectorTraits<Type>::ValueType>(norm(left), norm(right)));
				} else {
					return EqualTo<typename VectorTraits<Type>::ValueType>()(0, Math::dot(left, right));
				}
			}
		};



		namespace Impl {
			template <class M, class M2, int I, int J>
			struct MatrixEqualToCheck {
				bool operator()(const M& mat, const M2& mat2) const {
					return isEqualTo(get<I, J>(mat), get<I, J>(mat2));
				}
			};
		}


		template<class Type>
		struct MatrixEqualTo : public std::binary_function<Type, Type, bool> {
			bool operator()(const Type& left, const Type& right) {
				BOOST_MPL_ASSERT((typename MatrixTraits<Type>::IsMatrix));
				BOOST_MPL_ASSERT((boost::mpl::less<boost::mpl::int_<0>, typename MatrixTraits<Type>::RowDimension>));
				BOOST_MPL_ASSERT((boost::mpl::less<boost::mpl::int_<0>, typename MatrixTraits<Type>::ColDimension>));
				return Impl::MatrixCompileTimeIterator<Type>().template reduce<Impl::MatrixEqualToCheck, Type, bool, std::logical_and>(left, right);
			}
		};


		template<class Type>
		struct MatrixNotEqualTo : public std::binary_function<Type, Type, bool> {
			bool operator()(const Type& left, const Type& right) {
				return !MatrixEqualTo<Type>()(left, right);
			}
		};



		/**
		* \brief isEqualTo
		* \ingroup openOR_core
		*/
		template<typename Type>
		const bool isEqualTo(const Type& left, const Type& right) {
			typedef typename boost::mpl::if_ < typename VectorTraits<Type>::IsVector,
				VectorEqualTo<Type>,
				typename boost::mpl::if_ < typename MatrixTraits<Type>::IsMatrix,
				MatrixEqualTo<Type>,
				EqualTo<Type> >::type >::type Functor;
			return Functor()(left, right);
		}


		/**
		* \brief isNotEqualTo
		* \ingroup openOR_core
		*/
		template<typename Type>
		const bool isNotEqualTo(const Type& left, const Type& right) {
			typedef typename boost::mpl::if_ < typename VectorTraits<Type>::IsVector,
				VectorNotEqualTo<Type>,
				typename boost::mpl::if_ < typename MatrixTraits<Type>::IsMatrix,
				MatrixNotEqualTo<Type>,
				NotEqualTo<Type> >::type >::type Functor;

			return Functor()(left, right);
		}


		/**
		* @brief isLess
		* @ingroup openOR_core
		*/
		template<typename Type>
		const bool isLess(const Type& left, const Type& right) { return Less<Type>()(left, right); }


		/**
		* @brief isLessEqual
		* @ingroup openOR_core
		*/
		template<typename Type>
		const bool isLessEqual(const Type& left, const Type& right) { return LessEqual<Type>()(left, right); }


		/**
		* @brief isGreater
		* @ingroup openOR_core
		*/
		template<typename Type>
		const bool isGreater(const Type& left, const Type& right) { return Greater<Type>()(left, right); }


		/**
		* @brief isGreaterEqual
		* @ingroup openOR_core
		*/
		template<typename Type>
		const bool isGreaterEqual(const Type& left, const Type& right) { return GreaterEqual<Type>()(left, right); }


		/**
		* @brief isPerpendicularTo
		* @ingroup openOR_core
		*/
		template<typename Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::ConstVector<Type>)),
			(const bool))
			isPerpendicularTo(const Type& left, const Type& right) { return VectorPerpendicularTo<Type>()(left, right); }


		/**
		* @brief isOrthogonal
		* @ingroup openOR_core
		*/
		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::ConstMatrix<Type>)),
			(const bool))
			isOrthogonal(const Type& mat) {
				return (isPerpendicularTo(xAxis(mat), yAxis(mat)) &&
					isPerpendicularTo(xAxis(mat), zAxis(mat)) &&
					isPerpendicularTo(yAxis(mat), zAxis(mat)));
		}


		/**
		* @brief isHomogeneous
		* @ingroup openOR_core
		*/
		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::ConstMatrix<Type>)),
			(const bool))
			isHomogeneous(const Type& mat) {
				typedef typename MatrixTraits<Type>::ValueType ValueType;
				return (isOrthogonal(mat) &&
					isEqualTo(norm(xAxis(mat)), static_cast<ValueType>(1)) &&
					isEqualTo(norm(yAxis(mat)), static_cast<ValueType>(1)) &&
					isEqualTo(norm(zAxis(mat)), static_cast<ValueType>(1)) &&
					isEqualTo(get<3, 0>(mat), static_cast<ValueType>(0)) &&
					isEqualTo(get<3, 1>(mat), static_cast<ValueType>(0)) &&
					isEqualTo(get<3, 2>(mat), static_cast<ValueType>(0)) &&
					isEqualTo(get<3, 3>(mat), static_cast<ValueType>(1)));
		}

	}
}


#endif

