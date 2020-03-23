//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//! OPENOR_INTERFACE_FILE(Volume_VolumeRawData)
//****************************************************************************
/**
 * @file
 * @author Christian Winne
 * \ingroup Volume_VolumeRawData
 */

#ifndef openOR_VolumeDataRanges_hpp
#define openOR_VolumeDataRanges_hpp

#include <boost/tr1/memory.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/mpl/if.hpp>

#include <openOR/Math/vector.hpp>
#include <openOR/Math/vectorfunctions.hpp>
#include <openOR/Image/Image3DRawData.hpp>
#include <openOR/Utility/Types.hpp>

namespace openOR {

   namespace Image {

      namespace Detail
      {
      
         template<class _VolumeRawData, bool> /* _VolumeRawData is const type */
         struct VolumeRawDataAccess
         {

            typedef typename _VolumeRawData::ConstPointer ConstPointer;

            ConstPointer operator()(const std::tr1::shared_ptr<_VolumeRawData>& pVolumeRawData) const
            {
               return pVolumeRawData->data();
            }
         };


         template<class _VolumeRawData>
         struct VolumeRawDataAccess<_VolumeRawData, false>
         {
            typedef typename _VolumeRawData::Pointer Pointer;

            Pointer operator()(const std::tr1::shared_ptr<_VolumeRawData>& pVolumeRawData) const
            {
               return pVolumeRawData->mutableData();
            }
         };

      }
      /**
       * Range class to provide iterators for iteration through the whole volume.
       * Use this range class if you just want to access the element data. If you often need
       * the index position of the iterator, the IndexRange-iterators are more efficient.
       */
      template<class _VolumeRawData>
      class WholeVolumeRange
      {
      public:

         typedef typename _VolumeRawData::value_type _ValueType;
         typedef typename boost::mpl::if_<boost::is_const<_VolumeRawData>, 
                                          typename _VolumeRawData::ConstPointer,
                                          typename _VolumeRawData::Pointer>::type _Pointer;
         typedef typename boost::mpl::if_<boost::is_const<_VolumeRawData>, 
                                          typename _VolumeRawData::ConstReference,
                                          typename _VolumeRawData::Reference>::type _Reference;

         typedef std::ptrdiff_t _DiffType;
           
         typedef WholeVolumeRange<_VolumeRawData> Range;
         typedef _VolumeRawData VolumeRawData;
          
         typedef _ValueType value_type;
         typedef std::ptrdiff_t difference_type;
         typedef std::ptrdiff_t distance_type;	// retained
         typedef _Pointer pointer;
         typedef _Reference reference;

         /**
          * Iterator class provides access to volume-data elements for traversing a whole volume-data.
          * Iterator class provides full random access functionalities.
          */ 
         class Iterator : public std::iterator<std::random_access_iterator_tag, _ValueType, _DiffType, _Pointer, _Reference>
         {
         public:
            typedef std::random_access_iterator_tag iterator_category;
            typedef _ValueType value_type;
            typedef _DiffType difference_type;
            typedef _DiffType distance_type;	// retained
            typedef _Pointer pointer;
            typedef _Reference reference;

            Iterator() : m_pRange(NULL), m_pData(NULL) {}
            Iterator(pointer pData, const Range* pRange) : m_pRange(pRange), m_pData(pData) {}

            inline reference operator*() const { return (*m_pData); }
            inline pointer operator->() const { return (&**this); }

            inline Iterator& operator++() { ++m_pData; return (*this); } //!< preincrement
            inline Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }	//!< postincrement
            inline Iterator& operator--() { --m_pData; return (*this); } //!<predecrement
            inline Iterator operator--(int) { Iterator tmp = *this; --(*this); return tmp; }//!< postdecrement

            inline Iterator& operator+=(difference_type nOffset) { m_pData += nOffset; return (*this); }
            inline Iterator operator+(difference_type nOffset) const { Iterator tmp = *this; return tmp += nOffset; }
            inline Iterator& operator-=(difference_type nOffset) { m_pData -= nOffset; return (*this); }
            inline Iterator operator-(difference_type nOffset) const { Iterator tmp = *this; return tmp -= nOffset; }
            inline difference_type operator-(const Iterator& rhs) const
            {
               return (rhs.m_pData <= m_pData ? m_pData - rhs.m_pData : -static_cast<difference_type>(rhs.m_pData - m_pData));
            }

