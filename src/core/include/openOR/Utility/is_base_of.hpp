//****************************************************************************
// (c) 2007 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#ifndef openOR_Utility_is_base_of_hpp
#define openOR_Utility_is_base_of_hpp

#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/and.hpp>

namespace openOR {
   namespace Utility {
      
      /**
       * Checks if type B is a base type of type D.
       * Unlike boost::is_base_of, this functor also allows incomplete types.
       * \sa boost::is_base_of
       */
      template<typename B, typename D>
      struct is_base_of : boost::mpl::bool_< boost::mpl::and_< ::boost::is_convertible<D*, B*>, ::boost::is_base_of<B, D> >::value > {};

   }
}

#endif
