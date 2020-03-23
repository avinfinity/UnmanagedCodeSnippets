//****************************************************************************
// (c) 2014 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR commercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
#ifndef openOR_MTScanSurfaceSegmentor_hpp
#define openOR_MTScanSurfaceSegmentor_hpp

#include <openOR/Defs/Zeiss_ZeissBackend.hpp> 
#include <openOR/Image/Image3DData.hpp> //Image_ImageData
#include <openOR/Progressable.hpp> //basic

#include <openOR/Image/RegionOfInterest.hpp> //Image_Regions

namespace openOR {
   struct ROIContainer;

   struct SubVolumePartitioning {

      // create a map
      SubVolumePartitioning() {
         rois.resize(0xffff);
         rois_size = 0;
      }

      size_t start;
      size_t end;
      size_t rois_size;
      std::vector<Image::RegionOfInterest> rois;
   };

   struct OPENOR_ZEISS_ZEISSBACKEND_API MTScanSurfaceSegmentor : Progressable {

      MTScanSurfaceSegmentor();
      ~MTScanSurfaceSegmentor();

      void operator()();

      void setData(const AnyPtr& data, const std::string& tag);
      void setData(size_t objectThreshold);
      void setBorderWidth(unsigned int borderWidth);
      void setMinROIEdgeLength(unsigned int edgeLength);
      void setMinROIVolume(unsigned int volume);
      void setNumberSubvolumes(unsigned int numSubvolumes);
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
      void scanLine(SubVolumePartitioning&);

      // Helper for scanline
      void processVoxel(unsigned int x, unsigned int y, unsigned int z, std::vector<ObjectIdType>& neighborIds, SubVolumePartitioning&);
      bool pixelIsObject(unsigned int x, unsigned int y, unsigned int z);
      // returns number of neighboring voxels which are objects
      size_t neighborIsObject(std::vector<ObjectIdType>& neighborIds, unsigned int x, unsigned int y, unsigned int z, SubVolumePartitioning&);
      ObjectIdType& getObjectId(unsigned int x, unsigned int y, unsigned int z, const SubVolumePartitioning& part);
      void writeOut(ObjectIdType id, unsigned int x, unsigned int y, unsigned int z);
      void growROI(ObjectIdType id, unsigned int x, unsigned int y, unsigned int z, SubVolumePartitioning&);
      ObjectIdType connectROI(ObjectIdType original, ObjectIdType other, SubVolumePartitioning&);
      bool isToSmall(const Image::RegionOfInterest& roi);
      void rewriteRegionId(Image::RegionOfInterest& roi, ObjectIdType newId, SubVolumePartitioning& part);
      bool cannotConnect(const Image::RegionOfInterest& roi, unsigned int x, unsigned int y, unsigned int z, const SubVolumePartitioning&) const;
      void compactRegions(unsigned int x, unsigned int y, unsigned int z, SubVolumePartitioning&);

      void mergeRegions(const std::vector<SubVolumePartitioning>& parts, std::vector<Image::RegionOfInterest>& rois);

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
      unsigned int m_numSubVolumes;

      // Cancel
      bool         m_canceled;

      // Progress
      unsigned int m_prog_currentSlice;
      unsigned int m_prog_numSlices;
      std::string  m_prog_msg;

      // Temporary data
      size_t m_width, m_height, m_depth;
      Image::Image3DDataUI16::value_type* m_pVolumeDataPtr;
      ObjectIdType* m_pVolumeObjectDataPtr;
      ObjectIdType m_zero; // for getObjectId
      size_t m_maxObjectId;
   };
}

#endif
