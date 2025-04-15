#pragma once

#include <forward_list>
#include <list>
#include <vector>
#include <assert.h>

#include "ObjectValue.h"

#include "Token.h"
#include "Runtime.h"
#include "Error.h"
#include "Logs.h"

inline const char* getLineEnd( const char* text )
{
    for( const char* ptr = text; ; ptr++ )
    {
        if ( *ptr == '\n' || *ptr == '\r' || *ptr == 0 )
        {
            return ptr;
        }
    }
}


inline std::string_view getLine( const char* text, int lineNumber )
{
    if ( lineNumber == 0)
    {
        return std::string_view( text, getLineEnd(text) );
    }
    
    int number = 0;
    for( const char* ptr = text; *ptr != 0; ptr++ )
    {
        if ( *ptr == 0 )
        {
            return "<eof>";
        }
        
        if ( *ptr == '\n' )
        {
            number++;

            //todo '\r'
            if ( number == lineNumber )
            {
                return std::string_view( ptr+1, getLineEnd(ptr+1) );
            }
        }
    }
}

struct Namespace;

namespace expr {

void parseIdentifierWithScope( const Token& token, std::string_view& identifierName, std::vector<std::string_view>& namespaceSpec );

struct VarDeclaration;

struct using_of_uninitialized_variable: public std::runtime_error
{
    Namespace*      m_namespace;
    VarDeclaration* m_varDecl;
};

struct unknow_variable: public std::runtime_error
{
};

struct unknow_function: public std::runtime_error
{
};


inline const Token& nilToken{ NilToken, "NilToken" };

enum ExpressionType
{
    et_unary,
    et_dot,
    et_binary,
    et_identifier,
    et_int,
    et_float,
    et_string,
    et_assignment,
    et_func_call,
    et_namespace_op,
    et_return,

    et_var_decl,

    et_undefined
};

struct Expression
{
    const Token& m_token;

    Expression() : m_token(nilToken) {

    };
    Expression( const Token& token ) : m_token(token) {};

    virtual ~Expression() {

    };


    virtual enum ExpressionType type() { return et_undefined; }

    virtual void evaluate()
    {
        LOG("<Expression evaluate: NIL>");
    }
    virtual void printExpr()
    {
        LOG("<Expression evaluate: NIL>");
    }

    virtual ObjectValue* execute( ObjectValue& outValue, Runtime&, bool isGlobal ) = 0;
};

}
#include "ClassOrNamespace.h"
namespace expr
{

// Dijkstra
struct HelpExpression: Expression
{
    HelpExpression( const Token& token ) : Expression(token) {};

    ObjectValue* execute( ObjectValue& outValue, Runtime&, bool isGlobal ) override { return nullptr; }
};

inline int gEvaluateOffset = 0;

inline void evalPrint( Expression* ptr )
{
    gEvaluateOffset += 2;
    
    for( int i=0; i<gEvaluateOffset; i++ )
    {
        LOGX(' ')
    }
    if ( ptr == nullptr )
    {
        LOGX("<expr:nullptr>")
    }
    else
    {
        ptr->evaluate();
    }
    
    gEvaluateOffset -= 2;
}

inline void evalPrint( const std::string_view& lexeme )
{
    for( int i=0; i<gEvaluateOffset; i++ )
    {
        LOGX(' ')
    }
    LOG(lexeme)
}

inline void evalPrintOp2( const std::string_view& lexeme )
{
    for( int i=0; i<gEvaluateOffset; i++ )
    {
        LOGX(' ')
    }
    LOG(lexeme)
}

inline void evalPrint( int64_t number )
{
    for( int i=0; i<gEvaluateOffset; i++ )
    {
        LOGX(' ')
    }
    LOG(number)
}

inline void evalPrint( double number )
{
    for( int i=0; i<gEvaluateOffset; i++ )
    {
        LOGX(' ')
    }
    LOG(number)
}


// Block
struct ExpressionList: public Expression
{
    std::list<Expression*>  m_list;
    
    virtual ObjectValue* execute( ObjectValue& outValue, Runtime& runtime, bool isGlobal ) override
    {
        ObjectValue& result = outValue;
        for( auto& statement: m_list )
        {
            LOG("statement->m_token: " << statement->m_token.lexeme )
            statement->execute( result, runtime, isGlobal );
            if ( result.m_isReturned )
            {
                return;
            }

            if ( statement->type() == et_var_decl )
            {
                auto pair = runtime.m_localVarStack.back().emplace( statement->m_token.lexeme, result );
                if ( not pair.second )
                {
                    throw runtime_ex( "duplicate local variable: " + std::string(statement->m_token.lexeme) );
                }
            }
        }
        return;
    }
};

struct PrintFuncCall: public Expression
{
    bool m_withNewLine = false;
    
