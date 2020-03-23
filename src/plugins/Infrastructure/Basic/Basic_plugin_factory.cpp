//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//! OPENOR_FACTORY_FILE(Basic)
//****************************************************************************

#include <openOR/Plugin/Registration.hpp>
#include <openOR/ProgramState.hpp>
#include <openOR/Callable.hpp>
#include <openOR/PropertyAccess.hpp>

#include <openOR/ProgramStateImpl.hpp>
#include <openOR/UpdateSignalCallable.hpp>


/************************************************************************/
/*                                                                      */
/************************************************************************/


OPENOR_COMMENT_PLUGIN_BEGIN(openOR::ProgramStateImpl)
   OPENOR_REGISTER_PLUGIN(openOR::ProgramStateImpl)
   OPENOR_REGISTER_PLUGIN_ALIAS(openOR::ProgramStateImpl, ProgramState)
   OPENOR_COMMENT_PLUGIN_SIGNAL("updated", "void()", "Signal will be emitted if the state has been changed")
OPENOR_COMMENT_PLUGIN_END


/************************************************************************/
/*                                                                      */
/************************************************************************/


OPENOR_COMMENT_PLUGIN_BEGIN(openOR::UpdateSignalCallable)
   OPENOR_REGISTER_PLUGIN(openOR::UpdateSignalCallable)
   OPENOR_REGISTER_PLUGIN_ALIAS(openOR::UpdateSignalCallable, UpdateSignalCallable)
   OPENOR_COMMENT_PLUGIN_SIGNAL("updated", "void()", "Signal will be emitted if the Callable interface function is called")
OPENOR_COMMENT_PLUGIN_END

