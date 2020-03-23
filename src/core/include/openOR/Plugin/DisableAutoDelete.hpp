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

#ifndef openOR_Plugin_delete_hpp
#define openOR_Plugin_delete_hpp

   //----------------------------------------------------------------------------
   //! Disable object auto-deletion invoked by the smart pointer of a plugin type
   //----------------------------------------------------------------------------
   #define OPENOR_DISABLE_AUTO_DELETE(IMPL_TYPE)                                 \
      namespace openOR { namespace Plugin { namespace Detail {                   \
         template<>                                                              \
         struct WrapperAssigner<IMPL_TYPE, false> {                              \
            typedef Wrapper<IMPL_TYPE, false> WrapperType;                       \
         };                                                                      \
      } } }


namespace openOR {
   namespace Plugin {
      namespace Detail {

         //----------------------------------------------------------------------------
         // fwd declarations (from Detail/Wrapper.hpp)
         template<class ImplType, bool> struct Wrapper;
         template<class ImplType>       struct QObjectWrapper;
         
         //----------------------------------------------------------------------------
         //! \brief Definition of the wrapper type in the default case 
         //! (automatic object destruction if no more smart pointer point to the object)
         //! \internal
         //----------------------------------------------------------------------------
         template<class ImplType, bool>
         struct WrapperAssigner {
            typedef Wrapper<ImplType, true> WrapperType;
         };

         //----------------------------------------------------------------------------
         //! \brief Definition of the wrapper type if the object is derived from QObject (Qt).
         //! \internal
         //!
         //! The object is deleted if it was not deleted before or the object has no parent.
         //----------------------------------------------------------------------------
         template<class ImplType>
         struct WrapperAssigner<ImplType, true> {
            typedef QObjectWrapper<ImplType> WrapperType;
         };
 
      } // end NS
   }
}


 


#endif