    PrintFuncCall( std::vector<Expression*>&&  list )
    {
        m_list = std::move(list);
    }
    std::vector<Expression*>  m_list;
    
    virtual ObjectValue* execute( ObjectValue& outValue, Runtime& runtime, bool isGlobal ) override
    {
        for( auto* expr: m_list )
        {
            //LOG("PrintFunc item: " << (void*)expr )
            expr->execute( outValue, runtime, isGlobal );
            //LOG("PrintFunc item: " << outValue.pstring() )

            outValue.toStream( std::cout );
        }
       
        if ( m_withNewLine )
        {
            std::cout << std::endl;
        }
        return;
    }
};

// Unary expression with unary operator
struct UnaryExpression: public Expression
{
    enum Operator { plus, minus, negotiaton, star, ampersand };
    
    virtual enum ExpressionType type() override { return et_unary; }

    Operator       m_op;
    Expression*    m_expr = nullptr;

    UnaryExpression( const Token& token ) : Expression(token) {}

    virtual void evaluate() override
    {
        evalPrint( m_token.lexeme );
        evalPrint( m_expr );
    }
    virtual void printExpr() override
    {
        LOGX( "(" << m_token.lexeme << " " );
        m_expr->printExpr();
        LOGX( ")");
    }
    
    ObjectValue* execute( ObjectValue& outValue, Runtime& runtime, bool isGlobal ) override
    {
        m_expr->execute( outValue, runtime, isGlobal );

        switch (m_op) {
            case plus:
                break;
            case minus:
                if ( outValue.m_type == ot_int )
                {
                    outValue.m_intValue = -outValue.m_intValue;
                    break;
                }
                else if ( outValue.m_type == ot_double )
                {
                    outValue.m_doubleValue = -outValue.m_doubleValue;
                    break;
                }
                else if ( outValue.m_type == ot_bool )
                {
                    RUNTIME_EX3( "Cannot apply '-' to bool value: ", m_token );
                }
                else if ( outValue.m_type == ot_string )
                {
                    RUNTIME_EX3( "Cannot apply '-' to string value: ", m_token );
                }
            case negotiaton:
                if ( outValue.m_type == ot_bool )
                {
                    outValue.m_boolValue = -outValue.m_boolValue;
                    break;
                }
                else if ( outValue.m_type == ot_int )
                {
                    RUNTIME_EX3( "Cannot apply '!' to int value: ", m_token );
                }
                else if ( outValue.m_type == ot_double )
                {
                    RUNTIME_EX3( "Cannot apply '!' to double value: ", m_token );
                }
                else if ( outValue.m_type == ot_string )
                {
                    RUNTIME_EX3( "Cannot apply '-' to string value: ", m_token );
                }
                else if ( outValue.m_type == ot_null )
                {
                    RUNTIME_EX3( "Cannot apply '-' to null: ", m_token );
                }
            case star:
                RUNTIME_EX3( "'*' not supported: ", m_token );
            case ampersand:
                RUNTIME_EX3( "'&' not supported: ", m_token );

            default:
                break;
        }
        return nullptr;
    }
};

// Binary expression with unary operator
struct BinaryOpExpression: public Expression
{
    BinaryOpExpression( const Token& token ) : Expression(token)
    {
    }
    Expression*  m_expr  = nullptr;
    Expression*  m_expr2 = nullptr;
    
    virtual enum ExpressionType type() override { return et_binary; }

    virtual void evaluate() override
    {
        evalPrintOp2( m_token.lexeme );
        evalPrint( m_expr2 );
        evalPrint( m_expr );
    }
    
    virtual void printExpr() override
    {
        LOGX( "(" );
        m_expr2->printExpr();
        LOGX( " " << m_token.lexeme << " " );
        m_expr->printExpr();
        LOGX( ")");
    }

