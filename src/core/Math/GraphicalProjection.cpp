//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#include "../include/openOR/Math/GraphicalProjection.hpp"

#include <openOR/Math/constants.hpp>

namespace openOR {
	namespace Math {


		GraphicalProjection::GraphicalProjection(ProjectionType eProjectionType) : 
	m_eProjectionType(eProjectionType),
		m_bDataChanged(true)
	{}


	GraphicalProjection::GraphicalProjection(ProjectionType eProjectionType, 
		const double& fLeft, 
		const double& fRight, 
		const double& fBottom, 
		const double& fTop, 
		const double& fNear, 
		const double& fFar) :
	m_eProjectionType(eProjectionType),
		m_fLeft(fLeft),
		m_fRight(fRight),
		m_fBottom(fBottom),
		m_fTop(fTop),
		m_fNear(fNear),
		m_fFar(fFar),
		m_bDataChanged(true)
	{}


	GraphicalProjection::GraphicalProjection(const GraphicalProjection& other) : 
	m_eProjectionType(other.m_eProjectionType),
		m_fLeft(other.m_fLeft),
		m_fRight(other.m_fRight),
		m_fBottom(other.m_fBottom),
		m_fTop(other.m_fTop),
		m_fNear(other.m_fNear),
		m_fFar(other.m_fFar),
		m_bDataChanged(other.m_bDataChanged),
		m_matProjection(other.m_matProjection),
		m_matInverseProjection(other.m_matInverseProjection)
	{}


	GraphicalProjection::GraphicalProjection(const Matrix44d& matProjection) : 
	m_eProjectionType(PERSPECTIVE),
		m_bDataChanged(true)
	{
		if (std::abs(get<0, 1>(matProjection)) < GRANULARITY &&
			std::abs(get<0, 3>(matProjection)) < GRANULARITY &&
			std::abs(get<1, 0>(matProjection)) < GRANULARITY &&
			std::abs(get<1, 3>(matProjection)) < GRANULARITY &&
			std::abs(get<2, 0>(matProjection)) < GRANULARITY &&
			std::abs(get<2, 1>(matProjection)) < GRANULARITY &&
			std::abs(get<3, 0>(matProjection)) < GRANULARITY &&
			std::abs(get<3, 1>(matProjection)) < GRANULARITY &&
			std::abs(get<3, 2>(matProjection) + 1) < GRANULARITY && 
			std::abs(get<3, 3>(matProjection)) < GRANULARITY)
		{
			m_eProjectionType = PERSPECTIVE;
			computeFromProjectionMatrixPerspective(matProjection);
		}
		else if (std::abs(get<0, 1>(matProjection)) < GRANULARITY &&
			std::abs(get<0, 2>(matProjection)) < GRANULARITY &&
			std::abs(get<1, 0>(matProjection)) < GRANULARITY &&
			std::abs(get<1, 2>(matProjection)) < GRANULARITY &&
			std::abs(get<2, 0>(matProjection)) < GRANULARITY &&
			std::abs(get<2, 1>(matProjection)) < GRANULARITY &&
			std::abs(get<3, 0>(matProjection)) < GRANULARITY &&
			std::abs(get<3, 1>(matProjection)) < GRANULARITY &&
			std::abs(get<3, 2>(matProjection)) < GRANULARITY &&
			std::abs(get<3, 3>(matProjection) - 1.0) < GRANULARITY)
		{
			m_eProjectionType = ORTHOGRAPHIC;
			computeFromProjectionMatrixOrthographic(matProjection);
		} else {
			throw std::runtime_error("GraphicalProjection: No valid projection matrix");
		}
	}

	GraphicalProjection::GraphicalProjection(const double& fov, 
		const double& aspect, 
		const double& fNear, 
		const double& fFar) :
	m_eProjectionType(PERSPECTIVE),
		m_fNear(fNear),
		m_fFar(fFar),
		m_bDataChanged(true)
	{
		m_fTop = fNear * tan(fov * 0.5);
		m_fBottom = -m_fTop;
		m_fLeft = aspect * m_fBottom;
		m_fRight = aspect * m_fTop;
	}

