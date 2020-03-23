//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR commercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//#include <iostream>
#include <cstdio>

#include "../include/openOR/Log/Logger.hpp"

#include <boost/function_equal.hpp>

OPENOR_SINGLETON_INIT(openOR::Log::Logger)

namespace openOR {
   namespace Log {

      //! \brief Log level tags
      namespace Level {
         const char* Everything::Severity() { return ""; }
         const char* Debug::Severity() { return "Debug"; }
         const char* Info::Severity() { return "Info"; }
         const char* Warning::Severity() { return "Warning"; }
         const char* Error::Severity() { return "Error"; }
         const char* Fatal::Severity() { return "Fatal"; }
      }

      Attributes::create::create() : m_pAttributeMap(std::tr1::shared_ptr<AttributeMap> ()) {}
      Attributes::create& Attributes::create::operator()(const std::string& name, boost::any value) {

         if (!m_pAttributeMap) {
            m_pAttributeMap = std::tr1::shared_ptr<AttributeMap>(new AttributeMap);
         }

         (*m_pAttributeMap)[name] = value;
         return *this;
      }

      Attributes::Attributes() :
            m_pAttributeMap(std::tr1::shared_ptr<AttributeMap> ()),
            m_noValue() 
      {}
      Attributes::Attributes(const create& paramSet) :
            m_pAttributeMap(paramSet.m_pAttributeMap),
            m_noValue() 
      {}

      const boost::any& Attributes::get(const std::string& key) const {
         
         if (!m_pAttributeMap) { return m_noValue; }

         AttributeMap::const_iterator attr = m_pAttributeMap->find(key);
         if (attr != m_pAttributeMap->end()) {
            return (attr->second);
         }

         return m_noValue;
      }

      Attributes noAttribs = Attributes();


      Logger::Logger() :
         m_filters(),
         m_log(stderr),
         m_showModuleName(true)
      {}

      Logger::~Logger() {
         fflush(m_log);
         if (!(m_log == NULL || m_log == stdout || m_log == stderr)) {
            fclose(m_log);
         }
      }

      void Logger::add_filter(Filter_type filter) {
         m_filters.push_back(filter);
      }

      void Logger::remove_filter(Filter_type filter) {
         // TODO: can we have a sensible remove filter with this interface?
         //m_filters.erase( std::find(m_filters.begin(), m_filters.end(), filter) );
      }

      void Logger::outputModuleName(bool doOutput /*= true*/) {
         m_showModuleName = doOutput;
      }

      void Logger::set_log_file(const std::string& fileName) {

         fflush(m_log);
         if (!(m_log == NULL || m_log == stdout || m_log == stderr)) {
            fclose(m_log);
         }

         if (fileName.empty() || fileName == "stderr") {
            m_log = stderr;
         } else if (fileName == "stdout") {
            m_log = stdout;
         } else {
            m_log = fopen(fileName.c_str(), "wt");
            if (m_log == NULL) {
               m_log = stderr;
            }
         }
      }

      bool Logger::is_enabled(const Attributes& attr, size_t severityId) const {

         // all messages are logged by default.
         if (m_filters.empty()) {
            return true;
         }

         // if one filter matches the log message gets filtered out
         bool enabled = true;
         for (Filters::const_iterator it = m_filters.begin(), end = m_filters.end();
               it != end; ++it) {
            enabled &= !((*it)(attr, severityId));
         }
         return enabled;
      }

      void Logger::log_msg(const char severity[], const Attributes& attr, const char file[], int line, std::string msg) {

         #ifndef OPENOR_DISABLE_ALL_LOGGING
         if (m_showModuleName) {
            std::string moduleName = "Unknown Module";
            try {
               moduleName = boost::any_cast<std::string>(attr.get("Module"));
            } catch (...) {
               // ignore: its not fatal that we couldn't determine the modules name
            }
            fprintf(m_log, "[%s][%s]: %s\n", severity, moduleName.c_str(), msg.c_str());
         } else {
            fprintf(m_log, "[%s]: %s\n", severity, msg.c_str());
         }

         #ifndef NDEBUG
            fflush(m_log);
         #endif
         #endif
      }
   }
}

