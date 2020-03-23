//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.

#include <openOR/Image/InfoPatient.hpp>
#include <openOR/Math/math.hpp>
#include <string>

namespace openOR {
   namespace Image {

      InfoPatient::InfoPatient() :
         m_name(""),
         m_age(0),
         m_ageUnit(YEARS),
         m_birthDay(1),
         m_birthMonth(1),
         m_birthYear(1970)
      {

      }

      InfoPatient::~InfoPatient() {

      }
	  //overloading of equal operator
      bool InfoPatient::operator==(InfoPatient& other) {
         bool retval = true;

         retval = retval && (ageDcm().compare(other.ageDcm()) == 0);
         retval = retval && (birthdateDcm().compare(other.birthdateDcm()) == 0);
         retval = retval && (name().compare(other.name()) == 0);

         return retval;
      }

      bool InfoPatient::operator!=(InfoPatient& other) {
         return !operator==(other);
      }

      std::string InfoPatient::name() {
         return m_name;
      }

      void InfoPatient::setName(const std::string& name) {
         m_name = name;
      }
	  //convert patients age
      unsigned short InfoPatient::age() { 
         switch (m_ageUnit) {
         case DAYS:
            return m_age / 365;
         case WEEKS:
            return (unsigned short)((float)m_age / 52.5f);
         case MONTHS:
            return m_age / 12;
         default:
            return m_age;
         }
      }
	  //set the suitable age unit
      std::string InfoPatient::ageDcm() {
         std::stringstream ss;

         if (m_age < 100) { ss << "0"; }
         if (m_age < 10) { ss << "0"; }
         ss << m_age;

         switch (m_ageUnit) {
         case InfoPatient::DAYS:
            ss << "D";
            break;
         case InfoPatient::WEEKS:
            ss << "W";
            break;
         case InfoPatient::MONTHS:
            ss << "M";
            break;
         default:
            ss << "Y";
         }

         return ss.str();
      }
      void InfoPatient::setAge(unsigned short age, AgeUnit unit) {
         m_age = age;
         m_ageUnit = unit;
      }
      void InfoPatient::setAgeDcm(const std::string& strAgeDcm) {
         if (strAgeDcm.length() !=4 ) { return; }
         const char* unit = strAgeDcm.substr(3, 1).c_str();
         if (*unit == 'D') {
            m_ageUnit = InfoPatient::DAYS;
         } else if (*unit == 'W') {
            m_ageUnit = InfoPatient::WEEKS;
         } else if (*unit == 'M') {
            m_ageUnit = InfoPatient::MONTHS;
         } else {
            m_ageUnit = InfoPatient::YEARS;
         }
         std::stringstream ss(strAgeDcm);
         ss >> m_age;
      }

      std::string InfoPatient::birthdate() {
         std::stringstream ss;
         ss << m_birthDay << "/" << m_birthMonth << "/" << m_birthYear;
         return ss.str();
      }
      std::string InfoPatient::birthdateDcm() {
         std::stringstream ss;
         ss << m_birthYear;
         if (m_birthMonth < 10) { ss << "0"; }
         ss << m_birthMonth;
         if (m_birthDay < 10) { ss << "0"; }
         ss << m_birthDay;
         return ss.str();
      }

      void InfoPatient::setBirthdate(unsigned short day, unsigned short month, unsigned short year) {
         m_birthDay = std::min<unsigned short>(std::max<unsigned short>(day, 1), 31);
         m_birthMonth = std::min<unsigned short>(std::max<unsigned short>(day, 1), 12);
         m_birthYear = std::min<unsigned short>(std::max<unsigned short>(day, 1880), 2100);
      }
      void InfoPatient::setBirthdateDcm(std::string strBirthdate) {
         if (strBirthdate.size() != 10) {
            setBirthdate(1, 1, 1970);
         }
         std::stringstream ss(strBirthdate);
         int digit;
         ss >> digit;
         m_birthYear = digit / 10000;
         m_birthMonth = (digit - m_birthYear * 10000) / 100;
         m_birthDay = (digit - m_birthYear * 10000 - m_birthMonth * 100);
      }

      std::string InfoPatient::getInfoStr() {
         std::stringstream ss;
         ss << "Patient Name: " << m_name;
         ss << " | Age: " << m_age << " " << ageUnitStr();
         ss << " | Date of Birth: " << birthdate();
         return ss.str();
      }

      std::string InfoPatient::ageUnitStr() {
         switch (m_ageUnit) {
         case InfoPatient::DAYS:
            return "days";
         case InfoPatient::WEEKS:
            return "weeks";
         case InfoPatient::MONTHS:
            return "months";
         default:
            return "years";
         }
      }

   }
}
