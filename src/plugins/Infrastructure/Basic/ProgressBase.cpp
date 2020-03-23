//****************************************************************************
// (c) 2008 - 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR commercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#include <openOR/Utility/ProgressBase.hpp>

#include <openOR/Progressable.hpp>
#include <openOR/Callable.hpp>

#include <boost/timer.hpp>
#include <boost/thread.hpp>
#include <boost/exception_ptr.hpp>
#include <iostream>

#include <openOR/Log/Logger.hpp>
#   define OPENOR_MODULE_NAME "ProgressBase"
#   include <openOR/Log/ModuleFilter.hpp>

namespace openOR {
   namespace Utility {

      ProgressBase::ProgressBase() :
         m_pCallable(),
         m_pProgressable(),
         m_error(boost::exception_ptr()),
         m_finished(false)
      {}
      ProgressBase::~ProgressBase() {}

      void ProgressBase::set(const std::tr1::shared_ptr<openOR::Callable>& algo) {
         if (m_pCallable != algo) {
            m_finished = false;
            m_error = boost::exception_ptr();
         }
         m_pCallable = algo;
         m_pProgressable = interface_cast<Progressable>(algo);
      }

      void ProgressBase::operator()() {
         assert(m_pCallable && "Progress needs work to do.");

         m_finished = false;
         outputProgressInit(m_pProgressable != NULL);

         std::string workName = (m_pProgressable) ? m_pProgressable->description() : std::string("Last action");
         boost::timer stopwatch;
         // start the algorithm in a separate thread
         boost::thread worker = boost::thread(boost::bind(&ProgressBase::doWork, this));

         outputProgressWhileWorking(m_pProgressable, m_finished);

         worker.join();  // At this point we are ready to process the results of the thread and clean up.
         double took = stopwatch.elapsed();

         LOG(Log::Level::Info, OPENOR_MODULE, Log::msg("%1% took %2%s to complete.") % workName % took);

         outputProgressCleanup();

         if (m_error) {
            LOG(Log::Level::Debug, Log::noAttribs, Log::msg("  \\-> Rethrowing in thread %1%.") % boost::this_thread::get_id());
            boost::rethrow_exception(m_error);
         }

         // Do not add any code here!
         return;
      }

      void ProgressBase::doWork() {
         try {
            m_pCallable->operator()();
            m_error = boost::exception_ptr(); // No error occured.
         } catch (...) {
            m_error = boost::current_exception(); // saving exception, to propagate it.
            LOG(Log::Level::Debug, Log::noAttribs, Log::msg("Caught Exception in thread %1%.") % boost::this_thread::get_id());
         }
         m_finished = true; // With or without error, we are finished!

         return;
      }

      std::tr1::shared_ptr<openOR::Callable> ProgressBase::work() { return m_pCallable; }
      std::tr1::shared_ptr<openOR::Progressable> ProgressBase::progressInfo() { return m_pProgressable; }

      ProgressableMemFunCaller::ProgressableMemFunCaller() {}
      ProgressableMemFunCaller::~ProgressableMemFunCaller() {}
   }
}
