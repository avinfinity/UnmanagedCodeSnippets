//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
#ifndef openOR_RoiContainer_hpp
#define openOR_RoiContainer_hpp

#include <vector>

#include <openOR/Signaller.hpp> //basic
#include <openOR/Plugin/Registration.hpp> //openOR_core
#include <openOR/Image/RegionOfInterest.hpp> //Image_Regions

#include <openOR/Defs/Image_Regions.hpp> //Image_Regions

namespace openOR {
   //TODO: make this part of namespace Image

   struct OPENOR_IMAGE_REGIONS_API ROIContainer : public Signaller {

      virtual void add(const Image::RegionOfInterest& roi);
      virtual void clear();
      virtual size_t size() const;
      virtual bool empty() const;

      // TODO: we should really expose the iterators, it makes erase much easier
      virtual void erase(size_t idx);
      virtual Image::RegionOfInterest operator()(size_t n) const;
      virtual void set(size_t n, const Image::RegionOfInterest& roi);

      virtual void deferUpdateSignals();
      virtual void resumeUpdateSignals();

      OPENOR_CONNECT
         OPENOR_CONNECT_TO_UPDATESIGNAL(m_signalUpdate);
      OPENOR_CONNECT_END

   private:
      std::vector<openOR::Image::RegionOfInterest> m_rois;
      OPENOR_UPDATESIGNAL m_signalUpdate;
      bool m_deferUpdateSignals;
   };
}

OPENOR_REGISTER_INTERFACES((openOR::ROIContainer), (openOR::UpdateSignaller));


#endif