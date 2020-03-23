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
//****************************************************************************
/**
 * @file
 * @author Christian Winne
 * \ingroup Image_ImageData
 */

#ifndef openOR_Image_Image1DSize_hpp
#define openOR_Image_Image1DSize_hpp

#include <boost/tr1/memory.hpp>

#include <openOR/Plugin/CreateInterface.hpp> //openOr_core
#include <openOR/Math/vector.hpp>//openOr_core
#include <openOR/Math/matrix.hpp> //openOR_core
#include <openOR/Utility/Types.hpp> //openOr_core

namespace openOR {
   namespace Image {

      
      /**
       * \brief Interface that describes the size of an 1D image.
       * The image is described by its pixel count and its size in millimeter.
       * \ingroup Image_ImageData
       */
      struct Image1DSize
      {
         virtual unsigned int size() const = 0;
         virtual void setSize(const unsigned int& size) = 0;
         
         virtual double sizeMM() const = 0;  
         virtual void setSizeMM(const double& sizeMM) = 0;
         
      };    

   }

   /*
    * Getter for the voxel width in mm
    * @return The voxel width in mm
    */
   inline const double pixelWidthMM(std::tr1::shared_ptr<const Image::Image1DSize> pImageSize) { return pImageSize->sizeMM() / pImageSize->size(); }

  
   inline const double pixelSizeMM(std::tr1::shared_ptr<const Image::Image1DSize> pImageSize) 
   { 
      return pixelWidthMM(pImageSize);
   }    

   /**
    * Getter for the width of the whole volume object in mm
    * @return The width of the volume object
    */
   inline const double getMMWidth(std::tr1::shared_ptr<const Image::Image1DSize> pImageSize) { return pImageSize->sizeMM(); }

   inline const double getMMSize(std::tr1::shared_ptr<const Image::Image1DSize> pImageSize) { return pImageSize->sizeMM(); }


   /**
    * Getter for the width of the whole volume object in indices
    * @return The width of the volume object
    */
   inline const uint getWidth(std::tr1::shared_ptr<const Image::Image1DSize> pImageSize) { return pImageSize->size(); }

   /**
    * Getter for the depth of the whole volume object in indices
    * @return The depth of the volume object
    */
   inline const double getSize(std::tr1::shared_ptr<const Image::Image1DSize> pImageSize) { return pImageSize->size(); }


   // /**
    // * Checks whether given relative mm-based point is outside of volume
    // * @param  vecPoint The point to check for
    // * @return  True, if the point is outside the volume
    // */
   // inline const bool isOutside(std::tr1::shared_ptr<const Image::Image1DSize> pImageSize, const Math::Vector2d& vecPoint) { assert(true); return true; }

  // /**
   // * Checks whether given relative index-based point is outside of volume
   // * @param vecPoint The point to check for
   // * @return  True, if the point is outside the volume
   // */      
  // inline const bool isOutside(std::tr1::shared_ptr<const Image::Image2DSize> pImageSize, const Math::Vector2i& vecPoint) { assert(true); return true; }

  // /** 
   // * Returns the absolute BoundingBox around the volume.
   // * @return   The absolute BoundingBox around the volume
   // */
  // //Math::BoundingBoxD getBoundingBox(std::tr1::shared_ptr<const Volume> pVolume) {}


   /**
    * Returns index position corresponding to the given mm position.
    * @return The index position
    */
  inline const unsigned int convertToIndex(std::tr1::shared_ptr<const Image::Image1DSize> pImageSize, const unsigned int& pos)
  {
    return pos;
  }


  inline const unsigned int convertToIndex(std::tr1::shared_ptr<const Image::Image1DSize> pImageSize, const int& pos)
  {
    assert(pos >= 0 && pos < static_cast<int>(pImageSize->size()));

    unsigned int vec = static_cast<uint>(pos);
    return convertToIndex(pImageSize, vec);
  }


  inline const int convertToIndexPosition(std::tr1::shared_ptr<const Image::Image1DSize> pImageSize, const double& posMM)
  {
    return static_cast<int>(posMM * pImageSize->size() / pImageSize->sizeMM());
  }


  /**
   * Returns internal memory index corresponding to the given index/mm position.
   * @return The index
   */
  inline const unsigned int convertToIndex(std::tr1::shared_ptr<const Image::Image1DSize> pImageSize, const double& posMM)
  {
    return convertToIndex(pImageSize, convertToIndexPosition(pImageSize, posMM));
  }


}



OPENOR_CREATE_INTERFACE(openOR::Image::Image1DSize)
double sizeMM() const { return adaptee()->sizeMM(); }
void setSizeMM(const double& size) { adaptee()->setSizeMM(size); }
unsigned int size() const { return adaptee()->size(); }
void setSize(const unsigned int& size) { adaptee()->setSize(size); }
OPENOR_CREATE_INTERFACE_END





#endif 
