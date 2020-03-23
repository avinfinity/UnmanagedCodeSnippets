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
//****************************************************************************
//! \file
//! \ingroup Basic

#ifndef openOR_Cancelable_hpp
#define openOR_Cancelable_hpp

#include <openOR/Plugin/CreateInterface.hpp>

namespace openOR {
   
   //! \brief  Cancelable
   //! \ingroup Basic
   struct Cancelable
   {

      //! \brief
      //! call this method to cancel a cancelable algorithm
      virtual void cancel() = 0;

      //! @brief Indicates if the algorithm has been canceled.
      virtual bool isCanceled() const = 0;
   };
}

OPENOR_CREATE_INTERFACE(openOR::Cancelable)
   void cancel() { adaptee()->cancel(); }
   bool isCanceled() const { return adaptee()->isCanceled(); }
OPENOR_CREATE_INTERFACE_END

#endif 