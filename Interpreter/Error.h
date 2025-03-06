#pragma once

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

    syntax_error( const std::string& error, const Token& token ) : std::runtime_error(error),
      m_error( error ),
      m_line( token.line ),
      m_position( token.pos ),
      m_endPosition( token.pos+token.lexeme.size() )
    {}

//    runtime_error(const runtime_error&);
//    runtime_error& operator=(const runtime_error&);

    ~syntax_error() override {}
};

struct runtime_error: public std::runtime_error
{
    std::string m_error;
    int         m_line;
    int         m_position;
    int         m_endPosition;

    runtime_error( const std::string& error, int line, int position, int end ) : std::runtime_error(error),
      m_error(error),
      m_line(line),
      m_position(position),
      m_endPosition(end)
    {}

    runtime_error( const std::string& error, const Token& token ) : std::runtime_error(error),
      m_error( error ),
      m_line( token.line ),
      m_position( token.pos ),
      m_endPosition( token.pos+token.lexeme.size() )
    {}

//    runtime_error(const runtime_error&);
//    runtime_error& operator=(const runtime_error&);

    ~runtime_error() override {}
};