	void GraphicalProjection::setProjectionType(ProjectionType eProjectionType) {
		if (eProjectionType != m_eProjectionType) {
			m_eProjectionType = eProjectionType;
			m_bDataChanged = true;
		}
	}


	GraphicalProjection::ProjectionType GraphicalProjection::projectionType() const { return m_eProjectionType; }


	void GraphicalProjection::setProjection(const double& fLeft, 
		const double& fRight, 
		const double& fBottom, 
		const double& fTop, 
		const double& fNear, 
		const double& fFar)
	{
		m_fLeft = fLeft;
		m_fRight = fRight;
		m_fBottom = fBottom;
		m_fTop = fTop;
		m_fNear = fNear;
		m_fFar = fFar;
		m_bDataChanged = true;
	}


	const double& GraphicalProjection::leftCoord() const { return m_fLeft; }
	const double& GraphicalProjection::rightCoord() const { return m_fRight; }
	const double& GraphicalProjection::bottomCoord() const { return m_fBottom; }
	const double& GraphicalProjection::topCoord() const { return m_fTop; }
	const double& GraphicalProjection::nearCoord() const { return m_fNear; }
	const double& GraphicalProjection::farCoord() const { return m_fFar; }

	void GraphicalProjection::setLeftCoord(const double& fLeft) { m_fLeft = fLeft; m_bDataChanged = true; }
	void GraphicalProjection::setRightCoord(const double& fRight) { m_fRight = fRight; m_bDataChanged = true; }
	void GraphicalProjection::setBottomCoord(const double& fBottom) { m_fBottom = fBottom; m_bDataChanged = true; }
	void GraphicalProjection::setTopCoord(const double& fTop) { m_fTop = fTop; m_bDataChanged = true; }
	void GraphicalProjection::setNearCoord(const double& fNear) { m_fNear = fNear; m_bDataChanged = true; }
	void GraphicalProjection::setFarCoord(const double& fFar) { m_fFar = fFar; m_bDataChanged = true; }


	const Math::Matrix44d& GraphicalProjection::projectionMatrix() const {
		if (m_bDataChanged) { updateProjectionMatrices(); }
		return m_matProjection;
	}


	const Math::Matrix44d& GraphicalProjection::inverseProjectionMatrix() const {
		if (m_bDataChanged) { updateProjectionMatrices(); }
		return m_matInverseProjection;
	}


	const double* GraphicalProjection::data() const { return m_pData; }
	double* GraphicalProjection::mutableData() { return m_pData; }


