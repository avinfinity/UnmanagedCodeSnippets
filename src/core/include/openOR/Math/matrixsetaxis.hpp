//****************************************************************************
// (c) 2008 - 2011 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//! OPENOR_INTERFACE_FILE(openOR_core)
//****************************************************************************
//! @file
//! @ingroup openOR_core
#ifndef openOR_core_math_matrixsetaxis_hpp
#define openOR_core_math_matrixsetaxis_hpp

#include <openOR/Math/utilities.hpp>
#include <openOR/Math/matrix.hpp>
#include <openOR/Math/vector.hpp>

namespace openOR {
   namespace Math {
      
      /**
       * @brief Sets the x-axis vector of a 3x3 or 4x4 matrix.
       * @ingroup openOR_core
       */
      template <class Type, class Vec>
      inline
      OPENOR_CONCEPT_REQUIRES(
                              ((Concept::Matrix<Type>))
                              ((Concept::ConstVector<Vec>)),
                              (void))
      setXAxis(Type& mat, const Vec& vec) {
         get<0, 0>(mat) = get<0>(vec);
         get<1, 0>(mat) = get<1>(vec);
         get<2, 0>(mat) = get<2>(vec);
      }
      
      /**
       * @brief Sets the y-axis vector of a 3x3 or 4x4 matrix.
       * @ingroup openOR_core
       */
      template <class Type, class Vec>
      inline
      OPENOR_CONCEPT_REQUIRES(
                              ((Concept::Matrix<Type>))
                              ((Concept::ConstVector<Vec>)),
                              (void))
      setYAxis(Type& mat, const Vec& vec) {
         get<0, 1>(mat) = get<0>(vec);
         get<1, 1>(mat) = get<1>(vec);
         get<2, 1>(mat) = get<2>(vec);
      }
      
      /**
       * @brief Sets the z-axis vector of a 3x3 or 4x4 matrix.
       * @ingroup openOR_core
       */
      template <class Type, class Vec>
      inline
      OPENOR_CONCEPT_REQUIRES(
                              ((Concept::Matrix<Type>))
                              ((Concept::ConstVector<Vec>)),
                              (void))
      setZAxis(Type& mat, const Vec& vec) {
         get<0, 2>(mat) = get<0>(vec);
         get<1, 2>(mat) = get<1>(vec);
         get<2, 2>(mat) = get<2>(vec);
      }
      
      
      /**
       * @brief Sets the translation vector of a 4x4 matrix.
       * @ingroup openOR_core
       */
      template <class Type, class Vec>
      inline
      OPENOR_CONCEPT_REQUIRES(
                              ((Concept::Matrix<Type>))
                              ((Concept::ConstVector<Vec>)),
                              (void))
      setTranslation(Type& mat, const Vec& vec) {
         get<0, 3>(mat) = get<0>(vec);
         get<1, 3>(mat) = get<1>(vec);
         get<2, 3>(mat) = get<2>(vec);
      }
      
   }
}

#endif

