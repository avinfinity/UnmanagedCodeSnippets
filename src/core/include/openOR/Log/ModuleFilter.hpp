//****************************************************************************
// (c) 2008 - 2010 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//! OPENOR_INTERFACE_FILE(openOR_core)
//****************************************************************************

#ifndef openOR_Log_ModuleFilter_hpp
#define openOR_Log_ModuleFilter_hpp

#include "../coreDefs.hpp"

#define OPENOR_INTERNAL_PROTECT_MODULE_NAME
#include <openOR/Log/Logger.hpp>
#undef OPENOR_INTERNAL_PROTECT_MODULE_NAME

#include <string>

namespace openOR {
	namespace Log {

		//----------------------------------------------------------------------------
		//! \namespace openOR::Log
		//! \ingroup openOR_core
		//! \brief Attribute to set Module information for a given log message
		//! 
		//! usage is:
		//! \code
		//!   LOG( Level::Info, module("MyModule"), msg("Logging data %1%.") % 42 )
		//! \endcode
		//! \sa ModuleFilter
		//----------------------------------------------------------------------------      
		// This doesn't work, TODO fix, and make it own header like ModuleAttribute.hpp
		//Attributes::create& OPENOR_CORE_API module(const std::string& name);

		///-----> bereits GEFIXED?

		//----------------------------------------------------------------------------
		//! \namespace openOR::Log
		//! \ingroup openOR_core
		//! \brief Filter to selectivly enable log messages based on originating Module
		//! 
		//! usage to Filter out all messages form "NoisyModule":
		//! \code
		//!   Log::ModuleFilter moduleFilter;
		//!      moduleFilter.disableModuleLogging("NoisyModule");
		//!
		//!   Log::Logger::instance().add_filter(moduleFilter.filter());
		//! \endcode
		//! \sa module Attribute
		//----------------------------------------------------------------------------      
		struct OPENOR_CORE_API ModuleFilter {

			ModuleFilter();
			~ModuleFilter();

			Logger::Filter_type filter();

			static bool LogMessage;               // = true;
			static bool DoNotLogMessage;          // = false;

			void setDefaultModuleLogging(bool logMsgByDefault);

			void enableModuleLogging(const std::string& module);
			void disableModuleLogging(const std::string& module);


			bool predicate(const Attributes& attr , size_t level);


		private:

			bool isEnabled(const Attributes& attr , size_t level);
			bool isDisabled(const Attributes& attr , size_t level);

			bool m_logMsgByDefault; 

			typedef std::map<std::string, bool> LoggingStatusMap;
			LoggingStatusMap m_moduleLoggingStatus;
		};


	}
}

#endif // openOR_Log_ModuleFilter_hpp

// !!!!!!! This Block needs to stay outside of the header guards !!!!
#ifdef OPENOR_MODULE_NAME
#   ifdef OPENOR_MODULE
#      undef OPENOR_MODULE
#      undef OPENOR_MODULE_NAME_INTERNAL
#   endif
#   define OPENOR_MODULE_NAME_INTERNAL OPENOR_MODULE_NAME
//#   define OPENOR_MODULE openOR::Log::module(OPENOR_MODULE_NAME_INTERNAL)
#   define OPENOR_MODULE openOR::Log::Attributes::create()("Module", std::string(OPENOR_MODULE_NAME_INTERNAL))
//#   undef OPENOR_MODULE_NAME
#else
#   undef  OPENOR_MODULE
#   define OPENOR_MODULE openOR::Log::noAttribs
#endif






