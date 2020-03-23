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

#ifndef openOR_Image_Image3DSize_hpp
#define openOR_Image_Image3DSize_hpp

#include <boost/tr1/memory.hpp>

#include <openOR/Plugin/CreateInterface.hpp>
#include <openOR/Math/vector.hpp>
#include <openOR/Math/matrix.hpp>
#include <openOR/Math/vectorfunctions.hpp>
#include <openOR/Utility/Types.hpp>

namespace openOR {
   namespace Image {
     /**
      * Interface that describes a size of a cubic voxel based volume.
      * The volume is described by its voxel count, its size in millimeter.
      * \ingroup Image_ImageData
      */
     struct Image3DSize {

         /**
          * Getter for the volume size as a Size3D object in mm
          * @return The volume size in mm
          */
         virtual Math::Vector3d sizeMM() const = 0;  

         //! \brief	Setter for the volume-size as a Size3UI object in indices
         //! \param[in] sizeMM The volume size in indices
         virtual void setSizeMM(const Math::Vector3d& sizeMM) = 0;

         /**
          * Getter for the volume-size as a Size3UI object in indices
          * @return The volume size in indices
          */
         virtual Math::Vector3ui size() const = 0;

         /** 
          * Setter for the volume-size as a Size3UI object in indices
          * @param size The volume size in indices
          */
         virtual void setSize(const Math::Vector3ui& size) = 0;

     };    
   }  
  
  /*
   * Getter for the voxel width in mm
   * @return The voxel width in mm
   */
   inline const double voxelWidthMM(std::tr1::shared_ptr<const Image::Image3DSize> pVolumeSize) { return pVolumeSize->sizeMM()(0) / pVolumeSize->size()(0); }

  /**
  * Getter for the voxel height in mm
  * @return The voxel height in mm
  */
  inline const double voxelHeightMM(std::tr1::shared_ptr<const Image::Image3DSize> pVolumeSize) { return pVolumeSize->sizeMM()(1) / pVolumeSize->size()(1); }    

  /**
  * Getter for the voxel depth in mm
  * @return The voxel depth in mm
  */
  inline const double voxelDepthMM(std::tr1::shared_ptr<const Image::Image3DSize> pVolumeSize) { return pVolumeSize->sizeMM()(2) / pVolumeSize->size()(2); }    
  
  inline Math::Vector3d voxelSizeMM(std::tr1::shared_ptr<const Image::Image3DSize> pVolumeSize) 
  { 
    Math::Vector3d size;
    size(0) = voxelWidthMM(pVolumeSize);
    size(1) = voxelHeightMM(pVolumeSize);
    size(2) = voxelDepthMM(pVolumeSize);
    return size; 
  }    
  inline Math::Vector3d voxelSizeMMInverted(std::tr1::shared_ptr<const Image::Image3DSize> pVolumeSize) 
  { 
     Math::Vector3d size;
     size(0) = 1.0/voxelWidthMM(pVolumeSize);
     size(1) = 1.0/voxelHeightMM(pVolumeSize);
     size(2) = 1.0/voxelDepthMM(pVolumeSize);
     return size; 
  } 
  

  /**
   * Getter for the width of the whole volume object in mm
   * @return The width of the volume object
   */
  inline const double widthMM(std::tr1::shared_ptr<const Image::Image3DSize> pVolumeSize) { return pVolumeSize->sizeMM()(0); }

  /**
   * Getter for the height of the whole volume object in mm
   * @return The height of the volume object
   */
  inline const double heightMM(std::tr1::shared_ptr<const Image::Image3DSize> pVolumeSize) { return pVolumeSize->sizeMM()(1); }

  /**
   * Getter for the depth of the whole volume object in mm
   * @return The depth of the volume object
   */
  inline const double depthMM(std::tr1::shared_ptr<const Image::Image3DSize> pVolumeSize) { return pVolumeSize->sizeMM()(2); }


