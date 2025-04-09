#pragma once

#include "Expr.h"


namespace expr 
{
ObjectValue VarDeclaration::execute( Runtime& runtime, bool isGlobal )
{
    if ( m_initValue != nullptr )
    {
        ClassOrNamespace*   classDef = nullptr;
        ObjectValue         value;

        // var v = <SomeClassName>;
        // var v = <VariableName>;
        // var v = <SomeClassName>(...);
        // var v = <FuncName>(...);
        // var v = <Literal>;

        if ( m_initValue->type() == et_func_call )
        {
            auto* funcCall = (FunctionCall*) m_initValue;

            classDef = runtime.m_currentNamespace2->getClassDef( funcCall->m_functionName, funcCall->m_namespaceSpec );
        }
        else if ( m_initValue->type() == et_identifier )
        {
            auto* identifier = (IdentifierExpr*) m_initValue;

            classDef = runtime.m_currentNamespace2->getClassDef( identifier->m_name, identifier->m_namespaceSpec );
            if ( classDef == nullptr )
            {
                value = *runtime.m_currentNamespace2->getVarValue( identifier->m_name, identifier->m_namespaceSpec );
            }
        }
        else if ( m_initValue->type() == et_int ||
                 m_initValue->type() == et_float ||
                 m_initValue->type() == et_string || 
                 m_initValue->type() == et_unary ||
                 m_initValue->type() == et_binary )
        {
            value = m_initValue->execute( runtime, isGlobal );
            return value;
        }

        if ( classDef != nullptr )
        {
            value = ::createClassObject( runtime, isGlobal, *classDef );
        }
        else if ( value.m_type != ot_null )
        {
        }
        else
        {
            LOG( "Unknown value: " << m_initValue->m_token.lexeme )
            throw runtime_ex3( "Unknown value: ", m_initValue->m_token );
        }

        return value;
    }

    return gNullObject;
}
}
