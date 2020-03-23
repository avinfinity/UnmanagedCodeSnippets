//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR commercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#include "openOR/coreDefs.hpp"
#include "openor_config.h"

namespace openOR {
   const char* Version::number() { return OPENOR_VERSION ; }
   const char* Version::rev_id() { return OPENOR_BUILDID ; }
}
