//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
/**
* @file
* @author Weichen Liu
*/
#ifndef openOR_core_io_BinaryFileStream_hpp
#define openOR_core_io_BinaryFileStream_hpp

#include <fstream>


namespace openOR
{
   using namespace std;

  /**
   * This class should be used to binary write common data types into files and read them from
   * files.  It encapsules a STL-file-stream. 
   */
  class BinaryFileStream : public std::fstream
  {
    public:
      /**
       * Opens a file binary with the name of str and the file options provided by nMode(for example 
       * ios_base::in | ios_base::out for a file where input and output is allowed.
       * @param  str   The file name
       * @param  nMode The Bitmask for the file options.
       */
       BinaryFileStream(const std::string& str, int nMode)
          : fstream(str.c_str(), std::ios::binary | nMode) 
       {
       }

      /**
       * Writes n bytes of binary data from the address p to the file.
       * @param p  Void pointer to the data.
       * @param n  Size of the data package in bytes
       */
       BinaryFileStream& write(void* p, unsigned int n){
          fstream::write(reinterpret_cast<const char*>(p), n); 
          return *this; 
       }

      /**
       * Readss n bytes of binary data from the file to the address p
       * @param p  Void pointer to the destination.
       * @param n  Size of the data package in bytes
       */
       BinaryFileStream& read(void* p, unsigned int n){
          fstream::read(reinterpret_cast<char*>(p), n); 
          return *this; 
       }

      /**
       * This operator writes char-values binary to the file
       * @param stream   The file-stream 
       * @param n        The char to write
       */
       friend BinaryFileStream& operator << (BinaryFileStream& stream, char n){
          stream.write(&n, sizeof(char));
          return stream;
       }
      /**
       * This operator writes unsigned char-values binary to the file
       * @param stream   The file-stream 
       * @param n        The unsigned char to write
       */
       friend BinaryFileStream& operator << (BinaryFileStream& stream, unsigned char n){
          stream.write(&n, sizeof(unsigned char));
          return stream;
       }
      /**
       * This operator writes short-values binary to the file
       * @param stream   The file-stream 
       * @param n        The short to write
       */
       friend BinaryFileStream& operator << (BinaryFileStream& stream, short n){
          stream.write(&n, sizeof(short));
          return stream;
       }
      /**
       * This operator writes short-values binary to the file
       * @param stream   The file-stream 
       * @param n        The short to write
       */
       friend BinaryFileStream& operator << (BinaryFileStream& stream, unsigned short n){
          stream.write(&n, sizeof(unsigned short));
          return stream;
       }
      /**
       * This operator writes int-values binary to the file
       * @param stream   The file-stream 
       * @param n        The int to write
       */
       friend BinaryFileStream& operator << (BinaryFileStream& stream,  int n){
          stream.write(&n, sizeof(int));
          return stream;
       }
      /**
       * This operator writes unsigned int-values binary to the file
       * @param stream   The file-stream 
       * @param n        The unsigned int to write
       */
       friend BinaryFileStream& operator << (BinaryFileStream& stream, unsigned int n){
          stream.write(&n, sizeof(unsigned int));
          return stream;
       }
      /**
       * This operator writes long-values binary to the file
       * @param stream   The file-stream 
       * @param n        The long to write
       */
       friend BinaryFileStream& operator << (BinaryFileStream& stream, long n){
          stream.write(&n, sizeof(long));
          return stream;
       }
      /**
       * This operator writes unsigned long-values binary to the file
       * @param stream   The file-stream 
       * @param n        The unsigned long to write
       */
       friend BinaryFileStream& operator << (BinaryFileStream& stream, unsigned long n){
          stream.write(&n, sizeof(unsigned long));
          return stream;
       }
      /**
       * This operator writes float-values binary to the file
       * @param stream   The file-stream 
       * @param n        The float to write
       */
       friend BinaryFileStream& operator << (BinaryFileStream& stream, float n){
          stream.write(&n, sizeof(float));
          return stream;
       }
      /**
       * This operator writes double-values binary to the file
       * @param stream   The file-stream 
       * @param n        The double to write
       */
       friend BinaryFileStream& operator << (BinaryFileStream& stream, double n){
          stream.write(&n, sizeof(double));
          return stream;
       }


      /**
       * This operator reads char-values binary from a file
       * @param stream   The file-stream 
       * @param n        The address to get the data
       */
       friend BinaryFileStream& operator >> (BinaryFileStream& stream, char& n){
          stream.read(&n, sizeof(char));
          return stream;
       }
      /**
       * This operator reads unsigned char-values binary from a file
       * @param stream   The file-stream 
       * @param n        The address to get the data
       */
       friend BinaryFileStream& operator >> (BinaryFileStream& stream, unsigned char& n){
          stream.read(&n, sizeof(unsigned char));
          return stream;
       }
      /**
       * This operator reads short-values binary from a file
       * @param stream   The file-stream 
       * @param n        The address to get the data
       */
       friend BinaryFileStream& operator >> (BinaryFileStream& stream, short& n){
          stream.read(&n, sizeof(short));
          return stream;
       }
      /**
       * This operator reads unsigned short-values binary from a file
       * @param stream   The file-stream 
       * @param n        The address to get the data
       */
       friend BinaryFileStream& operator >> (BinaryFileStream& stream, unsigned short& n){
          stream.read(&n, sizeof(unsigned short));
          return stream;
       }
      /**
       * This operator reads int-values binary from a file
       * @param stream   The file-stream 
       * @param n        The address to get the data
       */
       friend BinaryFileStream& operator >> (BinaryFileStream& stream, int& n){
          stream.read(&n, sizeof(int));
          return stream;
       }
      /**
       * This operator reads unsigned int-values binary from a file
       * @param stream   The file-stream 
       * @param n        The address to get the data
       */
       friend BinaryFileStream& operator >> (BinaryFileStream& stream, unsigned int& n){
          stream.read(&n, sizeof(unsigned char));
          return stream;
       }
      /**
       * This operator reads long-values binary from a file
       * @param stream   The file-stream 
       * @param n        The address to get the data
       */
       friend BinaryFileStream& operator >> (BinaryFileStream& stream, long n){
          stream.read(&n, sizeof(long));
          return stream;
       }
      /**
       * This operator reads unsigned long-values binary from a file
       * @param stream   The file-stream 
       * @param n        The address to get the data
       */
       friend BinaryFileStream& operator >> (BinaryFileStream& stream, unsigned long n){
          stream.read(&n, sizeof(unsigned long));
          return stream;
       }
      /**
       * This operator reads float-values binary from a file
       * @param stream   The file-stream 
       * @param n        The address to get the data
       */
       friend BinaryFileStream& operator >> (BinaryFileStream& stream, float& n){
          stream.read(&n, sizeof(float));
          return stream;
       }
      /**
       * This operator reads double-values binary from a file
       * @param stream   The file-stream 
       * @param n        The address to get the data
       */
       friend BinaryFileStream& operator >> (BinaryFileStream& stream, double& n){
          stream.read(&n, sizeof(double));
          return stream;
       }
  };


}

#endif