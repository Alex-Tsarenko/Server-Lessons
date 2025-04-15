#pragma once

#include "Lexer.h"
#include "Expr.h"
#include "Runtime.h"
#include "getLineAndPos.h"
#include "Error.h"
#include <stack>
#include <cassert>

struct Parser
{
    std::string_view        m_programText;

    expr::ClassOrNamespace&                 m_topLevelNamespace;
    std::stack<expr::ClassOrNamespace*>     m_namespaceStack;

    std::vector<expr::Expression*>    m_program;
    std::vector<expr::Expression*>*   m_current;

    std::vector<Token>::const_iterator m_tokenIt;
    std::vector<Token>::const_iterator m_tokenEnd;
    
    int m_blockLevel = 0;
    
    std::vector<expr::Expression*>      m_output;
    std::vector<expr::Expression*>      m_operationStack;
    //std::vector<expr::FunctionCall*>      m_funcStack;

    bool unariOperatorTokenTable[EndOfTokenType] = {false};
    char operatorTokenTable[EndOfTokenType]      = {0};

    expr::HelpExpression leftParen{ *new Token{LeftParen,"("}};
    expr::HelpExpression comma{ *new Token{Comma,","}};
    expr::HelpExpression leftFuncParen{ *new Token{LeftParen,"f("}};
    expr::HelpExpression rightParen{ *new Token{RightParen,")"}};;
    
    std::vector<Lexer*> m_nestedLexers;
    
public:
    Parser( expr::ClassOrNamespace& aNamespace ) : m_topLevelNamespace(aNamespace)
    {
        m_namespaceStack.push( &m_topLevelNamespace );
    }

    ~Parser()
    {
        for( auto* lexer: m_nestedLexers )
        {
            delete lexer;
        }
    }

    expr::ClassOrNamespace& currentNamespace()
    {
        return *m_namespaceStack.top();
    }

    void onError( std::string errorText, int line, int pos ) {}

    void getLineAndPos( const Token& token, int& line, int& pos, int& endPos ) const
    {
        ::getLineAndPos( m_programText, token, line, pos, endPos );
    }

    expr::Expression* parsePrintExpr( const char* exprString, const std::vector<Token>& tokens )
    {
        m_tokenIt  = tokens.begin();
        m_tokenEnd = tokens.end();

        m_current = &m_program;
        
        assert( tokens.size() > 0 );
        assert( tokenIs(Begin) );
        _shiftToNextToken();

        try
        {
            auto* ex = parseExpr(  );
            assert( tokenIs(EndOfFile) );
            return ex;;
        }
        catch( syntax_error& error )
        {
            std::cerr << getLine( exprString, error.m_line ) << std::endl;
            std::cerr << "!!! Syntax error (" << error.m_line << "): " << error.what() << std::endl;
            for( int i=0; i<error.m_position-1; i++ )
            {
                std::cerr << ' ';
            }
            for( int i=error.m_position; i<error.m_endPosition; i++ )
            {
                std::cerr << '^';
            }
            std::cerr << std::endl;
            exit(0);
        }
    }

//    void printToken( const char* programText, const Token& token )
//    {
//        std::cerr << getLine( programText, token.line ) << std::endl;
//        for( int i=0; i<token.pos; i++ )
//        {
//            std::cerr << ' ';
//        }
//        for( int i=token.pos+1; i<token.pos+1+token.lexeme.size(); i++ )
//        {
//            std::cerr << '^';
//        }
//        std::cerr << std::endl;
//    }
    