    ObjectValue* execute( ObjectValue& outValue, Runtime& runtime, bool isGlobal ) override
    {
        ObjectValue value1;
        m_expr->execute( value1, runtime, isGlobal );
        if ( value1.isNull() )
        {
            if ( m_expr->type() == et_identifier )
            {
                RUNTIME_EX3( "undefined variable: ", m_expr->m_token )
            }
            RUNTIME_EX3( "unexpected null value: ", m_expr->m_token )
        }

        ObjectValue value2;
        m_expr2->execute( value2, runtime, isGlobal );
        if ( value2.isNull() )
        {
            if ( m_expr2->type() == et_identifier )
            {
                RUNTIME_EX3( "undefined variable: ", m_expr2->m_token )
            }
            RUNTIME_EX3( "unexpected null value: ", m_expr2->m_token )
        }

        if ( value1.m_type == ot_int && value2.m_type == ot_int )
        {
#define RETURN_INT_VALUE_OF(sign) \
    outValue.m_type = ot_int;\
    outValue.m_intValue = value1.m_intValue sign value2.m_intValue;\
    return nullptr;

            if ( m_token.type == Plus )
            {
                RETURN_INT_VALUE_OF(+)
            }
            if ( m_token.type == Minus )
            {
                RETURN_INT_VALUE_OF(-)
            }
            if ( m_token.type == Slash )
            {
                RETURN_INT_VALUE_OF(/)
            }
            if ( m_token.type == Star )
            {
                RETURN_INT_VALUE_OF(*)
            }
        }
        if ( (value1.m_type == ot_int || value1.m_type == ot_double) || (value2.m_type == ot_int || value2.m_type == ot_double) )
        {
#define RETURN_DOUBLE_VALUE_OF(sign) \
    outValue.m_type = ot_double;\
    outValue.m_doubleValue = value1.doubleValue() sign value2.doubleValue();\
    return nullptr;

            if ( m_token.type == Plus )
            {
                RETURN_DOUBLE_VALUE_OF(+)
            }
            if ( m_token.type == Minus )
            {
                RETURN_DOUBLE_VALUE_OF(-)
            }
            if ( m_token.type == Slash )
            {
                RETURN_DOUBLE_VALUE_OF(/)
            }
            if ( m_token.type == Star )
            {
                RETURN_DOUBLE_VALUE_OF(*)
            }
        }
        LOG( "????");
    }
};

struct DotExpr: public BinaryOpExpression
{
    DotExpr( const Token& token ) : BinaryOpExpression(token)
    {
    }

    virtual enum ExpressionType type() override { return et_dot; }

    ObjectValue* execute( ObjectValue& outValue, Runtime& runtime, bool isGlobal ) override;
};

struct VarDeclaration: public Expression
{
    VarDeclaration( const Token& token, const Token* varType ) : Expression(token), m_identifierName(token.lexeme), m_type(varType) {}

    bool               m_isPrivate = false;
    bool               m_isStatic = false;

    std::string_view   m_identifierName;
    const Token*       m_type;
    Expression*        m_initValue;
    //TODO: Value
    
    virtual enum ExpressionType type() override { return et_var_decl; }

    ObjectValue* execute( ObjectValue& outValue, Runtime& runtime, bool isGlobal ) override;

};

struct Argument
{
    std::string_view m_name;
    std::string_view m_type;
//    std::string_view m_defaultValue;
};


inline void parseIdentifierWithScope( const Token& token, std::string_view& identifierName, std::vector<std::string_view>& namespaceSpec )
{
    LOG( "??: " << token.lexeme );
    bool scopeOp = false;
    auto end = token.lexeme.data() + token.lexeme.size();
    for( const char* ptr = token.lexeme.data(); ptr < end; ptr++ )
    {
        if ( *ptr == ':' )
        {
            assert( *(ptr+1) == ':' );
            assert( !scopeOp );
            ptr++;
            if ( namespaceSpec.empty() )
            {
                // empty namespace name
                namespaceSpec.push_back( std::string_view{ ptr, 0 } );
            }
            scopeOp = true;
        }
        else
        {
            assert( scopeOp || namespaceSpec.empty() );

            auto* begin = ptr;
            while( ptr < end and ( isalpha(*ptr) or *ptr == '_' or isdigit(*ptr) ) )
            {
                ptr++;
            }
            assert( begin < ptr );

            if ( ptr==end )
            {
                identifierName = std::string_view{ begin, ptr };
            }
            else
            {
                namespaceSpec.push_back( std::string_view{ begin, ptr } );
                ptr--;
            }

            scopeOp = false;
        }
    }
}

struct IdentifierExpr : public Expression
{
    std::string_view     m_name;
    std::string_view     m_type;

