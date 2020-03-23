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

#ifndef openOR_Image_Image1DRawData_hpp
#define openOR_Image_Image1DRawData_hpp

#include <boost/tr1/memory.hpp>

#include <openOR/Plugin/CreateInterface.hpp> //openOR_core
#include <openOR/Math/vector.hpp> //openOR_core
#include <openOR/Utility/Types.hpp> //openOR_core

namespace openOR {
   namespace Image {

     /**
      * \brief Image1DRawData
      * \ingroup Image_ImageData
      */
     template<class Type>
     class Image1DRawData
     {
       public:

         typedef Type ValueType;
         typedef ValueType* Pointer;
         typedef const ValueType* ConstPointer;


         /**
         * Getter for the volume-size as a Size3UI object in indices
         * @return The volume size in indices
         */
         virtual unsigned int size() const = 0;

         /** 
         * Setter for the volume-size as a Size3UI object in indices
         * @param size The volume size in indices
         */
         virtual void setSize(const unsigned int& size) = 0;

         virtual Pointer mutableData() = 0;

         virtual ConstPointer data() const = 0;

     };    

     typedef Image1DRawData<uint8> Image1DRawDataUI8;
     typedef Image1DRawData<uint16> Image1DRawDataUI16;
     typedef Image1DRawData<uint32> Image1DRawDataUI32;
     typedef Image1DRawData<uint64> Image1DRawDataUI64;
     typedef Image1DRawData<float> Image1DRawDataFloat;
     typedef Image1DRawData<double> Image1DRawDataDouble;
     typedef Image1DRawData<UI8Quad> Image1DRawDataUI8Quad;
     typedef Image1DRawData<UI16Quad> Image1DRawDataUI16Quad;
     typedef Image1DRawData<FloatQuad> Image1DRawDataFloatQuad;
   }

  /**
  * Getter for the width of the whole volume object in indices
  * @return The width of the volume object
  */
  template<class Type>
  const uint width(std::tr1::shared_ptr<const Image::Image1DRawData<Type> > pImageData) { return pImageData->size(); }


  template<class Type>
  Type& voxelIndexRef(std::tr1::shared_ptr<Image::Image1DRawData<Type> > pImageData, const uint& pos) 
  {
    return *(pImageData->mutableData() + pos);
  }

  template<class Type>
  const Type& voxelIndexRef(std::tr1::shared_ptr<const Image::Image1DRawData<Type> > pImageData, const uint& pos)
  {
    return *(pImageData->data() + pos);
  }



  /************************************************************************/
  /*                                                                      */
  /************************************************************************/


}

OPENOR_CREATE_TEMPLATED_INTERFACE_1(Type, openOR::Image::Image1DRawData<Type>)

  uint size() const { return adaptee()->size(); }
  void setSize(const uint& size) { adaptee()->setSize(size); }

  Type* mutableData() { return adaptee()->mutableData(); }
  const Type* data() const { return adaptee()->data(); }

OPENOR_CREATE_INTERFACE_END



#endif