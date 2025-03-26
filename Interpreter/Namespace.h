#pragma once

#include <cassert>
#include <map>
#include <string>
#include <vector>
#include "Token.h"

struct Namespace
{
    std::string m_name;
    Namespace*  m_parentNamespace = nullptr;

    std::map< std::string, expr::VarDeclaration*>   m_variableMap; // global variable map
    
    std::map< std::string, bool>                    m_initializationVariableMap;  // true if variable is initializing now
    std::map<std::string,ObjectValue>               m_variableValueMap; // will be moved to runtime

    std::map<std::string,expr::FuncDefinition*>     m_functionMap;
    std::map<std::string,expr::ClassDefinition*>    m_classMap;

    std::map< std::string, Namespace>               m_innerNamespaceMap;

    Namespace* getTopNamespace()
    {
        if ( m_parentNamespace == nullptr )
        {
            return this;
        }
        return m_parentNamespace->getTopNamespace();
    }

    Namespace* getNamespace( std::vector<std::string_view>::iterator begin, std::vector<std::string_view>::iterator end )
    {
        assert( begin != end );

        for( auto* namespacePtr = this;;)
        {
            if ( auto it = namespacePtr->m_innerNamespaceMap.find( std::string(*begin) ); it == namespacePtr->m_innerNamespaceMap.end() )
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

    expr::FuncDefinition* getFunctionDef( const std::string& name, std::vector<std::string_view>& namespaceSpec )
    {
        auto* namespacePtr = getNamespace( namespaceSpec.begin(), namespaceSpec.end() );
        if ( namespacePtr != nullptr )
        {
            if ( auto* funcDef = namespacePtr->getFunctionDefInThisNamespace( name ); funcDef != nullptr )
            {
                return funcDef;
            }
        }

        // If our namespace is not top namespace, then try to find in top namespace
        if ( m_parentNamespace != nullptr )
        {
            auto* topNamespace = m_parentNamespace->getTopNamespace();
            auto* namespacePtr = topNamespace->getNamespace( namespaceSpec.begin(), namespaceSpec.end() );
            if ( namespacePtr != nullptr )
            {
                if ( auto* funcDef = namespacePtr->getFunctionDefInThisNamespace( name ); funcDef != nullptr )
                {
                    return funcDef;
                }
            }
        }

        return nullptr;
    }


    expr::FuncDefinition* getFunctionDef( const std::string& name )
    {
        auto* funcDef = getFunctionDefInThisNamespace( name );
        if ( funcDef != nullptr )
        {
            return funcDef;
        }

        for( auto* parent = m_parentNamespace; parent != nullptr; parent = parent->m_parentNamespace )
        {
            if ( auto* funcDef = parent->getFunctionDefInThisNamespace( name ); funcDef != nullptr )
            {
                return funcDef;
            }
        }

        return nullptr;
    }

    expr::FuncDefinition* getFunctionDefInThisNamespace( const std::string& name )
    {
        if ( auto it = m_functionMap.find( name ); it != m_functionMap.end() )
        {
            return it->second;
        }
        return nullptr;
    }

};
