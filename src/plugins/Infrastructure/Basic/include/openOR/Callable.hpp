//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//! OPENOR_INTERFACE_FILE(Basic)
//****************************************************************************
/**
 * @file
 * @ingroup Basic
 */
#ifndef openOR_Callable_hpp
#define openOR_Callable_hpp

#include <boost/tr1/memory.hpp>
#include <openOR/Plugin/CreateInterface.hpp>
#include <openOR/Plugin/interface_cast.hpp>
#include <openOR/Plugin/AnyPtr.hpp>

namespace openOR {
 
    //! \brief Interface for callable Plugins
    //!
    //! Basic Interface which allows generic plugins to be called without further
    //! knowledge.
    //!
    //! @ingroup Basic
    struct Callable {

       virtual void operator()() const = 0;

    };


    inline void call(Callable* pCallee) {(*pCallee)(); }
    inline void call(std::tr1::shared_ptr<Callable> pCallee) {(*pCallee)(); }
    inline void call(const AnyPtr& callee) {
      std::tr1::shared_ptr<Callable> pCallee = try_interface_cast<Callable>(callee);
      (*pCallee)();
    }

 }


OPENOR_CREATE_INTERFACE(openOR::Callable)
   void operator()() const { adaptee()->operator()(); }
OPENOR_CREATE_INTERFACE_END


#endif // openOR_Callable_hpp
