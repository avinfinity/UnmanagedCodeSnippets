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
#ifndef openOR_Image_Image3DData_hpp
#define openOR_Image_Image3DData_hpp

#include <cassert>
#include <memory>

#include <openOR/coreDefs.hpp>

//#include <boost/config.hpp>
//#include <boost/type_traits/has_trivial_destructor.hpp>
//#include <boost/type_traits/has_trivial_assign.hpp>

#include <openOR/Math/math.hpp>

#include <openOR/Image/Image3DSize.hpp>
#include <openOR/Image/Image3DRawData.hpp>
#include <openOR/OpenGL/DataInterpreter.hpp>
#include <openOR/Signaller.hpp>


namespace openOR
{
	namespace Image
	{

		/** A class, that holds volume data and provides methods for accessing.
		*
		* VolumeElements will be initialized by a given value or by default by a construction from zero. That means the 
		* given type for VolumeElements (Type) has to have an explicit public constructor from int, that constructs an initialized 
		* instance (e.g. MyVoxelElementType(int val))
		*
		* @tparam Type Type of volume element.
		* @tparam Alloc Type of allocator for element memory allocation.
		*
		* \ingroup Image_ImageData
		*/
		template<class Type, class Alloc = std::allocator<Type> >
		class Image3DData : public Image3DSize, public Image3DRawData<Type>, public OpenGL::DataInterpreter
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


			Image3DData()
				: m_pData(NULL),
				m_OpenGLDataFormat(OpenGL::getOpenGLDataFormatFromElementType<Type>()),
				m_OpenGLDataType(OpenGL::getOpenGLDataTypeFromElementType<Type>())
			{
				m_sizeVolume(0) = 0;
				m_sizeVolume(1) = 0;
				m_sizeVolume(2) = 0;
				m_sizeMMVolume(0) = 0;
				m_sizeMMVolume(1) = 0;
				m_sizeMMVolume(2) = 0;
			}


			/**
			* Destructor. Frees the memory used by internal data.
			*/
			virtual ~Image3DData()
			{
				if (m_pData)
				{
					destroyData(boost::has_trivial_destructor<Type>());
					m_Allocator.deallocate(m_pData, m_sizeVolume(0) * m_sizeVolume(1) * m_sizeVolume(2));
				}
			}

			virtual void setSize(const Math::Vector3ui& sizeVolume)
			{
				setSize(sizeVolume, Type());
			}

			virtual void setSize(const Math::Vector3ui& sizeVolume, const Type& initializeValue)
			{
				if (m_pData)
				{
					SizeType size = static_cast<SizeType>(m_sizeVolume(0)) * m_sizeVolume(1) * m_sizeVolume(2);
					m_Allocator.deallocate(m_pData, size);
					m_pData = NULL;
				}

				m_sizeVolume = sizeVolume;
				SizeType size = static_cast<SizeType>(m_sizeVolume(0)) * m_sizeVolume(1) * m_sizeVolume(2);
				if (size > 0)
				{
					Pointer p = m_pData = m_Allocator.allocate(size);
					std::uninitialized_fill(p, p + size, initializeValue);
				}
			}

			virtual Math::Vector3ui size() const { return m_sizeVolume; }


			virtual void setSizeMM(const Math::Vector3d& sizeMMVolume) { m_sizeMMVolume = sizeMMVolume; }

			virtual Math::Vector3d sizeMM() const { return m_sizeMMVolume; }

			virtual const void* dataPtr() const { return static_cast<const void*>(data()); }
			virtual void* mutableDataPtr() { return static_cast<void*>(mutableData()); }


			virtual const void* dataPtr(const Math::Vector3ui& indexPos) const 
			{ 
				return static_cast<const void*>(data() + indexPos(0) + m_sizeVolume(0) * (indexPos(1) + m_sizeVolume(1) * indexPos(2)));  
			}


			virtual void* mutableDataPtr(const Math::Vector3ui& indexPos) 
			{ 
				return static_cast<void*>(mutableData() + indexPos(0) + m_sizeVolume(0) * (indexPos(1) + m_sizeVolume(1) * indexPos(2)));  
			}

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
				for ( Pointer  p = m_pData, 
					pEnd = p + Math::multipliedElements( m_sizeVolume ); 
					p != pEnd; ++p) 
				{
					m_Allocator.destroy(p);
				}
			}

			Math::Vector3ui m_sizeVolume;
			Math::Vector3d m_sizeMMVolume;
			Pointer m_pData;
			AllocatorType m_Allocator;
			unsigned int m_OpenGLDataFormat;
			unsigned int m_OpenGLDataType;

		};


		// some probably often used specialized types
		typedef Image3DData<uint8, std::allocator<uint8> > Image3DDataUI8;
		typedef Image3DData<uint16, std::allocator<uint16> > Image3DDataUI16;
		typedef Image3DData<uint32, std::allocator<uint32> > Image3DDataUI32;
		typedef Image3DData<uint, std::allocator<uint> > Image3DDataUI;
		typedef Image3DData<double, std::allocator<double> > Image3DDataD;

	}

} 

#endif