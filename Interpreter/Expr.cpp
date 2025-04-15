#pragma once

#include "Expr.h"
#include "ClassObject.h"

namespace expr 
{
ObjectValue* VarDeclaration::execute( ObjectValue& outValue, Runtime& runtime, bool isGlobal )
{
    // var <var_name> : <type>
    if ( m_type != nullptr )
    {
        //todo: save type?
        if ( m_type->type == Identifier || m_type->type == IdentifierWithScope )
        {
            if ( m_type->lexeme == "Int" || m_type->lexeme == "Float" || m_type->lexeme == "String" )
            {

            }
            else
            {
                std::string_view identifierName;
                std::vector<std::string_view> namespaceSpec;
                parseIdentifierWithScope( *m_type, identifierName, namespaceSpec );

                auto* classDef = runtime.m_currentNamespace2->getClassDef( identifierName, namespaceSpec );
                if ( classDef == nullptr )
                {
                    throw runtime_ex3( "unknown type: " , *m_type );
                }
            }
        }
        else if ( m_type->type != Int && m_type->type != Float && m_type->type != String )
        {
            throw runtime_ex3( "unknown type: " , *m_type );
        }
    }

    // var <var_name> = <init_value>
    if ( m_initValue == nullptr )
    {
        return nullptr;
    }

    ClassOrNamespace*   classDef = nullptr;

    // Cases:
    // var v = <SomeClassName>;
    // var v = <VariableName>;
    // var v = <SomeClassName>(...);
    // var v = <FuncName>(...);
    // var v = <Literal>;

    if ( m_initValue->type() == et_func_call )
    {
        auto* funcCall = (FunctionCall*) m_initValue;

        classDef = runtime.m_currentNamespace2->getClassDef( funcCall->m_functionName, funcCall->m_namespaceSpec );
        if ( classDef != nullptr )
        {
            outValue = ::createClassObject( runtime, isGlobal, *classDef );
        }
    }
    // var x = <another_var>
    else if ( m_initValue->type() == et_identifier )
    {
        auto* identifier = (IdentifierExpr*) m_initValue;

        classDef = runtime.m_currentNamespace2->getClassDef( identifier->m_name, identifier->m_namespaceSpec );
        if ( classDef != nullptr )
        {
            outValue = ::createClassObject( runtime, isGlobal, *classDef );
        }
        else
        {
            auto* value = runtime.m_currentNamespace2->getVarValue( identifier->m_name, identifier->m_namespaceSpec );

            if ( value == nullptr )
            {
                LOG( "Unknown value: " << m_initValue->m_token.lexeme )
                throw runtime_ex3( "Unknown value: ", m_initValue->m_token );
            }
        }
    }
    else if ( m_initValue->type() == et_int ||
             m_initValue->type()  == et_float ||
             m_initValue->type()  == et_string ||
             m_initValue->type()  == et_unary ||
             m_initValue->type()  == et_binary )
    {
        m_initValue->execute( outValue, runtime, isGlobal );
    }

    return nullptr;
}

ObjectValue* DotExpr::execute( ObjectValue& outValue, Runtime& runtime, bool isGlobal )
{
    LOG( "m_token.type == Dot" );
    // <m_expr2>.<m_expr>
    m_expr2->execute( outValue, runtime, isGlobal );
    if ( outValue.m_type != ot_class_ptr )
    {
        throw runtime_error( runtime, "cannot applay operator '.' ", m_token );
    }
    else
    {
        ClassObject* obj = outValue.m_classObjPtr;

        if ( m_expr->type() == et_identifier )
        {
            IdentifierExpr* id = (IdentifierExpr*) m_expr;
            if ( auto it = obj->m_members.find( id->m_name ); it == obj->m_members.end() )
            {
                auto message = "no member named '" + std::string(id->m_name) + "' in '" + std::string(obj->m_definition->m_name) + "'";
                throw runtime_error( runtime, message , m_expr->m_token );
            }
            else
            {
                outValue = it->second;
                return &it->second;
            }
        }
        else if ( m_expr->type() == et_func_call )
        {
            throw runtime_error( runtime, "todo", m_expr->m_token );
        }

    }
    return nullptr;
}

}