  inline Math::Vector3d sizeMM(std::tr1::shared_ptr<const Image::Image3DSize> pVolumeSize) { return pVolumeSize->sizeMM(); }


  /**
   * Getter for the width of the whole volume object in indices
   * @return The width of the volume object
   */
  inline const uint width(std::tr1::shared_ptr<const Image::Image3DSize> pVolumeSize) { return pVolumeSize->size()(0); }

  /**
   * Getter for the height of the whole volume object in indices
   * @return The height of the volume object
   */
  inline const uint height(std::tr1::shared_ptr<const Image::Image3DSize> pVolumeSize) { return pVolumeSize->size()(1); }

  /**
   * Getter for the depth of the whole volume object in indices
   * @return The depth of the volume object
   */
  inline const uint depth(std::tr1::shared_ptr<const Image::Image3DSize> pVolumeSize) { return pVolumeSize->size()(2); }

  inline Math::Vector3ui size(std::tr1::shared_ptr<const Image::Image3DSize> pVolumeSize) { return pVolumeSize->size(); }


  /**
   * Checks whether given relative mm-based point is outside of volume
   * @param   vecPoint The point to check for
   * @return  True, if the point is outside the volume
   */
  inline const bool isOutside(std::tr1::shared_ptr<const Image::Image3DSize> pVolume, const Math::Vector3d& vecPoint) {
     Math::Vector3d sizeMM = pVolume->sizeMM();
     if (vecPoint(0) < 0 || vecPoint(0) > sizeMM(0) || 
         vecPoint(1) < 0 || vecPoint(1) > sizeMM(1) || 
         vecPoint(2) < 0 || vecPoint(2) > sizeMM(2))
     {
        return true;
     }
     return false; 
  }


  /**
   * Checks whether given relative index-based point is outside of volume
   * @param   vecPoint The point to check for
   * @return  True, if the point is outside the volume
   */      
  inline const bool isOutside(std::tr1::shared_ptr<const Image::Image3DSize> pVolume, const Math::Vector3i& vecPoint){
     Math::Vector3ui size = pVolume->size();
     if (vecPoint(0) < 0 || vecPoint(0) >= static_cast<int>(size(0)) || 
         vecPoint(1) < 0 || vecPoint(1) >= static_cast<int>(size(1)) || 
         vecPoint(2) < 0 || vecPoint(2) >= static_cast<int>(size(2)))
     {
        return true;
     }
     return false; 
  }

  inline const bool isOutside(std::tr1::shared_ptr<const Image::Image3DSize> pVolume, const Math::Vector3ui& vecPoint){
     Math::Vector3ui size = pVolume->size();
     // code review 25.09.2012: is it necessary to check for less than zero? isn't unsigned not always zero at minimum?
     if (vecPoint(0) < 0 || (unsigned int)vecPoint(0) >= size(0) || 
         vecPoint(1) < 0 || (unsigned int)vecPoint(1) >= size(1) || 
         vecPoint(2) < 0 || (unsigned int)vecPoint(2) >= size(2))
     {
        return true;
     }
     return false; 
  }

  /** 
   * Returns the absolute BoundingBox around the volume.
   * @return   The absolute BoundingBox around the volume
   */
  //Math::BoundingBoxD getBoundingBox(std::tr1::shared_ptr<const Volume> pVolume) {}


  /**
   * Returns index position corresponding to the given mm position.
   * @return The index position
   */

  inline void convertToVecPosition(std::tr1::shared_ptr<const Image::Image3DSize> pVolumeSize, const unsigned long index, Math::Vector3ui& vecPosition )
  {
     vecPosition(0) = ( index % pVolumeSize->size()(0) ) ;
     vecPosition(1) = ( (index % (pVolumeSize->size()(0) * pVolumeSize->size()(1))) / pVolumeSize->size()(0) );
     vecPosition(2) = index / (pVolumeSize->size()(0) * pVolumeSize->size()(1));
  }

