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
//! @file
//! @ingroup openOR_core
//****************************************************************************


#ifndef openOR_core_Math_math_hpp
#define openOR_core_Math_math_hpp

#ifndef M_PI
#define M_PI 3.141592653589793238462
#endif

#include <openOR/Math/traits.hpp>
#include <openOR/Math/vector.hpp>
#include <openOR/Math/matrix.hpp>
#include <openOR/Math/vectorfunctions.hpp>
#include <openOR/Math/matrixfunctions.hpp>

#include <openOR/Math/create.hpp>
#include <openOR/Math/comparison.hpp>
#include <openOR/Math/access.hpp>

#include <openOR/Math/constants.hpp>
#include <openOR/Math/utilities.hpp>
#include <openOR/Math/GraphicalProjection.hpp>

//TODO: wird eigentlich nicht echt gebraucht, trotzdem drin wegen doku. Raus? 

//!
//! \page math_docu How to use the openOR core math module?
//! 
//! The openOR core math module provides vector and matrix functionalities.
//! 
//! \section vectors Vectors
//! 
//! The vector functionality is divided into two parts. First, the definition of vector types
//! with a limited range of functions. Second, a collection of templated global functions for vector operations.
//!
//! \subsection vector_types Vector types
//! In order to use vectors, the following header file has to be includes:
//!
//! \code
//! #include <openOR/Math/vector.hpp>
//! \endcode
//! 
//! Basically all arbitrary vector types can be created. But openOR already offers some convenient and most often
//! used vector types. Their names are combinations of their dimension (2 to 4) and datatype (i, ui, f and d). Hence
//! a 3-dimensional vector of floats is called \em Vector3f and a 4-dimensional vector of unsigned shorts \em Vector4ui.
//!
//! Those predefined vector types are shorthand for
//! \code
//! boost::numeric::ublas::bounded_vector<float, 3> // Vector3f
//! boost::numeric::ublas::bounded_vector<unsigned short, 4> // Vector4ui
//! \endcode
//! and hence expandable. If you want to create your own vector type you must add this type to \verbatim vector_types.hpp \endverbatim.
//!
//! \subsubsection vector_creation Vector object creation
//! 
//! The vector object creation without initialization is a simple variable declaration. For example
//! the declaration of a variable of type Vector3d looks like this:
//! \code
//! openOR::Math::Vector3d vec;
//! \endcode
//! 
//! The creation of vectors with element initialization uses the openOR::Math::create function.
//! In order to use this function, the following header file must be included:
//!
//! \code
//! #include <openOR/Math/create.hpp>
//! \endcode
//!
//! See the examples section to see all vector creation possibilities.
//! 
//!
//! \subsection vector_traits Vector traits
//! 
//! Traits classes are defined for every Vector type in openOR using the struct VectorTraits. 
//!
//!
//! \subsection vector_functions Vector functions
//! In order to use vector functions, the following header file has to be included:
//!
//! \code
//! #include <openOR/Math/vectorfunctions.hpp>
//! \endcode
//! 
//! The following vector functions are available for a given VectorType (namespace: openOR::Math)
//! \code
//! VectorTraits<VectorType>::ValueType dot(const VectorType& vec);
//! VectorTraits<VectorType>::ValueType norm(const VectorType& vec);
//! VectorTraits<VectorType>::ValueType squaredNorm(const VectorType& vec);
//! VectorTraits<VectorType>::ValueType minimalElement(const VectorType& vec);
//! VectorTraits<VectorType>::ValueType maximalElement(const VectorType& vec); 
//! VectorTraits<VectorType>::ValueType summarizedElements(const VectorType& vec);
//! VectorTraits<VectorType>::ValueType multipliedElements(const VectorType& vec);
//! VectorType& normalize(VectorType& vec);
//! VectorTraits<VectorType>::RealVectorType normalized(const VectorType& vec);
//! \endcode
//!
//! \section vector_examples Examples
//! 
//! \code
//! // create with Math::create
//! Math::Vector2f a = Math::create<Math::Vector2f>(5.0f, 5.0f);
//!
//! // create by declaration and assignment
//! Math::Vector2f b;
//! b(0) = 5.0f;
//! b(1) = 8.0f;
//!
//! // create by operation
//! Math::Vector2f c = a - b;	// c = (0.0, -3.0)
//! Math::Vector2f d = 
//! \endcode
//!
//! \section matrices Matrices
//!
//! The matrix functionality is divided into two parts. First, the definition of matrix types
//! with a limited range of functions. Second, a collection of templated global functions for vector operations.
//!
//! \subsection matrix_types Matrix types
//! In order to use matrices, the following header file has to be includes:
//!
//! \code
//! #include <openOR/Math/matrix.hpp>
//! \endcode
//! 
//! Following matrix types are defined:
//! - Matrix22i
//! - Matrix22ui
//! - Matrix22f
//! - Matrix22d
//! - Matrix33i
//! - Matrix33ui
//! - Matrix33f
//! - Matrix33d
//! - Matrix44i
//! - Matrix44ui
//! - Matrix44f
//! - Matrix44d
//!
//! \subsubsection matrix_creation Matrix object creation
//! 
//! The matrix object creation without initialization is a simple variable declaration. For example
//! the declaration of a variable of type Matrix33d:
//! \code
//! openOR::Math::Matrix33d mat;
//! \endcode
//! There are two possibilities to initialize the matrix:
//! \code
//! openOR::Math::Matrix22d mat = openOR::Math::MatrixTraits<openOR::Math::Matrix22d>::ZEROS;
//! openOR::Math::Matrix44i mat = openOR::Math::MatrixTraits<openOR::Math::Matrix44i>::IDENTITY;
//! \endcode 
//! 
//! \subsection matrix_traits Matrix traits
//! 
//! Traits classes are defined for every matrix type in openOR using the struct MatrixTraits. 
//!
//!
//! \subsection matrix_functions Matrix functions
//! In order to use matrix functions, the following header file has to be includes:
//!
//! \code
//! #include <openOR/Math/matrixfunctions.hpp>
//! \endcode
//! 
//! The following matrix functions are available for a given MatrixType (namespace: openOR::Math)
//! \code
//! MathTraits<MatrixType>::Vector3Type xAxis(const MatrixType& vec);
//! MathTraits<MatrixType>::Vector3Type yAxis(const MatrixType& vec);
//! MathTraits<MatrixType>::Vector3Type zAxis(const MatrixType& vec);
//! MathTraits<MatrixType>::Vector3Type translation(const MatrixType& vec);
//! void setXAxis(MatrixType& vec, const VectorType& vec);
//! void setYAxis(MatrixType& vec, const VectorType& vec);
//! void setZAxis(MatrixType& vec, const VectorType& vec);
//! void setTranslation(MatrixType& vec, const VectorType& vec);
//! void setScale(MatrixType& vec, const VectorType& vec);
//! MatrixType& orthoInvert(MatrixType& mat);
//! MatrixType orthoInverse(const MatrixType& mat);
//! MatrixType& invert(MatrixType& mat);
//! MatrixType inverse(const MatrixType& mat);
//! MatrixType& transpose(MatrixType& mat);
//! MatrixType transposed(const MatrixType& mat);
//! MathTraits<MatrixType>::ValueType determinant(MatrixType& mat);
//! \endcode

#endif
