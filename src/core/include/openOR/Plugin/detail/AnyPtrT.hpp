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

#ifndef openOR_Plugin_detail_AnyPtrT_hpp
#define openOR_Plugin_detail_AnyPtrT_hpp

#include <typeinfo>

#include <boost/tr1/memory.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/type_traits/is_polymorphic.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <assert.h>

namespace openOR {
   namespace Plugin {
      namespace Detail {


         //----------------------------------------------------------------------------
         //! Type Erasure for a smart pointer.
         //! Extended functionalities:
         //! - casting object to a base type
         //! - SBO (Small Buffer Optimization): placeholder is stored at the stack 
         //!   to avoid time-consuming memory allocation at the heap
         //! \sa boost::any
         //! \internal
         //----------------------------------------------------------------------------
         template<class BaseType>
         struct AnyPtrT {

            AnyPtrT() : m_pContent(NULL), m_bValidBasePtr(false) {}


            //----------------------------------------------------------------------------
            //! \pre Type must be polymorphic in order to enable dynamic casting to BaseType class
            //----------------------------------------------------------------------------
            template <typename Type> 
            AnyPtrT(const std::tr1::shared_ptr<Type>& p) : 
               m_pContent(NULL), 
               m_bValidBasePtr(false)
            {
               //ensure that the Holder size is as expected for holder creation at the stack
               BOOST_MPL_ASSERT_RELATION(sizeof(Holder<Type>), ==, sizeof(Holder<char>));
               // Type must be polymorphic in order to enable dynamic casting to BaseType class
               BOOST_MPL_ASSERT((boost::is_polymorphic<Type>));

               if (p) {
                  m_pContent = reinterpret_cast<Placeholder*>(&m_pSpace); 
                  new (m_pSpace) Holder<Type>(p);
               }
            }


            AnyPtrT(const AnyPtrT& other) : 
               m_pContent(NULL), 
               m_bValidBasePtr(false)
            {
               if (other.m_pContent) {
                  m_pContent = reinterpret_cast<Placeholder*>(&m_pSpace); 
                  other.m_pContent->clone(m_pContent);
                  m_bValidBasePtr = other.m_bValidBasePtr;
                  m_pBase = other.m_pBase;
               }
            }


            ~AnyPtrT() {
               reset();
            }

            //----------------------------------------------------------------------------
            //! \pre Type must be polymorphic in order to enable dynamic casting to BaseType class
            //----------------------------------------------------------------------------
            template <typename Type> 
            AnyPtrT& operator=(const std::tr1::shared_ptr<Type>& p) {
               
               // ensure that the Holder size is as expected for holder creation at the stack
               BOOST_MPL_ASSERT_RELATION(sizeof(Holder<Type>), ==, sizeof(Holder<char>));
               // Type must be polymorphic in order to enable dynamic casting to BaseType class
               BOOST_MPL_ASSERT((boost::is_polymorphic<Type>));

               reset();
               if (p) {
                  m_pContent = reinterpret_cast<Placeholder*>(&m_pSpace);
                  new (m_pContent) Holder<Type>(p);
               }
               return *this;
            }


            AnyPtrT& operator=(const AnyPtrT& rhs) {

               reset();
               if (rhs.m_pContent) {
                  m_pContent = reinterpret_cast<Placeholder*>(&m_pSpace);
                  rhs.m_pContent->clone(m_pContent);
               }
               return *this;
            }


            const bool empty() const { return !m_pContent; }


            void reset() { 
               if (m_pContent)
                  m_pContent->~Placeholder();
               m_pContent = NULL;
               m_bValidBasePtr = false;
               m_pBase.reset();
            }


            const std::type_info & type() const {
               return m_pContent ? m_pContent->type() : typeid(void);
            }


            // additional queries
            template <typename Type> 
            std::tr1::shared_ptr<Type> as() const {
               
               std::tr1::shared_ptr<Type> result = 
                  (m_pContent && m_pContent->type() == typeid(Type))
                     ? static_cast<Holder<Type>*>(m_pContent)->m_pHeld 
                     : std::tr1::shared_ptr<Type>();

               if (boost::is_const<Type>::value == false && m_pContent && m_pContent->isConst()) {
                  result.reset();
               }
               return result;
            } 


            /**
             * \tparam Type is used to check if a const type is 
             */
            template <typename Type> 
            std::tr1::shared_ptr<BaseType> asBase() const {
               
               std::tr1::shared_ptr<BaseType> result;
               if (m_pContent) {
                  if (boost::is_const<Type>::value == false && m_pContent->isConst()) {
                     return std::tr1::shared_ptr<BaseType>();
                  }
                  if (m_bValidBasePtr) {
                     return m_pBase;
                  } else {
                     m_bValidBasePtr = true;
                     return m_pBase = m_pContent->castToBase();
                  }
               }
               return std::tr1::shared_ptr<BaseType>();
            }


