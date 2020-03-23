//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
#ifndef openOR_DataCollector_hpp
#define openOR_DataCollector_hpp

#include <memory>
#include <vector>

#include <openOR/Callable.hpp> //basic
#include <openOR/DataSettable.hpp> //basic

#include <openOR/Image/DataWrapper.hpp> //Image_Utility
#include <openOR/Plugin/create.hpp> //openOR_core

#include <openOR/Utility/Types.hpp> //openOR_core

#include <openOR/Defs/Image_Utility.hpp>

namespace openOR {
   namespace Image {
      template<typename In>
      class DataCollector : public DataSettable, public Callable {
      public:

         virtual ~DataCollector() {};

         virtual void reset(const bool& resetOut = false) = 0;
         virtual void setData(const In* data, const size_t& size, const std::string& name) {
            std::tr1::shared_ptr<DataWrapper<const In> > pTmpImg = createInstanceOf<DataWrapper<const In>, const In*, size_t>(data, size);
            setData(pTmpImg, name);
         }
         virtual void setData(const AnyPtr& data, const std::string& name) = 0;
         // post: all the input data has been processed and is no longer stored
         virtual void operator()() const = 0;
      };

      template<typename In>
      class DataCollectorMultiplexer : public DataCollector<In> {
      public:
         DataCollectorMultiplexer() {
         }
         virtual ~DataCollectorMultiplexer() {}

         void addCollector(const std::tr1::shared_ptr<DataCollector<In> >& collector) {
            m_vpCollectors.push_back(collector);
         }

         void reset(const bool& resetOut = false) {
            for (int i = 0; i < m_vpCollectors.size(); i++) {
               m_vpCollectors[i]->reset(resetOut);
            }
         }

         void setData(const AnyPtr& data, const std::string& name) {
            for (int i = 0; i < m_vpCollectors.size(); i++) {
               m_vpCollectors[i]->setData(data, name);
            }
         }

         void operator()() const {
            for (int i = 0; i < m_vpCollectors.size(); i++) {
               m_vpCollectors[i]->operator()();
            }
         }

      private:
         std::vector<std::tr1::shared_ptr<DataCollector<In> > > m_vpCollectors;
      };

      typedef DataCollectorMultiplexer<uint16> DataCollectorMultiplexerUI16;
   }
}

#endif //openOR_DataCollector_hpp