            inline reference operator[](difference_type nPos) const { return (*(*this + nPos)); }

            inline bool operator==(const Iterator& rhs) const { return m_pData == rhs.m_pData && m_pRange == rhs.m_pRange; }
            inline bool operator!=(const Iterator& rhs) const { return !(rhs == *this); }
            inline bool operator<(const Iterator& rhs) const { return m_pData < rhs.m_pData && m_pRange == rhs.m_pRange; }
            inline bool operator>(const Iterator& rhs) const { return rhs < *this; }
            inline bool operator<=(const Iterator& rhs) const { return !(rhs < *this); }
            inline bool operator>=(const Iterator& rhs) const { return !(*this < rhs); }

            //! @return The current index vector
            inline Math::Vector3ui indexPosition() const
            {
               Math::Vector3ui size = m_pRange->m_pVolumeRawData->size();
               unsigned int nIndex = index();
               unsigned int nWidth = size(0);
               unsigned int nArea = nWidth * size(1);
               return Math::create<Math::Vector3ui>(nIndex % nWidth, (nIndex % nArea) / nWidth, nIndex / nArea);
            }

            //! @return The current index
            inline unsigned int index() const
            {
               return m_pData - m_pRange->m_pVolumeRawData->data();
            }

         protected:
            const Range* m_pRange;                                //!< pointer to the parent range object
            pointer m_pData;                                      //!< pointer to data of the current volume-data element
         };

         typedef std::reverse_iterator<Iterator> ReverseIterator; //!< @warning ReverseIterator does not provide additional iterator functions (i.e. getIndex()...)

         //! Gets iterator as reference to the first volume-data element
         inline Iterator begin() const
         {
            return Iterator(Detail::VolumeRawDataAccess<_VolumeRawData, boost::is_const<_VolumeRawData>::value>()(m_pVolumeRawData), this);
         }

         //! Gets iterator as reference to the sampling point behind the last volume-data element
         inline Iterator end() const
         {
            return Iterator(Detail::VolumeRawDataAccess<_VolumeRawData, boost::is_const<_VolumeRawData>::value>()(m_pVolumeRawData) + size(), this);
         }

         //! Gets iterator as reference to the first volume-data element for reverse traversing
         inline ReverseIterator rbegin() const
         {
            return (ReverseIterator(end()));
         }
         
         //! Gets iterator as reference to the volume-data element behind the last volume-data element in the range for reverse traversing
         inline ReverseIterator rend() const  
         {
            return (ReverseIterator(begin()));
         }

         //! returns if the range contains sampling points
         inline bool empty() const
         {
            return size() == 0;
         }  

         //! returns the number of sampling points. 
         inline unsigned int size() const
         {
            return Math::multipliedElements(m_pVolumeRawData->size());
         }

         WholeVolumeRange(const std::tr1::shared_ptr<_VolumeRawData>& pVolumeRawData) : m_pVolumeRawData(pVolumeRawData) {}
           
         WholeVolumeRange(const WholeVolumeRange<_VolumeRawData>& rhs) : m_pVolumeRawData(rhs.m_pVolumeRawData) {}
           
      private:
          
         std::tr1::shared_ptr<_VolumeRawData> m_pVolumeRawData;

      };

      
      /** 
       * The class IndexRangePattern provide iterators for traversing over voxels out of a box, which is part of a volume and axis aligned.
       * The iterators (!= end()) points always into the volume. If you want to iterate through the whole volume and you only want to
       * access the voxel data, use WholeVolumeRangePattern instead.
       *
       */

      template<typename _VolumeRawData>
      class IndexRange
      {
      public:

         typedef typename _VolumeRawData::value_type _ValueType;
         typedef typename boost::mpl::if_<boost::is_const<_VolumeRawData>, 
                                          typename _VolumeRawData::ConstPointer,
                                          typename _VolumeRawData::Pointer>::type _Pointer;
         typedef typename boost::mpl::if_<boost::is_const<_VolumeRawData>, 
                                          typename _VolumeRawData::ConstReference,
                                          typename _VolumeRawData::Reference>::type _Reference;

         typedef std::ptrdiff_t _DiffType;

         typedef IndexRange<_VolumeRawData> Range;
         typedef _VolumeRawData VolumeRawData;

