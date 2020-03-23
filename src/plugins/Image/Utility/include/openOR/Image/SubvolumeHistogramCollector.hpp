//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#ifndef openOR_SubvolumeHistogramCollector_hpp
#define openOR_SubvolumeHistogramCollector_hpp

#include "openOR/Image/DataCollector.hpp" //image_Utility

#include <cstring>
#include <cstddef>
#include <climits>
#include <limits>
#include <memory>
#include <queue>
#include <algorithm>

#include <openOR/Image/Image1DData.hpp> //Image_ImageData
#include <openOR/Image/Image2DData.hpp>//Image_ImageData
#include <openOR/Image/Image3DData.hpp> //Image_ImageData
#include <openOR/Image/DataWrapper.hpp> //Image_Utility

#include <openOR/Plugin/AnyPtr.hpp> //openOR_core

#include <openOR/Defs/Image_Utility.hpp> 

namespace openOR {
   namespace Image {

	  typedef Image3DData<std::tr1::shared_ptr<Image1DDataUI64> > SubvolumeHistogram;

	  template<typename In, size_t min = 0>
	  class SubvolumeHistogramCollector : public DataCollector<In> {

	  public:

		 SubvolumeHistogramCollector():
			m_pUnderflows(new uint64(0)),
			m_pOverflows(new uint64(0)),
			m_pTodo(new std::vector<AnyPtr>()),
			m_pCurrentPos(new Math::Vector3ui())
		 {
			(*m_pCurrentPos)(0) = 0;
			(*m_pCurrentPos)(1) = 0;
			(*m_pCurrentPos)(2) = 0;
		 }

		 virtual ~SubvolumeHistogramCollector() {
		 }

		 //! Sets the data to be processed or the histogram holder.
		 //!
		 void setData(const AnyPtr& data, const std::string& name = "") {
			std::tr1::shared_ptr<SubvolumeHistogram> pHistograms = interface_cast<SubvolumeHistogram>(data);
			if (pHistograms) {
			   m_pHistograms = pHistograms;
			}

			std::tr1::shared_ptr<Image3DSize> pInSize = interface_cast<Image3DSize>(data);
			if (pInSize && name == "inSize") {
			   m_pInSize = pInSize;
			}

			if (name == "in") {
			   std::vector<AnyPtr>::iterator it = m_pTodo->begin();
			   m_pTodo->insert(it, data);
			}
		 }

