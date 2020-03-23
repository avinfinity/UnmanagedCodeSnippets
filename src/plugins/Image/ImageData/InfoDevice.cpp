//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#include <openOR/Image/InfoDevice.hpp>
#include <string>
#include <sstream>
#include <vector>

namespace openOR {
	namespace Image {

		InfoDevice::InfoDevice() :
	m_modality(UNKNOWN),
		m_manufacturer("n.a."),
		m_model("n.a.")
	{

	}

	InfoDevice::~InfoDevice() {

	}
	//overloading oof equal operator
	bool InfoDevice::operator==(InfoDevice& other) {
		bool retval = true;

		retval = retval && other.modality() == modality();
		retval = retval && other.manufacturer() == manufacturer();
		retval = retval && other.model() == model();

		return retval;
	}
	//getter modality
	InfoDevice::Device InfoDevice::modality() {
		return m_modality;
	}
	//getter modality-dcm
	std::string InfoDevice::modalityDcm() {
		if (!m_strModality.empty()) { return m_strModality; }
		switch (m_modality) {
		case InfoDevice::CT:
			return "CT";
		case InfoDevice::DVT:
			return "DVT";
		case InfoDevice::C_ARM:
			return "C-ARM";
		default:
			return "-";
		}
	}
	//setter modality
	void InfoDevice::setModality(const InfoDevice::Device& modality) {
		m_modality = modality;
	}
	//setter modality-dcm
	void InfoDevice::setModality(const std::string& mod) {
		m_strModality = mod;
		if (mod.find("CT") != std::string::npos) {
			m_modality = InfoDevice::CT;
		} else if (mod.find("C-ARM") != std::string::npos || mod.find("C-Arm") != std::string::npos) {
			m_modality = InfoDevice::C_ARM;
		} else if (mod.find("DVT") != std::string::npos) {
			m_modality = InfoDevice::DVT;
		} else {
			m_modality = InfoDevice::UNKNOWN;
		}
	}
	//getter infostring
	std::string InfoDevice::getInfoStr() {
		std::stringstream retval;

		retval << "Device: " << m_manufacturer << " " << m_model << " (";
		switch (m_modality) {
		case InfoDevice::CT:
			retval << "CT";
			break;
		case InfoDevice::C_ARM:
			retval << "C-Arm";
			break;
		case InfoDevice::DVT:
			retval << "DVT";
			break;
		default:
			retval << "n.a.";
		}
		retval << " | " << m_strModality << ")";

		return retval.str();
	}
	//getter manufacturer
	std::string InfoDevice::manufacturer() {
		return m_manufacturer;
	}
	//setter manufacturer
	void InfoDevice::setManufacturer(const std::string& manufacturer) {
		m_manufacturer = manufacturer;
	}
	//getter model
	std::string InfoDevice::model() {
		return m_model;
	}
	//setter model
	void InfoDevice::setModel(const std::string& mod) {
		m_model = mod;
	}
	//getter angle of robot
	std::vector<float>& InfoDevice::anglesOfRobot() {
		return m_anglesOfRobot;
	}
	//setter angle of robot
	void InfoDevice::setAnglesOfRobot(const std::vector<float>& angles) {
		m_anglesOfRobot = angles;
	}

	}
}