    std::vector<std::string_view>   m_namespaceSpec;

    IdentifierExpr( const Token& token ) : Expression(token)
    {
        if ( token.type == IdentifierWithScope )
        {
            parseIdentifierWithScope( token, m_name, m_namespaceSpec );
        }
        else
        {
            m_name = token.lexeme;
        }
    }

    virtual enum ExpressionType type() override { return et_identifier; }
    
    virtual void evaluate() override
    {
        evalPrint( m_name );
    }

    virtual void printExpr() override
    {
        LOGX( "'" << m_token.lexeme );
    }

    ObjectValue* execute( ObjectValue& outValue, Runtime& runtime, bool isGlobal ) override
    {
        LOG( "execute var: " << m_name )

        if ( isGlobal )
        {
            LOG( "execute var ???: " << m_name )
            if ( auto it = runtime.m_currentNamespace2->m_variableMap.find(m_name); it != runtime.m_currentNamespace2->m_variableMap.end() )
            {
                if ( auto it = runtime.m_currentNamespace2->m_variableValueMap.find(m_name); it != runtime.m_currentNamespace2->m_variableValueMap.end() )
                {
                    outValue = it->second;
                    return &it->second;
                }
            }
            else
            {
                // Iterate throw namespace stack
                auto* oldCurrentNamespace = runtime.m_currentNamespace2;
                for(;;)
                {
                    LOG( "-- runtime.m_currentNamespace : " << runtime.m_currentNamespace2->m_name )
                    if ( runtime.m_currentNamespace2 = runtime.m_currentNamespace2->m_parent; runtime.m_currentNamespace2 == nullptr )
                    {
                        LOG( "-- break: runtime.m_currentNamespace"  )
                        break;
                    }
                    LOG( "-- after runtime.m_currentNamespace : " << runtime.m_currentNamespace2->m_name )
                    if ( auto it = runtime.m_currentNamespace2->m_variableValueMap.find(m_name); it != runtime.m_currentNamespace2->m_variableValueMap.end() )
                    {
                        runtime.m_currentNamespace2 = oldCurrentNamespace;
                        outValue = it->second;
                        return &it->second;
                    }
                }
                runtime.m_currentNamespace2 = oldCurrentNamespace;

                throw runtime_error( runtime, "unknown variable: '" + std::string(m_token.lexeme) + "'", m_token );
            }
        }
        else
        {
            if ( auto* localVarValue = runtime.getLocalVarValue( m_name ); localVarValue != nullptr )
            {
                outValue = *localVarValue;
                return localVarValue;
            }

//            for( auto localVarMapIt = runtime.m_localVarStack.rbegin();
//                 localVarMapIt != runtime.m_localVarStack.rend();
//                 localVarMapIt++ )
//            {
//                if ( auto it = localVarMapIt->find(m_name); it != localVarMapIt->end() )
//                {
//                    outValue = it->second;
//                    return &it->second;
//                }
//            }

            // Iterate throw namespace stack
            auto* oldCurrentNamespace = runtime.m_currentNamespace2;
            for(;;)
            {
                LOG( "-- runtime.m_currentNamespace : " << runtime.m_currentNamespace2->m_name )
                if ( runtime.m_currentNamespace2 = runtime.m_currentNamespace2->m_parent; runtime.m_currentNamespace2 == nullptr )
                {
                    LOG( "-- break: runtime.m_currentNamespace"  )
                    break;
                }
                LOG( "-- after runtime.m_currentNamespace : " << runtime.m_currentNamespace2->m_name )
                if ( auto it = runtime.m_currentNamespace2->m_variableValueMap.find(m_name); it != runtime.m_currentNamespace2->m_variableValueMap.end() )
                {
                    runtime.m_currentNamespace2 = oldCurrentNamespace;
                    outValue = it->second;
                    return &it->second;
                }
            }
            runtime.m_currentNamespace2 = oldCurrentNamespace;

        }

        if ( auto* value = runtime.m_currentNamespace2->getVarValue(m_name, m_namespaceSpec); value != nullptr )
        {
            outValue = *value;
            return value;
        }

        if ( auto* classDef = runtime.m_currentNamespace2->getClassDef(m_name, m_namespaceSpec); classDef != nullptr )
        {
            outValue = createClassObject( runtime, isGlobal, *classDef );
            return &outValue;
        }

        LOG( "unknown variable: " << m_token.lexeme )
        throw runtime_error( runtime, "unknown variable: '" + std::string(m_token.lexeme) + "'", m_token );
        return nullptr;
    }

};

struct IntNumber : public Expression
{
    int64_t         m_value;
    
