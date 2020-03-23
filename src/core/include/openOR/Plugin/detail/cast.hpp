//****************************************************************************
// (c) 2008 - 2010 by the openOR Team
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

#ifndef openOR_Plugin_detail_cast_hpp
#define openOR_Plugin_detail_cast_hpp

#include <boost/tr1/memory.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/is_polymorphic.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/assert.hpp>

#include <openOR/Plugin/detail/Base.hpp>
#include <openOR/Plugin/exception.hpp>
#include <openOR/Log/Logger.hpp>

namespace openOR {
   namespace Plugin {
      namespace Detail {
         
         //----------------------------------------------------------------------------
         //! \brief Casting from a base object (wrapper or adapter object) to a supported interface.
         //! 
         //! If the base object is an adapter, the corresponding wrapper object is used for further computations.
         //! 
         //! The wrapper object is tried to cast toward the requested interface using direct inheritance.
         //! In case of failure, the wrapper object is tried to be casted using registered adapter classes.
         //!
         //! \param pBase smart pointer to a wrapper or an adapter object
         //! \param bThrow flag to set if an exception has to be thrown if the cast fails
         //! \throw std::bad_cast if bThrow == true and the cast fails
         //!
         //! \internal
         //----------------------------------------------------------------------------
         template<class InterfaceTo>
         std::tr1::shared_ptr<InterfaceTo> cast(std::tr1::shared_ptr<Detail::Base> pBase, bool bThrow) {
      
            if (!pBase) {
               if (bThrow) {
                  throw IncorrectObjectCreationError();
               } else {
                  return std::tr1::shared_ptr<InterfaceTo>();
               }
            }

            std::tr1::shared_ptr<InterfaceTo> pTo;
            if (!pBase->isWrapper()) {
               // pBase points to an adapter object
               pBase = pBase->wrapperBasePtr();
               if (!pBase || !pBase->isWrapper()) {
                  if (bThrow) {
                     throw IncorrectObjectCreationError();
                  } else { 
                     return std::tr1::shared_ptr<InterfaceTo>();
                  }
               }
               // pBase points now to the corresonding wrapper object
            }

            // check if there is direct inheritance between interface and implementation
            if ((pTo = std::tr1::dynamic_pointer_cast<InterfaceTo>(pBase))) {
               return pTo;
            }

            // try to cast to registered interfaces
            typedef typename boost::remove_const<InterfaceTo>::type InterfaceToNoConst;
            boost::any anyInterface = pBase->castToRegisteredInterface(typeid(InterfaceTo));
            if (!anyInterface.empty()) {
               try {
                  pTo = boost::any_cast< std::tr1::shared_ptr<InterfaceToNoConst> >(anyInterface);
               } catch (...) {
                  // Known reasons for failure:
                  // - Interface class declaration as 'class' and interface class definition as 'struct'
                  //   (MSVC saves struct/class information in type_info ; see raw_name() (MSVC 2005: U=struct, V=class))
                  //
                  const std::type_info& ti1 = typeid(std::tr1::shared_ptr<InterfaceToNoConst>);
                  const std::type_info& ti2 = anyInterface.type();
                  LOG(Log::Level::Warning, Log::noAttribs, 
                        Log::msg("openOR::interface_cast error: AnyCast to interface smart pointer failed: %1% != %2%")
                        % ti1.name()
                        % ti2.name()
                     );
               }
            }
            
            if (bThrow && !pTo) {
               throw UnsupportedInterfaceError(pBase->typeId().name(), typeid(InterfaceTo).name());
            }

            return pTo;   
         }

         
         //----------------------------------------------------------------------------
         //! \brief Template helper class to enable type dependent interface casting using template specialization.
         //! \param bThrow flag to set if an exception has to be thrown if the cast fails
         //! \throw std::bad_cast if bThrow == true and the cast fails
         //!
         //! \internal
         //----------------------------------------------------------------------------
         template<class InterfaceTo, class InterfaceFrom, bool>
         struct Caster
         {
            static std::tr1::shared_ptr<InterfaceTo> cast(const std::tr1::shared_ptr<InterfaceFrom>& pFrom, bool bThrow)
            {
               BOOST_MPL_ASSERT((boost::is_polymorphic<InterfaceFrom>));
               BOOST_MPL_ASSERT_NOT((boost::mpl::and_< boost::is_const<InterfaceFrom>, boost::mpl::not_<boost::is_const<InterfaceTo> > >));
               
               typedef typename boost::remove_const<InterfaceFrom>::type InterfaceFromNoConst;
               std::tr1::shared_ptr<InterfaceFromNoConst> pFromNoConst = std::tr1::const_pointer_cast<InterfaceFromNoConst>(pFrom);
               std::tr1::shared_ptr<Detail::Base> pBase = std::tr1::dynamic_pointer_cast<Detail::Base>(pFromNoConst);
               return Detail::cast<InterfaceTo>(pBase, bThrow);
            }
         };


         //----------------------------------------------------------------------------
         //! \brief Template helper class to enable type dependent interface casting using template specialization.
         //!
         //! Optimized casting to a base class interface (using direct inheritance).
         //!
         //! \internal
         //----------------------------------------------------------------------------
         template<class InterfaceTo, class InterfaceFrom>
         struct Caster<InterfaceTo, InterfaceFrom, true>
         {
            static std::tr1::shared_ptr<InterfaceTo> cast(const std::tr1::shared_ptr<InterfaceFrom>& pFrom, bool)
            {
               return std::tr1::static_pointer_cast<InterfaceTo>(pFrom);
            }
         };

      } // end NS Detail
   }
}

#endif
