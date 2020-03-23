//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//! OPENOR_INTERFACE_FILE(OpenGL_DataInterpreter)
//****************************************************************************
/**
 * @file
 * @author Christian Winne
 */
#ifndef openOR_OpenGL_DataInterpreter_hpp
#define openOR_OpenGL_DataInterpreter_hpp

#include <openOR/OpenGL/gl.hpp>
#include <openOR/Utility/Types.hpp>
#include <openOR/Plugin/CreateInterface.hpp>

namespace openOR {
   namespace OpenGL {
   
      /**
       * \brief Interface for access to image data and for querying the opengl data format and type information.
       * \ingroup OpenGL_DataInterpreter
       */
      struct DataInterpreter
      {
         virtual const void* dataPtr() const = 0;
         virtual void* mutableDataPtr() = 0;

         virtual const unsigned int dataFormat() const = 0;
         virtual void setDataFormat(unsigned int nDataFormat) = 0;
        
         virtual const unsigned int dataType() const = 0;
         virtual void setDataType(unsigned int nDataType) = 0;

      };   

      
      /**
       * \brief getOpenGLDataFormatFromElementType
       * \ingroup OpenGL_DataInterpreter
       */
      template<typename Type>
      inline unsigned int getOpenGLDataFormatFromElementType() { return 0; }


      /**
       * \brief getOpenGLDataTypeFromElementType
       * \ingroup OpenGL_DataInterpreter
       */
      template<typename Type>
      inline unsigned int getOpenGLDataTypeFromElementType() { return 0; }


      template <> inline unsigned int getOpenGLDataFormatFromElementType<uint8>()  { return GL_LUMINANCE; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<uint8>()  { return GL_UNSIGNED_BYTE; }
      template <> inline unsigned int getOpenGLDataFormatFromElementType<int8>()  { return GL_LUMINANCE; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<int8>()  { return GL_BYTE; }
      template <> inline unsigned int getOpenGLDataFormatFromElementType<uint16>()  { return GL_LUMINANCE; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<uint16>()  { return GL_UNSIGNED_SHORT; }
      template <> inline unsigned int getOpenGLDataFormatFromElementType<int16>()  { return GL_LUMINANCE; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<int16>()  { return GL_SHORT; }
      template <> inline unsigned int getOpenGLDataFormatFromElementType<uint32>()  { return GL_LUMINANCE; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<uint32>()  { return GL_UNSIGNED_INT; }
      template <> inline unsigned int getOpenGLDataFormatFromElementType<int32>()  { return GL_LUMINANCE; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<int32>()  { return GL_INT; }
      template <> inline unsigned int getOpenGLDataFormatFromElementType<float>()  { return GL_LUMINANCE; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<float>()  { return GL_FLOAT; }

      template <> inline unsigned int getOpenGLDataFormatFromElementType<UI8Pair>()  { return GL_LUMINANCE_ALPHA; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<UI8Pair>()  { return GL_UNSIGNED_BYTE; }
      template <> inline unsigned int getOpenGLDataFormatFromElementType<I8Pair>()  { return GL_LUMINANCE_ALPHA; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<I8Pair>()  { return GL_BYTE; }
      template <> inline unsigned int getOpenGLDataFormatFromElementType<UI16Pair>()  { return GL_LUMINANCE_ALPHA; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<UI16Pair>()  { return GL_UNSIGNED_SHORT; }
      template <> inline unsigned int getOpenGLDataFormatFromElementType<I16Pair>()  { return GL_LUMINANCE_ALPHA; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<I16Pair>()  { return GL_SHORT; }
      template <> inline unsigned int getOpenGLDataFormatFromElementType<UI32Pair>()  { return GL_LUMINANCE_ALPHA; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<UI32Pair>()  { return GL_UNSIGNED_INT; }
      template <> inline unsigned int getOpenGLDataFormatFromElementType<I32Pair>()  { return GL_LUMINANCE_ALPHA; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<I32Pair>()  { return GL_INT; }
      template <> inline unsigned int getOpenGLDataFormatFromElementType<FloatPair>()  { return GL_LUMINANCE_ALPHA; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<FloatPair>()  { return GL_FLOAT; }

      template <> inline unsigned int getOpenGLDataFormatFromElementType<UI8Triple>()  { return GL_RGB; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<UI8Triple>()  { return GL_UNSIGNED_BYTE; }
      template <> inline unsigned int getOpenGLDataFormatFromElementType<I8Triple>()  { return GL_RGB; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<I8Triple>()  { return GL_BYTE; }
      template <> inline unsigned int getOpenGLDataFormatFromElementType<UI16Triple>()  { return GL_RGB; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<UI16Triple>()  { return GL_UNSIGNED_SHORT; }
      template <> inline unsigned int getOpenGLDataFormatFromElementType<I16Triple>()  { return GL_RGB; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<I16Triple>()  { return GL_SHORT; }
      template <> inline unsigned int getOpenGLDataFormatFromElementType<UI32Triple>()  { return GL_RGB; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<UI32Triple>()  { return GL_UNSIGNED_INT; }
      template <> inline unsigned int getOpenGLDataFormatFromElementType<I32Triple>()  { return GL_RGB; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<I32Triple>()  { return GL_INT; }
      template <> inline unsigned int getOpenGLDataFormatFromElementType<FloatTriple>()  { return GL_RGB; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<FloatTriple>()  { return GL_FLOAT; }

      template <> inline unsigned int getOpenGLDataFormatFromElementType<UI8Quad>()  { return GL_RGBA; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<UI8Quad>()  { return GL_UNSIGNED_BYTE; }
      template <> inline unsigned int getOpenGLDataFormatFromElementType<I8Quad>()  { return GL_RGBA; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<I8Quad>()  { return GL_BYTE; }
      template <> inline unsigned int getOpenGLDataFormatFromElementType<UI16Quad>()  { return GL_RGBA; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<UI16Quad>()  { return GL_UNSIGNED_SHORT; }
      template <> inline unsigned int getOpenGLDataFormatFromElementType<I16Quad>()  { return GL_RGBA; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<I16Quad>()  { return GL_SHORT; }
      template <> inline unsigned int getOpenGLDataFormatFromElementType<UI32Quad>()  { return GL_RGBA; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<UI32Quad>()  { return GL_UNSIGNED_INT; }
      template <> inline unsigned int getOpenGLDataFormatFromElementType<I32Quad>()  { return GL_RGBA; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<I32Quad>()  { return GL_INT; }
      template <> inline unsigned int getOpenGLDataFormatFromElementType<FloatQuad>()  { return GL_RGBA; }
      template <> inline unsigned int getOpenGLDataTypeFromElementType<FloatQuad>()  { return GL_FLOAT; }

   } 
}

OPENOR_CREATE_INTERFACE(openOR::OpenGL::DataInterpreter)

  const void* dataPtr() const { return adaptee()->dataPtr(); }
  void* mutableDataPtr() { return adaptee()->mutableDataPtr(); }

  const unsigned int dataFormat() const { return adaptee()->dataFormat(); }
  void setDataFormat(unsigned int nDataFormat) { return adaptee()->setDataFormat(nDataFormat); }
  const unsigned int dataType() const { return adaptee()->dataType(); }
  void setDataType(unsigned int nDataType) { return adaptee()->setDataType(nDataType); }

OPENOR_CREATE_INTERFACE_END



#endif 
