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

#ifndef openOR_Image_Image3DRawData_hpp
#define openOR_Image_Image3DRawData_hpp

#include <boost/tr1/memory.hpp>

#include <openOR/Plugin/CreateInterface.hpp>
#include <openOR/Math/vector.hpp>
#include <openOR/Utility/Types.hpp>

namespace openOR {
   namespace Image {

     /**
      * \brief Image3DRawData
      * \ingroup Image_ImageData
      */
     template<class Type>
     class Image3DRawData
     {
       public:

         typedef Type ValueType;
         typedef ValueType* Pointer;
         typedef ValueType const* ConstPointer;
         typedef ValueType& Reference;
         typedef ValueType const& ConstReference;
         
         typedef ValueType value_type;
         typedef Pointer pointer;
         typedef ConstPointer const_pointer;
         typedef Reference reference;
         typedef ConstReference const_reference;


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

         virtual Pointer mutableData() = 0;

         virtual ConstPointer data() const = 0;

     };    


     typedef Image3DRawData<uint8> Image3DRawDataUI8;
     typedef Image3DRawData<uint16> Image3DRawDataUI16;
   
   }

  /**
  * Getter for the width of the whole volume object in indices
  * @return The width of the volume object
  */
  template<class Type>
  const uint width(std::tr1::shared_ptr<const Image::Image3DRawData<Type> > pVolumeData) { return pVolumeData->size()(0); }

  /**
  * Getter for the height of the whole volume object in indices
  * @return The height of the volume object
  */
  template<class Type>
  const uint height(std::tr1::shared_ptr<const Image::Image3DRawData<Type> > pVolumeData) { return pVolumeData->size()(1); }

  /**
  * Getter for the depth of the whole volume object in indices
  * @return The depth of the volume object
  */
  template<class Type>
  const uint depth(std::tr1::shared_ptr<const Image::Image3DRawData<Type> > pVolumeData) { return pVolumeData->size()(2); }


  template<class Type>
  unsigned int indexPosition(std::tr1::shared_ptr<const Image::Image3DRawData<Type> > pVolumeData, const Math::Vector3ui& vecPosition) 
  {
     return vecPosition(0) + width(pVolumeData) * (vecPosition(1) + height(pVolumeData) * vecPosition(2));
  }

  template<class Type>
  unsigned int indexPosition(std::tr1::shared_ptr<const Image::Image3DRawData<Type> > pVolumeData, unsigned int x, unsigned int y, unsigned int z) 
  {
     return x + width(pVolumeData) * (y + height(pVolumeData) * z);
  }

  template<class Type>
  Type& voxelIndexRef(std::tr1::shared_ptr<Image::Image3DRawData<Type> > pVolumeData, const Math::Vector3ui& vecPosition) 
  {
    std::tr1::shared_ptr<const Image::Image3DRawData<Type> > pConstVolumeData = pVolumeData;
    return *(pVolumeData->mutableData() + vecPosition(0) + width(pConstVolumeData) * (vecPosition(1) + height(pConstVolumeData) * vecPosition(2)));
  }


  template<class Type>
  const Type& voxelIndexRef(std::tr1::shared_ptr<const Image::Image3DRawData<Type> > pVolumeData, const Math::Vector3ui& vecPosition)
  {
    return *(pVolumeData->data() + vecPosition(0) + width(pVolumeData) * (vecPosition(1) + height(pVolumeData) * vecPosition(2)));
  }


  template<class Type>
  Type& voxelIndexRef(std::tr1::shared_ptr<Image::Image3DRawData<Type> > pVolumeData, const uint index)
  {
    return *(pVolumeData->mutableData() + index);
  }

  
  template<class Type>
  const Type& voxelIndexRef(std::tr1::shared_ptr<const Image::Image3DRawData<Type> > pVolumeData, const uint index)
  {
    return *(pVolumeData->data() + index);
  }




  /************************************************************************/
  /*                                                                      */
  /************************************************************************/

  ///**
  // * \brief VolumeCreator
  // * \ingroup Volume_VolumeRawData
  // */
  //template<class Volume>
  //struct VolumeCreator 
  //{
  //  typedef Volume VolumeType;

  //  virtual std::tr1::shared_ptr<VolumeType> operator()() const = 0;
  //  virtual void setVolume(std::tr1::shared_ptr<VolumeType> pVolume) = 0;
  //  virtual std::tr1::shared_ptr<VolumeType> getVolume() = 0;
  //};

  //typedef VolumeCreator<VolumeRawDataUI8> VolumeRawDataUI8Creator;
  //typedef VolumeCreator<VolumeRawDataUI16> VolumeRawDataUI16Creator;



  /************************************************************************/
  /*                                                                      */
  /************************************************************************/

}

OPENOR_CREATE_TEMPLATED_INTERFACE_1(Type, openOR::Image::Image3DRawData<Type>)

  Math::Vector3ui size() const { return adaptee()->size(); }
  void setSize(const Math::Vector3ui& size) { adaptee()->setSize(size); }

  Type* mutableData() { return adaptee()->mutableData(); }
  const Type* data() const { return adaptee()->data(); }

OPENOR_CREATE_INTERFACE_END

//
//OPENOR_CREATE_TEMPLATED_INTERFACE_1(Volume, openOR::VolumeCreator<Volume>)
//  std::tr1::shared_ptr<Volume> operator()() const { return adaptee()->operator()(); }
//  void setVolume(std::tr1::shared_ptr<Volume> pVolume) { adaptee()->setVolume(pVolume); }
//  std::tr1::shared_ptr<Volume> getVolume() { return adaptee()->getVolume(); }
//
//OPENOR_CREATE_INTERFACE_END


#endif 
