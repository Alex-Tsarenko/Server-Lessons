#pragma once

#include "TokenType.h"
#include "TokenTypeStrings.h"
#include <map>
#include <stack>

namespace expr
{
    struct FuncDefinition;
    struct ExpressionVarDecl;
    struct Expression;
}

struct ObjectValue;

struct Runtime
{
    Runtime() {}
    
    std::map<std::string,expr::FuncDefinition*>         m_funcMap;
    std::map<std::string,ObjectValue>                   m_globalVarMap;
    
    using LocalVarStack = std::vector< std::map<std::string,ObjectValue> >;
    
    LocalVarStack                           m_localVarStack;

    void run( const std::vector<expr::Expression*>& code, const std::string& sourceCode );
};
