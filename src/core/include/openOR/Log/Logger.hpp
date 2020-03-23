//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//! OPENOR_INTERFACE_FILE(openOR_core)
//****************************************************************************
//! \file
//! \ingroup openOR_core

// remove preextisting definitions to silence benign warnings
// Note: this needs to stay before the header guards.
#if defined (OPENOR_MODULE_NAME) && !defined (OPENOR_INTERNAL_PROTECT_MODULE_NAME)
   #undef OPENOR_MODULE_NAME
#endif

#ifndef openOR_Log_Logger_hpp
#define openOR_Log_Logger_hpp

#include "../coreDefs.hpp"
#include "../Utility/Singleton.hpp"

#include <boost/format.hpp>

#include <boost/tr1/memory.hpp>
#include <boost/any.hpp>

#include <boost/function.hpp>

#include <string>
#include <map>

#define LOG_ENABLED_FOR( SEVERITY, ATTRIB ) (openOR::Log::Detail::MsgDispatch<SEVERITY>::is_enabled(ATTRIB))
//----[ LOG Macro ]-------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------
//! \def LOG
//! \brief Write out a Log message;
//!
//! Use this macro to write out log messages. Provided that you use \c openOR::Log the basic usage is:
//! \code
//!   LOG( Level::Info, noAttribs, msg("Logging data %1%.") % 42 )
//! \endcode
//! the function \c msg is a shorthand for boost::format.
//! \ingroup openOR_core
//----------------------------------------------------------------------------
#define LOG( SEVERITY, ATTRIB, MSG ) {                                                             \
      if (LOG_ENABLED_FOR( SEVERITY, ATTRIB )) {                                                   \
         try {                                                                                     \
            openOR::Log::Detail::MsgDispatch<SEVERITY>::dispatch(ATTRIB, __FILE__,__LINE__, MSG);  \
         } catch (...) {                                                                           \
            openOR::Log::Detail::MsgDispatch<openOR::Log::Level::Error>::dispatch                  \
               (openOR::Log::noAttribs,__FILE__,__LINE__,                                          \
               boost::format("    Error in processing (last?) LOG message."));                     \
         }                                                                                         \
      }                                                                                            \
   }

//------------------------------------------------------------------------------------------------------------------------------

namespace openOR {

   //----------------------------------------------------------------------------
   //! \namespace openOR::Log
   //! \ingroup openOR_core
   //! \brief Logging facility and related stuff
   namespace Log {

      // Fwd Declaration
      namespace Detail { template <class MsgSeverityType> struct MsgDispatch; }

      namespace Level {
         //----[ Severity Level Tags ]-------------------------------------------------------------------
         // This first tag is for use with EnableGroup, you can now use EnableGroup("MyCode", Everything)
         //! \brief Log level tag
         struct OPENOR_CORE_API Everything   { static const char* Severity(); static const size_t Id = 1000000; };

         // These are the tags which can be used to log.
         //! \brief Log level tag
         struct OPENOR_CORE_API Debug        { static const char* Severity(); static const size_t Id = 50; };
         //! \brief Log level tag
         struct OPENOR_CORE_API Info         { static const char* Severity(); static const size_t Id = 40; };
         //! \brief Log level tag
         struct OPENOR_CORE_API Warning      { static const char* Severity(); static const size_t Id = 30; };
         //! \brief Log level tag
         struct OPENOR_CORE_API Error        { static const char* Severity(); static const size_t Id = 20; };
         //! \brief Log level tag
         struct OPENOR_CORE_API Fatal        { static const char* Severity(); static const size_t Id = 10; };
      }

      class OPENOR_CORE_API Attributes {

            typedef std::map<std::string, boost::any> AttributeMap;

         public:

            struct OPENOR_CORE_API create {
               create();
               create& operator()(const std::string& name, boost::any value = boost::any());

            private:
               friend class Attributes;
               std::tr1::shared_ptr<AttributeMap> m_pAttributeMap;
            };

            Attributes();
            Attributes(const create& paramSet);

            const boost::any& get(const std::string& key) const;

         private:
            std::tr1::shared_ptr<AttributeMap>  m_pAttributeMap;
            boost::any                          m_noValue;
      };

      OPENOR_CORE_API extern Attributes noAttribs; //!< default value for no Attributes

