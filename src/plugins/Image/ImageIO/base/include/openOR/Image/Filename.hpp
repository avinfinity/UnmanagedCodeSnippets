//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR commercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//! OPENOR_INTERFACE_FILE(Image_ImageIO)
//! \file
//! \ingroup Image_ImageIO
//****************************************************************************

#ifndef openOR_ImageIO_Filename_hpp
#define openOR_ImageIO_Filename_hpp

#include <openOR/Defs/Image_ImageIO.hpp>
#include <string>

namespace openOR {
   namespace Image {

      struct OPENOR_IMAGE_IMAGEIO_API Filename {
         Filename();
         Filename(const std::string& completeFilename);
         Filename(const Filename& f);
         virtual ~Filename();

         void refactor();
         bool empty();

         std::string path;
         std::string name;
         std::string ending;
         std::string complete;
      private:
         void split();
      };
   }
}


#endif