//****************************************************************************
// (c) 2012 by the openOR Team
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

#ifndef openOR_Image_InfoProjection_hpp
#define openOR_Image_InfoProjection_hpp

#include <openOR/Defs/Image_ImageData.hpp>
#include <openOR/Math/math.hpp>

namespace openOR {
	namespace Image {

        struct OPENOR_IMAGE_IMAGEDATA_API InfoProjection {
            InfoProjection();
            virtual ~InfoProjection();
			//overloadeing of equal and not-equal operators
            bool operator==(InfoProjection& other);
            bool operator!=(InfoProjection& other);
			//getter, setter image frame
            Math::Matrix44d& imageFrame();
            void setImageFrame(const Math::Matrix44d& frame);
			//getter, setter source frame
            Math::Matrix44d& sourceFrame();
            void setSourceFrame(const Math::Matrix44d& frame);
			//getter, setter object frame
            Math::Matrix44d& objectFrame();
            void setObjectFrame(const Math::Matrix44d& frame);
			//getter, setter dose  voltage
            float doseVoltage();
            void setDoseVoltage(const float& voltage);
			//getter, setter dose current
            float doseCurrent();
            void setDoseCurrent(const float& current);

        private:
            Math::Matrix44d m_imageFrame;
            Math::Matrix44d m_sourceFrame;
            Math::Matrix44d m_objectFrame;

            float m_doseCurrent;
            float m_doseVoltage;
        };
	}
}

#endif