//****************************************************************************
// (c) 2008 - 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR commercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#ifndef openOR_Utility_ProgressBase_hpp
#define openOR_Utility_ProgressBase_hpp

#include <openOR/Defs/Basic.hpp>

#include <openOR/Plugin/create.hpp> //openOr_core

#include <openOR/DataSettable.hpp> //basic
#include <openOR/Progressable.hpp> //basic
#include <openOR/Callable.hpp> //basic

#include <boost/tr1/functional.hpp>
#include <boost/tr1/memory.hpp>
#include <boost/exception_ptr.hpp>

namespace openOR {
   namespace Utility {

      //! Base class for anything that wants to indcate progress for any callable action.
      //! Just implement the three abstract outputProgress***() members.
      //! Using this starts the action in another thread, and - if supported by the action -
      //! can give progress information via the Progressable interface.
      //! the outputProgress*** members are always called on the same thread as operator().
      struct OPENOR_BASIC_API ProgressBase : Settable<Callable> {

      private:
         //! Any initialization you want to do before the worker thread starts
         //! \param reportsProgress is true if the action is Progressable
         virtual void outputProgressInit(bool reportsProgress) = 0;

         //! Any cleanup you want to do after the worker thread has finished.
         //! does get called even if the worker throws or is canceled.
         virtual void outputProgressCleanup() = 0;

         //! This is called once after the worker thread was started.
         //! It is allowed to block until finished is set to true.
         virtual void outputProgressWhileWorking(const std::tr1::shared_ptr<Progressable>& pProgress, const bool& finished) = 0;

      public:

         void set(const std::tr1::shared_ptr<openOR::Callable>& algo);
         void operator()();

      protected:

         ProgressBase();
         ~ProgressBase();

         std::tr1::shared_ptr<openOR::Callable>       work();
         std::tr1::shared_ptr<openOR::Progressable>   progressInfo();

      private:

         void doWork();

         std::tr1::shared_ptr<openOR::Callable>       m_pCallable;
         std::tr1::shared_ptr<openOR::Progressable>   m_pProgressable;
         boost::exception_ptr                         m_error;
         bool                                         m_finished;
      };


      //! Adaptes member functions to be called via in contexts where progress is shown.
      struct OPENOR_BASIC_API ProgressableMemFunCaller : Callable, Progressable {

         ProgressableMemFunCaller();
         ~ProgressableMemFunCaller();

         template<typename AClass>
         void setAdaptee(std::tr1::shared_ptr<AClass> pInst, void (AClass::*mfp)() ) {
            m_progress = interface_cast<Progressable>(pInst);
            m_call = std::tr1::bind(mfp, pInst);
         }

         inline double progress() const { return m_progress->progress(); }
         inline std::string description() const { return m_progress->description(); }

         inline void operator()() const { m_call(); }

      private:
         std::tr1::shared_ptr<Progressable> m_progress;
         boost::function<void ()>           m_call;
      };

      template<typename AC>
      std::tr1::shared_ptr<ProgressableMemFunCaller> make_progressable(std::tr1::shared_ptr<AC> pInst, void (AC::*mfp)()) {
         std::tr1::shared_ptr<ProgressableMemFunCaller> pPMFC = createInstanceOf<ProgressableMemFunCaller>();
         pPMFC->setAdaptee(pInst, mfp);
         return pPMFC;
      }

   }
}

#endif
