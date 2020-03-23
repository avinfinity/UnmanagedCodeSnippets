//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//! OPENOR_FACTORY_FILE(ZeissBackend)
//****************************************************************************
#include "openOR/ZeissBackend.hpp"
#include <openOR/Plugin/Registration.hpp>


OPENOR_COMMENT_PLUGIN_BEGIN(openOR::ZeissBackend);
   OPENOR_REGISTER_PLUGIN(openOR::ZeissBackend);
   OPENOR_REGISTER_PLUGIN_ALIAS(openOR::ZeissBackend, ZeissBackend);
OPENOR_COMMENT_PLUGIN_END