         typedef _ValueType value_type;
         typedef std::ptrdiff_t difference_type;
         typedef std::ptrdiff_t distance_type;	// retained
         typedef _Pointer pointer;
         typedef _Reference reference;

         /**
          * Iterator class provides access to volume-data elements for traversing an index-range.
          * Iterator class provides full random access functionalities.
          */
         class Iterator : public std::iterator<std::random_access_iterator_tag, _ValueType, _DiffType, _Pointer, _Reference>
         {
         public:
            typedef std::random_access_iterator_tag iterator_category;
            typedef _ValueType value_type;
            typedef _DiffType difference_type;
            typedef _DiffType distance_type;	// retained
            typedef _Pointer pointer;
            typedef _Reference reference;

            Iterator(): m_pRange(NULL), m_nIndex(0) {}

            Iterator(unsigned int nIndex, const Math::Vector3ui& vecIndexPosition, const Range* pRange)
            :  m_pRange(pRange),
               m_nIndex(nIndex),
               m_vecIndexPosition(vecIndexPosition)
            {
            }

            inline reference operator*() const 
            { 
               return (m_pRange->m_pData)[m_nIndex]; 
            }
            inline pointer operator->() const { return (&**this); }

            //!< preincrement
            inline Iterator& operator++()
            {
               const Math::Vector3ui& vecMax = m_pRange->m_vecMax;
               if (m_vecIndexPosition(0) < vecMax(0))
               {
                  ++m_nIndex;
                  ++m_vecIndexPosition(0);
               }
               else 
               { 
                  const Math::Vector3ui& vecMin = m_pRange->m_vecMin;
                  if (m_vecIndexPosition(1) < vecMax(1))
                  {
                     m_nIndex += m_pRange->m_nDeltaY;
                     unsigned int nMinX = vecMin(0);
                     m_vecIndexPosition(0) = nMinX;
                     ++m_vecIndexPosition(1);
                  }
                  else 
                  {
                     m_nIndex += m_pRange->m_nDeltaZ;
                     unsigned int nMinX = vecMin(0);
                     unsigned int nMinY = vecMin(1);
                     m_vecIndexPosition(0) = nMinX;
                     m_vecIndexPosition(1) = nMinY;
                     ++m_vecIndexPosition(2);
                  }
               }
               return (*this);
            }

            //! postincrement
            inline Iterator operator++(int)
            {
               Iterator tmp = *this;
               ++(*this);
               return tmp;
            }

            //! predecrement
            inline Iterator& operator--()
            {
               if (m_vecIndexPosition(0) > (m_pRange->m_vecMin)(0))
               {
                  --m_nIndex;
                  --m_vecIndexPosition(0);
               }
               else 
               { 
                  if (m_vecIndexPosition(1) > (m_pRange->m_vecMin)(1))
                  {
                     m_nIndex -= m_pRange->m_nDeltaY;
                     m_vecIndexPosition(0) = (m_pRange->m_vecMax)(0);
                     --m_vecIndexPosition(1);
                  }
                  else 
                  {
                     m_nIndex -= m_pRange->m_nDeltaZ;
                     m_vecIndexPosition(0) = (m_pRange->m_vecMax)(0);
                     m_vecIndexPosition(1) = (m_pRange->m_vecMax)(1);
                     --m_vecIndexPosition(2);
                  }
               }
               return (*this);
            }

            //! postdecrement
            inline Iterator operator--(int)
            {
               Iterator tmp = *this;
               --(*this);
               return tmp;
            }

            inline Iterator& operator+=(difference_type nOffset)
            {
               uint nWidthRegion = (m_pRange->m_sizeRegion)(0);
               uint nAreaRegion = nWidthRegion * (m_pRange->m_sizeRegion)(1);
               Math::Vector3ui vecRelativeIndexPosition = relativeIndexPosition();
               size_t nIndex = vecRelativeIndexPosition(2) * nAreaRegion + vecRelativeIndexPosition(1) * nWidthRegion + vecRelativeIndexPosition(0) + nOffset;
               vecRelativeIndexPosition(0) = nIndex % nWidthRegion;
               vecRelativeIndexPosition(1) = (nIndex % nAreaRegion) / nWidthRegion;
               vecRelativeIndexPosition(2) = nIndex / nAreaRegion;
               m_vecIndexPosition = vecRelativeIndexPosition + m_pRange->m_vecMin;
               Math::Vector3ui sizeVolume = m_pRange->m_pVolumeRawData->size();
               m_nIndex = (m_vecIndexPosition(2) * sizeVolume(1) + m_vecIndexPosition(1)) * sizeVolume(0) + m_vecIndexPosition(0);
               return (*this);
            }