    void parseProgram( const std::string_view& programText0, const std::vector<Token>& tokens )
    {
        m_programText = programText0;

//        for( auto& token: tokens )
//        {
//            if ( token.type == Newline ) continue;
//            
//            LOG( "---- " << token.lexeme )
//            printToken( programText, token );
//            std::cout << "\n";
//        }
        
        m_tokenIt  = tokens.begin();
        m_tokenEnd = tokens.end();

        m_current = &m_program;
        
        assert( tokens.size() > 0 );
        assert( tokenIs(Begin) );

        // skip 'Begin'
        m_tokenIt++;

        try
        {
            for(;;)
            {
                skipNewLinesAndComments();
                if ( isEof() )
                {
                    break;
                }
                LOG("-----------------------parseProgram: " << gTokenTypeStrings[m_tokenIt->type] )
                if ( auto expr = parseStatement<true>(); expr != nullptr )
                {
                    m_current->push_back( expr );
                }
            }
        }
        catch( syntax_error& error )
        {
            std::cerr << getLine( m_programText.data(), error.m_line ) << std::endl;
            std::cerr << "!!! Syntax error (" << error.m_line << "): " << error.what() << std::endl;

            for( int i=0; i<error.m_position-1; i++ )
            {
                std::cerr << ' ';
            }
            for( int i=error.m_position; i<error.m_endPosition; i++ )
            {
                std::cerr << '^';
            }
            std::cerr << std::endl;
            exit(0);
        }
    }

protected:

    void _shiftToNextToken()
    {
        while( m_tokenIt->type == Newline || m_tokenIt->type == Comment )
        {
            m_tokenIt++;
            if ( isEof() )
            {
                return;
            }
        }

        do
        {
            m_tokenIt++;
            LOG( "?? takeNextToken: " << m_tokenIt->lexeme)
            if ( isEof() )
            {
                return;
            }
        }
        // Убрать Newline-ы чтобы не мешали дальнейшему парсингу
        while( m_tokenIt->type == Newline || m_tokenIt->type == Comment );
    }

    void _shiftToPrevToken()
    {
        assert( m_tokenIt->type != Begin );
        m_tokenIt--;

        while( m_tokenIt->type == Newline || m_tokenIt->type == Comment )
        {
            m_tokenIt--;
        }
    }


    void _shiftToNextTokenIf( TokenType type )
    {
        _shiftToNextToken();
        if ( m_tokenIt->type != type )
        {
            LOG( gTokenTypeStrings[m_tokenIt->type] )
            throw syntax_error( this, std::string("expected: '")+gTokenTypeStrings[type]+"'",  *m_tokenIt );
        }
    }

    bool _nextTokenIs( TokenType type )
    {
        return ((m_tokenIt+1) != m_tokenEnd)  &&  ((m_tokenIt+1)->type == type);
    }

    bool _prevTokenIs( TokenType type )
    {
        assert( m_tokenIt->type != Begin );
        return ((m_tokenIt-1)->type == type);
    }


