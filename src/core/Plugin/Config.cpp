//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR commercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#include "../include/openOR/Plugin/Config.hpp"

#include "../include/openOR/Log/Logger.hpp"
#   define OPENOR_MODULE_NAME "Core.Plugin.Config"
#   include "../include/openOR/Log/ModuleFilter.hpp"

// #define BOOST_SPIRIT_DEBUG // Debug Cfg-file Parsing rules (verbose!)
#include <boost/spirit/include/classic_core.hpp>

#include <fstream>

namespace openOR {
   namespace Plugin {

      Config::SearchPath::const_iterator Config::SearchPath::begin() const { return m_searchPath.begin(); }
      Config::SearchPath::const_iterator Config::SearchPath::end() const { return m_searchPath.end(); }
      void Config::SearchPath::add(const boost::filesystem::path& path) { m_searchPath.push_back(path); }

      Config::Config() :
            m_pluginLibraryMap(),
            m_searchPath(),
            m_useSystemPath(true) {}

      boost::filesystem::path Config::full_library_path(const std::string& pluginName) const {

         PluginLibraryMap::const_iterator lib = m_pluginLibraryMap.find(pluginName);
         if ((lib != m_pluginLibraryMap.end())
               && (lib->second.find("/") != std::string::npos)
            ) {
            return boost::filesystem::path(lib->second);
         }

         return boost::filesystem::path();
      }

      std::string Config::library_base_name(const std::string& pluginName) const {

         PluginLibraryMap::const_iterator lib = m_pluginLibraryMap.find(pluginName);
         if (lib != m_pluginLibraryMap.end()) {
            // TODO: handle case where full or absolute path was given.
            return lib->second;
         }

         return pluginName;
      }

      void Config::register_plugin(const std::string& pluginName, const std::string& inLibrary) {
         // TODO: error handling
         LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("Registering Plugin '%1%' with Library '%2%'")
             % pluginName
             % inLibrary
            )
         m_pluginLibraryMap[pluginName] = inLibrary;
      }

      bool Config::search_system_path() const {
         return m_useSystemPath;
      }
      void Config::set_search_system_path(bool use) {
         m_useSystemPath = use;
      }

      const Config::SearchPath& Config::search_path() const {
         return m_searchPath;
      }

      void Config::add_search_path(const boost::filesystem::path& path) {
         m_searchPath.add(path);
      }

      namespace Detail {

         // Helper for the ini-file parser.
         struct Context {

            Context(Config& config, const std::string & fileName) :
                  m_config(config),
                  m_File(fileName),
                  m_curSection("invalid"),
                  m_curKey("invalid") {}

            void setSection(const std::string& section) {
               if (section == "SearchPath" || section == "PluginMap") {
                  m_curSection = section;
               } else {
                  LOG(Log::Level::Warning, OPENOR_MODULE,
                      Log::msg("(%1%) Invalid section [%2%], ignoring all entries.")
                      % m_File % section
                     );
               }
            }
            void addKey(const std::string& key) { m_curKey = key;}
            void addValue(const std::string& value) {

               if (m_curSection == "SearchPath") {
                  if (m_curKey == "path") {
                     m_config.add_search_path(boost::filesystem::path(value));
                  } else {
                     LOG(Log::Level::Warning, OPENOR_MODULE,
                         Log::msg("(%1%:[%2%]) Ignoring invalid entry '%3%' = '%4%'.")
                         % m_File % m_curSection
                         % m_curKey % value
                        );
                  }
               } else if (m_curSection == "PluginMap") {
                  m_config.register_plugin(m_curKey, value);
               }
            }

         private:
            Config&     m_config;
            std::string m_File;
            std::string m_curSection;
            std::string m_curKey;
         };

         struct addKey {
            addKey(Context& c) : m_context(c) {}
            void operator()(const char* b, const char* e) const { m_context.addKey(std::string(b, e)); }
         private:
            Context& m_context;
         };

         struct addValue {
            addValue(Context& c) : m_context(c) {}
            void operator()(const char* b, const char* e) const { m_context.addValue(std::string(b, e)); }
         private:
            Context& m_context;
         };

