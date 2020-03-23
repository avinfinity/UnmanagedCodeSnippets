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

#ifndef openOR_core_math_matrixfunctions_hpp
#define openOR_core_math_matrixfunctions_hpp

#include <boost/numeric/ublas/lu.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/equal_to.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/void.hpp>
#include <boost/type_traits/is_same.hpp>

#include <openOR/Utility/conceptcheck.hpp>
#include <openOR/Math/constants.hpp>
#include <openOR/Math/utilities.hpp>
#include <openOR/Math/matrix.hpp>
#include <openOR/Math/matrixsetaxis.hpp>
#include <openOR/Math/vector.hpp>
#include <openOR/Math/vectorfunctions.hpp>
#include <openOR/Math/create.hpp>

#include <openOR/Math/determinant.hpp>
#include <openOR/Math/detail/inverse_impl.hpp>

namespace openOR {
	namespace Math {

		/**
		* @brief Returns the x-axis vector of a 3x3 or 4x4 matrix.
		* @ingroup openOR_core
		*/
		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::ConstMatrix<Type>)),
			(typename MatrixTraits<Type>::Vector3Type))
			xAxis(const Type& mat) {
				return create<typename MatrixTraits<Type>::Vector3Type>(get<0, 0>(mat), get<1, 0>(mat), get<2, 0>(mat));
		}


		/**
		* @brief Returns the y-axis vector of a 3x3 or 4x4 matrix.
		* @ingroup openOR_core
		*/      
		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::ConstMatrix<Type>)),
			(typename MatrixTraits<Type>::Vector3Type))
			yAxis(const Type& mat) {
				return create<typename MatrixTraits<Type>::Vector3Type>(get<0, 1>(mat), get<1, 1>(mat), get<2, 1>(mat));
		}


		/**
		* @brief Returns the z-axis vector of a 3x3 or 4x4 matrix.
		* @ingroup openOR_core
		*/      
		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::ConstMatrix<Type>)),
			(typename MatrixTraits<Type>::Vector3Type))
			zAxis(const Type& mat) {
				return create<typename MatrixTraits<Type>::Vector3Type>(get<0, 2>(mat), get<1, 2>(mat), get<2, 2>(mat));
		}


		/**
		* @brief Returns the translation vector of a 4x4 matrix.
		* @ingroup openOR_core
		*/
		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::ConstMatrix<Type>)),
			(typename MatrixTraits<Type>::Vector3Type))
			translation(const Type& mat) {
				return create<typename MatrixTraits<Type>::Vector3Type>(get<0, 3>(mat), get<1, 3>(mat), get<2, 3>(mat));
		}


		namespace Impl {
			template <class Type, class Vec, int I, int Count>
			struct SetScaleHelp {
				void operator()(Type& mat, const Vec& vec) const {
					get<I, I>(mat) = get<I>(vec);
					SetScaleHelp < Type, Vec, I + 1, Count > ()(mat, vec);
				}
			};


			template <class Type, class Vec, int Count>
			struct SetScaleHelp<Type, Vec, Count, Count> {
				void operator()(Type& mat, const Vec& vec) const {}
			};
		}

		/**
		* @brief 
		* @ingroup openOR_core
		*/
		template <class Type, class Vec>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::Matrix<Type>))
			((Concept::ConstVector<Vec>)),
			(void))
			setScale(Type& mat, const Vec& vec) {
				BOOST_MPL_ASSERT((boost::mpl::equal_to<typename MatrixTraits<Type>::RowDimension, typename MatrixTraits<Type>::ColDimension>));
				Impl::SetScaleHelp<Type, Vec, 0, MatrixTraits<Type>::RowDimension::value>()(mat, vec);
		}


		namespace Impl {
			template <class Type, class Vec, int J, int Count, int Row>
			struct GetRowHelp {
				void operator()(Vec& vec, const Type& mat) const {
					get<J>(vec) = get<Row, J>(mat);
					GetRowHelp < Type, Vec, J + 1, Count, Row > ()(vec, mat);
				}
			};


			template <class Type, class Vec, int Count, int Row>
			struct GetRowHelp<Type, Vec, Count, Count, Row> {
				void operator()(Vec& vec, const Type& mat) const {}
			};


			template <class Type, class Vec, int I, int Count, int Col>
			struct GetColHelp {
				void operator()(Vec& vec, const Type& mat) const {
					get<I>(vec) = get<I, Col>(mat);
					GetColHelp < Type, Vec, I + 1, Count, Col > ()(vec, mat);
				}
			};


			template <class Type, class Vec, int Count, int Col>
			struct GetColHelp<Type, Vec, Count, Count, Col> {
				void operator()(Vec& vec, const Type& mat) const {}
			};
		}


		/**
		* @brief 
		* @ingroup openOR_core
		*/
		template <int I, class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::ConstMatrix<Type>))
			((Concept::Vector<typename MatrixTraits<Type>::RowVectorType>)),
			(typename MatrixTraits<Type>::RowVectorType))
			row(const Type& mat) {
				typedef typename MatrixTraits<Type>::RowVectorType VectorType;
				BOOST_MPL_ASSERT((boost::mpl::less<boost::mpl::int_<I>, typename MatrixTraits<Type>::RowDimension>));
				BOOST_MPL_ASSERT((boost::mpl::equal_to<typename VectorTraits<VectorType>::Dimension, typename MatrixTraits<Type>::RowDimension>));

				VectorType vec;
				Impl::GetRowHelp<Type, VectorType, 0, MatrixTraits<Type>::ColDimension::value, I>()(vec, mat);
				return vec;
		}


		/**
		* @brief 
		* @ingroup openOR_core
		*/
		template <int J, class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::ConstMatrix<Type>))
			((Concept::Vector<typename MatrixTraits<Type>::ColVectorType>)),
			(typename MatrixTraits<Type>::ColVectorType))
			col(const Type& mat) {
				typedef typename MatrixTraits<Type>::ColVectorType VectorType;
				BOOST_MPL_ASSERT((boost::mpl::less<boost::mpl::int_<J>, typename MatrixTraits<Type>::ColDimension>));
				BOOST_MPL_ASSERT((boost::mpl::equal_to<typename VectorTraits<VectorType>::Dimension, typename MatrixTraits<Type>::ColDimension>));

				VectorType vec;
				Impl::GetColHelp<Type, VectorType, 0, MatrixTraits<Type>::RowDimension::value, J>()(vec, mat);
				return vec;
		}


		/**
		* @brief 
		* @ingroup openOR_core
		*/
		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::Matrix<Type>)),
			(void))
			setRotationX(Type& mat, typename ScalarTraits<typename MatrixTraits<Type>::ValueType>::RealType angle) {
				BOOST_MPL_ASSERT((boost::mpl::greater_equal<typename MatrixTraits<Type>::RowDimension, boost::mpl::int_<3> >));
				get<0, 0>(mat) = 1;
				get<0, 1>(mat) = 0;
				get<0, 2>(mat) = 0;
				get<1, 0>(mat) = 0;
				get<1, 1>(mat) = cos(angle);
				get<1, 2>(mat) = -sin(angle);
				get<2, 0>(mat) = 0;
				get<2, 1>(mat) = sin(angle);
				get<2, 2>(mat) = cos(angle);
		}


		/**
		* @brief 
		* @ingroup openOR_core
		*/
		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::Matrix<Type>)),
			(void))
			setRotationY(Type& mat, typename ScalarTraits<typename MatrixTraits<Type>::ValueType>::RealType angle) {
				BOOST_MPL_ASSERT((boost::mpl::greater_equal<typename MatrixTraits<Type>::RowDimension, boost::mpl::int_<3> >));
				get<0, 0>(mat) = cos(angle);
				get<0, 1>(mat) = 0;
				get<0, 2>(mat) = sin(angle);
				get<1, 0>(mat) = 0;
				get<1, 1>(mat) = 1;
				get<1, 2>(mat) = 0;
				get<2, 0>(mat) = -sin(angle);
				get<2, 1>(mat) = 0;
				get<2, 2>(mat) = cos(angle);
		}



		/**
		* @brief 
		* @ingroup openOR_core
		*/
		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::Matrix<Type>)),
			(void))
			setRotationZ(Type& mat, typename ScalarTraits<typename MatrixTraits<Type>::ValueType>::RealType angle) {
				BOOST_MPL_ASSERT((boost::mpl::greater_equal<typename MatrixTraits<Type>::RowDimension, boost::mpl::int_<3> >));
				get<0, 0>(mat) = cos(angle);
				get<0, 1>(mat) = -sin(angle);
				get<0, 2>(mat) = 0;
				get<1, 0>(mat) = sin(angle);
				get<1, 1>(mat) = cos(angle);
				get<1, 2>(mat) = 0;
				get<2, 0>(mat) = 0;
				get<2, 1>(mat) = 0;
				get<2, 2>(mat) = 1;
		}



		/**
		* @brief 
		* @ingroup openOR_core
		*/
		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::Matrix<Type>)),
			(void))
			rotateX(Type& mat, typename ScalarTraits<typename MatrixTraits<Type>::ValueType>::RealType angle) {
				Type rotMat = MatrixTraits<Type>::IDENTITY;
				setRotationX(rotMat, angle);
				mat = Math::prod(rotMat, mat);
		}



		/**
		* @brief 
		* @ingroup openOR_core
		*/
		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::Matrix<Type>)),
			(void))
			rotateY(Type& mat, typename ScalarTraits<typename MatrixTraits<Type>::ValueType>::RealType angle) {
				Type rotMat = MatrixTraits<Type>::IDENTITY;
				setRotationY(rotMat, angle);
				mat = Math::prod(rotMat, mat);
		}



		/**
		* @brief 
		* @ingroup openOR_core
		*/
		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::Matrix<Type>)),
			(void))
			rotateZ(Type& mat, typename ScalarTraits<typename MatrixTraits<Type>::ValueType>::RealType angle) {
				Type rotMat = MatrixTraits<Type>::IDENTITY;
				setRotationZ(rotMat, angle);
				mat = Math::prod(rotMat, mat);
		}


		/**
		* @brief 
		* @ingroup openOR_core
		*/
		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::ConstMatrix<Type>)),
			(typename MatrixTraits<Type>::Matrix33Type))
			rotationAsMatrix(const Type& mat) {
				BOOST_MPL_ASSERT((boost::mpl::greater_equal<typename MatrixTraits<Type>::RowDimension, boost::mpl::int_<3> >));
				BOOST_MPL_ASSERT((boost::mpl::greater_equal<typename MatrixTraits<Type>::ColDimension, boost::mpl::int_<3> >));

				typename MatrixTraits<Type>::Matrix33Type matResult;
				get<0, 0>(matResult) = get<0, 0>(mat);
				get<0, 1>(matResult) = get<0, 1>(mat);
				get<0, 2>(matResult) = get<0, 2>(mat);
				get<1, 0>(matResult) = get<1, 0>(mat);
				get<1, 1>(matResult) = get<1, 1>(mat);
				get<1, 2>(matResult) = get<1, 2>(mat);
				get<2, 0>(matResult) = get<2, 0>(mat);
				get<2, 1>(matResult) = get<2, 1>(mat);
				get<2, 2>(matResult) = get<2, 2>(mat);
				return matResult;
		}


		/**
		* @brief 
		* @ingroup openOR_core
		*/
		template <class Type, class Type2>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::Matrix<Type>))
			((Concept::ConstMatrix<Type2>)),
			(void))
			setRotation(Type& mat, const Type2& matRotation) {
				BOOST_MPL_ASSERT((boost::mpl::greater_equal<typename MatrixTraits<Type>::RowDimension, boost::mpl::int_<3> >));
				BOOST_MPL_ASSERT((boost::mpl::greater_equal<typename MatrixTraits<Type>::ColDimension, boost::mpl::int_<3> >));
				get<0, 0>(mat) = get<0, 0>(matRotation);
				get<0, 1>(mat) = get<0, 1>(matRotation);
				get<0, 2>(mat) = get<0, 2>(matRotation);
				get<1, 0>(mat) = get<1, 0>(matRotation);
				get<1, 1>(mat) = get<1, 1>(matRotation);
				get<1, 2>(mat) = get<1, 2>(matRotation);
				get<2, 0>(mat) = get<2, 0>(matRotation);
				get<2, 1>(mat) = get<2, 1>(matRotation);
				get<2, 2>(mat) = get<2, 2>(matRotation);
		}



		/**
		* @brief 
		* @ingroup openOR_core
		*/
		template <class Mat, class Vec>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::Matrix<Mat>))
			((Concept::ConstVector<Vec>)),
			(void))
			setRotation(Mat& mat, const Vec& vecAxis, typename ScalarTraits<typename MatrixTraits<Mat>::ValueType>::RealType angle) {
				BOOST_MPL_ASSERT((boost::mpl::greater_equal<typename MatrixTraits<Mat>::RowDimension, boost::mpl::int_<3> >));
				BOOST_MPL_ASSERT((boost::mpl::greater_equal<typename MatrixTraits<Mat>::RowDimension, boost::mpl::int_<3> >));
				BOOST_MPL_ASSERT((boost::mpl::greater_equal<typename VectorTraits<Vec>::Dimension, boost::mpl::int_<3> >));

				//assert(std::abs(norm(vecAxis) - 1.0) <= 0.001);

				setXAxis(mat, rotate3(create<Vec>(1, 0, 0), vecAxis, angle));
				setYAxis(mat, rotate3(create<Vec>(0, 1, 0), vecAxis, angle));
				setZAxis(mat, rotate3(create<Vec>(0, 0, 1), vecAxis, angle));
		}



		/**
		* @brief 
		* @ingroup openOR_core
		*/
		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::ConstMatrix<Type>)),
			(boost::math::quaternion<typename ScalarTraits<typename MatrixTraits<Type>::ValueType>::RealType>))
			rotationAsQuaternions(const Type& mat) {

				typedef typename ScalarTraits<typename MatrixTraits<Type>::ValueType>::RealType RealType;
				typedef boost::math::quaternion<RealType> Quaternions;
				Quaternions q;

				RealType tr = get<0, 0>(mat) + get<1, 1>(mat) + get<2, 2>(mat);
				RealType s;
				RealType w, x, y, z;

				if (tr > 0.0)
				{
					s = sqrt(tr + static_cast<RealType>(1));     
					w = s * static_cast<RealType>(0.5);
					s = static_cast<RealType>(0.5) / s;
					x = static_cast<RealType>((get<2, 1>(mat) - get<1, 2>(mat)) * s);
					y = static_cast<RealType>((get<0, 2>(mat) - get<2, 0>(mat)) * s);
					z = static_cast<RealType>((get<1, 0>(mat) - get<0, 1>(mat)) * s);
				}
				else 
				{
					if (get<0, 0>(mat) > get<1, 1>(mat) && get<0, 0>(mat) > get<2, 2>(mat))
					{
						RealType s = 2.0 * sqrt(1.0 + get<0, 0>(mat) - get<1, 1>(mat) - get<2, 2>(mat));
						w = (get<2, 1>(mat) - get<1, 2>(mat)) / s;
						x = 0.25 * s;
						y = (get<1, 0>(mat) + get<0, 1>(mat)) / s;
						z = (get<0, 2>(mat) + get<2, 0>(mat)) / s;

					} else if (get<1, 1>(mat) > get<2, 2>(mat)) {
						RealType s = 2.0 * sqrt(1.0 + get<1, 1>(mat) - get<0, 0>(mat) - get<2, 2>(mat));
						w = (get<0, 2>(mat) - get<2, 0>(mat)) / s;
						x = (get<1, 0>(mat) + get<0, 1>(mat)) / s;
						y = 0.25 * s;
						z = (get<2, 1>(mat) + get<1, 2>(mat)) / s;
					} else {
						RealType s = 2.0 * sqrt(1.0 + get<2, 2>(mat) - get<0, 0>(mat) - get<1, 1>(mat));
						w = (get<1, 0>(mat) - get<0, 1>(mat)) / s;
						x = (get<0, 2>(mat) + get<2, 0>(mat)) / s;
						y = (get<2, 1>(mat) + get<1, 2>(mat)) / s;
						z = 0.25 * s;
					}
				}
				return Quaternions(w, x, y, z);
		}


		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::ConstMatrix<Type>)),
			(void))
			rotationAsEulerAngles(const Type& mat, double& heading, double& attitude, double& bank) 
		{
			typedef typename ScalarTraits<typename MatrixTraits<Type>::ValueType>::RealType RealType;
			// Assuming the angles are in radians.
			if (get<1, 0>(mat) > static_cast<RealType>(0.998)) { // singularity at north pole
				heading = atan2(get<0, 2>(mat), get<2, 2>(mat));
				attitude = PI / 2;
				bank = 0;
				return;
			}
			if (get<1, 0>(mat) < static_cast<RealType>(-0.998)) { // singularity at south pole
				heading = atan2(get<0, 2>(mat), get<2, 2>(mat));
				attitude = -PI / 2;
				bank = 0;
				return;
			}
			heading = atan2(-get<2, 0>(mat), get<0, 0>(mat));
			bank = atan2(-get<1, 2>(mat), get<1, 1>(mat));
			attitude = asin(get<1, 0>(mat));
		}


		/**
		* @brief 
		* @ingroup openOR_core
		*/
		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::Matrix<Type>)),
			(void))
			setRotation(Type& mat, const boost::math::quaternion<typename ScalarTraits<typename MatrixTraits<Type>::ValueType>::RealType>& quaternion) {

				typedef typename ScalarTraits<typename MatrixTraits<Type>::ValueType>::RealType RealType;
				typedef boost::math::quaternion<RealType> Quaternions;

				RealType s = static_cast<RealType>(2) / static_cast<RealType>(square(quaternion.R_component_1()) + 
					square(quaternion.R_component_2()) + 
					square(quaternion.R_component_3()) + 
					square(quaternion.R_component_4()));

				RealType xs = quaternion.R_component_2() * s;
				RealType ys = quaternion.R_component_3() * s;
				RealType zs = quaternion.R_component_4() * s;

				RealType wx = quaternion.R_component_1() * xs;
				RealType wy = quaternion.R_component_1() * ys;
				RealType wz = quaternion.R_component_1() * zs;

				RealType xx = quaternion.R_component_2() * xs;
				RealType xy = quaternion.R_component_2() * ys;
				RealType xz = quaternion.R_component_2() * zs;

				RealType yy = quaternion.R_component_3() * ys;
				RealType yz = quaternion.R_component_3() * zs;

				RealType zz = quaternion.R_component_4() * zs;

				get<0, 0>(mat) = static_cast<RealType>(1) - (yy + zz);
				get<1, 0>(mat) = xy + wz;
				get<2, 0>(mat) = xz - wy;

				get<0, 1>(mat) = xy - wz;
				get<1, 1>(mat) = static_cast<RealType>(1) - (xx + zz);
				get<2, 1>(mat) = yz + wx;

				get<0, 2>(mat) = xz + wy;
				get<1, 2>(mat) = yz - wx;
				get<2, 2>(mat) = static_cast<RealType>(1) - (xx + yy);
		}

		/**
		* @brief 
		* @ingroup openOR_core
		*/
		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::Matrix<Type>)),
			(void))
			setRotationAxis(Type& mat, const Math::Vector3d& axis, const double& angle) {
				typename MatrixTraits<Type>::ValueType c = cos(angle), s = sin(angle);
				typename MatrixTraits<Type>::ValueType cn = 1.0 - cos(angle);
				double nx = axis(0), ny = axis(1), nz = axis(2);
				double nxny = nx * ny, nxnz = nx * nz, nynz = ny * nz;
				double nxs = nx * s, nys = ny * s, nzs = nz * s;

				get<0, 0>(mat) = nx * nx * cn + c;
				get<0, 1>(mat) = nxny * cn - nzs;
				get<0, 2>(mat) = nxnz * cn + nys;

				get<1, 0>(mat) = nxny * cn + nzs;
				get<1, 1>(mat) = ny * ny * cn + c;
				get<1, 2>(mat) = nynz * cn - nxs;

				get<2, 0>(mat) = nxnz * cn - nys;
				get<2, 1>(mat) = nynz * cn + nxs;
				get<2, 2>(mat) = nz * nz * cn + c;
		}


		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::Matrix<Type>)),
			(void))
			setRotationStationaryZYX(Type& mat, 
			const typename MatrixTraits<Type>::ValueType& radZ,
			const typename MatrixTraits<Type>::ValueType& radY,
			const typename MatrixTraits<Type>::ValueType& radX) 
		{
			// angles must be in radians
			double cx = cos(radX);
			double sx = sin(radX);
			double cy = cos(radY);
			double sy = sin(radY);
			double cz = cos(radZ);
			double sz = sin(radZ);

			// describes the rotation with euler angles around stationary axes
			// is equal to R = Rx * Ry * Rz

			get<0, 0>(mat) = cy * cz;
			get<0, 1>(mat) = -cy * sz;
			get<0, 2>(mat) = -sy;
			get<1, 0>(mat) = -sx * sy * cz + cx * sz;
			get<1, 1>(mat) = sx * sy * sz + cx * cz;
			get<1, 2>(mat) = -sx * cy;
			get<2, 0>(mat) = cx * sy * cz + sx * sz;
			get<2, 1>(mat) = -cx * sy * sz + sx * cz;
			get<2, 2>(mat) = cx * cy;
		}

		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::ConstMatrix<Type>)),
			(void))
			rotationAsStationaryZYX(const Type& mat, double& radZ, double& radY, double& radX) 
		{
			typedef typename ScalarTraits<typename MatrixTraits<Type>::ValueType>::RealType RealType;

			double sx, sy, sz, cx, cy, cz, rx, ry, rz;

			ry = -asin(get<0, 2>(mat));
			cy = cos(ry);

			if (abs(cy) > 0.00005) {
				sx = -get<1, 2>(mat) / cy;
				cx = get<2, 2>(mat) / cy;

				sz = -get<0, 1>(mat) / cy;
				cz = get<0, 0>(mat) / cy;

				rx = atan2(sx, cx);
				rz = atan2(sz, cz);
			} else {
				rx = 0;

				sz = get<1, 0>(mat);
				cz = get<2, 0>(mat);
				rz = atan2(sz, cz);
			}

			radZ = rz; radY = ry; radX = rx;
		}

		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::Matrix<Type>)),
			(void))
			setRotation(Type& mat, 
			const typename MatrixTraits<Type>::ValueType& heading,
			const typename MatrixTraits<Type>::ValueType& attitude,
			const typename MatrixTraits<Type>::ValueType& bank) 
		{
			// angles must be in radians
			double ch = cos(heading);
			double sh = sin(heading);
			double ca = cos(attitude);
			double sa = sin(attitude);
			double cb = cos(bank);
			double sb = sin(bank);

			get<0, 0>(mat) = ch * ca;
			get<0, 1>(mat) = sh * sb - ch * sa * cb;
			get<0, 2>(mat) = ch * sa * sb + sh * cb;
			get<1, 0>(mat) = sa;
			get<1, 1>(mat) = ca * cb;
			get<1, 2>(mat) = -ca * sb;
			get<2, 0>(mat) = -sh * ca;
			get<2, 1>(mat) = sh * sa * cb + ch * sb;
			get<2, 2>(mat) = -sh * sa * sb + ch * cb;
		}


		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::Matrix<Type>)),
			(void))
			setRotationYprDeg(Type& mat, 
			const typename MatrixTraits<Type>::ValueType& degYaw,
			const typename MatrixTraits<Type>::ValueType& degPitch,
			const typename MatrixTraits<Type>::ValueType& degRoll) 
		{
			typename MatrixTraits<Type>::ValueType cy, sy;
			typename MatrixTraits<Type>::ValueType cp = cos(degPitch * M_PI / 180.0), sp = sin(degPitch * M_PI / 180.0);
			typename MatrixTraits<Type>::ValueType cr = cos(degRoll * M_PI / 180.0), sr = sin(degRoll * M_PI / 180.0);

			cy = cos(degYaw * M_PI / 180.0); sy = sin(degYaw * M_PI / 180.0);

			// correct cos
			if (degYaw == 90.0 || degYaw == 270.0) cy = 0.0;
			if (degPitch == 90.0 || degPitch == 270.0) cp = 0.0;
			if (degRoll == 90.0 || degRoll == 270.0) cr = 0.0;

			// correct sin
			if (degYaw == 0.0 || degYaw == 180.0) sy = 0.0;
			if (degPitch == 0.0 || degPitch == 180.0) sp = 0.0;
			if (degRoll == 0.0 || degRoll == 180.0) sr = 0.0;

			get<0, 0>(mat) = cr * cp;
			get<0, 1>(mat) = cr * sp * sy - sr * cy;
			get<0, 2>(mat) = cr * sp * cy + sr * sy;
			get<1, 0>(mat) = sr * cp;
			get<1, 1>(mat) = sr * sp * sy + cr * cy;
			get<1, 2>(mat) = sr * sp * cy - cr * sy;
			get<2, 0>(mat) = -sp;
			get<2, 1>(mat) = cp * sy;
			get<2, 2>(mat) = cp * cy;

			//get<0, 0>(mat) = cp * cy;
			//get<0, 1>(mat) = cp * sy;
			//get<0, 2>(mat) = -sp;
			//get<1, 0>(mat) = sr * sp * cy - sr * cy;
			//get<1, 1>(mat) = sr * sp * sy + cr * cy;
			//get<1, 2>(mat) = sr * cp;
			//get<2, 0>(mat) = cr * sp * cy + sr * sy;
			//get<2, 1>(mat) = cr * sp * sy - sr * cy;
			//get<2, 2>(mat) = cr * cp;
		}

		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::ConstMatrix<Type>)),
			(void))
			rotationAsYpr(const Type& mat, double& radYaw, double& radPitch, double& radRoll) 
		{
			typedef typename ScalarTraits<typename MatrixTraits<Type>::ValueType>::RealType RealType;

			double sy, sp, sr, cy, cp, cr, ry, rp, rr;

			rp = -asin(get<2, 0>(mat));
			cp = cos(rp);
			if (abs(cp) > 0.0005) {
				sy = get<2, 1>(mat) / cp;
				cy = get<2, 2>(mat) / cp;

				sr = get<1, 0>(mat) / cp;
				cr = get<0, 0>(mat) / cp;

				ry = atan2(sy, cy);
				rr = atan2(sr, cr);
			} else {
				cy = get<0, 1>(mat);
				sy = get<1, 1>(mat);

				rr = M_PI * -0.5;
				ry = atan2(sy, cy);
			}

			radYaw = ry; radPitch = rp; radRoll = rr;
		}

		/**
		* @brief 
		* @ingroup openOR_core
		*/
		template<class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::Matrix<Type>)),
			(Type))
			orthoInverse(const Type& mat) {
				BOOST_MPL_ASSERT((boost::mpl::equal_to<typename MatrixTraits<Type>::RowDimension, typename MatrixTraits<Type>::ColDimension>));
				//assert(isHomogeneous(mat));
				Type matResult;
				get<0, 0>(matResult) = get<0, 0>(mat);
				get<0, 1>(matResult) = get<1, 0>(mat);
				get<0, 2>(matResult) = get<2, 0>(mat);
				get<1, 0>(matResult) = get<0, 1>(mat);
				get<1, 1>(matResult) = get<1, 1>(mat);
				get<1, 2>(matResult) = get<2, 1>(mat);
				get<2, 0>(matResult) = get<0, 2>(mat);
				get<2, 1>(matResult) = get<1, 2>(mat);
				get<2, 2>(matResult) = get<2, 2>(mat);
				get<3, 0>(matResult) = 0;
				get<3, 1>(matResult) = 0;
				get<3, 2>(matResult) = 0;
				get<3, 3>(matResult) = 1;
				get<0, 3>(matResult) = -(get<0, 0>(mat) * get<0, 3>(mat) + get<1, 0>(mat) * get<1, 3>(mat) + get<2, 0>(mat) * get<2, 3>(mat));
				get<1, 3>(matResult) = -(get<0, 1>(mat) * get<0, 3>(mat) + get<1, 1>(mat) * get<1, 3>(mat) + get<2, 1>(mat) * get<2, 3>(mat));
				get<2, 3>(matResult) = -(get<0, 2>(mat) * get<0, 3>(mat) + get<1, 2>(mat) * get<1, 3>(mat) + get<2, 2>(mat) * get<2, 3>(mat));
				return matResult;
		}


		/**
		* @brief 
		* @ingroup openOR_core
		*/
		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::Matrix<Type>)),
			(Type&))
			orthoInvert(Type& mat) {
				BOOST_MPL_ASSERT((boost::mpl::equal_to<typename MatrixTraits<Type>::RowDimension, typename MatrixTraits<Type>::ColDimension>));
				mat = orthoInverse(mat);
				return mat;
		}      

		/**
		* @brief 
		* @ingroup openOR_core
		*/
		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::Matrix<Type>)),
			(Type))
			inverse(const Type& mat) {
				BOOST_MPL_ASSERT((boost::mpl::equal_to<typename MatrixTraits<Type>::RowDimension, typename MatrixTraits<Type>::ColDimension>));
				return Impl::Inverse<Type, MatrixTraits<Type>::RowDimension::value>::get(mat);
		}

		/**
		* @brief 
		* @ingroup openOR_core
		*/
		template<class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::Matrix<Type>)),
			(void))
			invert(Type& mat) {
				mat = inverse(mat);
		}


		namespace Impl {
			template <class Mat, class MatSrc, int I, int J>
			struct TransposeOp {
				inline void operator()(Mat& mat, const MatSrc& matSrc) const {
					Math::get<I, J>(mat) = Math::get<J, I>(matSrc);
				}
			};
		}

		/**
		* @brief 
		* @ingroup openOR_core
		*/
		template<class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::Matrix<Type>)),
			(Type))
			transposed(const Type& mat) {
				typedef typename MatrixTraits<Type>::RowDimension RowDimension;
				typedef typename MatrixTraits<Type>::ColDimension ColDimension;
				BOOST_MPL_ASSERT((boost::mpl::equal_to<RowDimension, ColDimension>));
				Type matTransposed;
				Impl::MatrixCompileTimeIterator<Type, 0, RowDimension::value, 0, ColDimension::value>().template apply<Impl::TransposeOp, Type>(matTransposed, mat);
				return matTransposed;
		}

		/**
		* @brief 
		* @ingroup openOR_core
		*/
		template<class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::Matrix<Type>)),
			(Type&))
			transpose(Type& mat) {
				BOOST_MPL_ASSERT((boost::mpl::equal_to<typename MatrixTraits<Type>::RowDimension, typename MatrixTraits<Type>::ColDimension>));
				mat = transposed(mat);
				return mat;
		}      

		/**
		* @brief 
		* @ingroup openOR_core_math
		*/
		template<class Type, class Vec>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::ConstMatrix<Type>))
			((Concept::ConstVector<Vec>))
			((Concept::Vector<typename MatrixTraits<Type>::Vector3Type>)),
			(typename MatrixTraits<Type>::Vector3Type))
			prod44x3(const Type& mat, const Vec& vec) {
				typename MatrixTraits<Type>::Vector3Type result;
				get<0>(result) = get<0, 0>(mat) * get<0>(vec) + get<0, 1>(mat) * get<1>(vec) + get<0, 2>(mat) * get<2>(vec) + get<0, 3>(mat);
				get<1>(result) = get<1, 0>(mat) * get<0>(vec) + get<1, 1>(mat) * get<1>(vec) + get<1, 2>(mat) * get<2>(vec) + get<1, 3>(mat);
				get<2>(result) = get<2, 0>(mat) * get<0>(vec) + get<2, 1>(mat) * get<1>(vec) + get<2, 2>(mat) * get<2>(vec) + get<2, 3>(mat);
				return result;
		}

		/**
		* @brief 
		* @ingroup openOR_core_math
		*/
		template<class Type, class Vec>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::ConstMatrix<Type>))
			((Concept::ConstVector<Vec>))
			((Concept::Vector<typename MatrixTraits<Type>::Vector2Type>)),
			(typename MatrixTraits<Type>::Vector2Type))
			prod33x2(const Type& mat, const Vec& vec) {
				typename MatrixTraits<Type>::Vector2Type result;
				get<0>(result) = get<0, 0>(mat) * get<0>(vec) + get<0, 1>(mat) * get<1>(vec) + get<0, 2>(mat);
				get<1>(result) = get<1, 0>(mat) * get<0>(vec) + get<1, 1>(mat) * get<1>(vec) + get<1, 2>(mat);
				return result;
		}


		template <class Type>
		inline
			OPENOR_CONCEPT_REQUIRES(
			((Concept::Matrix<Type>)),
			(void))
			makeHomogeneous(Type& mat, int unmodifiedAxis = 2) // 0: xAxis; 1: yAxis; 2: zAxis 
		{
			get<3, 0>(mat) = 0;
			get<3, 1>(mat) = 0;
			get<3, 2>(mat) = 0;
			get<3, 3>(mat) = 1;
			typename MatrixTraits<Type>::Vector3Type vecXAxis = xAxis(mat);
			typename MatrixTraits<Type>::Vector3Type vecYAxis = yAxis(mat);
			typename MatrixTraits<Type>::Vector3Type vecZAxis = zAxis(mat);

			switch (unmodifiedAxis)
			{
			case 0:                
				normalize(vecXAxis);
				vecZAxis = cross(vecXAxis, vecYAxis);
				normalize(vecZAxis);
				vecYAxis = cross(vecZAxis, vecXAxis);
				break;
			case 1:
				normalize(vecYAxis);
				vecZAxis = cross(vecXAxis, vecYAxis);
				normalize(vecZAxis);
				vecXAxis = cross(vecYAxis, vecZAxis);
				break;
			default:
				normalize(vecZAxis);
				vecXAxis = cross(vecYAxis, vecZAxis);
				normalize(vecXAxis);
				vecYAxis = cross(vecZAxis, vecXAxis);
				break;
			}
			setXAxis(mat, vecXAxis);
			setYAxis(mat, vecYAxis);
			setZAxis(mat, vecZAxis);
		}


	}
}


#endif
