//****************************************************************************
// (c) 2008 by the openOR Team
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
 * @author Fabio Fracassi
 * @ingroup Basic
 */
#ifndef openOR_Application_hpp
#define openOR_Application_hpp

#include <openOR/Plugin/CreateInterface.hpp>

namespace openOR {

  //----------------------------------------------------------------------------
  //! \brief Interface for Application Plugins
  //!
  //! Basic Interface for plugins which can parse command line options
  //! \ingroup Basic
  //----------------------------------------------------------------------------
  struct Application {
     virtual ~Application() {}

     virtual void operator()(int argc, char** argv) = 0;
  };

}

OPENOR_CREATE_INTERFACE(openOR::Application)
   void operator()(int argc, char** argv) { adaptee()->operator()(argc, argv); }
OPENOR_CREATE_INTERFACE_END

#endif // openOR_Application_hpp
