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
 * @author Christian Winne
 * \ingroup Image_ImageData
 */
#ifndef openOR_Image_Image1DData_hpp
#define openOR_Image_Image1DData_hpp

#include <cassert>
#include <memory>

#include <openOR/coreDefs.hpp> //openOR_core

//#include <boost/config.hpp>
#include <boost/type_traits/has_trivial_destructor.hpp>
#include <boost/type_traits/has_trivial_assign.hpp>

#include <openOR/Math/vector.hpp> //openOr_core
#include <openOR/Math/vectorfunctions.hpp> //openOr_core
#include <openOR/Math/create.hpp> //openOr_core
#include <openOR/Utility/Types.hpp> //openOr_core
#include <openOR/Image/Image1DSize.hpp> 
#include <openOR/Image/Image1DRawData.hpp>
#include <openOR/OpenGL/DataInterpreter.hpp>


namespace openOR
{
   namespace Image 
   {

     /** A class, that holds image data and provides methods for accessing.
      *
      * ImageElements will be initialized by a given value or by default by a construction from zero. That means the 
      * given type for VolumeElements (Type) has to have an explicit public constructor from int, that constructs an initialized 
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
     class Image1DData : public Image1DSize, public Image1DRawData<Type>, public OpenGL::DataInterpreter
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


         Image1DData()
         : m_pData(NULL),
           m_OpenGLDataFormat(OpenGL::getOpenGLDataFormatFromElementType<Type>()),
           m_OpenGLDataType(OpenGL::getOpenGLDataTypeFromElementType<Type>())
         {
           m_sizeImage = 0;
           m_sizeMMImage = 1;
         }


         /**
          * Destructor. Frees the memory used by internal data.
          */
         virtual ~Image1DData()
         {
           if (m_pData)
           {
             destroyData(boost::has_trivial_destructor<Type>());
             m_Allocator.deallocate(m_pData, m_sizeImage);
           }
         }

         
         virtual void setSize(const unsigned int& sizeImage)
         {
            setSize(sizeImage, Type());
         }


         void setSize(const unsigned int& sizeImage, const Type& initializeValue)
         {
           if (m_pData)
           {
             m_Allocator.deallocate(m_pData, m_sizeImage);
             m_pData = NULL;
           }

           m_sizeImage = sizeImage;
           if (size() > 0)
           {
             Pointer p = m_pData = m_Allocator.allocate(m_sizeImage);
             std::uninitialized_fill(p, p + m_sizeImage, initializeValue);
           }
         }

         virtual unsigned int size() const { return m_sizeImage; }

         virtual void setSizeMM(const double& sizeMMImage) { m_sizeMMImage = sizeMMImage; }

         virtual double sizeMM() const { return m_sizeMMImage; }

         virtual const void* dataPtr() const { return static_cast<const void*>(data()); }
         
         virtual void* mutableDataPtr() { return static_cast<void*>(mutableData()); }
         
         virtual const void* dataPtr(const unsigned int& indexPos) const { return static_cast<const void*>(data() + indexPos);  }
         
         virtual void* mutableDataPtr(const unsigned int& indexPos) { return static_cast<void*>(mutableData() + indexPos);  }

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
                           pEnd = p + m_sizeImage;
                 p != pEnd; ++p)
           {
             m_Allocator.destroy(p);
           }
         }

         unsigned int m_sizeImage;
         double m_sizeMMImage;
         Pointer m_pData;
         AllocatorType m_Allocator;
         unsigned int m_OpenGLDataFormat;
         unsigned int m_OpenGLDataType;

      };


      // some probably often used specialized types
      typedef Image1DData<uint8, std::allocator<uint8> > Image1DDataUI8;
      typedef Image1DData<uint16, std::allocator<uint16> > Image1DDataUI16;
      typedef Image1DData<uint32, std::allocator<uint32> > Image1DDataUI32;
      typedef Image1DData<uint64, std::allocator<uint64> > Image1DDataUI64;
      typedef Image1DData<uint, std::allocator<uint> > Image1DDataUI;
      typedef Image1DData<float, std::allocator<float> > Image1DDataFloat;
      typedef Image1DData<double, std::allocator<double> > Image1DDataDouble;
      typedef Image1DData<FloatQuad, std::allocator<FloatQuad> > Image1DDataFloatQuad;
   
   }
} 

#endif