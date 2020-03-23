//****************************************************************************
// (c) 2008 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
/**
* @file
* @author Adam Wieckowski
* \ingroup Image_Utilities
*/
#ifndef openOR_Image_DataWrapper_hpp
#define openOR_Image_DataWrapper_hpp

#include <cassert>
#include <memory>

#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_const.hpp>

#include <openOR/coreDefs.hpp> //openOR_core

#include <openOR/Utility/Types.hpp> //openOR_core


namespace openOR {
   namespace Image {

      /** A class, that wraps existing data as an image. No resizing is possible. The ownership over the data
      * is not changed, thus the class does not take care of deinitialization and deletion after.
      * 
      * The type is introduced to convinently pass sized data pointers around as AnyPtr.
      *
      * @param Type Type of volume element.
      */
      template<typename Type>
      class DataWrapper {

      public:

         typedef typename boost::is_const<Type>::type IsTypeConst;
         typedef typename boost::mpl::if_<IsTypeConst, const void*, void*>::type VoidPointerType;

         DataWrapper(Type* pData, const size_t& width) {
            m_pData = pData;
            m_size = width;
         }

         /**
         * Destructor. Frees the memory used by internal data.
         */
         virtual ~DataWrapper() {
         }

         virtual size_t size() const { return m_size; }

         virtual const void* dataPtr() const { return static_cast<const void*>(data()); }

         virtual const void* dataPtr(const size_t& indexPos) const { return static_cast<const void*>(data() + indexPos); }

         virtual const Type* data() const { return m_pData; }

         virtual VoidPointerType mutableDataPtr() { return static_cast<const void*>(mutableData()); }

         virtual VoidPointerType mutableDataPtr(const size_t& indexPos) { return static_cast<const void*>(mutableData() + indexPos); }

         virtual Type* mutableData() { return m_pData; }

      private:

         unsigned int m_size;
         Type* m_pData;

      };

      // some probably often used specialized types
      typedef DataWrapper<uint8_t> DataWrapperUI8;
      typedef DataWrapper<uint16> DataWrapperUI16;
      typedef DataWrapper<uint32> DataWrapperUI32;
      typedef DataWrapper<uint64> DataWrapperUI64;
      typedef DataWrapper<uint> DataWrapperUI;
      typedef DataWrapper<float> DataWrapperFloat;
      typedef DataWrapper<double> DataWrapperDouble;
      typedef DataWrapper<FloatQuad> DataWrapperFloatQuad;

      typedef DataWrapper<const uint8_t> ConstDataWrapperUI8;
      typedef DataWrapper<const uint16> ConstDataWrapperUI16;
      typedef DataWrapper<const uint32> ConstDataWrapperUI32;
      typedef DataWrapper<const uint64> ConstDataWrapperUI64;
      typedef DataWrapper<const uint> ConstDataWrapperUI;
      typedef DataWrapper<const float> ConstDataWrapperFloat;
      typedef DataWrapper<const double> ConstDataWrapperDouble;
      typedef DataWrapper<const FloatQuad> ConstDataWrapperFloatQuad;
   }
} 

#endif