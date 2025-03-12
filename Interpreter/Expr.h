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


inline std::string getLine( const char* text, int lineNumber )
{
    if ( lineNumber == 0)
    {
        return std::string( text, getLineEnd(text) );
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
                return std::string( ptr+1, getLineEnd(ptr+1) );
            }
        }
    }
}

struct Namespace;

namespace expr {

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
    et_binary,
    et_identifier,
    et_int,
    et_float,
    et_string,
    et_assignment,
    et_func_call,
    et_namespace_op,
    et_return,

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
    
    virtual ObjectValue execute( Runtime&, bool isGlobal ) = 0;
};

struct HelpExpression: Expression
{
    HelpExpression( const Token& token ) : Expression(token) {};

    ObjectValue execute( Runtime&, bool isGlobal ) override { return gNullObject; }
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

inline void evalPrint( const std::string& lexeme )
{
    for( int i=0; i<gEvaluateOffset; i++ )
    {
        LOGX(' ')
    }
    LOG(lexeme)
}

inline void evalPrintOp2( const std::string& lexeme )
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
    
    virtual ObjectValue execute( Runtime& runtime, bool isGlobal ) override
    {
        ObjectValue result;
        for( auto& statement: m_list )
        {
            result = statement->execute( runtime, isGlobal );
            if ( result.m_isReturned )
            {
                return result;
            }
        }
        return result;
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
    
    virtual ObjectValue execute( Runtime& runtime, bool isGlobal ) override
    {
        for( auto* expr: m_list )
        {
            LOG("PrintFunc item: " << (void*)expr )
            ObjectValue value = expr->execute( runtime, isGlobal );
            LOG("PrintFunc item: " << value.pstring() )

            value.toStream( std::cout );
        }
       
        if ( m_withNewLine )
        {
            std::cout << std::endl;
        }
        return gNullObject;
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
    
    ObjectValue execute( Runtime& runtime, bool isGlobal ) override
    {
        ObjectValue value = m_expr->execute( runtime, isGlobal );

        switch (m_op) {
            case plus:
                break;
            case minus:
                if ( value.m_type == ot_int )
                {
                    value.m_intValue = -value.m_intValue;
                    break;
                }
                else if ( value.m_type == ot_double )
                {
                    value.m_doubleValue = -value.m_doubleValue;
                    break;
                }
                else if ( value.m_type == ot_bool )
                {
                    RUNTIME_EX3( "Cannot apply '-' to bool value: ", m_token );
                }
                else if ( value.m_type == ot_string )
                {
                    RUNTIME_EX3( "Cannot apply '-' to string value: ", m_token );
                }
            case negotiaton:
                if ( value.m_type == ot_bool )
                {
                    value.m_boolValue = -value.m_boolValue;
                    break;
                }
                else if ( value.m_type == ot_int )
                {
                    RUNTIME_EX3( "Cannot apply '!' to int value: ", m_token );
                }
                else if ( value.m_type == ot_double )
                {
                    RUNTIME_EX3( "Cannot apply '!' to double value: ", m_token );
                }
                else if ( value.m_type == ot_string )
                {
                    RUNTIME_EX3( "Cannot apply '-' to string value: ", m_token );
                }
                else if ( value.m_type == ot_null )
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
        return value;
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

    ObjectValue execute( Runtime& runtime, bool isGlobal ) override
    {
        ObjectValue value1 = m_expr->execute( runtime, isGlobal );
        if ( value1.isNull() )
        {
            if ( m_expr->type() == et_identifier )
            {
                RUNTIME_EX3( "undefined variable: ", m_expr->m_token )
            }
            RUNTIME_EX3( "unexpected null value: ", m_expr->m_token )
        }

        ObjectValue value2 = m_expr2->execute( runtime, isGlobal );
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
    ObjectValue value;\
    value.m_type = ot_int;\
    value.m_intValue = value1.m_intValue sign value2.m_intValue;\
    return value;

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
    ObjectValue value;\
    value.m_type = ot_double;\
    value.m_doubleValue = value1.doubleValue() sign value2.doubleValue();\
    return value;

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

struct VarDeclaration: public Expression
{
    VarDeclaration( const Token& token, const std::string& varType ) : Expression(token), m_identifierName(token.lexeme), m_type(varType) {}
    
    std::string   m_identifierName;
    std::string   m_type;
    Expression*   m_initValue;
    //TODO: Value
    
    ObjectValue execute( Runtime& runtime, bool isGlobal ) override
    {
        if ( m_initValue != nullptr )
        {
            ObjectValue value = m_initValue->execute(runtime, isGlobal);
            return value;
        }

        return gNullObject;
    }
};

struct Argument
{
    std::string m_name;
    std::string m_type;
    std::string m_defaultValue;
};

struct Identifier : public Expression
{
    Identifier( const Token& token ) : Expression(token), m_name(token.lexeme) {}

    const std::string&  m_name;
    std::string         m_type;
    
    virtual enum ExpressionType type() override { return et_identifier; }
    
    virtual void evaluate() override
    {
        evalPrint( m_name );
    }

    virtual void printExpr() override
    {
        LOGX( "'" << m_token.lexeme );
    }

    ObjectValue execute( Runtime& runtime, bool isGlobal ) override
    {
        LOG( "execute var: " << m_name )

        if ( isGlobal )
        {
            LOG( "execute var ???: " << m_name )
            if ( auto it = runtime.m_currentNamespace->m_variableMap.find(m_name); it != runtime.m_currentNamespace->m_variableMap.end() )
            {
                if ( auto it = runtime.m_currentNamespace->m_variableValueMap.find(m_name); it != runtime.m_currentNamespace->m_variableValueMap.end() )
                {
                    return it->second;
                }
            }
            else
            {
                auto* oldCurrentNamespace = runtime.m_currentNamespace;
                for(;;)
                {
                    LOG( "-- runtime.m_currentNamespace : " << runtime.m_currentNamespace->m_name )
                    if ( runtime.m_currentNamespace = runtime.m_currentNamespace->m_parentNamespace; runtime.m_currentNamespace == nullptr )
                    {
                        LOG( "-- break: runtime.m_currentNamespace"  )
                        break;
                    }
                    LOG( "-- after runtime.m_currentNamespace : " << runtime.m_currentNamespace->m_name )
                    if ( auto it = runtime.m_currentNamespace->m_variableValueMap.find(m_name); it != runtime.m_currentNamespace->m_variableValueMap.end() )
                    {
                        runtime.m_currentNamespace = oldCurrentNamespace;
                        return it->second;
                    }
                }
                runtime.m_currentNamespace = oldCurrentNamespace;

                throw runtime_error( "unknown variable: '" + m_token.lexeme + "'", m_token );
            }
        }
        else
        {
            for( auto localVarMapIt = runtime.m_localVarStack.rbegin();
                 localVarMapIt != runtime.m_localVarStack.rend();
                 localVarMapIt++ )
            {
                if ( auto it = localVarMapIt->find(m_name); it != localVarMapIt->end() )
                {
                    return it->second;
                }
            }
        }

        if ( auto it = runtime.m_globalVariableMap.m_namespaceGlobalVariableMap.find(m_name); it != runtime.m_globalVariableMap.m_namespaceGlobalVariableMap.end() )
        {
            return it->second;
        }
        
//        if ( runtime.m_currentNamespace->findVar(m_name) )
//        {
//            throw using_of_uninitialized_variable{ runtime.m_currentNamespace, runtime.m_currentNamespace.findVar(m_name) };
//        }
//        else
//        {
//            throw unknow_variable{};
//        }

        return gNullObject;
    }

};

struct IntNumber : public Expression
{
    int64_t         m_value;
    
    IntNumber( const Token& lexeme ) : Expression(lexeme)
    {
        m_value = std::stol( m_token.lexeme );
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
    
    ObjectValue execute( Runtime&, bool isGlobal ) override
    {
        ObjectValue value;
        value.m_type = ot_int;
        value.m_intValue = m_value;
        return value;
    }
};

struct FloatNumber : public Expression
{
    double          m_value;
    
    FloatNumber( const Token& lexeme ) : Expression(lexeme)
    {
        m_value = std::stod( m_token.lexeme );
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
    
    ObjectValue execute( Runtime&, bool isGlobal ) override
    {
        ObjectValue value;
        value.m_type = ot_double;
        value.m_doubleValue = m_value;
        return value;
    }
};

struct String : public Expression
{
    std::string m_value;
    
    String( const Token& lexeme ) : Expression(lexeme)
    {
        m_value = m_token.lexeme;
    }
    
    String( const Token& lexeme, const char* begin, const char* end ) : Expression(lexeme)
    {
        m_value = std::string( begin, end );
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
    
    ObjectValue execute( Runtime&, bool isGlobal ) override
    {
        ObjectValue value;
        value.m_type = ot_string;
        value.m_stringValue = new std::string{ m_value };
        return value;
    }

};

struct FuncDefinition : public Expression
{
    std::string             m_name;
    std::vector<Argument>   m_argList;
    ExpressionList          m_body;
    
    ObjectValue execute( Runtime& runtime, bool isGlobal ) override
    {
        auto result = runtime.m_topLevelNamespace.m_functionMap.emplace( m_name, this);
        if ( result.second )
        {
            // we added function
            return gNullObject;
        }

        // Insertion failed, meaning m_functionName already exists
        RUNTIME_EX2( std::string("Double function declaration: ") + m_name );
    }
};

struct ClassDefinition : public Expression
{
    struct BaseClassInfo
    {
        bool        m_isPrivate;
        std::string m_name;
    };
    
    struct VarInfo
    {
        bool                m_isPrivate;
        VarDeclaration*  m_var;
    };
    
    struct ConstructorInfo: FuncDefinition
    {
        bool                        m_isPrivate;
        std::vector<Expression*>    m_baseClassInitList;
    };

    struct FuncInfo
    {
        bool            m_isPrivate;
        FuncDefinition* m_var;
    };

    std::string                 m_name;
    std::vector<std::string>    m_outerClasses;

    std::vector<ConstructorInfo*>   m_constuctors;

    // members
    std::vector<BaseClassInfo>      m_baseClasses;
    std::vector<VarInfo>            m_vars;
    std::vector<FuncInfo>           m_funcs;
    
    std::map<std::string,ClassDefinition*> m_innerClasses;

    ClassDefinition( const Token& lexeme ) : Expression(lexeme), m_name(lexeme.lexeme) {}
    ClassDefinition( const Token& lexeme, const std::string& outerClassName, const std::vector<std::string>& outerClasses  )
        : Expression(lexeme),
          m_name(lexeme.lexeme)
    {
        m_outerClasses = std::vector<std::string>{ outerClasses };
        m_outerClasses.push_back( outerClassName );
    }

    ObjectValue execute( Runtime& runtime, bool isGlobal ) override
    {
    }
};

struct AssignmentStatement: public Expression
{
    std::string                 m_varName;
    expr::Expression*           m_expr = nullptr;
    
    AssignmentStatement( const Token& token ) : Expression(token), m_varName(token.lexeme) {}
    
    virtual enum ExpressionType type() override { return et_assignment; }
    
    ObjectValue execute( Runtime& runtime, bool isGlobal ) override
    {
        assert("todo");
    }
};

// Binary expression with unary operator
struct FunctionCall: public Expression
{
    std::string                 m_functionName;
    std::vector<Expression*>    m_parameters;

    FunctionCall( const Token& token ) : Expression(token), m_functionName(token.lexeme) {}
    
    virtual enum ExpressionType type() override { return et_func_call; }
    
    virtual void printExpr() override
    {
        LOGX( m_token.lexeme << "(" );
        for( auto& parameter: m_parameters )
        {
            LOGX( " ");
            parameter->printExpr();
            LOGX( ",");
        }
        LOGX( ")");
    }
    
    ObjectValue execute( Runtime& runtime, bool isGlobal ) override
    {
        Namespace* namespacePtr = &runtime.m_topLevelNamespace;
        return execute( runtime, namespacePtr, isGlobal );
    }

    ObjectValue execute( Runtime& runtime, Namespace* namespacePtr, bool isGlobal )
    {
        //TODO
        LOG("execute function?: " << m_functionName )
        if ( auto it = namespacePtr->m_functionMap.find( m_functionName ); it == runtime.m_topLevelNamespace.m_functionMap.end() )
        {
            RUNTIME_EX2( std::string("Undefined function: ") + m_functionName );
        }
        else
        {
            FuncDefinition* fDefinition = it->second;
            return execute( runtime, fDefinition, isGlobal );
        }
    }

    ObjectValue execute( Runtime& runtime, FuncDefinition* fDefinition, bool isGlobal )
    {
        runtime.m_localVarStack.push_back({});

        auto paramIt = m_parameters.begin();
        for( const auto& arg: fDefinition->m_argList )
        {
            if ( paramIt != m_parameters.end() )
            {
                ObjectValue paramValue = (*paramIt)->execute( runtime, isGlobal );
                LOG("execute function: " << m_functionName << ": arg: " << arg.m_name << " = " << paramValue.pstring() )
                auto result = runtime.m_localVarStack.back().emplace( arg.m_name, paramValue );
                paramIt++;
            }
            else
            {
                auto result = runtime.m_localVarStack.back().emplace( arg.m_name, ObjectValue{} );
            }
        }

        auto result = fDefinition->m_body.execute( runtime, isGlobal );
        result.m_isReturned = 0;

        runtime.m_localVarStack.pop_back();
        return result;
    }

};

struct FunctionCallWithNamespace: public FunctionCall
{
    std::vector<Token*>  m_namespaceSpec;

    FunctionCallWithNamespace( const Token& token, std::vector<Token*>&& namespaceSpec ) : FunctionCall(token), m_namespaceSpec(namespaceSpec) {}

    virtual enum ExpressionType type() override { return et_func_call; }

    virtual void printExpr() override
    {
        LOGX( m_token.lexeme << "(" );
        for( auto& parameter: m_parameters )
        {
            LOGX( " ");
            parameter->printExpr();
            LOGX( ",");
        }
        LOGX( ")");
    }

    ObjectValue execute( Runtime& runtime, bool isGlobal ) override
    {
        auto* fDefinition = runtime.m_currentNamespace->getFunctionDef( m_token.lexeme, m_namespaceSpec );
        if ( fDefinition == nullptr )
        {
            throw runtime_error( "unknow function: ", m_token );
        }
        FunctionCall::execute( runtime, fDefinition, isGlobal );
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
    
    ObjectValue execute( Runtime& runtime, bool isGlobal ) override
    {
        if ( m_returnValue != nullptr )
        {
            auto returnValue = m_returnValue->execute( runtime, isGlobal );
            returnValue.m_isReturned = 0xff;
            return returnValue;
        }
        return gNullObject;
    }
};

struct Print : public Expression
{
    ExpressionList  m_printParameters;
};

}
