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

#ifndef openOR_Image_InfoDevice_hpp
#define openOR_Image_InfoDevice_hpp

#include <openOR/Defs/Image_ImageData.hpp>

#include <string>
#include <vector>

namespace openOR {
	namespace Image {

        struct OPENOR_IMAGE_IMAGEDATA_API InfoDevice {

            enum Device {
               UNKNOWN, CT, DVT, C_ARM
            };

            InfoDevice();
            virtual ~InfoDevice();
			//overloading of equal operator
            bool operator==(InfoDevice& other);
			//getter, setter for modality, and modality-dcm
            Device modality();
            std::string modalityDcm();
            void setModality(const InfoDevice::Device& modality);
            void setModality(const std::string& modality);
			//getter, setter manufacturer
            std::string manufacturer();
            void setManufacturer(const std::string& manufacturer);
			//getter, setter model
            std::string model();
            void setModel(const std::string& model);
			//getter, setter for angle of robot
			std::vector<float>& anglesOfRobot();
            void setAnglesOfRobot(const std::vector<float>& angles);
			//getter for info string
            std::string getInfoStr();

        private:
           
           Device m_modality;
           std::string m_manufacturer;
           std::string m_model;
           std::vector<float> m_anglesOfRobot;

           std::string m_strModality;
        };

	}
}

#endif