            inline Iterator operator+(difference_type nOffset) const
            {
               Iterator tmp = *this;
               return tmp += nOffset;
            }
            inline Iterator& operator-=(difference_type nOffset)
            {
               uint nWidthRegion = (m_pRange->m_sizeRegion)(0);
               uint nAreaRegion = nWidthRegion * (m_pRange->m_sizeRegion)(1);
               Math::Vector3ui vecRelativeIndexPosition = relativeIndexPosition();
               size_t nIndex = vecRelativeIndexPosition(2) * nAreaRegion + vecRelativeIndexPosition(1) * nWidthRegion + vecRelativeIndexPosition(0) - nOffset;
               vecRelativeIndexPosition(0) = nIndex % nWidthRegion;
               vecRelativeIndexPosition(1) = (nIndex % nAreaRegion) / nWidthRegion;
               vecRelativeIndexPosition(2) = nIndex / nAreaRegion;
               m_vecIndexPosition = vecRelativeIndexPosition + m_pRange->m_vecMin;
               Math::Vector3ui sizeVolume = m_pRange->m_pVolumeRawData->size();
               m_nIndex = (m_vecIndexPosition(2) * sizeVolume(1) + m_vecIndexPosition(1)) * sizeVolume(0) + m_vecIndexPosition(0);
               return (*this);
            }
            inline Iterator operator-(difference_type nOffset) const
            {
               Iterator tmp = *this;
               return (tmp -= nOffset);
            }
            inline difference_type operator-(const Iterator& rhs) const
            {
               return (relativeIndex() - rhs.relativeIndex());
            }

            inline reference operator[](difference_type nPos) const
            {
               return (*(*this + nPos));
            }

            inline bool operator==(const Iterator& rhs) const 
            {
               return m_nIndex == rhs.m_nIndex && m_pRange == rhs.m_pRange; 
            }
            inline bool operator!=(const Iterator& rhs) const { return !(rhs == *this); }
            inline bool operator<(const Iterator& rhs) const 
            { 
               return m_nIndex < rhs.m_nIndex && m_pRange == rhs.m_pRange; 
            }
            inline bool operator>(const Iterator& rhs) const { return rhs < *this; }
            inline bool operator<=(const Iterator& rhs) const { return !(rhs < *this); }
            inline bool operator>=(const Iterator& rhs) const { return !(*this < rhs); }

            //! @return The current index vector
            inline const Math::Vector3ui& indexPosition() const
            {
               return m_vecIndexPosition;
            }
            //! @return The current index
            inline unsigned int index() const
            {
               return m_nIndex;  
            }

            //! @return The current relative index vector
            inline Math::Vector3ui relativeIndexPosition() const
            {
               return m_vecIndexPosition - m_pRange->m_vecMin;
            }

            //! @return The current relative index
            inline unsigned int relativeIndex() const
            {
               Math::Vector3ui vec = relativeIndexPosition();
               return (vec(2) * m_pRange->m_sizeRegion(1) + vec(1)) * m_pRange->m_sizeRegion(0) + vec(0);
            }

         protected:
            const Range* m_pRange;                            //!< pointer to the parent range object
            uint m_nIndex;                               //!< current index in the whole volume-data
            Math::Vector3ui m_vecIndexPosition;                      //!< current index vector in the whole volume-data
         };

         //! @warning ReverseIterator does not provide additional iterator functions (i.e. index()...)
         typedef std::reverse_iterator<Iterator> ReverseIterator; 

         //! Gets iterator as reference to the first sampling point
         inline Iterator begin() const 
         {
            return Iterator(m_nIndexBegin, m_vecMin, this); 
         }
         
         //! Gets iterator as reference to the sampling point behind the last sampling point in the range
         inline Iterator end() const
         {
            Math::Vector3ui tmp = m_vecMin;
            tmp(2) += m_sizeRegion(2);
            return Iterator(m_nIndexEnd, tmp, this);
         }

