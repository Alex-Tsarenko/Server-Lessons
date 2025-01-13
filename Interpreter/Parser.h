#pragma once

#include "Expr.h"
#include "Runtime.h"
#include <stack>
#include <cassert>

struct syntax_error: public std::runtime_error
{
    std::string m_error;
    int         m_line;
    int         m_position;
    int         m_endPosition;

    syntax_error( const std::string& error, int line, int position, int end ) : std::runtime_error(error),
      m_error(error),
      m_line(line),
      m_position(position),
      m_endPosition(end)
    {}

//    runtime_error(const runtime_error&);
//    runtime_error& operator=(const runtime_error&);

    ~syntax_error() override {}
};

struct Parser
{
    Runtime&    m_runtime;
    
    std::vector<expr::Expression*>    m_program;
    std::vector<expr::Expression*>*   m_current;

    std::vector<Token>::const_iterator m_tokenIt;
    std::vector<Token>::const_iterator m_tokenEnd;
    
    int m_blockLevel = 0;
    
    std::vector<expr::Expression*>      m_output;
    std::vector<expr::Expression*>      m_operationStack;
    std::vector<expr::FunctionCall*>      m_funcStack;
    
    bool unariOperatorTokenTable[EndOfTokenType] = {false};
    char operatorTokenTable[EndOfTokenType]      = {0};

    expr::HelpExpression leftParen{ *new Token{LeftParen,"("}};
    expr::HelpExpression comma{ *new Token{Comma,","}};
    expr::HelpExpression leftFuncParen{ *new Token{LeftParen,"f("}};
    expr::HelpExpression rightParen{ *new Token{RightParen,")"}};;
    
    std::vector<Lexer*> m_nestedLexers;
    
public:
    Parser( Runtime& runtime ) : m_runtime(runtime)
    {
    }

    ~Parser()
    {
        for( auto* lexer: m_nestedLexers )
        {
            delete lexer;
        }
    }

    void onError( std::string errorText, int line, int pos ) {}

    expr::Expression* parsePrintExpr( const char* exprString, const std::vector<Token>& tokens )
    {
        m_tokenIt  = tokens.begin();
        m_tokenEnd = tokens.end();

        m_current = &m_program;
        
        assert( tokens.size() > 0 );
        assert( tokenIs(Begin) );

        try
        {
            return parseExpr( RightParen );
        }
        catch( syntax_error& error )
        {
            std::cerr << "!!! Syntax error: " << error.what() << std::endl;
            std::cerr << getLine( exprString, error.m_line+1 ) << std::endl;
            for( int i=0; i<error.m_position-1; i++ )
            {
                std::cerr << ' ';
            }
            for( int i=error.m_position; i<error.m_endPosition; i++ )
            {
                std::cerr << '^';
            }
            std::cerr << std::endl;
        }
    }

    void parseProgram( const char* programText, const std::vector<Token>& tokens )
    {
        m_tokenIt  = tokens.begin();
        m_tokenEnd = tokens.end();

        m_current = &m_program;
        
        assert( tokens.size() > 0 );
        assert( tokenIs(Begin) );
        nextToken();

        try
        {
            for(;;)
            {
                skipNewLines();
                if ( isEof() )
                {
                    break;
                }
                //LOG("+it::: " << gTokenTypeStrings[m_tokenIt->type] )
                m_current->push_back( parseStatement() );
            }
        }
        catch( syntax_error& error )
        {
            std::cerr << "!!! Syntax error: " << error.what() << std::endl;
            std::cerr << getLine( programText, error.m_line+1 ) << std::endl;
            for( int i=0; i<error.m_position-1; i++ )
            {
                std::cerr << ' ';
            }
            for( int i=error.m_position; i<error.m_endPosition; i++ )
            {
                std::cerr << '^';
            }
            std::cerr << std::endl;
        }
    }

protected:

