//****************************************************************************
// (c) 2007 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#ifndef openOR_Utility_if_const_hpp
#define openOR_Utility_if_const_hpp

#include <boost/type_traits/is_const.hpp>
#include <boost/mpl/if.hpp>

namespace openOR {
   
   template<typename arg_type, typename const_type, typename nonconst_type>
   struct if_const {
      typedef typename boost::mpl::if_< boost::is_const< arg_type >, 
         const_type, 
         nonconst_type>::type type;
   };
   
}

#endif // openOR_Utility_if_const_hpp