    IntNumber( const Token& lexeme ) : Expression(lexeme)
    {
        m_value = std::stol( std::string(m_token.lexeme) );
    }
    
    virtual enum ExpressionType type() override { return et_int; }

    virtual void evaluate() override
    {
        evalPrint( m_value );
    }
    
    virtual void printExpr() override
    {
        LOGX( m_value );
    }
    
    ObjectValue* execute( ObjectValue& outValue, Runtime&, bool isGlobal ) override
    {
        outValue.m_type = ot_int;
        outValue.m_intValue = m_value;
        return nullptr;
    }
};

struct FloatNumber : public Expression
{
    double          m_value;
    
    FloatNumber( const Token& lexeme ) : Expression(lexeme)
    {
        m_value = std::stod( std::string(m_token.lexeme) );
    }
    
    virtual enum ExpressionType type() override { return et_float; }
    
    virtual void evaluate() override
    {
        evalPrint( m_value );
    }
    
    virtual void printExpr() override
    {
        LOGX( m_value );
    }
    
    ObjectValue* execute( ObjectValue& outValue, Runtime&, bool isGlobal ) override
    {
        outValue.m_type = ot_double;
        outValue.m_doubleValue = m_value;
        return nullptr;
    }
};

struct StringExpr : public Expression
{
    std::string_view m_value;

    StringExpr( const Token& lexeme ) : Expression(lexeme)
    {
        m_value = m_token.lexeme;
    }
    
    StringExpr( const Token& lexeme, const char* begin, const char* end ) : Expression(lexeme)
    {
        m_value = std::string_view( begin, end );
    }
    
    virtual enum ExpressionType type() override { return et_string; }

    virtual void evaluate() override
    {
        evalPrint( m_value );
    }
    
    virtual void printExpr() override
    {
        LOGX( m_value );
    }
    
    ObjectValue* execute( ObjectValue& outValue, Runtime&, bool isGlobal ) override
    {
        outValue.m_type = ot_string;
        outValue.m_stringValue = new std::string{ m_value };
        return nullptr;
    }

};

struct FuncDefinition : public Expression
{
    bool                    m_isPrivate = false;
    bool                    m_isStatic = false;
    std::string_view        m_name;
    std::vector<Argument>   m_argList;
    ExpressionList          m_body;
    ClassOrNamespace*       m_whereFuctionWasDefined = nullptr;

    ObjectValue* execute( ObjectValue& outValue, Runtime& runtime, bool isGlobal ) override
    {
        auto result = runtime.m_topLevelNamespace.m_functionMap.emplace( m_name, this);
        if ( result.second )
        {
            // we added function
            return nullptr;
        }

        // Insertion failed, meaning m_functionName already exists
        RUNTIME_EX2( std::string("Double function declaration: ") + std::string(m_name) );
    }
};

struct ConstructorInfo: FuncDefinition
{
    bool                        m_isPrivate;
    std::vector<Expression*>    m_baseClassInitList;
};

struct ClassDefinition : public Expression
{
    struct BaseClassInfo
    {
        bool             m_isPrivate;
        std::string_view m_name;
    };
    
    struct ConstructorInfo: FuncDefinition
    {
        bool                        m_isPrivate;
        std::vector<Expression*>    m_baseClassInitList;
    };

    std::vector<ConstructorInfo*>   m_constuctors;

    // members
    std::vector<BaseClassInfo>      m_baseClasses;
    
    std::map<std::string_view,ClassDefinition*> m_innerClasses;

    ClassDefinition( const Token& lexeme ) : Expression(lexeme)/*, m_name(lexeme.lexeme)*/ {}
    ClassDefinition( const Token& lexeme, const std::string_view& outerClassName, const std::vector<std::string_view>& outerClasses  )
        : Expression(lexeme)
    {
    }

    ObjectValue* execute( ObjectValue& outValue, Runtime& runtime, bool isGlobal ) override
    {
    }
};

struct AssignmentStatement: public Expression
{
    expr::Expression*           m_left = nullptr;
    expr::Expression*           m_right = nullptr;