         //! Gets iterator as reference to the first sampling point for reverse traversing
         inline ReverseIterator rbegin() const 
         {
            return ReverseIterator(end());
         }
         
         //! Gets iterator as reference to the sampling point behind the last sampling point in the range for reverse traversing
         inline ReverseIterator rend() const
         {
            return ReverseIterator(begin());
         }

         //! returns if the range contains sampling points
         inline bool empty() const
         {
            return size() == 0;
         }

         //! returns the number of sampling points. 
         inline unsigned int size() const
         {
            return Math::multipliedElements(m_sizeRegion);
         }

         IndexRange(const std::tr1::shared_ptr<_VolumeRawData>& pVolumeRawData, const Math::Vector3ui& vecPosition, const Math::Vector3ui& sizeRegion)
         :  m_pVolumeRawData(pVolumeRawData)
         {
            Math::Vector3ui size = pVolumeRawData->size();
            m_pData = Detail::VolumeRawDataAccess<_VolumeRawData, boost::is_const<_VolumeRawData>::value>()(m_pVolumeRawData);
            
            m_vecMin(0) = Math::clamp<unsigned int>(vecPosition(0), static_cast<unsigned int>(0), size(0));
            m_vecMin(1) = Math::clamp<unsigned int>(vecPosition(1), static_cast<unsigned int>(0), size(1));
            m_vecMin(2) = Math::clamp<unsigned int>(vecPosition(2), static_cast<unsigned int>(0), size(2));
            
            //wl this does not work, this is really wired!!! ???
            // boost is not type safe! todo - find correct work around
            //Math::Vector3ui posOffset = Math::create<Math::Vector3ui>(vecPosition(0) + sizeRegion(0), vecPosition(1) + sizeRegion(1), vecPosition(2) + sizeRegion(2));
            Math::Vector3ui posOffset = Math::create<Math::Vector3ui>(0, 0, 0); 
            //wl try this work around!
            unsigned int nVecPos0 = vecPosition(0);
            unsigned int nVecPos1 = vecPosition(1);
            unsigned int nVecPos2 = vecPosition(2);
            unsigned int nSizeRegion0 = sizeRegion(0);
            unsigned int nSizeRegion1 = sizeRegion(1);
            unsigned int nSizeRegion2 = sizeRegion(2);
            posOffset(0) = nVecPos0 + nSizeRegion0;
            posOffset(1) = nVecPos1 + nSizeRegion1;
            posOffset(2) = nVecPos2 + nSizeRegion2;

            m_vecMax(0) = Math::clamp<unsigned int>(posOffset(0), static_cast<unsigned int>(0), size(0));
            m_vecMax(1) = Math::clamp<unsigned int>(posOffset(1), static_cast<unsigned int>(0), size(1));
            m_vecMax(2) = Math::clamp<unsigned int>(posOffset(2), static_cast<unsigned int>(0), size(2));
            m_sizeRegion(0) = m_vecMax(0) - m_vecMin(0);
            m_sizeRegion(1) = m_vecMax(1) - m_vecMin(1);
            m_sizeRegion(2) = m_vecMax(2) - m_vecMin(2);

            if (Math::multipliedElements(m_sizeRegion) == 0)
            {
               m_sizeRegion = m_vecMin = m_vecMax = Math::create<Math::Vector3ui>(0, 0, 0);
               m_nIndexBegin = m_nIndexEnd = m_nDeltaY = m_nDeltaZ = 0;
            }
            else
            {
               --m_vecMax(0);
               --m_vecMax(1);
               --m_vecMax(2);

               assert(m_vecMin(0) <= m_vecMax(0));
               assert(m_vecMin(1) <= m_vecMax(1));
               assert(m_vecMin(2) <= m_vecMax(2));

               m_nIndexBegin = (m_vecMin(2) * size(1) + m_vecMin(1)) * size(0) + m_vecMin(0);
               m_nIndexEnd = m_nIndexBegin + size(0) * size(1) * m_sizeRegion(2);

               m_nDeltaY = static_cast<int>(size(0)) - m_sizeRegion(0) + 1;
               m_nDeltaZ = (static_cast<int>(size(1)) - m_sizeRegion(1)) * static_cast<int>(size(0)) + static_cast<int>(size(0)) - m_sizeRegion(0) + 1;
            }
         }