	void GraphicalProjection::updateProjectionMatrices() const {

		if (m_eProjectionType == PERSPECTIVE) {

			get<0, 0>(m_matProjection) = 2.0 * m_fNear / (m_fRight - m_fLeft);
			get<1, 0>(m_matProjection) = 0;
			get<2, 0>(m_matProjection) = 0;
			get<3, 0>(m_matProjection) = 0;
			get<0, 1>(m_matProjection) = 0;
			get<1, 1>(m_matProjection) = 2.0 * m_fNear / (m_fTop - m_fBottom);
			get<2, 1>(m_matProjection) = 0;
			get<3, 1>(m_matProjection) = 0;
			get<0, 2>(m_matProjection) = (m_fRight + m_fLeft) / (m_fRight - m_fLeft);
			get<1, 2>(m_matProjection) = (m_fTop + m_fBottom) / (m_fTop - m_fBottom);
			get<2, 2>(m_matProjection) = -(m_fFar + m_fNear) / (m_fFar - m_fNear);
			get<3, 2>(m_matProjection) = -1;
			get<0, 3>(m_matProjection) = 0;
			get<1, 3>(m_matProjection) = 0;
			get<2, 3>(m_matProjection) = -2.0 * m_fFar * m_fNear / (m_fFar - m_fNear);
			get<3, 3>(m_matProjection) = 0;

			get<0, 0>(m_matInverseProjection) = (m_fRight - m_fLeft) / (2.0 * m_fNear);
			get<1, 0>(m_matInverseProjection) = 0;
			get<2, 0>(m_matInverseProjection) = 0;
			get<3, 0>(m_matInverseProjection) = 0;
			get<0, 1>(m_matInverseProjection) = 0;
			get<1, 1>(m_matInverseProjection) = (m_fTop - m_fBottom) / (2.0 * m_fNear);
			get<2, 1>(m_matInverseProjection) = 0;
			get<3, 1>(m_matInverseProjection) = 0;
			get<0, 2>(m_matInverseProjection) = 0;
			get<1, 2>(m_matInverseProjection) = 0;
			get<2, 2>(m_matInverseProjection) = 0;
			get<3, 2>(m_matInverseProjection) = -(m_fFar - m_fNear) / (2.0 * m_fFar * m_fNear);
			get<0, 3>(m_matInverseProjection) = (m_fRight + m_fLeft) / (2.0 * m_fNear);
			get<1, 3>(m_matInverseProjection) = (m_fTop + m_fBottom) / (2.0 * m_fNear);
			get<2, 3>(m_matInverseProjection) = -1;
			get<3, 3>(m_matInverseProjection) = (m_fFar + m_fNear) / (2.0 * m_fFar * m_fNear);

		} else {

			// m_eProjectionType == ORTHOGRAPHIC
			get<0, 0>(m_matProjection) = 2.0 / (m_fRight - m_fLeft);
			get<1, 0>(m_matProjection) = 0;
			get<2, 0>(m_matProjection) = 0;
			get<3, 0>(m_matProjection) = 0;
			get<0, 1>(m_matProjection) = 0;
			get<1, 1>(m_matProjection) = 2.0 / (m_fTop - m_fBottom);
			get<2, 1>(m_matProjection) = 0;
			get<3, 1>(m_matProjection) = 0;
			get<0, 2>(m_matProjection) = 0;
			get<1, 2>(m_matProjection) = 0;
			get<2, 2>(m_matProjection) = -2.0 / (m_fFar - m_fNear);
			get<3, 2>(m_matProjection) = 0;
			get<0, 3>(m_matProjection) = -(m_fRight + m_fLeft) / (m_fRight - m_fLeft);
			get<1, 3>(m_matProjection) = -(m_fTop + m_fBottom) / (m_fTop - m_fBottom);
			get<2, 3>(m_matProjection) = -(m_fFar + m_fNear) / (m_fFar - m_fNear);
			get<3, 3>(m_matProjection) = 1;

			get<0, 0>(m_matInverseProjection) = (m_fRight - m_fLeft) / 2.0;
			get<1, 0>(m_matInverseProjection) = 0;
			get<2, 0>(m_matInverseProjection) = 0;
			get<3, 0>(m_matInverseProjection) = 0;
			get<0, 1>(m_matInverseProjection) = 0;
			get<1, 1>(m_matInverseProjection) = (m_fTop - m_fBottom) / 2.0;
			get<2, 1>(m_matInverseProjection) = 0;
			get<3, 1>(m_matInverseProjection) = 0;
			get<0, 2>(m_matInverseProjection) = 0;
			get<1, 2>(m_matInverseProjection) = 0;
			get<2, 2>(m_matInverseProjection) = (m_fFar - m_fNear) / (-2.0);
			get<3, 2>(m_matInverseProjection) = 0;
			get<0, 3>(m_matInverseProjection) = (m_fRight + m_fLeft) / 2.0;
			get<1, 3>(m_matInverseProjection) = (m_fTop + m_fBottom) / 2.0;
			get<2, 3>(m_matInverseProjection) = (m_fFar + m_fNear) / (-2.0);
			get<3, 3>(m_matInverseProjection) = 1;
		}
		m_bDataChanged = false;

		return;
	}


