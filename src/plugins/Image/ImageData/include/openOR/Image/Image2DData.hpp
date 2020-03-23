//****************************************************************************
// (c) 2008 by the openOR Team
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
#ifndef openOR_Image_Image2DData_hpp
#define openOR_Image_Image2DData_hpp

#include <cassert>
#include <memory>

#include <openOR/coreDefs.hpp>

//#include <boost/config.hpp>
#include <boost/type_traits/has_trivial_destructor.hpp>
#include <boost/type_traits/has_trivial_assign.hpp>

#include <openOR/Math/vector.hpp>
#include <openOR/Math/vectorfunctions.hpp>
#include <openOR/Math/create.hpp>
#include <openOR/Utility/Types.hpp>
#include <openOR/Image/Image2DSize.hpp>
#include <openOR/Image/Image2DRawData.hpp>
#include <openOR/OpenGL/DataInterpreter.hpp>


namespace openOR
{
   namespace Image 
   {

     /** A class, that holds image data and provides methods for accessing.
      *
      * ImageElements will be initialized by a given value or by default by a construction from zero. That means the 
      * given type for VolumeElements (Type) must have an explicit public constructor from int, that constructs an initialized 
      * instance (e.g. MyVoxelElementType(int val))
      *
      * @param Type Type of volume element.
      * @param Alloc Type of allocator for element memory allocation.
      *
      * @author Christian Winne
      *
      * @bug -clipping of SamplingRegion at ConstMMIterator constructor does not work correctly
      *
      * @todo -documentation
      *       -testcases
      *
      * @ingroup Image_ImageData
      */
     template<class Type, class Alloc = std::allocator<Type> >
     class Image2DData : public Image2DSize, public Image2DRawData<Type>, public OpenGL::DataInterpreter
     {
       public:

         typedef typename Alloc::template rebind<Type>::other AllocatorType;
         typedef typename AllocatorType::pointer _pointerType;
         typedef typename AllocatorType::pointer Pointer;
         typedef typename AllocatorType::const_pointer ConstPointer;
         typedef typename AllocatorType::reference _referenceType;
         typedef typename AllocatorType::reference Reference;
         typedef typename AllocatorType::const_reference ConstReference;
         typedef typename AllocatorType::value_type ValueType;
         typedef typename AllocatorType::size_type SizeType;
         typedef typename AllocatorType::difference_type DifferenceType;


         Image2DData()
         : m_pData(NULL),
           m_OpenGLDataFormat(OpenGL::getOpenGLDataFormatFromElementType<Type>()),
           m_OpenGLDataType(OpenGL::getOpenGLDataTypeFromElementType<Type>())
         {
           m_sizeImage(0) = 0;
           m_sizeImage(1) = 0;
           m_sizeMMImage(0) = 1;
           m_sizeMMImage(1) = 1;
         }


         /**
          * Destructor. Frees the memory used by internal data.
          */
         virtual ~Image2DData()
         {
           if (m_pData)
           {
             destroyData(boost::has_trivial_destructor<Type>());
             m_Allocator.deallocate(m_pData, m_sizeImage(0) * m_sizeImage(1));
           }
         }

         
         virtual void setSize(const Math::Vector2ui& sizeImage)
         {
            setSize(sizeImage, Type());
         }


         void setSize(const Math::Vector2ui& sizeImage, const Type& initializeValue)
         {
           if (m_pData)
           {
             SizeType size = m_sizeImage(0) * m_sizeImage(1);
             m_Allocator.deallocate(m_pData, size);
             m_pData = NULL;
           }

           m_sizeImage = sizeImage;
           SizeType size = m_sizeImage(0) * m_sizeImage(1);
           if (size > 0)
           {
             Pointer p = m_pData = m_Allocator.allocate(size);
             std::uninitialized_fill(p, p + size, initializeValue);
           }
         }

         virtual Math::Vector2ui size() const { return m_sizeImage; }


         virtual void setSizeMM(const Math::Vector2d& sizeMMImage) { m_sizeMMImage = sizeMMImage; }

         virtual Math::Vector2d sizeMM() const { return m_sizeMMImage; }

         virtual const void* dataPtr() const { return static_cast<const void*>(data()); }
         virtual void* mutableDataPtr() { return static_cast<void*>(mutableData()); }
         
         virtual const void* dataPtr(const Math::Vector2ui& indexPos) const { return static_cast<const void*>(data() + indexPos(0) + m_sizeImage(0) * indexPos(1));  }
         virtual void* mutableDataPtr(const Math::Vector2ui& indexPos) { return static_cast<void*>(mutableData() + indexPos(0) + m_sizeImage(0) * indexPos(1));  }

         virtual const unsigned int dataFormat() const { return m_OpenGLDataFormat; }
         virtual void setDataFormat(unsigned int nDataFormat) { m_OpenGLDataFormat = nDataFormat; }
         virtual const unsigned int dataType() const { return m_OpenGLDataType; }
         virtual void setDataType(unsigned int nDataType) { m_OpenGLDataType = nDataType; }



         /**
          * Getter for direct access to voxel data.
          * @return   A pointer to the first object of the voxel data
          */
         virtual Type* mutableData() { return m_pData; }
         virtual const Type* data() const { return m_pData; }


         AllocatorType allocator() const { return m_Allocator; }


       protected:

         void destroyData(const boost::mpl::true_&) {}

         void destroyData(const boost::mpl::false_&) {
           
           for ( Pointer   p = m_pData,
                           pEnd = p + Math::multipliedElements(m_sizeImage); 
                 p != pEnd; ++p)
           {
             m_Allocator.destroy(p);
           }
         }

         Math::Vector2ui m_sizeImage;
         Math::Vector2d m_sizeMMImage;
         Pointer m_pData;
         AllocatorType m_Allocator;
         unsigned int m_OpenGLDataFormat;
         unsigned int m_OpenGLDataType;

      };


      // some probably often used specialized types
      typedef Image2DData<bool, std::allocator<bool> > Image2DDataBool;
      typedef Image2DData<uint8, std::allocator<uint8> > Image2DDataUI8;
      typedef Image2DData<openOR::UI8Triple, std::allocator<openOR::UI8Triple> > Image2DDataUI8Triple;
      typedef Image2DData<uint16, std::allocator<uint16> > Image2DDataUI16;
      typedef Image2DData<uint32, std::allocator<uint32> > Image2DDataUI32;
      typedef Image2DData<uint, std::allocator<uint> > Image2DDataUI;
      typedef Image2DData<float, std::allocator<float> > Image2DDataFloat;
      typedef Image2DData<double, std::allocator<double> > Image2DDataDouble;
      typedef Image2DData<FloatQuad, std::allocator<FloatQuad> > Image2DDataFloatQuad;
   
      typedef Image2DData<UI16Pair, std::allocator<UI16Pair> > Image2DDataUI16Pair;
   }
} 

#endif