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
/**
 * @file
 * @author Weichen Liu
 * @ingroup Basic
 */
#ifndef openOR_Progressable_hpp
#define openOR_Progressable_hpp

#include <openOR/Plugin/CreateInterface.hpp>
#include <string>

namespace openOR {
   
   //----------------------------------------------------------------------------
   //! \brief  Progressable is a Interface for Callable which provide an interface 
   //!         to access the progress of the algorithm.
   //! @ingroup Basic
   //----------------------------------------------------------------------------
   struct Progressable
   {

      //! \brief
      //! Returns the current state of the algorithm. 
      //! @return A value from 0.0 to 1.0 that defines the current progress state of the
      //!         algorithm. 0.0 is for just started and 1.0 is set when the algorithm has
      //!         finished its work.
      virtual double progress() const = 0;

      virtual std::string description() const = 0;

   };
}

OPENOR_CREATE_INTERFACE(openOR::Progressable)
   double progress() const                        { return adaptee()->progress(); }
   std::string description() const                { return adaptee()->description(); }
OPENOR_CREATE_INTERFACE_END

#endif 
