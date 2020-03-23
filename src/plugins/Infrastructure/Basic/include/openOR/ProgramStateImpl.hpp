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
#ifndef openOR_ProgramStateImpl_hpp
#define openOR_ProgramStateImpl_hpp

#include <openOR/Signaller.hpp>
#include <openOR/ProgramState.hpp>
#include <openOR/Plugin/Registration.hpp>
#include <openOR/Defs/Basic.hpp>



namespace openOR {

   class OPENOR_BASIC_API ProgramStateImpl : public ProgramState, public Signaller
   {
      
      public:
         ProgramStateImpl();
         
         ~ProgramStateImpl();

         void setState(unsigned int nState);
         unsigned int currentState() const;

         OPENOR_CONNECT
            OPENOR_CONNECT_TO_UPDATESIGNAL(m_signalUpdate);
         OPENOR_CONNECT_END

      private:
         unsigned int m_nState;  
         OPENOR_UPDATESIGNAL m_signalUpdate;
   };
   
}

OPENOR_REGISTER_INTERFACES((openOR::ProgramStateImpl), 
                           (openOR::UpdateSignaller))


#endif

