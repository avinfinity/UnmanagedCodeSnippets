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

#include "openOR/ProgramStateImpl.hpp"

namespace openOR {

   
   ProgramStateImpl::ProgramStateImpl() 
   : m_nState(static_cast<unsigned int>(-1))
   {
   }
         
         
   ProgramStateImpl::~ProgramStateImpl() 
   {
   }
         
   void ProgramStateImpl::setState(unsigned int nState) 
   {
      if (m_nState != nState) { m_nState = nState; m_signalUpdate(); } 
   }
   
   unsigned int ProgramStateImpl::currentState() const 
   { 
      return m_nState; 
   }


}
