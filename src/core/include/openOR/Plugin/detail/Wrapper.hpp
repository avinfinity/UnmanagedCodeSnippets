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
//! @file
//! @ingroup openOR_core

#ifndef openOR_Plugin_detail_Wrapper_hpp
#define openOR_Plugin_detail_Wrapper_hpp

#include <boost/tr1/memory.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/assert.hpp>

#include <openOR/Utility/is_base_of.hpp>
#include <openOR/Plugin/detail/WrapperBase.hpp>

// This is for use with Qt objects, so that we can handle Qt's memory 
// management transparently. Neither run-/link- nor compile-time 
// dependencies are introduced!
class QObject; 

namespace openOR {
   namespace Plugin {
      namespace Detail {

         //----------------------------------------------------------------------------
         //! \brief Wrapper for objects with auto-delete or without delete.
         //! \tparam ImplType implementation type
         //! \tparam AutoDelete If true, the object will be deleted if no smart pointer points to it; if false, no deleting is called
         //! \internal
         //----------------------------------------------------------------------------
         template<class ImplType, bool AutoDelete = true>
         struct Wrapper : public WrapperBase<ImplType> {

            struct DefaultDelete {
               void operator()(Wrapper* p) { delete p; }
            };

            struct NoDelete {
               void operator()(Wrapper* p) {}
            };

            typedef Wrapper WrapperType; 
            typedef typename boost::mpl::if_c<AutoDelete, DefaultDelete, NoDelete>::type Deleter;


            Wrapper() :  
               WrapperBase<ImplType>() 
            {}

            template<class Param0>
            Wrapper(Param0 param0) : 
               WrapperBase<ImplType>(param0) 
            {}

            template<class Param0, class Param1>
            Wrapper(Param0 param0, Param1 param1) :  
               WrapperBase<ImplType>(param0, param1) 
            {}

            template<class Param0, class Param1, class Param2>
            Wrapper(Param0 param0, Param1 param1, Param2 param2) : 
               WrapperBase<ImplType>(param0, param1, param2) 
            {}

            virtual ~Wrapper() {}

            //----------------------------------------------------------------------------
            //! \brief Returns deleter for smart pointer creation.
            //----------------------------------------------------------------------------
            Deleter deleter() const {
               return Deleter();
            }

         };


         //----------------------------------------------------------------------------
         //! \brief Wrapper for objects derived from QObject (Qt)
         //! \tparam ImplType implementation type (ImplType must be derived from QObject)
         //! \internal
         //----------------------------------------------------------------------------
         template<class ImplType>
         struct QObjectWrapper : public WrapperBase<ImplType> {

            BOOST_MPL_ASSERT_MSG((Utility::is_base_of<QObject,ImplType>::value), FOLLOWING_TYPE_MUST_BE_DERIVED_FROM_QOBJECT, (ImplType));
            typedef QObjectWrapper WrapperType; 

            struct Deleter {
               
               Deleter(const std::tr1::shared_ptr<bool>& pIsQObjectAlreadyDeleted) : 
                  m_pIsQObjectAlreadyDeleted(pIsQObjectAlreadyDeleted) 
               {}
               Deleter(const Deleter& rhs) : 
                  m_pIsQObjectAlreadyDeleted(rhs.m_pIsQObjectAlreadyDeleted) 
               {}
               
               void operator()(QObjectWrapper* p) {
                  if (!(*m_pIsQObjectAlreadyDeleted || p->parent())) {
                     delete p; 
                  }
               }
               
            private:
               const std::tr1::shared_ptr<bool> m_pIsQObjectAlreadyDeleted;
               
               // To avoid MSVC warning C4512 explicitly define (private) assignment operator for this class.
               Deleter& operator=(const Deleter& tmp);
            };


            QObjectWrapper() :  
               WrapperBase<ImplType>() 
            {}

            template<class Param0>
            QObjectWrapper(Param0 param0) :
               WrapperBase<ImplType>(param0) 
            {}

            template<class Param0, class Param1>
            QObjectWrapper(Param0 param0, Param1 param1) :  
               WrapperBase<ImplType>(param0, param1) 
            {}

            template<class Param0, class Param1, class Param2>
            QObjectWrapper(Param0 param0, Param1 param1, Param2 param2) :  
               WrapperBase<ImplType>(param0, param1, param2) 
            {}

            virtual ~QObjectWrapper() {
               if (m_pIsQObjectAlreadyDeleted) {
                  *m_pIsQObjectAlreadyDeleted = true;
               }
            }

            //----------------------------------------------------------------------------
            //! \brief Returns deleter for smart pointer creation.
            //----------------------------------------------------------------------------
            Deleter deleter() const {
               m_pIsQObjectAlreadyDeleted = std::tr1::shared_ptr<bool>(new bool(false));
               return Deleter(m_pIsQObjectAlreadyDeleted);
            }

         private:
            mutable std::tr1::shared_ptr<bool> m_pIsQObjectAlreadyDeleted;

         };
         
      } // end NS 
   }
}



#endif