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

 *
 * @file
 * @author Christian Winne
 */


#include <openOR/Signaller.hpp>
#include "openOR/UpdateSignalCallable.hpp"

namespace openOR {

         
   void UpdateSignalCallable::operator()() const
   {
      m_signal();
   }
         


}