    expr::Expression* parseStatement()
    {
        auto* exprList = new expr::ExpressionList{};
        
        skipNewLines();
        
        //TODO ')' ',' ';'
        if ( isEof() )
        {
            //if ( m_blockLevel != 0 )
            {
                throw syntax_error( "unexpected end of file, expected '}'", (m_tokenIt-1)->line, (m_tokenIt-1)->pos, (m_tokenIt-1)->endPos );
            }
            return nullptr;
        }

        LOG( "m_tokenIt->type: " << gTokenTypeStrings[m_tokenIt->type] )
        switch ( m_tokenIt->type ) {
            case Var:
                exprList->m_list.push_back( parseVar() );
                break;
            case Func:
                exprList->m_list.push_back(  parseFuncDef() );
                break;
            case If:
                exprList->m_list.push_back(  parseIf() );
                break;
            case For:
                assert(0);
                break;
            case Return:
                exprList->m_list.push_back(  parseReturn() );
                break;
            case Print:
                exprList->m_list.push_back(  parsePrint() );
                break;
            case ClassT:
                exprList->m_list.push_back(  parseClass() );
                break;

            case RightBrace:
                if ( m_blockLevel == 0 )
                {
                    throw syntax_error( std::string("unexpected ")+gTokenTypeStrings[RightBrace], (m_tokenIt-1)->line, (m_tokenIt-1)->pos, (m_tokenIt-1)->endPos );
                }
                m_tokenIt--;
                break;

            case Comment:
                m_tokenIt++;
                break;


            default:
                break;
        }
        
        return exprList;
    }

    void nextToken()
    {
        do
        {
            m_tokenIt++;
            if ( isEof() )
            {
                return;
                //throw syntax_error( "unexpected end of file", (m_tokenIt-1)->line, (m_tokenIt-1)->pos, (m_tokenIt-1)->endPos );
            }
        }
        // Убрать Newline-ы чтобы не мешали дальнейшему парсингу
        while( m_tokenIt->type == Newline );
    }
    
    void tokenBack()
    {
        assert( m_tokenIt->type != Begin );
        m_tokenIt--;
    }
    