  inline void convertToPosition (std::tr1::shared_ptr<const Image::Image3DSize> pVolumeSize, unsigned long index, Math::Vector3d &pos){
     Math::Vector3d voxl_mm = voxelSizeMM(pVolumeSize);
     Math::Vector3ui vecPos;
     convertToVecPosition(pVolumeSize, index, vecPos);
     pos = Math::elementProd<Math::Vector3d>(vecPos, voxl_mm);
  }



  inline uint convertToIndex(std::tr1::shared_ptr<const Image::Image3DSize> pVolumeSize, const Math::Vector3ui& vecPosition)
  {
    return (vecPosition(2) * pVolumeSize->size()(1) + vecPosition(1)) * pVolumeSize->size()(0) + vecPosition(0);
  }


  inline uint convertToIndex(std::tr1::shared_ptr<const Image::Image3DSize> pVolumeSize, const Math::Vector3i& vecPosition)
  {
    assert(vecPosition(0) >= 0 && vecPosition(0) < static_cast<int>(pVolumeSize->size()(0)) &&
      vecPosition(1) >= 0 && vecPosition(1) < static_cast<int>(pVolumeSize->size()(1)) &&
      vecPosition(2) >= 0 && vecPosition(2) < static_cast<int>(pVolumeSize->size()(2)));

    Math::Vector3ui vec;
    vec(0) = static_cast<uint>(vecPosition(0));
    vec(1) = static_cast<uint>(vecPosition(1));
    vec(2) = static_cast<uint>(vecPosition(2));
    return convertToIndex(pVolumeSize, vec);
  }


  inline Math::Vector3i convertToIndexPosition(std::tr1::shared_ptr<const Image::Image3DSize> pVolumeSize, const Math::Vector3d& vecPosition)
  {
    Math::Vector3i result;
    result(0) = static_cast<int>(vecPosition(0) * pVolumeSize->size()(0) / pVolumeSize->sizeMM()(0));
    result(1) = static_cast<int>(vecPosition(1) * pVolumeSize->size()(1) / pVolumeSize->sizeMM()(1));
    result(2) = static_cast<int>(vecPosition(2) * pVolumeSize->size()(2) / pVolumeSize->sizeMM()(2));
    return result;
  }


  /**
   * Returns internal memory index corresponding to the given index/mm position.
   * @return The index
   */
  inline uint convertToIndex(std::tr1::shared_ptr<const Image::Image3DSize> pVolumeSize, const Math::Vector3d& vecPosition)
  {
    return convertToIndex(pVolumeSize, convertToIndexPosition(pVolumeSize, vecPosition));
  }


  inline Math::Matrix44d normalizedTVolume(std::tr1::shared_ptr<const Image::Image3DSize> pVolumeSize)
  {
    Math::Matrix44d mat = Math::MatrixTraits<Math::Matrix44d>::IDENTITY;
    mat(0, 0) = 1.0 / widthMM(pVolumeSize);
    mat(1, 1) = 1.0 / heightMM(pVolumeSize);
    mat(2, 2) = 1.0 / depthMM(pVolumeSize);
    return mat;
  }


  inline Math::Matrix44d volumeTNormalized(std::tr1::shared_ptr<const Image::Image3DSize> pVolumeSize)
  {
    Math::Matrix44d mat = Math::MatrixTraits<Math::Matrix44d>::IDENTITY;
    mat(0, 0) = widthMM(pVolumeSize);
    mat(1, 1) = heightMM(pVolumeSize);
    mat(2, 2) = depthMM(pVolumeSize);
    return mat;
  }

}

OPENOR_CREATE_INTERFACE(openOR::Image::Image3DSize)
   Math::Vector3d sizeMM() const { return adaptee()->sizeMM(); }
   void setSizeMM(const Math::Vector3d& size) { adaptee()->setSizeMM(size); }

   Math::Vector3ui size() const { return adaptee()->size(); }
   void setSize(const Math::Vector3ui& size) { adaptee()->setSize(size); }
OPENOR_CREATE_INTERFACE_END


#endif 
