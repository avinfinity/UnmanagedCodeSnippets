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
 * @author Christian Winne
 * @ingroup openOR_core
 */

#ifndef openOR_core_Math_constants_hpp
#define openOR_core_Math_constants_hpp

#include <cmath>

namespace openOR {
   namespace Math {

		// Undefine possibly existing PI-macro
		#ifdef PI
			#undef PI
		#endif /* PI */

      /**
       * @brief definition of PI
       * @ingroup openOR_core
       */
		static double const PI = acos(-1.0);

      /**
       * @brief granularity for floating point comparision
       * @ingroup openOR_core
       */
		const double GRANULARITY = 0.001;

   
      /**
       * @brief granularity for floating point comparision
       * @ingroup openOR_core
       */
      const double GRANULARITY_SQUARE = (GRANULARITY * GRANULARITY);

   }
}

#endif /* openOR_core_Math_constants_hpp */