      //TODO: struct OPENOR_CORE_API Sink {};

      //! Alias for boost::format, for brevity and clarity of the log messages
      inline boost::format msg(const char* cs) { return boost::format(cs); }
      //! Alias for boost::format, for brevity and clarity of the log messages
      inline boost::format msg(const std::string& s) {return boost::format(s); }

      //----[ Logger ]---------------------------------------------------------------------------------------------
      struct OPENOR_CORE_API Logger : Utility::Singleton<Logger>  {

         typedef boost::function < bool(const Attributes&, size_t) > Filter_type;
         //----[ Functions ]-----------------------------------------------------------------------------

         void add_filter(Filter_type filter);
         void remove_filter(Filter_type filter);

         void outputModuleName(bool doOutput = true);

         // TODO: replace this with a sink system.
         void set_log_file(const std::string& fileName);

         static Logger& instance();
      private:
         friend class Utility::Singleton<Logger>;
         template<class MsgSeverityType> friend struct Detail::MsgDispatch;

         Logger();
         Logger(const Logger&);
         Logger& operator=(const Logger&);
         ~Logger();

         bool is_enabled(const Attributes& attr, size_t severityId) const;
         void log_msg(const char severity[], const Attributes& attr, const char file[], int line, std::string msg);

         typedef std::vector<Filter_type> Filters;

         Filters  m_filters;
         FILE*    m_log;
         bool     m_showModuleName;  // on by default
      };
   
      //----[ Implementation Details ]-------------------------------------------------------------------------------
      namespace Detail {

         // This is a simple forwarder class.
         // All levels are activated by default.
         // Levels which are activated forward calls to the real logging object.
         // This forwarding is static and will be optimized out.
         template < class MsgSeverityType >
         struct MsgDispatch {

            inline static bool is_enabled(const Attributes& attr) {
               return Logger::instance().is_enabled(attr, MsgSeverityType::Id);
            }
            inline static void dispatch(
               const Attributes& attr, const char file[], int line, const boost::format& msg
            ) {
               Logger::instance().log_msg(MsgSeverityType::Severity(), attr, file, line, msg.str());
               return;
            }
         };
         
      } // End NS Detail;
   } // End NS Log
} // End NS openOR


// explicitly deactivate a level with a template specialisation, (this macro)
// which provides noops for all functions.
// They will be optimized away completely.
#define OPENOR_DISABLE_LOG_LEVEL( SEVERITY )                                                    \
namespace openOR { namespace Log { namespace Detail {                                           \
      template <> inline bool openOR::Log::Detail::MsgDispatch<SEVERITY>::is_enabled (          \
         const Attributes& attr                                                                 \
      ) { return false; }                                                                       \
      template <> inline void openOR::Log::Detail::MsgDispatch<SEVERITY>::dispatch (            \
         const Attributes& attr, const char file[], int line, const boost::format& msg          \
      ) { return; }                                                                             \
      } } }


#ifdef NDEBUG
   OPENOR_DISABLE_LOG_LEVEL(openOR::Log::Level::Debug)
#endif

namespace openOR {
   namespace Log {
      namespace Detail {

         struct dbgLogEntryExit {
   
         #ifdef NDEBUG
            inline dbgLogEntryExit(const std::string& blockName) {}
            inline ~dbgLogEntryExit() {}
         #else
            inline dbgLogEntryExit(const std::string& blockName) : 
               m_blockName( blockName )
            {
               LOG(Level::Debug, Attributes::create()("Module", std::string("Core.Tracing.Block")), 
                   msg(">>> Entering %1% ") % m_blockName );
            }
            
            inline ~dbgLogEntryExit() {
               LOG(Level::Debug, Attributes::create()("Module", std::string("Core.Tracing.Block")), 
                   msg("<<< Leaving %1% ") % m_blockName );              
            }
   
            private:
               std::string m_blockName;
         #endif
         };

      } // End NS Detail;
   } // End NS Log
} // End NS openOR

#define OPENOR_TRACE_BLOCK(NAME) openOR::Log::Detail::dbgLogEntryExit dbgLEE(NAME)
#define OPENOR_TRACE_FUNCTION OPENOR_TRACE_BLOCK(OPENOR_CURRENT_FUNCTION)

#endif // openOR_Log_Logger_hpp