         IndexRange(const IndexRange<_VolumeRawData>& other)
         :  m_pVolumeRawData(other.m_pVolumeRawData),
            m_pData(other.m_pData),
            m_vecMin(other.m_vecMin),
            m_vecMax(other.m_vecMax),
            m_sizeRegion(other.m_sizeRegion),
            m_nDeltaY(other.m_nDeltaY),
            m_nDeltaZ(other.m_nDeltaZ),
            m_nIndexBegin(other.m_nIndexBegin),
            m_nIndexEnd(other.m_nIndexEnd)
         {
         }

      protected:
         


         std::tr1::shared_ptr<_VolumeRawData> m_pVolumeRawData;
         pointer m_pData;
         Math::Vector3ui m_vecMin;
         Math::Vector3ui m_vecMax;
         Math::Vector3ui m_sizeRegion;
         unsigned int m_nDeltaY;
         unsigned int m_nDeltaZ;
         unsigned int m_nIndexBegin;
         unsigned int m_nIndexEnd;

      };
      
      
      
      /**
      * This class MillimeterRangePattern provides iterators for traversing over a volume described by a 
      * sampling region in mm-steps with a certain resolution. 
      *
      * @pre sampling-resolution of segments of SamplingRegion has to be constant (ray.getDir().getLength() have to be equal in all segments)
      */
      template<typename _VolumeRawData, typename _SamplingPointContainerIterator>
      class SamplingRange
      {
      public:

         typedef typename _VolumeRawData::value_type _ValueType;
         typedef typename boost::mpl::if_<boost::is_const<_VolumeRawData>, 
            typename _VolumeRawData::ConstPointer,
            typename _VolumeRawData::Pointer>::type _Pointer;
         typedef typename boost::mpl::if_<boost::is_const<_VolumeRawData>, 
            typename _VolumeRawData::ConstReference,
            typename _VolumeRawData::Reference>::type _Reference;

         typedef std::ptrdiff_t _DiffType;

         typedef SamplingRange<_VolumeRawData, _SamplingPointContainerIterator> Range;
         typedef _VolumeRawData VolumeRawData;
         typedef _SamplingPointContainerIterator SamplingPointContainerIterator;

         typedef _ValueType value_type;
         typedef std::ptrdiff_t difference_type;
         typedef std::ptrdiff_t distance_type;	// retained
         typedef _Pointer pointer;
         typedef _Reference reference;
         typedef size_t size_type;

         


         /**
          * Iterator class provides access to volume-data elements for traversing an millimeter-range.
          * Iterator class provides forward-iterator functionalities.
          */
         class Iterator : public std::iterator<std::forward_iterator_tag, _ValueType, _DiffType, _Pointer, _Reference>
         {
         public:
            typedef std::forward_iterator_tag iterator_category;
            typedef _ValueType value_type;
            typedef _DiffType difference_type;
            typedef _DiffType distance_type;	// retained
            typedef _Pointer pointer;
            typedef _Reference reference;


            Iterator() : m_pRange(NULL)
            {
            }

            Iterator(SamplingPointContainerIterator itSampling, const Range* pRange) : 
               m_pRange(pRange),
               m_itSampling(itSampling)
            {
            }

            //! access to volume-data element without checking if current position is located within the volume-data
            inline reference operator*() const
            {
               Math::Vector3i pos = indexPosition();
               if (isOutside(pos))
                  return m_pRange->m_valResultOnOutsideAccess;
               return (m_pRange->m_pData)[this->index(pos)];
            }

            inline pointer operator->() const { return (&**this); }

            //! pre-increment
            inline Iterator& operator++()
            {
               ++m_itSampling;
               return (*this);
            }
            
            //! postincrement
            inline Iterator operator++(int)
            {
               Iterator tmp = *this;
               ++(*this);
               return tmp;
            }

            inline bool operator==(const Iterator& rhs) const 
            {
               return m_itSampling == rhs.m_itSampling && m_pRange == rhs.m_pRange; 
            }

            inline bool operator!=(const Iterator& rhs) const { return !(rhs == *this); }
            
            inline bool operator<(const Iterator& rhs) const 
            { 
               return m_itSampling < rhs.m_itSampling && m_pRange == rhs.m_pRange; 
            }

