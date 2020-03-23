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
* @ingroup openOR_core_math
*/

#ifndef openOR_core_Math_detail_determinant_impl_hpp
#define openOR_core_Math_detail_determinant_impl_hpp

#include <boost/mpl/assert.hpp>
#include <boost/mpl/void.hpp>
#include <boost/type_traits/is_same.hpp>

#include <openOR/Math/matrix.hpp>


namespace openOR {
	namespace Math {
		namespace Impl {

			template<class Type, int Dim>
			struct Determinant {
				typename MatrixTraits<Type>::ValueType operator()(const Type& mat) const {
					// function has to be implemented

					BOOST_MPL_ASSERT((boost::is_same<Type, boost::mpl::void_>));
				}
			};

			//determinant of 2x2 matrix
			template<class Type>
			struct Determinant<Type, 2> {
				typename MatrixTraits<Type>::ValueType operator()(const Type& mat) const {
					return Math::get<0, 0>(mat) * Math::get<1, 1>(mat) - Math::get<0, 1>(mat) * Math::get<1, 0>(mat);
				}
			};

			//determinant of 3x3 matrix
			template<class Type>
			struct Determinant<Type, 3> {
				typename MatrixTraits<Type>::ValueType operator()(const Type& mat) const {
					return Math::get<0, 0>(mat) * Math::get<1, 1>(mat) * Math::get<2, 2>(mat) +
						Math::get<0, 1>(mat) * Math::get<1, 2>(mat) * Math::get<2, 0>(mat) +
						Math::get<0, 2>(mat) * Math::get<1, 0>(mat) * Math::get<2, 1>(mat) -
						Math::get<2, 0>(mat) * Math::get<1, 1>(mat) * Math::get<0, 2>(mat) -
						Math::get<2, 1>(mat) * Math::get<1, 2>(mat) * Math::get<0, 0>(mat) -
						Math::get<2, 2>(mat) * Math::get<1, 0>(mat) * Math::get<0, 1>(mat);
				}
			};
			//determinant of 4x4 matrix
			template<class Type>
			struct Determinant<Type, 4> {
				typename MatrixTraits<Type>::ValueType operator()(const Type& mat) const {
					return   Math::get<0, 0>(mat) * Math::get<1, 1>(mat) * Math::get<2, 2>(mat) * Math::get<3, 3>(mat) +
						Math::get<0, 0>(mat) * Math::get<1, 2>(mat) * Math::get<2, 3>(mat) * Math::get<3, 1>(mat) +
						Math::get<0, 0>(mat) * Math::get<1, 3>(mat) * Math::get<2, 1>(mat) * Math::get<3, 2>(mat) +

						Math::get<0, 1>(mat) * Math::get<1, 0>(mat) * Math::get<2, 3>(mat) * Math::get<3, 2>(mat) +
						Math::get<0, 1>(mat) * Math::get<1, 2>(mat) * Math::get<2, 0>(mat) * Math::get<3, 3>(mat) +
						Math::get<0, 1>(mat) * Math::get<1, 3>(mat) * Math::get<2, 2>(mat) * Math::get<3, 0>(mat) +

						Math::get<0, 2>(mat) * Math::get<1, 0>(mat) * Math::get<2, 1>(mat) * Math::get<3, 3>(mat) +
						Math::get<0, 2>(mat) * Math::get<1, 1>(mat) * Math::get<2, 3>(mat) * Math::get<3, 0>(mat) +
						Math::get<0, 2>(mat) * Math::get<1, 3>(mat) * Math::get<2, 0>(mat) * Math::get<3, 1>(mat) +

						Math::get<0, 3>(mat) * Math::get<1, 0>(mat) * Math::get<2, 2>(mat) * Math::get<3, 1>(mat) +
						Math::get<0, 3>(mat) * Math::get<1, 1>(mat) * Math::get<2, 0>(mat) * Math::get<3, 2>(mat) +
						Math::get<0, 3>(mat) * Math::get<1, 2>(mat) * Math::get<2, 1>(mat) * Math::get<3, 0>(mat) -

						Math::get<0, 0>(mat) * Math::get<1, 1>(mat) * Math::get<2, 3>(mat) * Math::get<3, 2>(mat) -
						Math::get<0, 0>(mat) * Math::get<1, 2>(mat) * Math::get<2, 1>(mat) * Math::get<3, 3>(mat) -
						Math::get<0, 0>(mat) * Math::get<1, 3>(mat) * Math::get<2, 2>(mat) * Math::get<3, 1>(mat) -

						Math::get<0, 1>(mat) * Math::get<1, 0>(mat) * Math::get<2, 2>(mat) * Math::get<3, 3>(mat) -
						Math::get<0, 1>(mat) * Math::get<1, 2>(mat) * Math::get<2, 3>(mat) * Math::get<3, 0>(mat) - 
						Math::get<0, 1>(mat) * Math::get<1, 3>(mat) * Math::get<2, 0>(mat) * Math::get<3, 2>(mat) -

						Math::get<0, 2>(mat) * Math::get<1, 0>(mat) * Math::get<2, 3>(mat) * Math::get<3, 1>(mat) -
						Math::get<0, 2>(mat) * Math::get<1, 1>(mat) * Math::get<2, 0>(mat) * Math::get<3, 3>(mat) -
						Math::get<0, 2>(mat) * Math::get<1, 3>(mat) * Math::get<2, 1>(mat) * Math::get<3, 0>(mat) -

						Math::get<0, 3>(mat) * Math::get<1, 0>(mat) * Math::get<2, 1>(mat) * Math::get<3, 2>(mat) -
						Math::get<0, 3>(mat) * Math::get<1, 1>(mat) * Math::get<2, 2>(mat) * Math::get<3, 0>(mat) -
						Math::get<0, 3>(mat) * Math::get<1, 2>(mat) * Math::get<2, 0>(mat) * Math::get<3, 1>(mat);
				}
			};


		}
	}
}


#endif
