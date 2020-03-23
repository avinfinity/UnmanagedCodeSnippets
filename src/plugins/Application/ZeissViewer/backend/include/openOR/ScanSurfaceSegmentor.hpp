//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
#ifndef openOR_ScanSurfaceSegmentor_hpp
#define openOR_ScanSurfaceSegmentor_hpp

#include <openOR/Defs/Zeiss_ZeissBackend.hpp>
#include <openOR/Image/Image3DData.hpp> //Image_ImagData
#include <openOR/Progressable.hpp> //basic

#include <vector>

namespace openOR {
   struct ROIContainer;
   namespace Image { struct RegionOfInterest; }

   struct OPENOR_ZEISS_ZEISSBACKEND_API ScanSurfaceSegmentor : Progressable {

      ScanSurfaceSegmentor();
      ~ScanSurfaceSegmentor();

      void operator()();

      void setData(const AnyPtr& data, const std::string& tag);
      void setData(size_t objectThreshold);
      void setBorderWidth(unsigned int borderWidth);
      void setMinROIEdgeLength(unsigned int edgeLength);
      void setMinROIVolume(unsigned int volume);

      // cancelable interface
      void cancel();
      bool isCanceled() const;

      // progressable interface
      double progress() const;
      std::string description() const;

   private:
      typedef Image::Image3DDataUI16      ObjectMapType;
      typedef ObjectMapType::value_type   ObjectIdType;

      void createBinaryGraphData();
      void scanLine();

      // Helper for scanline
      void processVoxel(unsigned int x, unsigned int y, unsigned int z);
      bool pixelIsObject(unsigned int x, unsigned int y, unsigned int z);
      std::vector<ObjectIdType> neighborIsObject(unsigned int x, unsigned int y, unsigned int z);
      ObjectIdType getObjectId(unsigned int x, unsigned int y, unsigned int z);
      void writeOut(ObjectIdType id, unsigned int x, unsigned int y, unsigned int z);
      void growROI(ObjectIdType id, unsigned int x, unsigned int y, unsigned int z);
      ObjectIdType connectROI(ObjectIdType original, ObjectIdType other);
      bool isToSmall(const Image::RegionOfInterest& roi);
      void rewriteRegionId(Image::RegionOfInterest& roi, ObjectIdType newId);
      bool cannotConnect(const Image::RegionOfInterest& roi, unsigned int x, unsigned int y, unsigned int z);
      void compactRegions(unsigned int x = 0, unsigned int y = 0, unsigned int z = 0);

      //Debug for issue #498
      //void liveIDs(unsigned int x=0, unsigned int y=0, unsigned int z=0);

      //Data
      std::tr1::shared_ptr<Image::Image3DDataUI16> m_pVolumeData;       // Input
      uint16 m_objectThreshold;

      std::tr1::shared_ptr<ROIContainer> m_pObjectBoundingBoxes;        // Output 1
      std::tr1::shared_ptr<ObjectMapType> m_pVolumeObjectMap;           // Output 2 - TODO: Image3d char

      // Derived data
      Math::Vector3d m_voxelInMMSize;
      bool           m_useMMParams;

      // Parameter
      unsigned int m_borderWidth;
      unsigned int m_minROIEdgeLength;
      unsigned int m_minROIVolume;

      // Cancel
      bool         m_canceled;

      // Progress
      unsigned int m_prog_currentSlice;
      unsigned int m_prog_numSlices;
      std::string  m_prog_msg;
   };
}

#endif
