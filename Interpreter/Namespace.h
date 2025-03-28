#pragma once

#include <cassert>
#include <map>
#include <string>
#include <vector>
#include "Token.h"

struct NamespaceBase
{
    bool m_isClass = false;

    std::map<std::string_view,expr::FuncDefinition*>     m_functionMap;
    std::map<std::string_view,expr::ClassDefinition*>    m_classMap;
    std::map< std::string_view, expr::VarDeclaration*>   m_variableMap; // global/static variable map

    expr::FuncDefinition* getFunctionDef( const std::string_view& name )
    {
        if ( auto it = m_functionMap.find( name ); it != m_functionMap.end() )
        {
            return it->second;
        }
        return nullptr;
    }

    expr::ClassDefinition* getClassDef( const std::string_view& name )
    {
        if ( auto it = m_classMap.find( name ); it != m_classMap.end() )
        {
            return it->second;
        }
        return nullptr;
    }

    expr::VarDeclaration* getVarDecl( const std::string_view& name )
    {
        if ( auto it = m_variableMap.find( name ); it != m_variableMap.end() )
        {
            return it->second;
        }
        return nullptr;
    }

    template<class T>
    T* getInThisNamespace( const std::string_view& name )
    {
        if constexpr( std::is_same<T, expr::FuncDefinition>::value )
        {
            return getFunctionDef( name );
        }
        if constexpr( std::is_same<T, expr::ClassDefinition>::value )
        {
            return getClassDef( name );
        }
        if constexpr( std::is_same<T, expr::VarDeclaration>::value )
        {
            return getVarDecl( name );
        }
        return nullptr;
    }

};

struct Namespace: public NamespaceBase
{
    std::string_view m_name;
    Namespace*  m_parentNamespace = nullptr;

    Namespace() = default;

    Namespace( const std::string_view& name, Namespace* parentNamespace )
        : m_name(name), m_parentNamespace(parentNamespace)
    {}

    Namespace( const Namespace& ) = default;
    Namespace& operator=( const Namespace& ) = default;

    std::map< std::string_view, Namespace>               m_innerNamespaceMap;

    std::map<std::string_view,expr::FuncDefinition*>     m_functionMap;
    std::map<std::string_view,expr::ClassDefinition*>    m_classMap;
    std::map< std::string_view, expr::VarDeclaration*>   m_variableMap; // global/static variable map

    std::map< std::string_view, bool>                    m_initializationVariableMap;  // true if variable is initializing now
    std::map<std::string_view,ObjectValue>               m_variableValueMap;           // will be used in runtime



    Namespace* getTopNamespace()
    {
        if ( m_parentNamespace == nullptr )
        {
            return this;
        }
        return m_parentNamespace->getTopNamespace();
    }

    NamespaceBase* getNamespace( std::vector<std::string_view>::iterator begin, std::vector<std::string_view>::iterator end )
    {
        assert( begin != end );

        for( auto* namespacePtr = this;;)
        {
            if ( auto it = namespacePtr->m_innerNamespaceMap.find( *begin ); it == namespacePtr->m_innerNamespaceMap.end() )
            {
                return nullptr;
            }
            else
            {
                if ( begin+1 == end )
                {
                    return &it->second;
                }
                begin++;
                namespacePtr = &it->second;
            }
        }

        return nullptr;
    }

    expr::FuncDefinition* getFunctionDef( const std::string_view& name, std::vector<std::string_view>& namespaceSpec )
    {
        return get<expr::FuncDefinition>( name, namespaceSpec );
    }

    expr::ClassDefinition* getClassDef( const std::string_view& name, std::vector<std::string_view>& namespaceSpec )
    {
        return get<expr::ClassDefinition>( name, namespaceSpec );
    }

    expr::VarDeclaration* getVarDecl( const std::string_view& name, std::vector<std::string_view>& namespaceSpec )
    {
        return get<expr::VarDeclaration>( name, namespaceSpec );
    }

    template<class T>
    T* get( const std::string_view& name, std::vector<std::string_view>& namespaceSpec )
    {
        auto* namespacePtr = getNamespace( namespaceSpec.begin(), namespaceSpec.end() );
        if ( namespacePtr != nullptr )
        {
            if ( auto* obj = namespacePtr->getInThisNamespace<T>( name ); obj != nullptr )
            {
                return obj;
            }
        }

        // If our namespace is not top namespace, then try to find in top namespace
        if ( m_parentNamespace != nullptr )
        {
            auto* topNamespace = m_parentNamespace->getTopNamespace();
            auto* namespacePtr = topNamespace->getNamespace( namespaceSpec.begin(), namespaceSpec.end() );
            if ( namespacePtr != nullptr )
            {
                if ( auto* obj = namespacePtr->getInThisNamespace<T>( name ); obj != nullptr )
                {
                    return obj;
                }
            }
        }

        return nullptr;
    }

//    expr::FuncDefinition* getFunctionDef( const std::string_view& name, std::vector<std::string_view>& namespaceSpec )
//    {
//        auto* namespacePtr = getNamespace( namespaceSpec.begin(), namespaceSpec.end() );
//        if ( namespacePtr != nullptr )
//        {
//            if ( auto* funcDef = namespacePtr->getFunctionDefInThisNamespace( name ); funcDef != nullptr )
//            {
//                return funcDef;
//            }
//        }
//
//        // If our namespace is not top namespace, then try to find in top namespace
//        if ( m_parentNamespace != nullptr )
//        {
//            auto* topNamespace = m_parentNamespace->getTopNamespace();
//            auto* namespacePtr = topNamespace->getNamespace( namespaceSpec.begin(), namespaceSpec.end() );
//            if ( namespacePtr != nullptr )
//            {
//                if ( auto* funcDef = namespacePtr->getFunctionDefInThisNamespace( name ); funcDef != nullptr )
//                {
//                    return funcDef;
//                }
//            }
//        }
//
//        return nullptr;
//    }


//    expr::FuncDefinition* getFunctionDef( const std::string_view& name )
//    {
//        auto* funcDef = getFunctionDefInThisNamespace( name );
//        if ( funcDef != nullptr )
//        {
//            return funcDef;
//        }
//
//        for( auto* parent = m_parentNamespace; parent != nullptr; parent = parent->m_parentNamespace )
//        {
//            if ( auto* funcDef = parent->getFunctionDefInThisNamespace( name ); funcDef != nullptr )
//            {
//                return funcDef;
//            }
//        }
//
//        return nullptr;
//    }

//    expr::FuncDefinition* getFunctionDefInThisNamespace( const std::string_view& name )
//    {
//        if ( auto it = m_functionMap.find( name ); it != m_functionMap.end() )
//        {
//            return it->second;
//        }
//        return nullptr;
//    }

};
