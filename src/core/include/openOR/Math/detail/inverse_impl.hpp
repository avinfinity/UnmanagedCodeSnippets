//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//!
//! @file
//! @ingroup openOR_core_math

#ifndef openOR_core_Math_detail_inverse_hpp
#define openOR_core_Math_detail_inverse_hpp

#include <boost/numeric/ublas/lu.hpp>

#include <boost/mpl/assert.hpp>
#include <boost/mpl/void.hpp>
#include <boost/type_traits/is_same.hpp>

#include <openOR/Log/Logger.hpp>
#include <openOR/Math/matrix.hpp>

#include <openOR/Math/determinant.hpp>

namespace openOR {
	namespace Math {

		struct no_inverse_matrix_error : public std::runtime_error {
			no_inverse_matrix_error() : std::runtime_error("Could not get matrix inverse") {
				LOG(Log::Level::Debug, Log::noAttribs, Log::msg("Exception thrown: Could not get matrix inverse") );
			}
		};

		namespace Impl {

			template <class Type, int Dim>
			struct Inverse {
				static Type get(const Type& mat) {
					// function has to be specialized
					BOOST_MPL_ASSERT((boost::is_same<Type, boost::mpl::void_>));
				}
			};


			template <class Type>
			struct Inverse<Type, 2> {
				static Type get(const Type& mat) {
					Type matResult;
					Math::get<0, 0>(matResult) = Math::get<1, 1>(mat);
					Math::get<0, 1>(matResult) = -Math::get<0, 1>(mat);
					Math::get<1, 0>(matResult) = -Math::get<1, 0>(mat);
					Math::get<1, 1>(matResult) = Math::get<0, 0>(mat);
					matResult /= Math::determinant(mat);
					return matResult;
				}
			};


			template <class Type>
			struct Inverse<Type, 3> {
				static Type get(const Type& mat) {
					Type matResult;
					Math::get<0, 0>(matResult) = Math::get<1, 1>(mat) * Math::get<2, 2>(mat) - Math::get<1, 2>(mat) * Math::get<2, 1>(mat);
					Math::get<0, 1>(matResult) = Math::get<0, 2>(mat) * Math::get<2, 1>(mat) - Math::get<0, 1>(mat) * Math::get<2, 2>(mat);
					Math::get<0, 2>(matResult) = Math::get<0, 1>(mat) * Math::get<1, 2>(mat) - Math::get<0, 2>(mat) * Math::get<1, 1>(mat);

					Math::get<1, 0>(matResult) = Math::get<1, 2>(mat) * Math::get<2, 0>(mat) - Math::get<1, 0>(mat) * Math::get<2, 2>(mat);
					Math::get<1, 1>(matResult) = Math::get<0, 0>(mat) * Math::get<2, 2>(mat) - Math::get<0, 2>(mat) * Math::get<2, 0>(mat);
					Math::get<1, 2>(matResult) = Math::get<0, 2>(mat) * Math::get<1, 0>(mat) - Math::get<0, 0>(mat) * Math::get<1, 2>(mat);

					Math::get<2, 0>(matResult) = Math::get<1, 0>(mat) * Math::get<2, 1>(mat) - Math::get<1, 1>(mat) * Math::get<2, 0>(mat);
					Math::get<2, 1>(matResult) = Math::get<0, 1>(mat) * Math::get<2, 0>(mat) - Math::get<0, 0>(mat) * Math::get<2, 1>(mat);
					Math::get<2, 2>(matResult) = Math::get<0, 0>(mat) * Math::get<1, 1>(mat) - Math::get<0, 1>(mat) * Math::get<1, 0>(mat);

					matResult /= Math::determinant(mat);
					return matResult;
				}
			};


			template <class Type>
			struct Inverse<Type, 4> {
				static Type get(const Type& mat) {
					Type inverse;

					using namespace boost::numeric::ublas;
					typedef permutation_matrix<std::size_t> pmatrix;

					// create a working copy of the input
					Type A(mat);

					// create a permutation matrix for the LU-factorization
					pmatrix pm(A.size1());

					// perform LU-factorization
					int res = lu_factorize(A,pm);
					if( res != 0 ) { throw no_inverse_matrix_error(); }

					// create identity matrix of "inverse"
					inverse.assign( Math::MatrixTraits<Type>::IDENTITY );

					// backsubstitute to get the inverse
					lu_substitute(A, pm, inverse);

					return inverse;
				}
			};

		}
	}
}


#endif
