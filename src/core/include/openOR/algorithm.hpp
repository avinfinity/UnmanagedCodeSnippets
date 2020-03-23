//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#ifndef openOR_algorithm_hpp
#define openOR_algorithm_hpp

#include <algorithm>

namespace openOR {

   //----------------------------------------------------------------------------
   //! \brief copy_if is curriously missing in C++03.
   //!
   //! Used exactly like std::copy_if would, if it existed.
   //----------------------------------------------------------------------------
   template < class In, class Out, class Pred>
   inline Out copy_if(In first, In last, Out res, Pred p) {
      while (first != last) {
         if (p(*first)) { *res++ = *first; }
         ++first;
      }
      return res;
   }
}



#endif //openOR_algorithm_hpp
