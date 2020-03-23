//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//! OPENOR_INTERFACE_FILE(Basic)
//! \ingroup Basic
//! \file
//! \author Christian Winne
//****************************************************************************

#ifndef openOR_Signaller_hpp
#define openOR_Signaller_hpp

#include <boost/tr1/memory.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/any.hpp>

#include <openOR/Plugin/CreateInterface.hpp>
#include <openOR/Plugin/interface_cast.hpp>
#include <openOR/Plugin/AnyPtr.hpp>

#include <openOR/Log/Logger.hpp>

namespace openOR {
 
    //----------------------------------------------------------------------------
    //! \brief Interface for Signaller Plugins
    //! \ingroup Basic
    //----------------------------------------------------------------------------
    struct Signaller {

      typedef boost::signals2::connection Connection;

      /**
       * \brief function to connect a slot function to a signal of this Signaller object
       *
       * \warning Do not call the function directly. For slot connection please use the helper 
       * functions openOR::connect(...)
       *
       * \note For implementation of the Signaller interface, the macros OPENOR_CONNECT, 
       * OPENOR_CONNECT_END and OPENOR_CONNECT_TO_SIGNAL should be used!
       * 
       * @param signalname name of the signal the slot should be connected with
       * @param slot function pointer of type: boost::signals2::signal<Signature>::slot_type
       */
      virtual Connection connect(const std::string& signalname, const boost::any& slot) = 0;

    };
    
    
    /** 
     * \brief Signaller of Update signals.
     *
     * This interface models a signaller which only emits update signals.
     * A plugin which fulfills the UpdateSignaller interface also should fulfil the 
     * Signaller interface and support the signal name "updated" with the slot signature <void ()>.
     * 
     * \ingroup Basic
     */
    struct UpdateSignaller
    {
       virtual Signaller::Connection connect(boost::signals2::signal<void()>::slot_type slot) = 0;
    };


    /**
     * \brief helper function for calling Signaller::connect() function
     * \ingroup Basic
     * \param pSignaller object which fulfills the Signaller interface and which emits signals
     * \param signalname identification name of the signal
     * \param slot function to be called if the signal is emitted
     * \return connection object to enable a disconnection if the slot function is not longer valid
     */
    template<class Signature>
    Signaller::Connection connect(std::tr1::shared_ptr<Signaller> pSignaller, const std::string& signalname, typename boost::signals2::signal<Signature>::slot_type slot)
    {
      boost::any any = slot;
      return pSignaller->connect(signalname, any);
    }

    // CW: not used and there is trouble with castings because of direct inheritance
    ///**
    // * \brief helper function for calling Signaller::connect() function
    // * \ingroup Basic
    // * \param sender data which should be castable into a Signaller object
    // * \param signalname identification name of the signal
    // * \param slot function to be called if the signal is emitted
    // * \return connection object to enable a disconnection if the slot function is not longer valid
    // */
    //template<class Signature>
    //Signaller::Connection connect(const AnyPtr& sender, const std::string& signalname, typename boost::signals2::signal<Signature>::slot_type slot)
    //{
    //   std::tr1::shared_ptr<Signaller> pSignaller = interface_cast<Signaller>(sender);
    //   if (!pSignaller) return Signaller::Connection();
    //   
    //   boost::any any = slot;
    //   return pSignaller->connect(signalname, any);
    //}





    struct TriggerableUpdateSignaller : UpdateSignaller
   {
      virtual void trigger() = 0;
   };


 }

/************************************************************************/
/*                                                                      */
/************************************************************************/


OPENOR_CREATE_INTERFACE(openOR::Signaller)
   openOR::Signaller::Connection connect(const std::string& signalname, const boost::any& slot) { return adaptee()->connect(signalname, slot); }
OPENOR_CREATE_INTERFACE_END


OPENOR_CREATE_INTERFACE(openOR::UpdateSignaller)
   openOR::Signaller::Connection connect(boost::signals2::signal<void()>::slot_type slot) { boost::any s = slot; return adaptee()->connect("updated", s); }
OPENOR_CREATE_INTERFACE_END


OPENOR_CREATE_INTERFACE(openOR::TriggerableUpdateSignaller)
   openOR::Signaller::Connection connect(boost::signals2::signal<void()>::slot_type slot) { boost::any s = slot; return adaptee()->connect("updated", s); }
   void trigger() { adaptee()->trigger(); }
OPENOR_CREATE_INTERFACE_END


/************************************************************************/
/*                                                                      */
/************************************************************************/


#define OPENOR_CONNECT_DECLARATION virtual openOR::Signaller::Connection connect(const std::string& signalname, const boost::any& slot)

#define OPENOR_CONNECT_IMPLEMENTATION(CLASS_NAME) \
   openOR::Signaller::Connection CLASS_NAME::connect(const std::string& signalname, const boost::any& slot) \
   { \
      openOR::Signaller::Connection connection; \


/**
 * \brief macro to simplify the implementation of the Signaller interface
 * \ingroup Basic
 */
#define OPENOR_CONNECT \
  virtual openOR::Signaller::Connection connect(const std::string& signalname, const boost::any& slot) \
  { \
    openOR::Signaller::Connection connection; \
 
/**
 * \brief macro to simplify the implementation of the Signaller interface
 * \ingroup Basic
 */
#define OPENOR_CONNECT_END \
    LOG(Log::Level::Warning, Log::noAttribs, Log::msg("Signal not found.")); \
    return connection; \
  } \

/**
 * \brief macro to simplify the implementation of the Signaller interface
 * \ingroup Basic
 */
#define OPENOR_CONNECT_TO_SIGNAL(SIGNAL_NAME, SLOT_SIGNATURE, SIGNAL_VAR) \
  if (signalname == SIGNAL_NAME) \
  { \
    try { \
      boost::signals2::signal<SLOT_SIGNATURE>::slot_type s = boost::any_cast<boost::signals2::signal<SLOT_SIGNATURE>::slot_type>(slot); \
      connection = SIGNAL_VAR.connect(s); \
      LOG(Log::Level::Debug, Log::Attributes::create()("Module", std::string("Core.Signals")), Log::msg("Signal connected.")); \
      return connection; \
    } \
    catch(...) \
    { \
      /* Log error*/ \
    } \
  } \


#define OPENOR_CONNECT_TO_UPDATESIGNAL(SIGNAL_VAR) OPENOR_CONNECT_TO_SIGNAL("updated", void(), SIGNAL_VAR)


#define OPENOR_SIGNAL(SLOT_SIGNATURE) boost::signals2::signal<SLOT_SIGNATURE>
typedef boost::signals2::signal<void()> OPENOR_UPDATESIGNAL;


#endif 
