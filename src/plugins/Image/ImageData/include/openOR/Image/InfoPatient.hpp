//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//! OPENOR_INTERFACE_FILE(Image_ImageData)
//! \file
//! \ingroup Image_ImageData
//****************************************************************************

#ifndef openOR_Image_InfoPatient_hpp
#define openOR_Image_InfoPatient_hpp

#include <string>
#include <openOR/Defs/Image_ImageData.hpp>
#include <openOR/Math/math.hpp>

namespace openOR {
	namespace Image {
		struct OPENOR_IMAGE_IMAGEDATA_API InfoPatient {

			InfoPatient();
			virtual ~InfoPatient();

			enum AgeUnit {
				DAYS, WEEKS, MONTHS, YEARS
			};
			//overloading equal and not-equal operators
			bool operator==(InfoPatient& other);
			bool operator!=(InfoPatient& other);
			//getter, setter for name
			std::string name();
			void setName(const std::string& name);
			//getter, setter for age and dcm-age
			unsigned short age();
			std::string ageDcm();
			void setAge(unsigned short age, AgeUnit unit = InfoPatient::YEARS);
			void setAgeDcm(const std::string& strAgeDcm);
			//getter, setter for birthdate and birthdate-dcm
			std::string birthdate();
			std::string birthdateDcm();
			void setBirthdate(unsigned short day, unsigned short month, unsigned short year);
			void setBirthdateDcm(std::string strBirthdate);
			//getter for infostring
			std::string getInfoStr();

		private:
			//converter for ageUnitString
			std::string ageUnitStr();

			std::string m_name;
			unsigned short m_age;
			AgeUnit m_ageUnit;

			unsigned short m_birthDay;
			unsigned short m_birthMonth;
			unsigned short m_birthYear;
		};

	}
}

#endif