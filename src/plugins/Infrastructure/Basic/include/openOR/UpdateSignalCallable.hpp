//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
/**
 * @file
 * @author Christian Winne
 */
#ifndef openOR_UpdateSignalCallable_hpp
#define openOR_UpdateSignalCallable_hpp

#include <openOR/Defs/Basic.hpp>
#include <openOR/Signaller.hpp>
#include <openOR/Callable.hpp>
#include <openOR/Plugin/Registration.hpp>



namespace openOR {

   /**
    * \brief UpdateSignalCallable emits the update signal if the function 'operator()()' (Callable interface) in called.
    */
   class OPENOR_BASIC_API UpdateSignalCallable : public Signaller, public Callable
   {
      public:
         
         virtual void operator()() const;
         
         OPENOR_CONNECT
            OPENOR_CONNECT_TO_UPDATESIGNAL(m_signal);
         OPENOR_CONNECT_END

      private:

         OPENOR_UPDATESIGNAL m_signal;
   };
   
}

OPENOR_REGISTER_INTERFACES((openOR::UpdateSignalCallable), 
                           (openOR::UpdateSignaller))



#endif
