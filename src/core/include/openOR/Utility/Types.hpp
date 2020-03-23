//****************************************************************************
// (c) 2007 by the openOR Team
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
 * @author Christian Winne
 * @ingroup openOR_core
 */
#ifndef  openOR_Utility_Types_hpp
#define  openOR_Utility_Types_hpp

#include <ostream>
#include <istream>

#include <boost/cstdint.hpp>

namespace openOR {
   
   struct invalid_type {};

   // we should get rid of those, there are perfectly 
   // acceptable standard or nearly standard types for 
   // this purposes, so there is no need for us to define
   // our own standards.
   typedef unsigned char uchar;
   typedef unsigned short ushort;
   typedef unsigned int uint;
   
   typedef boost::int8_t int8;
   typedef boost::int16_t int16;
   typedef boost::int32_t int32;
   typedef boost::int64_t int64;  
   
   typedef boost::uint8_t uint8;
   typedef boost::uint16_t uint16;
   typedef boost::uint32_t uint32;
   typedef boost::uint64_t uint64;
   
   
   
   // I don't think it is a good idea recreate standard facilities with subtly
   // different semantics.
   template<typename Type>
   struct Pair {
      typedef Type value_type;
      
      Pair() {}
      Pair(const Type& t0, const Type& t1) : first(t0), second(t1) {}
      Type& operator[](uint8 nIndex) { return data[nIndex]; }
      const Type& operator[](uint nIndex) const { return data[nIndex]; }
      

      // Unions can not be leagally used for aliasing purposes,
      // although it will probably work on most compilers.
      // TODO: investiagate a better way of doing this.
      union {
         Type data[2];
         struct {
            Type first;
            Type second;
         };
      };
   };
   
   template <typename Type>
   std::ostream& operator<< (std::ostream& out, const Pair<Type>& rhs) {
      out << rhs.first << " " << rhs.second;
      return out;
   }
   
   template <class Type>
   std::istream& operator>> (std::istream& in, const Pair<Type>& rhs) {
      in >> rhs.first >> rhs.second;
      return in;
   }
   
   
   template<typename Type>
   struct Triple {
      typedef Type value_type;
      
      Triple() {}
      Triple(const Type& t0, const Type& t1, const Type& t2) : first(t0), second(t1), third(t2) {}
      Type& operator[](uint8 nIndex) { return data[nIndex]; }
      const Type& operator[](uint nIndex) const { return data[nIndex]; }
      
      // Unions can not be legally used for aliasing purposes,
      // although it will probably work on most compilers.
      // TODO: investigate a better way of doing this.
      union {
         Type data[3];
         struct {
            Type first;
            Type second;
            Type third;
         };
      };
   };
   
   template <typename Type>
   std::ostream& operator<< (std::ostream& out, const Triple<Type>& rhs)  {
      out << rhs.first << " " << rhs.second << " " << rhs.third;
      return out;
   }
   
 
   template <class Type>
   std::istream& operator>> (std::istream& in, const Triple<Type>& rhs) {
      in >> rhs.first >> rhs.second >> rhs.third;
      return in;
   }
   
   
   template<typename Type>
   struct Quad {
      typedef Type value_type;
      
      Quad() {}
      Quad(const Type& t0, const Type& t1, const Type& t2, const Type& t3) : first(t0), second(t1), third(t2), fourth(t3) {}
      Type& operator[](uint8 nIndex) { return data[nIndex]; }
      const Type& operator[](uint nIndex) const { return data[nIndex]; }
      
      // Unions can not be leagally used for aliasing purposes,
      // although it will probably work on most compilers.
      // TODO: investiagate a better way of doing this.
      union {
         Type data[4];
         struct {
            Type first;
            Type second;
            Type third;
            Type fourth;
         };
      };
   };
   
   
   template <typename Type>
   std::ostream& operator<< (std::ostream& out, const Quad<Type>& rhs) {
      out << rhs.first << " " << rhs.second << " " << rhs.third << " " << rhs.fourth;
      return out;
   }
   
   template <class Type>
   std::istream& operator>> (std::istream& in, const Quad<Type>& rhs) {
      in >> rhs.first >> rhs.second >> rhs.third >> rhs.fourth;
      return in;
   }
   
   
   typedef Pair<uint8> UI8Pair;
   typedef Pair<int8> I8Pair;
   typedef Pair<uint16> UI16Pair;
   typedef Pair<int16> I16Pair;
   typedef Pair<uint32> UI32Pair;
   typedef Pair<int32> I32Pair;
   typedef Pair<float> FloatPair;
   typedef Pair<double> DoublePair;
   
   typedef Triple<uint8> UI8Triple;
   typedef Triple<int8> I8Triple;
   typedef Triple<uint16> UI16Triple;
   typedef Triple<int16> I16Triple;
   typedef Triple<uint32> UI32Triple;
   typedef Triple<int32> I32Triple;
   typedef Triple<float> FloatTriple;
   typedef Triple<double> DoubleTriple;
   
   typedef Quad<uint8> UI8Quad;
   typedef Quad<int8> I8Quad;
   typedef Quad<uint16> UI16Quad;
   typedef Quad<int16> I16Quad;
   typedef Quad<uint32> UI32Quad;
   typedef Quad<int32> I32Quad;
   typedef Quad<float> FloatQuad;
   typedef Quad<double> DoubleQuad;
   
}


#endif

