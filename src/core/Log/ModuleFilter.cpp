//****************************************************************************
// (c) 2008 - 2010 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#include "../include/openOR/Log/ModuleFilter.hpp"

#include <boost/bind.hpp>
#include <boost/any.hpp>

namespace openOR {
	namespace Log {

		//Attributes::create& module(const std::string& name) {
		//   return Attributes::create()("Module", std::string(name)); // the explicit copy is intended!
		//}


		bool ModuleFilter::LogMessage = true;
		bool ModuleFilter::DoNotLogMessage = false;

		ModuleFilter::ModuleFilter() :
		m_logMsgByDefault(LogMessage),
			m_moduleLoggingStatus()
		{
			// Disable these by Default
			m_moduleLoggingStatus["Core.Tracing.Plugin"] = DoNotLogMessage;
			m_moduleLoggingStatus["Core.Tracing.Memory"] = DoNotLogMessage;
			m_moduleLoggingStatus["Core.Tracing.Block"] = DoNotLogMessage;

			// TODO: Support for Module Groups
		}

		ModuleFilter::~ModuleFilter() {}

		void ModuleFilter::setDefaultModuleLogging(bool logMsgByDefault) {
			m_logMsgByDefault = logMsgByDefault;
		}

		void ModuleFilter::enableModuleLogging(const std::string& module) {
			m_moduleLoggingStatus[module] = LogMessage;
		}

		void ModuleFilter::disableModuleLogging(const std::string& module) {
			m_moduleLoggingStatus[module] = DoNotLogMessage;

		}

		Logger::Filter_type ModuleFilter::filter() {
			// Todo lifetime Management??
			return boost::bind(&ModuleFilter::predicate, this, _1, _2 );
		}


		bool ModuleFilter::predicate(const Attributes& attr , size_t level) {

			return !(  (m_logMsgByDefault && !isDisabled(attr, level)) 
				|| (!m_logMsgByDefault && isEnabled(attr, level))
				);
		}


		bool ModuleFilter::isEnabled(const Attributes& attr , size_t level) {

			boost::any modName = attr.get("Module");
			if (modName.empty()) { return true; }     // If the message has no Module
			// attribute set it will be logged

			LoggingStatusMap::const_iterator module = 
				m_moduleLoggingStatus.find( 
				boost::any_cast<std::string>(modName) 
				);
			if ( module != m_moduleLoggingStatus.end() ) {
				return module->second;
			} else {
				return false;
			}
		}

		bool ModuleFilter::isDisabled(const Attributes& attr , size_t level) {

			boost::any modName = attr.get("Module");
			if (modName.empty()) { return false; }    // If the message has no Module
			// attribute set it will be logged
			LoggingStatusMap::const_iterator module = 
				m_moduleLoggingStatus.find( 
				boost::any_cast<std::string>(modName) 
				);
			if ( module != m_moduleLoggingStatus.end() ) {
				return !module->second;
			} else {
				return false;
			}
		}


	}
}