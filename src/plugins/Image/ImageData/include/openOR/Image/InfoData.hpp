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

#ifndef openOR_Image_InfoData_hpp
#define openOR_Image_InfoData_hpp

namespace openOR {
   namespace Image {

      template<typename T>
      struct InfoData {

         InfoData();
         ~InfoData();

         T minimum() {
            return m_min;
         }

         void setMinimum(const T& min) {
            m_min = min;
         }

         T maximum() {
            return m_max;
         }

         void setMaximum(const T& max) {
            m_max = max;
         }

      private:
         T m_min;
         T m_max;
      };

   }
}

#endif