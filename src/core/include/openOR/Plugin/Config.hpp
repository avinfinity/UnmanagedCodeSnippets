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
/**
 * @file
 * @ingroup openOR_core
 */
#ifndef openOR_Plugin_Config_hpp
#define openOR_Plugin_Config_hpp

#include <openOR/coreDefs.hpp>

#include <vector>
#include <map>
#include <string>

#include <boost/filesystem.hpp>

namespace openOR {
   namespace Plugin {

      //----------------------------------------------------------------------------
      //! \brief Configuration for plugin autoloading.
      //!
      //! This can be used to guide the plugin autoload mechanism to on which
      //! plugin class resides in which library file. Used by Loader::autoload.
      //! \todo This might be a bit brittle. It is tested only in the simplest
      //!       config where all plugins reside in the same directory as the
      //!       executable and the pwd is correctly set.
      //----------------------------------------------------------------------------
      struct OPENOR_CORE_API Config {

         //----------------------------------------------------------------------------
         //! \brief Application specific plugin search path
         //!
         //! A simple collection of paths that will be searched in addition
         //! to the default locations.
         //! You can read the pathes using the const iterators retained from
         //! begin() / end(), and add paths using add()
         //----------------------------------------------------------------------------
         struct OPENOR_CORE_API SearchPath {
            typedef std::vector<boost::filesystem::path>::const_iterator const_iterator;

            const_iterator begin() const;
            const_iterator end() const;

            void add(const boost::filesystem::path& path);

         private:
            std::vector<boost::filesystem::path> m_searchPath;
         };

         //----------------------------------------------------------------------------
         //! \brief Create an empty config object.
         //!
         //! The default configuration is to search the system path and the current pwd
         //----------------------------------------------------------------------------
         Config();

         //----------------------------------------------------------------------------
         //! \brief Read the plugin configuration from a file.
         //!
         //! \todo Think of an infrastructure how this can effectively and
         //!        modularly parse only parts of a config file.
         //----------------------------------------------------------------------------
         void read_from_file(const std::string& cfgFile);

         //----------------------------------------------------------------------------
         //! \brief Save the plugin configuration to a file.
         //!
         //! \todo This should take a file stream, and append to it.
         //----------------------------------------------------------------------------
         void save_to_file(const std::string& cfgFile);

         //----------------------------------------------------------------------------
         //! \brief Returns the full path of a library file if one was configured.
         //!
         //! \return the full path of the library file that contains the requested
         //!   plugin class. If non was registerd for the particular class an empty
         //!   path will be returned.
         //----------------------------------------------------------------------------
         boost::filesystem::path full_library_path(const std::string& pluginName) const;

         //----------------------------------------------------------------------------
         //! \brief Retruns the base name of the library file
         //!
         //! The base name is the filename without platform specific embellishments.
         //! \sa Plugin::Library
         //! \return the base name of the library file that contains the requested
         //!   plugin class. If non was registerd for the particular class the
         //!   class name will be returned.
         //----------------------------------------------------------------------------
         std::string library_base_name(const std::string& pluginName) const;

         //----------------------------------------------------------------------------
         //! \brief Register the connection of a library filename which
         //!   contains a plugin class.
         //!
         //! \param[in] pluginName Must exactly match the registered plugin name.
         //! \param[in] inLibrary Can be either be a library base name or
         //!   a full path.
         //----------------------------------------------------------------------------
         void register_plugin(const std::string& pluginName, const std::string& inLibrary);

         //----------------------------------------------------------------------------
         //! \brief true if the default system library path should be searched.
         //----------------------------------------------------------------------------
         bool search_system_path() const;

         //----------------------------------------------------------------------------
         //! \brief Set wether the system path should be searched. (Default = true)
         //----------------------------------------------------------------------------
         void set_search_system_path(bool use = true);

         //----------------------------------------------------------------------------
         //! \brief Returns the custom search path object.
         //----------------------------------------------------------------------------
         const SearchPath& search_path() const;

         //----------------------------------------------------------------------------
         //! \brief Add a path to the custom search paths.
         //----------------------------------------------------------------------------
         void add_search_path(const boost::filesystem::path& path);

      private:
         typedef std::map<std::string, std::string> PluginLibraryMap;

         PluginLibraryMap  m_pluginLibraryMap;
         SearchPath        m_searchPath;
         bool           m_useSystemPath;
      };

   }
}

#endif //openOR_Plugin_Config_hpp
