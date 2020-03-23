//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR commercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#include <openOR/Defs/Image_ImageIO.hpp>
#include <openOR/Image/Filename.hpp>
#include <string>
#include <sstream>

namespace openOR {
   namespace Image {

      Filename::Filename() {

      }
      Filename::Filename(const std::string& completeFilename) {
         complete = completeFilename;
         split();
      }
      Filename::Filename(const Filename& f) {
         complete = f.complete;
         path = f.path;
         name = f.name;
         ending = f.ending;
      }

      Filename::~Filename() {

      }

      void Filename::refactor() {
         std::stringstream ss;
         if (!path.empty()) { ss << path << "/"; }
         ss << name;
         if (!ending.empty()) { ss << "." << ending; }
         complete = ss.str();
      }

      bool Filename::empty() {
         return Filename::name.empty();
      }

      void Filename::split() {
         size_t posPath = complete.find_last_of("/\\");
         std::string filename;
         path = (posPath != std::string::npos) ? complete.substr(0, posPath) : "";
         filename = (posPath != std::string::npos) ? complete.substr(posPath + 1) : complete;

         size_t posEnd = filename.find_last_of(".");
         name = (posEnd != std::string::npos) ? filename.substr(0, posEnd) : filename;
         ending = (posEnd != std::string::npos) ? filename.substr(posEnd + 1) : "";
      }
   }
}
