//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#include <openOR/Image/InfoProjection.hpp>
#include <openOR/Math/math.hpp>

namespace openOR {
	namespace Image {

      InfoProjection::InfoProjection() :
         m_imageFrame(Math::MatrixTraits<Math::Matrix44d>::IDENTITY),
         m_sourceFrame(Math::MatrixTraits<Math::Matrix44d>::IDENTITY),
         m_objectFrame(Math::MatrixTraits<Math::Matrix44d>::IDENTITY)
      {

      }

      InfoProjection::~InfoProjection() {

      }
	  //overloading of equal operator
      bool InfoProjection::operator==(InfoProjection& other) {
         bool retval = true;

         retval = retval && (Math::isEqualTo<Math::Matrix44d>(imageFrame(), other.imageFrame()));
         retval = retval && (Math::isEqualTo<Math::Matrix44d>(sourceFrame(), other.sourceFrame()));
         retval = retval && (Math::isEqualTo<Math::Matrix44d>(objectFrame(), other.objectFrame()));
         retval = retval && (doseCurrent() == other.doseCurrent());
         retval = retval && (doseVoltage() == other.doseVoltage());

         return retval;
      }
	  //overloading of not equal operator
      bool InfoProjection::operator!=(InfoProjection& other) {
         return !operator==(other);
      }
	  //getter fo image frame
      Math::Matrix44d& InfoProjection::imageFrame() {
         return m_imageFrame;
      }
	  //setter for iage frame
      void InfoProjection::setImageFrame(const Math::Matrix44d& frame) {
         m_imageFrame = frame;
      }
	  //getter for source frame
      Math::Matrix44d& InfoProjection::sourceFrame() {
         return m_sourceFrame;
      }
	  //setter for source frame
      void InfoProjection::setSourceFrame(const Math::Matrix44d& frame) {
         m_sourceFrame = frame;
      }
	  //getter for object frame
      Math::Matrix44d& InfoProjection::objectFrame() {
         return m_objectFrame;
      }
	  //setter for object frame
      void InfoProjection::setObjectFrame(const Math::Matrix44d& frame) {
         m_objectFrame = frame;
      }
	  //getter for dose voltage
      float InfoProjection::doseVoltage() {
         return m_doseVoltage;
      }
	  //setter for dose voltage
      void InfoProjection::setDoseVoltage(const float& voltage) {
         m_doseVoltage = voltage;
      }
	  //getter for dose current
      float InfoProjection::doseCurrent() {
         return m_doseCurrent;
      }
	  //setter for dose current
      void InfoProjection::setDoseCurrent(const float& current) {
         m_doseCurrent = current;
      }
	}
}
