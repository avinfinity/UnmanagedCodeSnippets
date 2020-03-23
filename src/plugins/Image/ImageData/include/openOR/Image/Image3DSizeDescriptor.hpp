//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
/**
* @file
* @author Christian Winne
* \ingroup Image_ImageData
*/
#ifndef openOR_Image_Image3DSizeDescriptor_hpp
#define openOR_Image_Image3DSizeDescriptor_hpp

#include <openOR/Math/math.hpp>
#include <openOR/Math/vector.hpp>

#include <openOR/Image/Image3DSize.hpp>


namespace openOR
{
   namespace Image
   {
      class Image3DSizeDescriptor : public Image3DSize {
      public:

         Image3DSizeDescriptor() {
            m_sizeVolume(0) = 0;
            m_sizeVolume(1) = 0;
            m_sizeVolume(2) = 0;
            m_sizeMMVolume(0) = 0;
            m_sizeMMVolume(1) = 0;
            m_sizeMMVolume(2) = 0;
         }

         virtual ~Image3DSizeDescriptor() {
         }

         virtual void setSize(const Math::Vector3ui& sizeVolume) {
            m_sizeVolume = sizeVolume;
         }

         virtual Math::Vector3ui size() const { return m_sizeVolume; }


         virtual void setSizeMM(const Math::Vector3d& sizeMMVolume) { m_sizeMMVolume = sizeMMVolume; }

         virtual Math::Vector3d sizeMM() const { return m_sizeMMVolume; }

         operator Math::Vector3ui() const {
            return size();
         }

         operator Math::Vector3d() const {
            return sizeMM();
         }

         void operator = (const Math::Vector3d& sizeMMVolume) {
            setSizeMM(sizeMMVolume);
         }

         void operator = (const Math::Vector3ui& sizeVolume) {
            setSize(sizeVolume);
         }

      protected:

         Math::Vector3ui m_sizeVolume;
         Math::Vector3d m_sizeMMVolume;

      };
   }

} 

#endif // openOR_Image_Image3DSizeDescriptor_hpp