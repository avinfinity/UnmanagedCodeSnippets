//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#include <openOR/Image/ROIContainer.hpp>

namespace openOR {

   void ROIContainer::add(const Image::RegionOfInterest& roi) {
      m_rois.push_back(roi);
      if (!m_deferUpdateSignals) { m_signalUpdate(); }
   }

   void ROIContainer::clear() {
      bool b = m_rois.empty();

      m_rois.clear();

      if (!b) {
         if (!m_deferUpdateSignals) { m_signalUpdate(); }
      }
   }

   size_t ROIContainer::size() const {
      return m_rois.size();
   }

   bool ROIContainer::empty() const {
      return (m_rois.size() == 0);
   }

   void ROIContainer::erase(size_t idx) {
      assert(idx < m_rois.size() && "ROIContainer access violation");
      std::vector<openOR::Image::RegionOfInterest>::iterator pos = m_rois.begin();
      std::advance(pos, idx);
      m_rois.erase(pos);
      if (!m_deferUpdateSignals) { m_signalUpdate(); }
   }

   Image::RegionOfInterest ROIContainer::operator()(size_t idx) const {
      assert(idx < m_rois.size() && "ROIContainer access violation");
      return m_rois[idx];
   }

   void ROIContainer::set(size_t idx, const Image::RegionOfInterest& pos) {
      assert(idx < m_rois.size() && "ROIContainer access violation");

      m_rois[idx] = pos;
      if (!m_deferUpdateSignals) { m_signalUpdate(); }
   }

   void ROIContainer::deferUpdateSignals() { m_deferUpdateSignals = true; }
   void ROIContainer::resumeUpdateSignals() {
      m_deferUpdateSignals=false;
      m_signalUpdate();
   }

}