            inline bool operator>(const Iterator& rhs) const { return rhs < *this; }
            
            inline bool operator<=(const Iterator& rhs) const { return !(rhs < *this); }
            
            inline bool operator>=(const Iterator& rhs) const { return !(*this < rhs); }

            
            //! @return The current index vector
            inline Math::Vector3i indexPosition() const
            {
               Math::Vector3i pos;
               pos(0) = static_cast<int>(floor(((positionMM())(0) / (m_pRange->m_sizeMM)(0)) * (m_pRange->m_size)(0)));
               pos(1) = static_cast<int>(floor(((positionMM())(1) / (m_pRange->m_sizeMM)(1)) * (m_pRange->m_size)(1)));
               pos(2) = static_cast<int>(floor(((positionMM())(2) / (m_pRange->m_sizeMM)(2)) * (m_pRange->m_size)(2)));
               return pos;
            }

            
            inline unsigned int index(const Math::Vector3i& pos) const
            {
               assert(pos(0) >= 0);
               assert(pos(1) >= 0);
               assert(pos(2) >= 0);
               const Math::Vector3ui& size = m_pRange->m_size;
               return static_cast<unsigned int>(pos(0)) + size(0) * (static_cast<unsigned int>(pos(1)) + size(1) * static_cast<unsigned int>(pos(2)));
            }

            
            //! @return The current index
            inline unsigned int index() const
            {
               return index(indexPosition());
            }

            
            //!< @return The current position in mm
            inline const Math::Vector3d& positionMM() const
            {
               return *m_itSampling;
            }

            //! @return True if current position is outside of the volume, else false.
            inline bool isOutside(const Math::Vector3i& pos) const
            {
               if (pos(0) < 0 || pos(0) >= static_cast<int>((m_pRange->m_size)(0)) || 
                   pos(1) < 0 || pos(1) >= static_cast<int>((m_pRange->m_size)(1)) || 
                   pos(2) < 0 || pos(2) >= static_cast<int>((m_pRange->m_size)(2)))
                  return true;

               return false; 
            }

            //! @return True if current position is outside of the volume, else false.
            inline bool isOutside() const
            {
               return isOutside(indexPosition());
            }

         protected:
            const Range* m_pRange;
            SamplingPointContainerIterator m_itSampling;
         };


         //! Gets iterator as reference to the first sampling point
         inline Iterator begin() const
         {
            return Iterator(m_itSamplingBegin, this);
         }

         //! Gets iterator as reference to the sampling point behind the last sampling point in the range
         inline Iterator end() const
         {
            return Iterator(m_itSamplingEnd, this);
         }

         //! returns if the range contains sampling points
         inline bool empty() const
         {
            return m_itSamplingBegin == m_itSamplingEnd;
         }

         //! returns the number of sampling points. @warning expensive!!! (have to iterate through whole range)
         inline size_type size() const {
            
            return std::distance(m_itSamplingBegin, m_itSamplingEnd);
         }

         SamplingRange(const std::tr1::shared_ptr<_VolumeRawData>& pVolumeRawData,
                       const std::tr1::shared_ptr<const Image::Image3DSize>& pVolumeSize,
                       SamplingPointContainerIterator itBegin,
                       SamplingPointContainerIterator itEnd, 
                       const value_type& outsidedValue) :
            m_pData(NULL),
            m_size(Math::create<Math::Vector3ui>(0, 0, 0)),
            m_sizeMM(Math::create<Math::Vector3d>(0, 0, 0)),
            m_itSamplingBegin(itBegin),
            m_itSamplingEnd(itEnd),
            m_valResultOnOutsideAccess(outsidedValue)
         {
            if (pVolumeRawData)
            {
               m_pData = Detail::VolumeRawDataAccess<_VolumeRawData, boost::is_const<_VolumeRawData>::value>()(pVolumeRawData);
               m_size = pVolumeRawData->size();
            }
            if (pVolumeSize)
               m_sizeMM = pVolumeSize->sizeMM();
         }

      private:

         pointer m_pData;
         Math::Vector3ui m_size;
         Math::Vector3d m_sizeMM;
         SamplingPointContainerIterator m_itSamplingBegin;
         SamplingPointContainerIterator m_itSamplingEnd;
         value_type m_valResultOnOutsideAccess;

      };

   }
}

#endif