    template<bool isGlobal>
    expr::Expression* parseStatement()
    {
        skipNewLines();
        
        //TODO ')' ',' ';'
        if ( isEof() )
        {
            //if ( m_blockLevel != 0 )
            {
                throw syntax_error( this, "unexpected end of file, expected '}'", *(m_tokenIt) );
            }
            return nullptr;
        }

        LOG( "---------------PraseStatement: m_tokenIt->type: " << gTokenTypeStrings[m_tokenIt->type] )
        switch ( m_tokenIt->type )
        {
            case Var:
            {
                if constexpr (isGlobal)
                {
                    auto* varDef = parseVar();
                    LOG( "global Var: " << varDef->m_identifierName )
                    if ( ! currentNamespace().emplaceVar( varDef->m_identifierName, varDef ) )
                    {
                        throw syntax_error( this, std::string("duplicate variable name: "), varDef->m_token );
                    }

                    return nullptr;
                }
                auto* varDef = parseVar();
                LOG( "local Var: " << varDef->m_identifierName )
                return varDef;
            }
            case Func:
            {
                auto* funcDef = parseFuncDef< expr::FuncDefinition >();

                if ( auto result = currentNamespace().m_functionMap.emplace( funcDef->m_name, funcDef ); !result.second )
                {
                    throw syntax_error( this, std::string("duplicate function name: "), funcDef->m_token );
                }
                return nullptr;
            }
            case If:
                return parseIf();
            case For:
                assert(0);
            case Return:
                _shiftToNextToken();
                return parseReturn();
            case Print:
                return parsePrint();
            case NamespaceKw:
            {
                if constexpr (!isGlobal)
                {
                    throw syntax_error( this, "namespace is not allowed here", *m_tokenIt );
                }
                else
                {
                    parseNamespace();
                    return nullptr;
                }
            }
            case ClassKw:
            {
                auto* classDefinition = parseClass();
                auto result = currentNamespace().m_namespaceMap.emplace( classDefinition->m_name, classDefinition );
                if ( not result.second )
                {
                    throw syntax_error( this, "duplicate class/namespace name: ", classDefinition->m_token );
                }
                return nullptr;
            }
            case Identifier:
            case IdentifierWithScope:
            {
                auto* ex = parseExpr();
                if ( tokenIs(Semicolon) )
                {
                    _shiftToNextToken();
                    return ex;
                }
                else if ( tokenIs(Assignment) )
                {
                    _shiftToNextToken();
                    auto* ex2 = parseExpr();
                    if ( tokenIs(Semicolon) )
                    {
                        _shiftToNextToken();
                    }
                    auto* assignment = new expr::AssignmentStatement( *m_tokenIt, ex, ex2 );
                    return assignment;
                }
                throw syntax_error( this, std::string("unexpected ")+gTokenTypeStrings[RightBrace], *m_tokenIt );


//                if ( _nextTokenIs(LeftParen) )
//                {
//                    // f(...) function call
//                    return parseExpr(Semicolon);
//                }
//                else
//                {
//                    // assignment
//                    // x = ...;
//                    auto* assignment = new expr::AssignmentStatement( *m_tokenIt );
//                    _shiftToNextTokenIf( Assignment );
//                    _shiftToNextToken();
//                    assignment->m_expr = parseExpr( Semicolon );
//                    return assignment;
//                }
            }
            case RightBrace:
                if ( m_blockLevel == 0 )
                {
                    throw syntax_error( this, std::string("unexpected ")+gTokenTypeStrings[RightBrace], *m_tokenIt );
                }
                m_tokenIt--;
                return nullptr;

            case Comment:
                _shiftToNextToken();
                return nullptr;


            default:
                LOG( "parseStatement: unexpected token: " << gTokenTypeStrings[m_tokenIt->type]  );
                throw syntax_error( this, std::string("parseStatement: unexpected token: ")+gTokenTypeStrings[m_tokenIt->type],  *m_tokenIt );
                break;
        }
        
        assert(0);
        return nullptr;
    }

    void tokenBack()
    {
        assert( m_tokenIt->type != Begin );
        m_tokenIt--;
    }
    
//    void tokenMustBe( TokenType type )
//    {
//        if ( m_tokenIt->type != type )
//        {
//            throw syntax_error( this, std::string("expected: '")+gTokenTypeStrings[type]+"'",  *m_tokenIt );
//        }
//    }
    

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

    void skipNewLinesAndComments()
    {
        while( !isEof() && (m_tokenIt->type == Newline || m_tokenIt->type == Comment))
        {
            m_tokenIt++;
        }
    }