    AssignmentStatement( const Token& token, expr::Expression* left, expr::Expression* right ) 
    : Expression(token), m_left(left), m_right(right)
    {
    }

    virtual enum ExpressionType type() override { return et_assignment; }
    
    ObjectValue* execute( ObjectValue& outValue, Runtime& runtime, bool isGlobal ) override
    {
        auto* left = m_left->execute( outValue, runtime, isGlobal );
        if ( left == nullptr )
        {
            throw runtime_ex3( "expression is not assignable: ", m_left->m_token);
        }
        auto* retValue = m_right->execute( outValue, runtime, isGlobal );
        //TODO:
        *left = outValue;
        return retValue;
    }
};

// Binary expression with unary operator
struct FunctionCall: public Expression
{
    std::string_view            m_functionName;
    std::vector<Expression*>    m_parameters;

    std::vector<std::string_view>   m_namespaceSpec;

    FunctionCall( const Token& token ) : Expression(token)
    {
        if ( token.type == IdentifierWithScope )
        {
            parseIdentifierWithScope( token, m_functionName, m_namespaceSpec );
        }
        else
        {
            m_functionName = token.lexeme;
        }
    }

    virtual enum ExpressionType type() override { return et_func_call; }
    
    virtual void printExpr() override
    {
        LOGX( m_token.lexeme << "(" );
        bool atFirst = true;
        for( auto& parameter: m_parameters )
        {
            if ( atFirst )
            {
                atFirst = false;
            }
            else
            {
                LOGX( ",");
            }
            LOGX( " ");
            parameter->printExpr();
        }
        LOGX( ")");
    }
    
    ObjectValue* execute( ObjectValue& outValue, Runtime& runtime, bool isGlobal ) override
    {
        FuncDefinition* funcDef = nullptr;

        // ::func()
        funcDef = runtime.m_currentNamespace2->getFunctionDef( m_functionName, m_namespaceSpec );

        if ( funcDef == nullptr )
        {
            throw runtime_error( runtime, "unknow function: ", m_token );
        }

        auto oldCurrentNamespace = runtime.m_currentNamespace2;
        runtime.m_currentNamespace2 = funcDef->m_whereFuctionWasDefined;
        //{
            auto result = FunctionCall::doExecute( outValue, runtime, funcDef, isGlobal );
        //}
        runtime.m_currentNamespace2 = oldCurrentNamespace;

        return result;
    }

    ObjectValue* doExecute( ObjectValue& outValue, Runtime& runtime, FuncDefinition* fDefinition, bool isGlobal )
    {
        runtime.m_localVarStack.push_back({});

        auto paramIt = m_parameters.begin();
        for( const auto& arg: fDefinition->m_argList )
        {
            if ( paramIt != m_parameters.end() )
            {
                ObjectValue paramValue;
                (*paramIt)->execute( paramValue, runtime, isGlobal );
                LOG("execute function: " << m_functionName << ": arg: " << arg.m_name << " = " << paramValue.pstring() )
                runtime.m_localVarStack.back().emplace( arg.m_name, paramValue );
                paramIt++;
            }
            else
            {
                runtime.m_localVarStack.back().emplace( arg.m_name, ObjectValue{} );
            }
        }

        auto result = fDefinition->m_body.execute( outValue, runtime, isGlobal );
        outValue.m_isReturned = 0;

        runtime.m_localVarStack.pop_back();
        return result;
    }

};

struct If : public Expression
{
    Expression*             m_condition;
    ExpressionList          m_yesBlock;
    ExpressionList          m_noBlock;
};

//struct VarDefExpression : public Expression
//{
//    std::string     m_name;
//    std::string     m_type;
//    Expression*     m_value;
//};

struct For : public Expression
{
    std::string     m_iteratorName;
    Expression*     m_condition;
    Expression*     m_getNext;
};

struct Return : public Expression
{
    Expression*     m_returnValue;
    
    Return( Expression* returnValue ) : m_returnValue(returnValue) {}
    
    ObjectValue* execute( ObjectValue& outValue, Runtime& runtime, bool isGlobal ) override
    {
        if ( m_returnValue != nullptr )
        {
            auto returnValue = m_returnValue->execute( outValue, runtime, isGlobal );
            returnValue->m_isReturned = 0xff;
            return returnValue;
        }
        return nullptr;
    }
};

struct Print : public Expression
{
    ExpressionList  m_printParameters;
};

}

