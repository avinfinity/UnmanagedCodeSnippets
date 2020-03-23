//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
#ifndef openOR_HistogramCollector_hpp
#define openOR_HistogramCollector_hpp

#include "openOR/Image/DataCollector.hpp"

#include <cstring>
#include <cstddef>
#include <climits>
#include <limits>
#include <memory>
#include <queue>
#include <algorithm>

#include <openOR/Image/Image1DData.hpp>
#include <openOR/Image/Image2DData.hpp>
#include <openOR/Image/Image3DData.hpp>
#include <openOR/Image/DataWrapper.hpp>

#include <openOR/Plugin/AnyPtr.hpp>

#include <openOR/Defs/Image_Utility.hpp>

namespace openOR {
   namespace Image {

      template<typename In, size_t min = 0>
      class HistogramCollector : public DataCollector<In> {

      public:

         HistogramCollector():
            m_pUnderflows(new uint64(0)),
            m_pOverflows(new uint64(0)),
            m_pTodo(new std::vector<AnyPtr>())
         {
         }

         virtual ~HistogramCollector() {
         }

         //! Sets the data to be processed. If the data is a Image1DDataUI64 and In is uint64, it
         //! is not obvious whether it is meant as input or output, so an additional classifier
         //! is needed. If name = "in" it is treated as input, in all other cases as output.
         //! If In != uint64, Image1DDataUI64 is treated as output and all other image
         //! types are treated as input.
         //!
         void setData(const AnyPtr& data, const std::string& name = "") {
            std::tr1::shared_ptr<Image1DDataUI64> pImage1DUI64 = interface_cast<Image1DDataUI64>(data);
            if (pImage1DUI64) {
               if (typeid(In) == typeid(uint64) && name == "in") {
                  std::vector<AnyPtr>::iterator it = m_pTodo->begin();
                  m_pTodo->insert(it, data);
               } else {
                  m_pHistogram = pImage1DUI64;
                  m_width = getWidth(m_pHistogram);
               }
               return;
            }

            if (name == "in") {
               std::vector<AnyPtr>::iterator it = m_pTodo->begin();
               m_pTodo->insert(it, data);
            }
         }

         //! Commit operation
         void operator()() const {
            if (!m_pHistogram) {
               // TODO: error
               return;
            }

            while (!m_pTodo->empty()) {
               AnyPtr ptr = m_pTodo->back(); m_pTodo->pop_back();

               const In* data = NULL;
               size_t size = 0;
               //depending on the data type set the corresponding size
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
                  size = static_cast<size_t>(width(pImage3D)) * height(pImage3D) * depth(pImage3D);
               }

               if (std::tr1::shared_ptr<DataWrapper<const In> > pWrapper = interface_cast<DataWrapper<const In> >(ptr)) {
                  data = pWrapper->data();
                  size = pWrapper->size();
               }

               uint64* pDataOut = m_pHistogram->mutableData();

               if (data) {
                  for (size_t i = 0; i < size; i++) {
                     // TODO: possibly implement transformation at this point
                     size_t value = static_cast<size_t>(data[i]);
                     if (value < min) {
                        (*m_pUnderflows)++;
                     } else {
                        value -= min;
                        if (value >= m_width) {
                           (*m_pOverflows)++;
                        } else {
                           pDataOut[value]++;
                        }
                     }
                  }
               }
            }
         }

         void reset(const bool& clearHistogram = false) {
            *m_pUnderflows = 0;
            *m_pOverflows = 0;
            m_pTodo = std::tr1::shared_ptr<std::vector<AnyPtr> >(new std::vector<AnyPtr>());
            if (clearHistogram) m_pHistogram.reset();
         }

      private:

         std::tr1::shared_ptr<uint64> m_pUnderflows, m_pOverflows;
         size_t m_width;
         std::tr1::shared_ptr<Image1DDataUI64> m_pHistogram;
         std::tr1::shared_ptr<std::vector<AnyPtr> > m_pTodo;
      };

      typedef HistogramCollector<uint16> HistogramCollectorUI16;
   }
}

#endif //openOR_HistogramCollector_hpp