//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR commercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#ifndef openOR_Plugin_detail_TypePair_hpp
#define openOR_Plugin_detail_TypePair_hpp

#include <openOR/coreDefs.hpp>

namespace openOR {
   namespace Plugin {
      namespace Detail {

         //----------------------------------------------------------------------------
         //! \brief INTERNAL Plugin/Interface type id pair for Reflection map.
         //----------------------------------------------------------------------------
         struct OPENOR_CORE_API TypePair {

            TypePair(const std::type_info* pPlugin, const std::type_info* pInterface) :
                  m_pPlugin(pPlugin),
                  m_pInterface(pInterface) {}
            ~TypePair() {}

            inline bool operator==(const TypePair& rhs) const {
               return ((*m_pPlugin == *(rhs.m_pPlugin)) && (*m_pInterface == *(rhs.m_pInterface)));
            }
            inline const std::type_info& plugin_type() const     { return *m_pPlugin; }
            inline const std::type_info& interface_type() const  { return *m_pInterface; }

         private:
            friend struct TypeHash;
            const std::type_info* m_pPlugin;
            const std::type_info* m_pInterface;
         };

         //----------------------------------------------------------------------------
         //! \brief INTERNAL Plugin/Interface type id hash for Reflection map.
         //----------------------------------------------------------------------------
         struct OPENOR_CORE_API TypeHash : std::unary_function<TypePair, size_t> {
            TypeHash() {}
            size_t operator()(TypePair const& v) const;
         };

      }
   }
}

#endif //openOR_Plugin_detail_TypePair_hpp