    void tokenMustBe( TokenType type )
    {
        if ( m_tokenIt->type != type )
        {
            throw syntax_error( std::string("expected: '")+gTokenTypeStrings[type]+"'", m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
        }
    }
    
    void nextToken( TokenType type )
    {
        nextToken();
        if ( m_tokenIt->type != type )
        {
            throw syntax_error( std::string("expected: '")+gTokenTypeStrings[type]+"'", m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
        }
    }
    
    bool isEof()
    {
        return m_tokenIt->type == EndOfFile;
        //return m_tokenIt == m_tokenEnd;
    }

    void skipNewLines()
    {
        while( !isEof() && m_tokenIt->type == Newline )
        {
            m_tokenIt++;
        }
    }

    bool tokenIs( TokenType type )
    {
        return m_tokenIt->type == type;
    }
    
    bool nextTokenIs( TokenType type )
    {
        return ((m_tokenIt+1) != m_tokenEnd)  &&  ((m_tokenIt+1)->type == type);
    }
    
    bool prevTokenIs( TokenType type )
    {
        assert( m_tokenIt->type != Begin );
        return ((m_tokenIt-1)->type == type);
    }
    
    void tokenMustBeType()
    {
        switch( m_tokenIt->type )
        {
            case Int:
            case Float:
            case StringLiteral:
                break;
                
            default:
                throw syntax_error( std::string("expected type: "), m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
        }
    }

    
    expr::Expression* parseExpr( TokenType endToken )
    {
        doParseExpr( endToken );
        
        for( auto i = 0; i<m_output.size(); i++)
        {
            if ( m_output[i]->type() == expr::et_func_call )
            {
                LOG( "### func_call: " << m_output[i]->m_token.lexeme << ": " << ((expr::FunctionCall*)m_output[i])->m_parameters.size() );
            }
            else
            {
                LOG( "### dbg: " <<  m_output[i]->m_token.type << " " << m_output[i]->m_token.lexeme.c_str() );
            }
        }
        
        auto* result = constructExpr();
        
        result->evaluate2();
        return result;
    }
    
    void moveOpToOutput()
    {
        if ( m_operationStack.back() == &leftParen )
        {
            throw syntax_error( std::string("expected right parenthesis: "), m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
        }
        
        if ( m_operationStack.back()->type() == expr::et_func_call )
        {
            assert( m_funcStack.size() > 0 );
            m_funcStack.pop_back();
        }

        LOG( "@@@ operations->output: " << m_operationStack.back()->m_token.lexeme )

        m_output.push_back( m_operationStack.back() );
        m_operationStack.pop_back();
    }
    
    void doParseExpr( TokenType endTokenType )
    {
        // https://en.cppreference.com/w/cpp/language/operator_precedence
        
        if ( ! unariOperatorTokenTable[Not] )
        {
            unariOperatorTokenTable[Not] = true;
            unariOperatorTokenTable[PlusPlusRight] = true;
            unariOperatorTokenTable[PlusPlusLeft] = true;
            unariOperatorTokenTable[MinusMinusRight] = true;
            unariOperatorTokenTable[MinusMinusLeft] = true;
        }

        if ( operatorTokenTable[Plus] == 0 )
        {
            operatorTokenTable[LeftParen] = 100;
            operatorTokenTable[Identifier] = 100;

            operatorTokenTable[PlusPlusRight] = 2;
            operatorTokenTable[MinusMinusRight] = 2;
            operatorTokenTable[PlusPlusLeft] = 3;
            operatorTokenTable[MinusMinusLeft] = 3;
            operatorTokenTable[Not] = 3;
            operatorTokenTable[Tilde] = 3;
            //opratorTokenTable[Ampersand] = 3;
            operatorTokenTable[Star] = 5;
            operatorTokenTable[Slash] = 5;
            operatorTokenTable[And] = 5;
            operatorTokenTable[Mod] = 5;
            operatorTokenTable[Plus] = 6;
            operatorTokenTable[Minus] = 6;
            operatorTokenTable[LeftShift] = 6;
            operatorTokenTable[RightShift] = 6;
            operatorTokenTable[Less] = 9;
            operatorTokenTable[LessEqual] = 9;
            operatorTokenTable[Greater] = 9;
            operatorTokenTable[GreaterEqual] = 9;
            operatorTokenTable[EqualEqual] = 10;
            operatorTokenTable[NotEqual] = 10;
            operatorTokenTable[BitAnd] = 11;
            operatorTokenTable[Xor] = 11;
            operatorTokenTable[BitOr] = 13;
            operatorTokenTable[And] = 14;
            operatorTokenTable[Or] = 15;
            
            operatorTokenTable[Comma] = 17;
        }

        assert( m_output.size() == 0 );
        assert( m_operationStack.size() == 0 );

        bool theEnd = false;
        
        while( !theEnd )
        {
            nextToken();

            LOG( "parseExpr: " << gTokenTypeStrings[m_tokenIt->type] << " " << m_tokenIt->lexeme );
            switch( m_tokenIt->type )
            {
                case Identifier:
                {

                    if ( nextTokenIs( LeftParen ) )
                    {
                        LOG( "@@@ Func_Call to stack: " << m_tokenIt->lexeme )

                        // Function call
                        m_funcStack.push_back( new expr::FunctionCall( { *m_tokenIt }  ) );
                        nextToken();
                        m_operationStack.push_back( m_funcStack.back() );
                    }
                    else
                    {
                        // Identifier
                        LOG( "@@@ Identifier to output: " << m_tokenIt->lexeme )

                        m_output.push_back( new expr::Identifier{ *m_tokenIt } );
                    }
                    break;
                }
                case Comma:
                {
                    LOG( "@@@ Coma: " << m_tokenIt->lexeme )
                    if ( !m_funcStack.empty() )
                    {
                        m_funcStack.back()->m_parameters.push_back(nullptr);
                    }

                    
                    for(;;)
                    {
                        if ( m_operationStack.empty() )
                        {
                            throw syntax_error( std::string("unexpected ',': "), m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
                        }
                        if ( m_operationStack.back()->type() == expr::et_func_call )
                        {
                            break;
                        }
                        moveOpToOutput();
                    }
                    //m_operationStack.push_back( &comma );
                    break;
                }
                case Int:
                {
                    LOG( "@@@ Int to output: " << m_tokenIt->lexeme )
                    m_output.push_back( new expr::IntNumber{ *m_tokenIt } );
                    break;
                }
                case Float:
                {
                    LOG( "@@@ Float to output: " << m_tokenIt->lexeme )
                    m_output.push_back( new expr::FloatNumber{ *m_tokenIt } );
                    break;
                }
                case StringLiteral:
                {
                    LOG( "@@@ Float to output: " << m_tokenIt->lexeme )
                    m_output.push_back( new expr::String{ *m_tokenIt } );
                    break;
                }
                case LeftParen:
                {
                    // Before '(' must be <function name> or '=' or <sign/operation>
                    if ( operatorTokenTable[(m_tokenIt-1)->type] == 0
                             && ! prevTokenIs( LeftParen )
                             && ! prevTokenIs( Comma )
                             && ! prevTokenIs( Assignment ) )
                    {
                        throw syntax_error( std::string("unexpected left parenthesis: "), m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
                    }
                    if ( nextTokenIs(RightParen) )
                    {
                        throw syntax_error( std::string("empty closure expression (): "), m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
                    }
                    LOG( "@@@ leftParen to stack: " )
                    m_operationStack.push_back( &leftParen );
                    break;
                }
                case RightParen:
                {
                    if ( m_operationStack.empty() && endTokenType==RightParen )
                    {
                        return;
                    }

                    // handle 'function call' w/o arguments
                    if ( m_operationStack.back()->type() == expr::et_func_call && prevTokenIs(LeftParen) )
                    {
                        break;
                    }
                    
                    for(;;)
                    {
                        if ( m_operationStack.size() == 0 )
                        {
                            if ( endTokenType == RightParen )
                            {
                                //TODO ???
                                while( m_operationStack.size() > 0 )
                                {
                                    moveOpToOutput();
                                }
                                return;
                            }
                            
                            throw syntax_error( std::string("unexpected right parenthesis: "), m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
                        }
                        
//                        // Move ',' from op-stack to operand-stack
//                        if ( m_operationStack.back() == &comma )
//                        {
//                            assert( m_output.size() > 0 );
//                            if( m_funcStack.size() == 0 )
//                            {
//                                throw syntax_error( std::string("unexpected coma: "), m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
//                            }
//                            m_funcStack.back()->m_parameters.push_back( nullptr );
//                            moveOpToOutput();
//                        }
//
//                        // Replace 'leftFuncParen' by fubction call
//                        else 
                        
                        if ( m_operationStack.back()->type() == expr::et_func_call )
                        {
                            m_funcStack.back()->m_parameters.push_back( nullptr );
                            moveOpToOutput();
                            break;
                        }
                        
                        else if ( m_operationStack.back() == &leftParen )
                        {
                            //output.push_back( pStack.back() );
                            m_operationStack.pop_back();
                            break;
                        }

                        else
                        {
                            moveOpToOutput();
                        }
                    }
                    break;
                }
                case Semicolon:
                {
                    while( m_operationStack.size() > 0 )
                    {
//                        if ( m_operationStack.back() == &leftParen )
//                        {
//                            throw syntax_error( std::string("missing ')': "), m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
//                        }
                        LOG( "--?? " << (m_operationStack.size()-1) << " " << m_operationStack.back()->m_token.lexeme )
                        moveOpToOutput();
                    }
                    
                    if ( m_output.size() == 0 )
                    {
                        throw syntax_error( std::string("expected expression: "), m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
                    }

                    return;
                }
                case LeftSqBracket:
                {
                    //TODO:
                    assert(0);
                    break;
                }
                case RightSqBracket:
                {
                    //TODO:
                    assert(0);
                    break;
                }
                default:
                {
                    LOG( "default: " << m_tokenIt->lexeme )
                    auto precedence = operatorTokenTable[m_tokenIt->type];
                    if ( precedence == 0 )
                    {
                        tokenBack();
                        theEnd = true;
                        break;
                    }
                    
                    while( m_operationStack.size() > 0 )
                    {
                        // If current 'operator' has precedence >= m_operationStack.back()
                        // (Note: '(' has highest precedence )
                        if ( operatorTokenTable[(m_operationStack.back())->m_token.type] <= precedence )
                        {
                            moveOpToOutput();
                        }
                        else
                        {
                            break;
                        }
                    }
                    
                    if ( unariOperatorTokenTable[m_tokenIt->type] )
                    {
                        LOG( "@@@ unariOp to stack: " << m_tokenIt->lexeme )
                        m_operationStack.push_back( new expr::UnaryExpression(*m_tokenIt) );
                    }
                    else
                    {
                        LOG( "@@@ binaryOp to stack: " << m_tokenIt->lexeme )
                        m_operationStack.push_back( new expr::BinaryOpExpression(*m_tokenIt) );
                    }
                }
            }
        }
    }
    
    expr::Expression* constructExpr()
    {
        if ( m_output.empty() )
        {
            return nullptr;
        }
        
        auto* expr = m_output.back();
        m_output.pop_back();
        
        switch( expr->type() )
        {
            case expr::et_identifier:
            case expr::et_int:
            case expr::et_float:
            case expr::et_string:
                return expr;

            case expr::et_func_call:
            {
                auto* funcCallExpr = dynamic_cast<expr::FunctionCall*>(expr);
                for( size_t i=funcCallExpr->m_parameters.size(); i>0; )
                {
                    i--;
                    assert( funcCallExpr->m_parameters[i] == nullptr );
                    funcCallExpr->m_parameters[i] = constructExpr();
                }
                return expr;
            }
            case expr::et_unary:
            {
                ((expr::UnaryExpression*)expr)->m_expr = constructExpr();
                
                if ( ((expr::UnaryExpression*)expr)->m_expr == nullptr )
                {
                    auto& token = expr->m_token;
                    throw syntax_error( std::string("expression syntax error: "), token.line, token.pos, token.endPos );
                }
                return expr;
            }
            case expr::et_binary:
            {
                ((expr::BinaryOpExpression*)expr)->m_expr = constructExpr();
                
                if ( ((expr::BinaryOpExpression*)expr)->m_expr == nullptr )
                {
                    auto& token = expr->m_token;
                    throw syntax_error( std::string("expression syntax error: "), token.line, token.pos, token.endPos );
                }

                ((expr::BinaryOpExpression*)expr)->m_expr2 = constructExpr();
                
                if ( ((expr::BinaryOpExpression*)expr)->m_expr2 == nullptr )
                {
                    auto& token = expr->m_token;
                    throw syntax_error( std::string("expression syntax error: "), token.line, token.pos, token.endPos );
                }

                return expr;
            }
            case expr::et_undefined:
            {
                auto& token = expr->m_token;
                throw syntax_error( std::string("unexpexted et_undefined: "), token.line, token.pos, token.endPos );
            }
        }
        return expr;
    }
    
    expr::ExpressionVarDecl* parseVar()
    {
        //
        // "var" <indentifier> [ "=" <expression> ]
        //
        nextToken( Identifier );
        const Token& varName = *m_tokenIt;
        expr::Expression* expr = nullptr;
        if ( nextTokenIs( Assignment ) )
        {
            // skip assignment
            nextToken();
            expr = parseExpr( Semicolon );
        }
        
        auto* varExpr = new expr::ExpressionVarDecl{varName};
        varExpr->m_initValue = expr;
        
        if ( auto it = m_runtime.m_globalVarMap.find(varName.lexeme); it != m_runtime.m_globalVarMap.end() )
        {
            throw syntax_error( std::string("function with same name already exist: "), m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
        }
        
        nextToken();
        
        return varExpr;
    }

    expr::Expression* parsePrint()
    {
        nextToken( LeftParen );
        nextToken( StringLiteral );
        
        auto& argument = m_tokenIt->lexeme;
        
        std::vector<expr::Expression*> arguments = parsePrintArgument( argument, *m_tokenIt );
        
        // print( "x1=\(x1)d" )
        // std::cout << "x1=" << x1 << "d";

        nextToken( RightParen );
        nextToken( Semicolon );
        nextToken();

        return new expr::PrintFuncCall{ std::move(arguments) };
    }
    
    expr::Expression* parseClass()
    {
        nextToken( Identifier );
        const Token& className = *m_tokenIt;
        expr::ClassDefinition classDefinition{className.lexeme};
        
        nextToken( LeftBrace );
        
        // parse members
        nextToken();
        while( m_tokenIt->type != RightBrace )
        {
            if ( nextTokenIs(Var) )
            {
                //...
            }
            else if ( nextTokenIs(Func) )
            {
                //...
            }
            else if ( nextTokenIs(ClassT) )
            {
                //...
            }
            else
            {
                throw syntax_error( std::string("unexpected lexeme into class definition: "), m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
            }
        }
    }
    
    expr::Expression* parseReturn()
    {
        auto* returnValue = parseExpr( Semicolon );
        assert( m_tokenIt->type==Semicolon );
        nextToken();
        return new expr::Return{ returnValue };// expr;
    }
    
    std::vector<expr::Expression*> parsePrintArgument( const std::string& printString, const Token& printStringToken )
    {
        const char* ptr = printString.data();
        const char* end = ptr + printString.size();
        
        std::vector<expr::Expression*> result;
        
        const char* startOfLiteral = ptr;
        while( ptr < end )
        {
            if ( *ptr == '\\' )
            {
                ptr++;
                if ( ptr >= end )
                {
                    throw syntax_error( "printString: internal error: '\\' at the end of string", m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
                }
                if ( *ptr=='(' )
                {
                    LOG( "dbg0: " << std::string(startOfLiteral, ptr-1) )
                    result.push_back( new expr::String( printStringToken, startOfLiteral, ptr-1 ));
                    
                    // find right ')'
                    ptr++;
                    const char* exprBegin = ptr;

                    int nestingLevel = 0;
                    while( ptr < end )
                    {
                        if ( *ptr == ')' )
                        {
                            if ( nestingLevel == 0 )
                            {
                                break;
                            }
                            nestingLevel--;
                        }
                        else if ( *ptr == '(' )
                        {
                            nestingLevel++;
                        }
                        ptr++;
                    }
                    
//                    std::string exprString(exprBegin,ptr);
//                    LOG( "exprString: " << exprString << " " << ptr-exprBegin);
                    
                    Lexer& exprLexer = * new Lexer{ exprBegin-1, ptr };
                    m_nestedLexers.push_back(&exprLexer);
                    
                    exprLexer.run();
                    Parser parser(m_runtime);
                    
                    LOG("----ParsePrintArgument: " << std::string(exprBegin, ptr) );
//                    for( auto& token: exprLexer.tokens() )
//                    {
//                        LOG("----Parse token: " << token.lexeme );
//                    }
                    auto* expression = parser.parsePrintExpr( exprBegin-1, exprLexer.tokens() );
                    LOG("--- expression: " << (void*)expression );
                    result.push_back( expression );
                    startOfLiteral = ptr+1;
                }
            }
            ptr++;
        }
        LOG( "dbg2: " << std::string(startOfLiteral, ptr) )

        if ( result.empty() || startOfLiteral!=ptr )
        {
            result.push_back( new expr::String( printStringToken, startOfLiteral, ptr ));
        }
        
        return result;
    }

    expr::FuncDefinition* parseFuncDef()
    {
        nextToken();
        auto* func = new expr::FuncDefinition;
        
        // Save function name
        func->m_name = m_tokenIt->lexeme;
        
        if ( auto it = m_runtime.m_funcMap.find(func->m_name); it != m_runtime.m_funcMap.end() )
        {
            throw syntax_error( std::string("function with same name already exist: "), m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
        }
        
        //m_runtime.m_funcMap[func->m_name] = func;
        
        nextToken( LeftParen );
        nextToken();
        
        // Parse arguments
        int argumentNumber = 0;
        while( m_tokenIt->type != RightParen )
        {
            if ( m_tokenIt->type != Identifier )
            {
                if ( argumentNumber == 0 )
                {
                    throw syntax_error( std::string("expected ')': "), m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
                }
                throw syntax_error( std::string("expected argument name: "), m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
            }

            // Argument name
            std::string name = m_tokenIt->lexeme;
            
            nextToken( Colon );

            // Parse argument type
            nextToken();
            tokenMustBeType();
            
            LOG( "argument: " << name << " Type:" << m_tokenIt->lexeme )
            func->m_argList.emplace_back( expr::Argument{std::move(name), m_tokenIt->lexeme} );

            nextToken();
            if ( ! tokenIs( Comma ) )
            {
                tokenMustBe( RightParen );
                break;
            }
            nextToken();

        }

        nextToken( LeftBrace );
        m_blockLevel++;
        
        nextToken();
        while( m_tokenIt->type != RightBrace )
        {
            func->m_body.m_list.push_back( parseStatement() );
        }

        m_blockLevel--;
        
        m_tokenIt++;
        
        return func;
    }
    
    expr::Expression* parseIf()
    {
        nextToken( LeftParen );
        
        throw syntax_error( std::string("TODO: "), m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
        return nullptr;
    }


};
