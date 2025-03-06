#pragma once

#include "TokenType.h"
#include "TokenTypeStrings.h"
#include "ClassObject.h"
#include "Namespace.h"
#include <map>
#include <stack>

namespace expr
{
    struct FuncDefinition;
    struct VarDeclaration;
    struct ClassDefinition;
    struct Expression;
}

struct circular_reference_error: public std::runtime_error
{
    circular_reference_error( const Token& token1, const Token& token2 ) : std::runtime_error("circular reference error"),
        m_token1(token1),
        m_token2(token2)
    {}

    const Token& m_token1;
    const Token& m_token2;
};

struct ObjectValue;

struct GlobalVariableMap
{
    std::map<std::string,GlobalVariableMap> m_nestedNamespaceMap;

    std::map<std::string,ObjectValue>       m_namespaceGlobalVariableMap;

    //todo get code from namespace
};

struct Runtime
{
    Namespace&          m_topLevelNamespace;
    GlobalVariableMap   m_globalVariableMap;

    Namespace*          m_currentNamespace = nullptr;

    //???
    using LocalVarStack = std::vector< std::map<std::string,ObjectValue> >;
    LocalVarStack       m_localVarStack;

    Runtime( Namespace& globaNamespace ) : m_topLevelNamespace(globaNamespace) {}

    void initGlobalVariables();

    void run( const std::vector<expr::Expression*>& code, const std::string& sourceCode );

private:
    void initGlobalVariablesR( Namespace& namespaceRef );
};