    bool tokenIs( TokenType type )
    {
        return m_tokenIt->type == type;
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
                throw syntax_error( this, std::string("expected type: "), *m_tokenIt );
        }
    }

    
    expr::Expression* parseExpr()
    {
        // 1-st pass of shunting-yard algorithm
        doParseExpr();

        while( !m_operationStack.empty() )
        {
            if ( m_operationStack.back() != &leftParen )
            {
                m_output.push_back( m_operationStack.back() );
            }
            m_operationStack.pop_back();
        }

        for( auto i = 0; i<m_output.size(); i++)
        {
            if ( m_output[i]->type() == expr::et_func_call )
            {
                LOG( "##parseExpr# func_call: " << m_output[i]->m_token.lexeme << ": " << ((expr::FunctionCall*)m_output[i])->m_parameters.size() );
            }
            else
            {
                LOG( "##parseExpr# dbg: " <<  gTokenTypeStrings[m_output[i]->m_token.type] << " " << m_output[i]->m_token.lexeme );
            }
        }
        
        // 2-d pass of shunting-yard algorithm
        auto* result = constructExpr();
        
        // Print parsed expr
        //result->printExpr("");
        //LOGX("\n");

        return result;
    }
    
    void moveOpToOutput()
    {
        if ( m_operationStack.back() == &leftParen )
        {
            throw syntax_error( this, std::string("expected right parenthesis: "),  *m_tokenIt );
        }
        
//        if ( m_operationStack.back()->type() == expr::et_func_call )
//        {
//            assert( m_funcStack.size() > 0 );
//            LOG( "@@@ ??? m_funcStack.pop_back: fn: " << m_funcStack.back()->m_functionName )
//
//            m_funcStack.pop_back();
//        }

        LOG( "@@@ ??? operations->output: " << m_operationStack.back()->m_token.lexeme )

        m_output.push_back( m_operationStack.back() );
        m_operationStack.pop_back();
    }
    
    void initParseExprTables()
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

            operatorTokenTable[Dot] = 1;
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

    }

    void doParseExpr()
    {
        initParseExprTables();

        assert( m_output.size() == 0 );
        assert( m_operationStack.size() == 0 );

        for( ;; _shiftToNextToken() )
        {
            LOG( "@@@ doParseExpr: " << gTokenTypeStrings[m_tokenIt->type] << " " << m_tokenIt->lexeme );

            switch( m_tokenIt->type )
            {
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
                    m_output.push_back( new expr::StringExpr{ *m_tokenIt } );
                    break;
                }
                case Identifier:
                case IdentifierWithScope:
                {
                    LOG( "@@@ Identifier: " << m_tokenIt->lexeme )

                    //
                    // Проверяем, является ли идентификатор функцией
                    //

                    if ( _nextTokenIs( LeftParen ) )
                    {
                        LOG( "@@@ ??? Func_Call to stack: " << m_tokenIt->lexeme )

                        // Function call
                        m_operationStack.push_back( new expr::FunctionCall( { *m_tokenIt } ));

                        // skip '('
                        _shiftToNextToken();
                    }
                    else
                    {
                        // Identifier
                        LOG( "@@@ Identifier to output: " << m_tokenIt->lexeme )

                        m_output.push_back( new expr::IdentifierExpr{ *m_tokenIt } );
                    }
                    break;
                }
                case Comma:
                {
                    LOG( "@@@ Coma: " << m_tokenIt->lexeme )

                    // Выталкиваем операторы до открывающей скобки текущего аргумента
                    while ( !m_operationStack.empty() &&
                           (m_operationStack.back() != &leftParen && m_operationStack.back()->type() != expr::et_func_call) )
                    {
                        m_output.push_back( m_operationStack.back() );
                        m_operationStack.pop_back();
                    }

                    if ( m_operationStack.empty() )
                    {
                        //TODO
                        return;
                    }

                    // Увеличиваем счетчик аргументов текущей функции верхнего уровня
                    if ( m_operationStack.back()->type() == expr::et_func_call )
                    {
                        auto* functionCall = ((expr::FunctionCall*)m_operationStack.back());
                        functionCall->m_parameters.push_back(nullptr);
                    }

                    break;
                }
                case RightParen:
                {
                    // Выталкиваем операторы до открывающей скобки текущего аргумента
                    while ( !m_operationStack.empty() &&
                           (m_operationStack.back() != &leftParen && m_operationStack.back()->type() != expr::et_func_call) )
                    {
                        m_output.push_back( m_operationStack.back() );
                        m_operationStack.pop_back();
                    }

                    if ( m_operationStack.empty() )
                    {
                        //TODO
                        return;
                    }

                    if ( m_operationStack.back()->type() != expr::et_func_call )
                    {
                        m_operationStack.pop_back();
                    }
                    else
                    {
                        auto* functionCall = ((expr::FunctionCall*)(m_operationStack.back()));

                        // Всегда увеличиваем счетчик параметров, если это не вызов функции без параметров
                        if ( /*functionCall->m_parameters.size() > 0 ||*/ !_prevTokenIs(LeftParen) )
                        {
                            functionCall->m_parameters.push_back(nullptr);
                        }

                        m_output.push_back( m_operationStack.back() );
                        m_operationStack.pop_back();
                    }

                    break;
                }
                case LeftParen:
                {
                    m_operationStack.push_back( &leftParen );
                    break;
                }
                case Semicolon:
                {
                    //TODO
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
                        //TODO
                        return;
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
                        if ( m_tokenIt->type == Dot )
                        {
                            m_operationStack.push_back( new expr::DotExpr(*m_tokenIt) );
                        }
                        else
                        {
                            m_operationStack.push_back( new expr::BinaryOpExpression(*m_tokenIt) );
                        }
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
        LOG( "m_output.size:" << m_output.size() )

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
                    throw syntax_error( this, std::string("expression syntax error: "),  *m_tokenIt );
                }
                return expr;
            }
            case expr::et_binary:
            case expr::et_dot:
            {
                ((expr::BinaryOpExpression*)expr)->m_expr = constructExpr();
                
                if ( ((expr::BinaryOpExpression*)expr)->m_expr == nullptr )
                {
                    auto& token = expr->m_token;
                    throw syntax_error( this, std::string("expression syntax error: "),  *m_tokenIt );
                }

                ((expr::BinaryOpExpression*)expr)->m_expr2 = constructExpr();
                
                if ( ((expr::BinaryOpExpression*)expr)->m_expr2 == nullptr )
                {
                    auto& token = expr->m_token;
                    throw syntax_error( this, std::string("expression syntax error: "), *m_tokenIt );
                }

                return expr;
            }
            case expr::et_undefined:
            {
                auto& token = expr->m_token;
                LOG( "unexpexted et_undefined: " << m_tokenIt->lexeme )
                throw syntax_error( this, std::string("unexpexted et_undefined: "), *m_tokenIt );
            }
            case expr::et_assignment:
                break;
            case expr::et_namespace_op:
                break;
            case expr::et_return:
                break;
        }
        return expr;
    }
    
    void parseNamespace()
    {
        _shiftToNextTokenIf( Identifier );
        const Token& namespaceName = *m_tokenIt;

        auto it = currentNamespace().m_namespaceMap.find( namespaceName.lexeme );
        bool alreadyExist = it != currentNamespace().m_namespaceMap.end();
        if ( alreadyExist  )
        {
            if ( it->second->m_isClass )
            {
                throw syntax_error( this, std::string("duplicate class name: "), namespaceName );
            }
        }

        if ( alreadyExist )
        {
            m_namespaceStack.push( it->second );
        }
        else
        {
            auto* thisClassOrNamespace = currentNamespace().emplaceNamespace( namespaceName.lexeme );
            m_namespaceStack.push( thisClassOrNamespace );
        }

        _shiftToNextTokenIf( LeftBrace );
        _shiftToNextToken();

        for(;;)
        {
            if ( tokenIs(RightBrace) )
            {
                break;
            }
            if ( tokenIs(EndOfFile) )
            {
                throw syntax_error( this, "unexpected end of file, expected '}' of namespace", *m_tokenIt );
            }
            parseStatement<true>();
        }
        _shiftToNextToken();
        m_namespaceStack.pop();
    }

    expr::VarDeclaration* parseVar( bool isClassMember = false )
    {
        //
        // "var" <indentifier> [ "=" <expression> ]
        //
        if ( !isClassMember )
        {
            // skip 'var' keyword
            _shiftToNextTokenIf( Identifier );
        }
        
        const Token& varName   = *m_tokenIt;
        const Token* varType   = nullptr;
        expr::Expression* expr = nullptr;

        _shiftToNextToken();
        
        if ( tokenIs( Colon ) )
        {
            _shiftToNextToken();
            if ( ! tokenIs(Identifier) && ! tokenIs(IdentifierWithScope) )
            {
                throw syntax_error( this, std::string("expected type: "), *m_tokenIt );
            }
            LOG( "After colon (var type): " << m_tokenIt->lexeme );
            varType = &(*m_tokenIt);

            _shiftToNextToken();
        }

        if ( tokenIs( Assignment ) )
        {
            // skip assignment
            _shiftToNextToken();
            expr = parseExpr();
        }

        if ( tokenIs( Semicolon ) )
        {
            _shiftToNextToken();
        }
        
        auto* varExpr = new expr::VarDeclaration{varName,varType};
        varExpr->m_initValue = expr;
        
        return varExpr;
    }

    expr::Expression* parsePrint()
    {
        _shiftToNextTokenIf( LeftParen );
        _shiftToNextTokenIf( StringLiteral );
        
        auto& argument = m_tokenIt->lexeme;
        
        std::vector<expr::Expression*> arguments = parsePrintArgument( argument, *m_tokenIt );
        
        // print( "x1=\(x1)d" )
        // std::cout << "x1=" << x1 << "d";

        //_shiftToNextTokenIf( RightParen );
        _shiftToNextTokenIf( Semicolon );
        _shiftToNextToken();

        return new expr::PrintFuncCall{ std::move(arguments) };
    }
    
    expr::ClassOrNamespace* parseClass()
    {
        _shiftToNextTokenIf( Identifier );
        const Token& className = *m_tokenIt;
        LOG("className: " << className.lexeme )

        if ( auto it = currentNamespace().m_namespaceMap.find( className.lexeme ); it != currentNamespace().m_namespaceMap.end() )
        {
            throw syntax_error( this, std::string("duplicate name: "), className );
        }

        // Create class definition structure
        expr::ClassOrNamespace* thisClassOrNamespace = new expr::ClassOrNamespace{ true, className.lexeme, &currentNamespace() };

        // Parse base classes
        if ( _nextTokenIs(Colon) )
        {
            _shiftToNextTokenIf( Colon );
            
            bool firstBaseClass = true;
            while( firstBaseClass || _nextTokenIs(Comma) )
            {
                if ( firstBaseClass )
                {
                    firstBaseClass = false;
                }
                else
                {
                    _shiftToNextTokenIf( Comma );
                }
                _shiftToNextToken();
                
                bool isPrivate = false;
                if ( tokenIs(Private) )
                {
                    isPrivate = true;
                    _shiftToNextToken();
                }
                
                if ( ! tokenIs(Identifier) )
                {
                    throw syntax_error( this, "expected base class name: ", *m_tokenIt );
                }
                
                // Base class name
                thisClassOrNamespace->m_baseClasses.push_back( { isPrivate, (m_tokenIt)->lexeme } );
            }
        }

        _shiftToNextTokenIf( LeftBrace );

        // parse members
        _shiftToNextToken();
        while( m_tokenIt->type != RightBrace )
        {
            LOG("???: " << m_tokenIt->lexeme );
            LOG("???:+1: " << (m_tokenIt+1)->lexeme );
            
            bool isPrivate = false;
            if ( tokenIs(Private) )
            {
                isPrivate = true;
                _shiftToNextToken();
            }
            
            //todo++ var&
            if ( tokenIs(Var) )
            {
                _shiftToNextTokenIf(Identifier);

                // parse variable
                auto* varDef = parseVar( true );
                varDef->m_isPrivate = isPrivate;
                thisClassOrNamespace->m_variableMap[varDef->m_identifierName] = varDef;
            }
            else if ( tokenIs(Func) )
            {
                auto* funcDef = parseFuncDef< expr::FuncDefinition >();
                funcDef->m_isPrivate = isPrivate;
                thisClassOrNamespace->m_functionMap[funcDef->m_name] = funcDef;
            }
            else if ( tokenIs(ClassKw) )
            {
                auto* innerClass = parseClass();
                //auto result = thisClassOrNamespace->emplaceClass( innerClass->m_name, innerClass );
//                if ( !result.second )
//                {
//                    throw syntax_error( this, std::string("duplicate class name: "), innerClass->m_token );
//                }
            }
//            else if ( tokenIs(Int) || tokenIs(Float) || tokenIs(String) || tokenIs(Identifier) || tokenIs(StringLiteral) )
//            {
//                
//            }
            else
            {
                LOG( std::string("unexpected lexeme into class definition: ") << m_tokenIt->lexeme );
                throw syntax_error( this, std::string("unexpected lexeme into class definition: "), *m_tokenIt );
            }
        }
        
        assert( m_tokenIt->type == RightBrace );
        _shiftToNextToken();
        
        return thisClassOrNamespace;
    }
    
    expr::Expression* parseReturn()
    {
        auto* returnValue = parseExpr();
        if ( tokenIs( Semicolon ) )
        {
            _shiftToNextToken();
        }
        return new expr::Return{ returnValue };// expr;
    }
    
    std::vector<expr::Expression*> parsePrintArgument( const std::string_view& printString, const Token& printStringToken )
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
                    throw syntax_error( this, "printString: internal error: '\\' at the end of string", *m_tokenIt );
                }
                if ( *ptr=='(' )
                {
                    LOG( "dbg0: " << std::string(startOfLiteral, ptr-1) )
                    result.push_back( new expr::StringExpr( printStringToken, startOfLiteral, ptr-1 ));
                    
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
                    
                    Lexer& exprLexer = * new Lexer{ exprBegin, ptr };
                    m_nestedLexers.push_back(&exprLexer);
                    
                    exprLexer.run();
                    Parser parser(m_topLevelNamespace);

                    LOG("----ParsePrintArgument: " << std::string(exprBegin, ptr) );
//                    for( auto& token: exprLexer.tokens() )
//                    {
//                        LOG("----Parse token: " << token.lexeme );
//                    }
                    auto* expression = parser.parsePrintExpr( exprBegin, exprLexer.tokens() );
                    LOG("--- expression: " << (void*)expression );
                    if ( expression != nullptr )
                    {
                        result.push_back( expression );
                    }
                    startOfLiteral = ptr+1;
                }
            }
            ptr++;
        }
        LOG( "dbg2: " << std::string(startOfLiteral, ptr) )

        if ( result.empty() || startOfLiteral!=ptr )
        {
            result.push_back( new expr::StringExpr( printStringToken, startOfLiteral, ptr ));
        }
        
        return result;
    }

    template<class T>
    T* parseFuncDef()
    {
        auto* func = new T{};
        if constexpr (std::is_same<T, expr::FuncDefinition>::value)
        {
            func->m_whereFuctionWasDefined = &currentNamespace();

            // Save function name
            _shiftToNextToken();
            func->m_name = m_tokenIt->lexeme;

            if ( auto it = currentNamespace().m_functionMap.find( func->m_name); it != currentNamespace().m_functionMap.end() )
            {
                throw syntax_error( this, std::string("function with same name already exist: "), *m_tokenIt );
            }
        }
        
        //m_runtime.m_funcMap[func->m_name] = func;
        
        _shiftToNextTokenIf( LeftParen );
        _shiftToNextToken();
        
        // Parse arguments
        int argumentNumber = 0;
        while( m_tokenIt->type != RightParen )
        {
            if ( m_tokenIt->type != Identifier )
            {
                if ( argumentNumber == 0 )
                {
                    throw syntax_error( this, std::string("expected ')': "), *m_tokenIt );
                }
                throw syntax_error( this, std::string("expected argument name: "), *m_tokenIt );
            }

            // Argument name
            std::string_view name = m_tokenIt->lexeme;

            _shiftToNextTokenIf( Colon );

            // Parse argument type
            _shiftToNextToken();
            tokenMustBeType();
            
            auto argType = m_tokenIt->lexeme;
            LOG( "argument: " << name << " Type: " << argType )
            func->m_argList.emplace_back( expr::Argument{std::move(name), argType} );

            _shiftToNextToken();
            if ( ! tokenIs( Comma ) )
            {
                break;
            }
            _shiftToNextToken();
        }
        
        if constexpr (std::is_same<T, expr::ClassDefinition::ConstructorInfo>::value)
        {
            // parse constructor initializer list
            if ( _nextTokenIs(Colon) )
            {
                _shiftToNextToken();

                // until '{'
                while( ! _nextTokenIs(LeftBrace) )
                {
                    _shiftToNextTokenIf( Identifier );
                    
                    auto className = m_tokenIt->lexeme;
                    _shiftToNextTokenIf( LeftParen );
                    
                    while( ! tokenIs(RightParen) )
                    {
                        _shiftToNextToken();
                        auto* expr = parseExpr();
                        assert ( tokenIs( RightParen ) );
                        tokenBack();
                        func->m_baseClassInitList.push_back( expr );
                    }
                }
            }
        }
        
        _shiftToNextTokenIf( LeftBrace );
        m_blockLevel++;
        
        _shiftToNextToken();
        while( m_tokenIt->type != RightBrace )
        {
            func->m_body.m_list.push_back( parseStatement<false>() );
        }

        m_blockLevel--;
        
        _shiftToNextToken();
        
        return func;
    }
    
    expr::Expression* parseIf()
    {
        _shiftToNextTokenIf( LeftParen );
        //???
        throw syntax_error( this, std::string("TODO: "), *m_tokenIt );
        return nullptr;
    }


};