		 //! Commit operation
		 void operator()() const {
			if (!m_pHistograms || !m_pInSize) {
			   // TODO: error
			   return;
			}

			size_t width = m_pInSize->size()(0);
			size_t height = m_pInSize->size()(1);
			size_t depth = m_pInSize->size()(2);

			double fHWidth = m_pHistograms->size()(0);
			double fHHeight = m_pHistograms->size()(1);
			double fHDepth = m_pHistograms->size()(2);

			size_t hWidth = m_pHistograms->size()(0);
			size_t hHeight = m_pHistograms->size()(1);
			size_t hDepth = m_pHistograms->size()(2);

			double fVWidth = width / (fHWidth + 1);
			double fVHeight = height / (fHHeight + 1);
			double fVDepth = depth / (fHDepth + 1);

			while (!m_pTodo->empty()) {
			   AnyPtr ptr = m_pTodo->back(); m_pTodo->pop_back();

			   //depending on the data type set the corresponding size
			   const In* data = NULL;
			   size_t size = 0;
			   
			   if (std::tr1::shared_ptr<Image1DData<In> > pImage1D = interface_cast<Image1DData<In> >(ptr)) {
				  data = pImage1D->data();
				  size = static_cast<size_t>(getWidth(pImage1D));
			   }

			   if (std::tr1::shared_ptr<Image2DData<In> > pImage2D = interface_cast<Image2DData<In> >(ptr)) {
				  data = pImage2D->data();
				  size = static_cast<size_t>(getWidth(pImage2D)) * getHeight(pImage2D);
			   }

			   if (std::tr1::shared_ptr<Image3DData<In> > pImage3D = interface_cast<Image3DData<In> >(ptr)) {
				  data = pImage3D->data();
				  size = static_cast<size_t>(openOR::width(pImage3D)) * openOR::height(pImage3D) * openOR::depth(pImage3D);
			   }

			   if (std::tr1::shared_ptr<DataWrapper<const In> > pWrapper = interface_cast<DataWrapper<const In> >(ptr)) {
				  data = pWrapper->data();
				  size = pWrapper->size();
			   }

			   if (data) {
				  for (size_t i = 0; i < size; i++) {
					 if ((*m_pCurrentPos)(2) == depth && i < size) {
						LOG(Log::Level::Warning, OPENOR_MODULE,
						   Log::msg("[ZeissRawExtendedExporter] Got too much data - igoring the rest")
						);

						return;
					 }

					 size_t x0 = static_cast<size_t>(std::floor((*m_pCurrentPos)(0) / fVWidth));
					 size_t y0 = static_cast<size_t>(std::floor((*m_pCurrentPos)(1) / fVHeight));
					 size_t z0 = static_cast<size_t>(std::floor((*m_pCurrentPos)(2) / fVDepth));

					 size_t x1 = x0 + 1;
					 size_t y1 = y0 + 1;
					 size_t z1 = z0 + 1;

					 if (x0 > 0) x0--;
					 if (y0 > 0) y0--;
					 if (z0 > 0) z0--;

					 if (x1 > hWidth) x1 = hWidth;
					 if (y1 > hWidth) y1 = hHeight;
					 if (z1 > hWidth) z1 = hDepth;

					 size_t value = static_cast<size_t>(data[i]);
					 if (value < min) {
						(*m_pUnderflows)++;
					 } else {
						value -= min;
					 }

					 for (size_t z = z0; z < z1; z++) {
						for (size_t y = y0; y < y1; y++) {
						   for (size_t x = x0; x < x1; x++) {
							  size_t index = x + y * hWidth + z * hHeight * hWidth;
							  std::shared_ptr<Image1DDataUI64> pHistogram = m_pHistograms->data()[index];

							  if (value < pHistogram->size()) pHistogram->mutableData()[value]++;
							  else (*m_pOverflows)++;
						   }
						}
					 }

					 size_t currX = ++((*m_pCurrentPos)(0));
					 if (currX == width) {
						(*m_pCurrentPos)(0) = 0;
						size_t currY = ++((*m_pCurrentPos)(1));
						if (currY == height) {
						   (*m_pCurrentPos)(1) = 0;
						   ++((*m_pCurrentPos)(2));
						}
					 }
				  }
			   }
			}
		 }

		 void reset(const bool& clearHistogram = false) {
			*m_pUnderflows = 0;
			*m_pOverflows = 0;
			(*m_pCurrentPos)(0) = 0;
			(*m_pCurrentPos)(1) = 0;
			(*m_pCurrentPos)(2) = 0;
			m_pTodo = std::tr1::shared_ptr<std::vector<AnyPtr> >(new std::vector<AnyPtr>());
			m_pInSize.reset();

			if (clearHistogram) {
			   m_pHistograms.reset();
			}
		 }

	  private:

		 std::tr1::shared_ptr<uint64> m_pUnderflows, m_pOverflows;
		 std::tr1::shared_ptr<std::vector<AnyPtr> > m_pTodo;
		 std::tr1::shared_ptr<SubvolumeHistogram> m_pHistograms;
		 std::tr1::shared_ptr<Math::Vector3ui> m_pCurrentPos;
		 std::tr1::shared_ptr<Image3DSize> m_pInSize;
	  };

	  typedef SubvolumeHistogramCollector<uint16> SubvolumeHistogramCollectorUI16;
   }
}

#endif //openOR_SubvolumeHistogramCollector_hpp