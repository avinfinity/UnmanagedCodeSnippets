//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
#ifndef openOR_Image_RegionOfInterest_hpp
#define openOR_Image_RegionOfInterest_hpp

#include <openOR/Defs/Image_Regions.hpp>

#include <openOR/Utility/Types.hpp> //openOR_core

#include <openOR/Math/vector.hpp> //openOR_core
#include <openOR/Math/create.hpp>//openOR_core

namespace openOR {
   namespace Image {

      struct OPENOR_IMAGE_REGIONS_API RegionOfInterest {
         RegionOfInterest() :
            m_regionColor(Math::create<Math::Vector3d>(0, 0, 0)),
            m_frontLowerLeft(Math::create<Math::Vector3ui>(0, 0, 0)),
            m_backUpperRight(Math::create<Math::Vector3ui>(0, 0, 0)),
            m_index()
         {}

         ~RegionOfInterest() {}

         void setRegionColor(const Math::Vector3d& regionColor) {
            m_regionColor = regionColor;
         }

         void setFrontLowerLeft(const Math::Vector3ui& frontLowerLeft) {
            m_frontLowerLeft = frontLowerLeft;
         }

         void setBackUpperRight(const Math::Vector3ui& backUpperRight) {
            m_backUpperRight = backUpperRight;
         }

         void setIndex(const uint32& index) {
            m_index = index;
         }

         const uint32& index() const {
            return m_index;
         }

         uint32& index() {
            return m_index;
         }

         const Math::Vector3d& regionColor() const {
            return m_regionColor;
         }

         Math::Vector3d& regionColor() {
            return m_regionColor;
         }

         const Math::Vector3ui& frontLowerLeft() const {
            return m_frontLowerLeft;
         }

         Math::Vector3ui& frontLowerLeft() {
            return m_frontLowerLeft;
         }

         const Math::Vector3ui& backUpperRight() const {
            return m_backUpperRight;
         }

         Math::Vector3ui& backUpperRight() {
            return m_backUpperRight;
         }


      private:
         Math::Vector3d m_regionColor;
         Math::Vector3ui m_frontLowerLeft;
         Math::Vector3ui m_backUpperRight;
         uint32 m_index;
      };
   }
}

#endif