         struct setSection {
            setSection(Context& c) : m_context(c) {}
            void operator()(const char* b, const char* e) const { m_context.setSection(std::string(b, e)); }
         private:
            Context& m_context;
         };
      }


      void Config::read_from_file(const std::string& cfgFileName) {

         using namespace Detail;
         Context context(*this, cfgFileName);

         // Define INI File grammer.
         using namespace BOOST_SPIRIT_CLASSIC_NS;
         rule<> identifier = (alpha_p | ch_p('_')) >> *(alnum_p | ch_p('_') | ch_p(':') | ch_p('<') | ch_p('>'));

         rule<> norm_value = +((alnum_p | punct_p) - (ch_p('"') | ch_p("#") | ch_p(';')));
         rule<> esc_value  = +((alnum_p | punct_p | blank_p) - (ch_p('"') | ch_p("#") | ch_p(';')));
         rule<> value      = norm_value[addValue(context)] | (ch_p('"') >> esc_value[addValue(context)] >> ch_p('"'));

         rule<> comment = (ch_p("#") | ch_p(';')) >> *(print_p - eol_p);
         rule<> section = ch_p('[') >> *space_p >> identifier[setSection(context)] >> *space_p >> ch_p(']');
         rule<> entry = identifier[addKey(context)] >> *(space_p) >> ch_p('=') >> *(space_p) >> value; // >> !( *space_p >> comment ) ;

         rule<> line = *space_p >> !(entry | section | comment) >> eol_p;
         rule<> inifile = *line;

         //// Debug grammer, verbose, make sure to also
         //// comment in BOOST_SPIRIT_DEBUG before the includes.
         //BOOST_SPIRIT_DEBUG_RULE(identifier);
         //BOOST_SPIRIT_DEBUG_RULE(norm_value);
         //BOOST_SPIRIT_DEBUG_RULE(esc_value);
         //BOOST_SPIRIT_DEBUG_RULE(value);
         //BOOST_SPIRIT_DEBUG_RULE(entry);
         //BOOST_SPIRIT_DEBUG_RULE(section);
         //BOOST_SPIRIT_DEBUG_RULE(comment);
         //BOOST_SPIRIT_DEBUG_RULE(line);
         //BOOST_SPIRIT_DEBUG_RULE(inifile);

         // read file from disk
         std::ifstream cfgFile;
         cfgFile.open(cfgFileName.c_str(), std::ios::binary);
         cfgFile.seekg(0, std::ios::end);
         size_t fileSize = (size_t)cfgFile.tellg();
         cfgFile.seekg(0, std::ios::beg);
         std::vector<char> buffer(fileSize + 1);
         cfgFile.read(&buffer[0], buffer.size());
         cfgFile.close();
         buffer[buffer.size()-1] = 0;

         // Do the actual parsing
         parse_info<> res = parse(&buffer[0], inifile);
         if (!res.full) {
            //std::string errorContext = std::string(
            //          res.stop - std::min(15, std::distance(res.stop, &buffer[0])),
            //          res.stop + std::min(15, std::distance(res.stop, (&buffer[0] + buffer.size()) ))
            //       );
            LOG(Log::Level::Warning, OPENOR_MODULE,
                Log::msg("(%1%) !!! Parse error around token '%2%'")
                % cfgFileName % "unknown"
               );
         }

         return;
      }

      void Config::save_to_file(const std::string& cfgFileName) {

         std::ofstream cfgFile;
         cfgFile.open(cfgFileName.c_str());

         cfgFile << "# Open OR plugin loader config file, please DO NOT MODIFY manually\n";
         cfgFile << "# if you do not know EXACTLY what you are doing.\n";
         cfgFile << "[SearchPath]\n";
         for (SearchPath::const_iterator  sp = m_searchPath.begin(),
               end = m_searchPath.end();
               sp != end; ++sp) {
            cfgFile << "path = \"" << *sp << "\"\n";
         }
         cfgFile << "\n";
         cfgFile << "[PluginMap]\n";
         for (PluginLibraryMap::const_iterator  pl = m_pluginLibraryMap.begin(),
               end = m_pluginLibraryMap.end();
               pl != end; ++pl) {
            cfgFile << pl->first << " = \"" << pl->second << "\"\n";
         }
         cfgFile.close();

         return;
      }

   }
}
