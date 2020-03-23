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
//! @ingroup openOR_core_math

#ifndef openOR_core_math_GraphicalProjection_hpp
#define openOR_core_math_GraphicalProjection_hpp

#include "../coreDefs.hpp"

#include <openOR/Math/matrix.hpp>

namespace openOR {
	namespace Math {

		//----------------------------------------------------------------------------
		//! \brief GraphicalProjection calculates the six values (left, right, bottom, far, near, far)
		//! which describe an orthographic or perspective projection.
		//! The corresponding projection matrices are also provided.
		//! @ingroup openOR_core
		//----------------------------------------------------------------------------
		struct OPENOR_CORE_API GraphicalProjection {

			enum ProjectionType {
				PERSPECTIVE,
				ORTHOGRAPHIC
			};

			GraphicalProjection(ProjectionType eProjectionType = ORTHOGRAPHIC);

			GraphicalProjection(ProjectionType eProjectionType, 
				const double& fLeft, 
				const double& fRight, 
				const double& fBottom, 
				const double& fTop, 
				const double& fNear, 
				const double& fFar);

			GraphicalProjection(const GraphicalProjection& other);

			GraphicalProjection(const Matrix44d& matProjection);

			GraphicalProjection(const double& fov, const double& aspect, const double& fNear, const double& fFar);

			void setProjectionType(ProjectionType eProjectionType);
			ProjectionType projectionType() const;

			void setProjection(const double& fLeft, 
				const double& fRight, 
				const double& fBottom, 
				const double& fTop, 
				const double& fNear, 
				const double& fFar);

			const double& leftCoord() const;
			const double& rightCoord() const;
			const double& bottomCoord() const;
			const double& topCoord() const;
			const double& nearCoord() const;
			const double& farCoord() const;

			void setLeftCoord(const double& fLeft);
			void setRightCoord(const double& fRight);
			void setBottomCoord(const double& fBottom);
			void setTopCoord(const double& fTop);
			void setNearCoord(const double& fNear);
			void setFarCoord(const double& fFar);


			const Math::Matrix44d& projectionMatrix() const;
			const Math::Matrix44d& inverseProjectionMatrix() const;

			const double* data() const;
			double* mutableData();

		private:

			void updateProjectionMatrices() const;
			void computeFromProjectionMatrixPerspective(const Matrix44d& matProjection);
			void computeFromProjectionMatrixOrthographic(const Matrix44d& matProjection);

			ProjectionType m_eProjectionType;

			// Unions can not be leagally used for aliasing purposes,
			// although it will probably work on most compilers.
			// TODO: investiagate a better way of doing this. -->Raus?
			union {
				double m_pData[6];

				struct
				{
					double m_fLeft;
					double m_fRight;
					double m_fBottom;
					double m_fTop;
					double m_fNear;
					double m_fFar;
				};
			};

			mutable bool m_bDataChanged;
			mutable Matrix44d m_matProjection;
			mutable Matrix44d m_matInverseProjection;
		};

	}
}

#endif