            const bool pointsToSame(const AnyPtrT& rhs) const {
               return (m_pContent && rhs.m_pContent)
                  ? m_pContent->pointsToSame(rhs.m_pContent) 
                  : m_pContent == rhs.m_pContent;
            }

            //! ordering for placement in ordered containers.
            const bool pointsToBefore(const AnyPtrT& rhs) const {

               // empty containers are considered the smallest element.
               return (m_pContent)
                  ? ((rhs.m_pContent)
                     ? m_pContent->pointsToBefore(rhs.m_pContent)
                     : false )
                  : m_pContent != rhs.m_pContent;
            }

         private:

            struct Placeholder {
               virtual ~Placeholder() {}

               virtual void clone(Placeholder* p) const = 0; 

               virtual const std::type_info & type() const  = 0;
               virtual bool pointsToSame (const Placeholder* rhs) const = 0;
               virtual bool pointsToBefore (const Placeholder* rhs) const = 0;

               virtual std::tr1::shared_ptr<Base> castToBase() const = 0;
               virtual bool isConst() const = 0;
               virtual size_t size() const = 0;
            };


            template<typename Type, bool b>
            struct DynamicCast2Base {
               std::tr1::shared_ptr<Base> operator()(std::tr1::shared_ptr<Type> p) const {
                  typedef typename boost::remove_const<Type>::type Type_noconst;
                  std::tr1::shared_ptr<Type_noconst> pNoConst = std::tr1::const_pointer_cast<Type_noconst>(p);
                  return std::tr1::dynamic_pointer_cast<Base>(pNoConst);
               }
            };


            template<typename Type>
            struct DynamicCast2Base<Type, false> {
               std::tr1::shared_ptr<Base> operator()(std::tr1::shared_ptr<Type> p) const {
                  return std::tr1::shared_ptr<Base>();
               }
            };


            template <typename Type> 
            struct Holder : Placeholder {
               
               Holder (const std::tr1::shared_ptr<Type>& p) : m_pHeld(p) {}
               virtual ~Holder () {}

               virtual void clone(Placeholder* p) const      { new (p) Holder(m_pHeld); } 
               virtual const std::type_info& type() const    { return typeid(Type); }
               virtual bool pointsToSame (const Placeholder* rhs) const { 
                  assert (rhs && "openOR::Detail::AnyPtr::pointsToSame (rhs == 0)");
                  return type() == rhs->type() ? m_pHeld == static_cast<const typename AnyPtrT::template Holder<Type>*>(rhs)->m_pHeld
                                               : castToBase() == rhs->castToBase();
               }
               virtual bool pointsToBefore (const Placeholder* rhs) const {
                  assert (rhs && "openOR::Detail::AnyPtr::pointsToBefore (rhs == 0)");
                  return type() == rhs->type() ? m_pHeld < static_cast<const typename AnyPtrT::template Holder<Type>*>(rhs)->m_pHeld
                                               : castToBase() < rhs->castToBase();
               }

               virtual std::tr1::shared_ptr<Base> castToBase() const {
                  return DynamicCast2Base<Type,  boost::is_polymorphic<Type>::value>()(m_pHeld);
               }
               virtual bool isConst() const { return boost::is_const<Type>::value; }
               virtual size_t size() const { return sizeof(Holder<Type>); }

               std::tr1::shared_ptr<Type> m_pHeld;
            };

            Placeholder* m_pContent;                     //!< pointer to the placeholder object
            char m_pSpace[sizeof(Holder<char>)];         //!< memory for placeholder object (avoid memory allocation at the heap)
            mutable std::tr1::shared_ptr<Base> m_pBase;  //!< cached base pointer
            mutable bool m_bValidBasePtr;                //!< flag if the base pointer cache value is valid 
         };

         template<class BaseType>
         bool operator==(const AnyPtrT<BaseType>& lhs, const AnyPtrT<BaseType>& rhs) { return lhs.pointsToSame(rhs); }

         template<class BaseType>
         struct OrderingAnyPtrT : public std::binary_function<AnyPtrT<BaseType>, AnyPtrT<BaseType>, bool> {
            bool operator()(const AnyPtrT<BaseType>& lhs, const AnyPtrT<BaseType>& rhs) const { return lhs.pointsToBefore(rhs); }
         };

      } // end NS
   }
}

#endif