	void GraphicalProjection::computeFromProjectionMatrixPerspective(const Matrix44d& matProjection) {

		assert(std::abs(get<0, 1>(matProjection)) < GRANULARITY);
		assert(std::abs(get<0, 3>(matProjection)) < GRANULARITY);
		assert(std::abs(get<1, 0>(matProjection)) < GRANULARITY);
		assert(std::abs(get<1, 3>(matProjection)) < GRANULARITY);
		assert(std::abs(get<2, 0>(matProjection)) < GRANULARITY);
		assert(std::abs(get<2, 1>(matProjection)) < GRANULARITY);
		assert(std::abs(get<3, 0>(matProjection)) < GRANULARITY);
		assert(std::abs(get<3, 1>(matProjection)) < GRANULARITY);
		assert(std::abs(get<3, 2>(matProjection) + 1.0) < GRANULARITY);
		assert(std::abs(get<3, 3>(matProjection)) < GRANULARITY);

		double f0 = (get<2, 2>(matProjection) - 1.0) / (get<2, 2>(matProjection) + 1.0);
		m_fNear = get<2, 3>(matProjection) * (f0 - 1.0) / (-2.0 * f0);
		m_fFar = f0 * m_fNear;

		double f1 = (get<0, 2>(matProjection) + 1.0) / (get<0, 2>(matProjection) - 1.0);
		m_fLeft = 2.0 * m_fNear / (get<0, 0>(matProjection) * (f1 - 1.0));
		m_fRight = f1 * m_fLeft;

		double f2 = (get<1, 2>(matProjection) + 1.0) / (get<1, 2>(matProjection) - 1.0);
		m_fBottom = 2.0 * m_fNear / (get<1, 1>(matProjection) * (f2 - 1.0));
		m_fTop = f2 * m_fBottom;

		return;
	}


	void GraphicalProjection::computeFromProjectionMatrixOrthographic(const Matrix44d& matProjection) {

		assert(std::abs(get<0, 1>(matProjection)) < GRANULARITY);
		assert(std::abs(get<0, 2>(matProjection)) < GRANULARITY);
		assert(std::abs(get<1, 0>(matProjection)) < GRANULARITY);
		assert(std::abs(get<1, 2>(matProjection)) < GRANULARITY);
		assert(std::abs(get<2, 0>(matProjection)) < GRANULARITY);
		assert(std::abs(get<2, 1>(matProjection)) < GRANULARITY);
		assert(std::abs(get<3, 0>(matProjection)) < GRANULARITY);
		assert(std::abs(get<3, 1>(matProjection)) < GRANULARITY);
		assert(std::abs(get<3, 2>(matProjection)) < GRANULARITY);
		assert(std::abs(get<3, 3>(matProjection) - 1.0) < GRANULARITY);

		//double f0 = (get<2, 3>(matProjection) - 1.0) / (get<2, 3>(matProjection) + 1.0);
		//m_fNear = -2.0 / (get<2, 2>(matProjection) * (f0 - 1.0));
		//m_fFar = f0 * m_fNear;
		m_fNear = (get<2, 3>(matProjection) + 1.0) / get<2, 2>(matProjection);
		m_fFar  = (get<2, 3>(matProjection) - 1.0) / get<2, 2>(matProjection);

		//double f1 = (get<0, 3>(matProjection) - 1.0) / (get<0, 3>(matProjection) + 1.0);
		//m_fLeft = 2.0 / (get<0, 0>(matProjection) * (f1 - 1.0));
		//m_fRight = f1 * m_fLeft;
		m_fLeft  = (-get<0, 3>(matProjection) - 1.0) / get<0, 0>(matProjection);
		m_fRight = (-get<0, 3>(matProjection) + 1.0) / get<0, 0>(matProjection);

		//double f2 = (get<1, 3>(matProjection) - 1.0) / (get<1, 3>(matProjection) + 1.0);
		//m_fBottom = 2.0 / (get<1, 1>(matProjection) * (f2 - 1.0));
		//m_fTop = f2 * m_fBottom;
		m_fBottom = (-get<1, 3>(matProjection) - 1.0) / get<1, 1>(matProjection);
		m_fTop    = (-get<1, 3>(matProjection) + 1.0) / get<1, 1>(matProjection);

		return;
	}

	}
}
