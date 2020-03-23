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
#ifndef openOR_Utility_conceptcheck_hpp
#define openOR_Utility_conceptcheck_hpp


#ifndef ENABLE_OPENOR_CONCEPTCHECK
#define ENABLE_OPENOR_CONCEPTCHECK 0
#endif

#if (ENABLE_OPENOR_CONCEPTCHECK == 0)

# include <boost/parameter/aux_/parenthesized_type.hpp>
#define OPENOR_CONCEPT_REQUIRES(models, result) typename BOOST_PARAMETER_PARENTHESIZED_TYPE(result) 

#else

#include <boost/concept/requires.hpp>
#define OPENOR_CONCEPT_REQUIRES(models, result) BOOST_CONCEPT_REQUIRES(models, result)

#endif

#endif // openOR_Utility_conceptcheck_hpp
