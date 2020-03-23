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
 * @author Fabio Fracassi
 * @ingroup openOR_core
 */
#ifndef  openOR_Utility_Singleton_hpp
#define  openOR_Utility_Singleton_hpp

#include "../coreDefs.hpp"

namespace openOR {
   namespace Utility {

      //-------------------------------------------------------------------------------
      //!   Base Class Template to build Singleton Objects.
      /**
            If you have an Object that you want to use as a Singleton you simply need
            to define a singleton typedef, and make sure that the object can not be
            created by any other means. See below:

            \code
            class MySingleton : public Singleton<MySingleton> {
               public:
                  void do_something();

                  // This is important for linking reasons
                  static MySingleton& instance();

               private:
                  MySingleton();
                  ~MySingleton();

                  // The singleton baseclass needs to be able to construct MySingleton
                  friend class Singleton<MySingleton>;

                  // Singletons can never be copied.
                  MySingleton(const MySingletonObject& copy);
                  MySingleton& operator=(const MySingletonObject& copy);

            };

            OPENOR_SINGLETON_INIT(MySingleton)
            \endcode

            To use the Singleton use:

            \code
            MySingleton::instance().do_something();
            \endcode
      */
      //-------------------------------------------------------------------------------
      template <class T>
      class OPENOR_CORE_API Singleton {

         public:
            static T& instance() {

               // This is not thread save.
               // \todo Make this Thread save. see "Double-checked locking pattern"
               // in e.g. "Modern C++ Design, Sec. 6.9.1"
               if (! s_pInstance) {
                  s_pInstance = new T;
               }

               return *s_pInstance;
            }

         protected:
            // Only subclasses (the actual singletons can be created)
            Singleton() {}
            ~Singleton() {}
            // the destructor is non virtual on purpose.
            // There is no point in accessing any class through a Singleton<T> pointer.

         private:

            // Singletons can never be copied.
            // Fix warning which occures in visual studio.
            Singleton(const Singleton& copy);
            Singleton& operator=(const Singleton& copy);

            static T* s_pInstance;
      };

      //-------------------------------------------------------------------------------

   } // End NS Utility
} // End NS openOR

//With the code below the singleton breaks,
// since there will be one instance in each dll
// they need to be explicitly created in the client with the macro below.
//template <class T>
//T* Singleton<T>::p_Instance = 0;

//----[ JSINGLETON_INIT Macro ]--------------------------------------------------
/*!   \def OPENOR_SINGLETON_INIT( SINGLETON_TYPE )
      \brief Correctly initializes the instance variable.
*/
#define OPENOR_SINGLETON_INIT( SINGLETON_TYPE )                         \
   namespace openOR {                                                   \
      namespace Utility {                                               \
         template <> SINGLETON_TYPE * Singleton<SINGLETON_TYPE>::s_pInstance = 0; \
      }                                                                 \
   }                                                                    \
   SINGLETON_TYPE & SINGLETON_TYPE ::instance() {                       \
      return openOR::Utility::Singleton< SINGLETON_TYPE >::instance();  \
   }

#endif // openOR_Utility_Singleton_hpp
