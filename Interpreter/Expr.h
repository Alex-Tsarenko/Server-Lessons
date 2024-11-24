#pragma once

#include <forward_list>
#include <list>
#include <vector>
#include "Object.h"

class Executor;

const char* getLineEnd( const char* text )
{
    for( const char* ptr = text; ; ptr++ )
    {
        if ( *ptr == '\n' || *ptr == '\r' || *ptr == 0 )
        {
            return ptr;
        }
    }
}


std::string getLine( const char* text, int lineNumber )
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
            return "";
        }
        
        if ( *ptr == '\n' || *ptr == '\r' )
        {
            number++;
            while( *ptr == '\n' || *ptr == '\r' )
            {
                ptr++;
                if ( *ptr == 0 )
                {
                    return "";
                }
            }
            if ( number == lineNumber )
            {
                return std::string( ptr, getLineEnd(ptr) );
            }
        }
    }
}

namespace expr {

inline const Token& nilToken{ EndOfTokenType };

struct Expression
{
    const Token& m_token;

    Expression() : m_token(nilToken) {
        
    };
    Expression( const Token& token ) : m_token(token) {};
    virtual ~Expression() {
        
    };
    
    virtual void evaluate()
    {
        LOG("evaluate: NIL");
    }
};

//using ExpressionPtr = Expression*;

// It is used for auto-deleting of "Expression"
struct ExpressionPtr
{
    ExpressionPtr() : m_expr(nullptr) {}

    ExpressionPtr( Expression* expr ) : m_expr(expr) {}

    ExpressionPtr( ExpressionPtr&& exprPtr ) : m_expr(exprPtr.m_expr)
    {
        exprPtr.m_expr = nullptr;
    }
    
    ExpressionPtr& operator=( ExpressionPtr&& exprPtr )
    {
        m_expr = exprPtr.m_expr;
        if ( this != &exprPtr )
        {
            exprPtr.m_expr = nullptr;
        }
    }

    ~ExpressionPtr() {
        delete m_expr;
    }
    
    Expression* m_expr = nullptr;
};

// Unary expression with unary operator
struct UnaryExpression: public Expression
{
    enum Operator { plus, minus, negation, star, ampersand };
    
    Operator       m_op;
    Expression*    m_expr;
};

// Binary expression with unary operator
struct BinaryOpExpression: public Expression
{
    BinaryOpExpression( const Token& token ) : Expression(token) {}
    Expression*  m_expr;
    Expression*  m_expr2;
};

struct UnaryOpExpression: public Expression
{
    UnaryOpExpression( const Token& token ) : Expression(token) {}
    Expression*  m_expr;
};

struct ExpressionVarDecl: public Expression
{
    ExpressionVarDecl( const Token& token ) : Expression(token), m_identifierName(token.lexeme) {}
    
    std::string   m_identifierName;
    Expression*   m_expr;
};


// Binary expression with unary operator
struct FunctionCall: public Expression
{
    std::string                 m_functionName;
    std::vector<Expression*>    m_parameters;
};

struct ExpressionList: public Expression
{
    std::list<Expression*>  m_list;
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
};

struct IntNumber : public Expression
{
    IntNumber( const Token& lexeme ) : Expression(lexeme)
    {
        m_value = std::stol( m_token.lexeme );
    }

    int64_t         m_value;
};

struct FloatNumber : public Expression
{
    FloatNumber( const Token& lexeme ) : Expression(lexeme)
    {
        m_value = std::stod( m_token.lexeme );
    }

    double          m_value;
};

struct Func : public Expression
{
    std::string             m_name;
    std::vector<Argument>   m_argList;
    ExpressionList          m_body;
};

struct If : public Expression
{
    ExpressionPtr           m_condition;
    ExpressionList          m_yesBlock;
    ExpressionList          m_noBlock;
};

struct VariableDefinition : public Expression
{
    std::string     m_name;
    std::string     m_type;
    ExpressionPtr   m_value;
};

struct For : public Expression
{
    std::string     m_iteratorName;
    ExpressionPtr   m_condition;
    ExpressionPtr   m_getNext;
};

struct Return : public Expression
{
    ExpressionPtr   m_returnValue;
};

struct Print : public Expression
{
    ExpressionList  m_printParameters;
};

}
