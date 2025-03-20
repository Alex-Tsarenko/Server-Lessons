#include "Error.h"
#include "Parser.h"
#include "Runtime.h"

syntax_error::syntax_error( const Parser* parser, const std::string& error, const Token& token )
    : std::runtime_error(error),
      m_error( error )
{
    parser->getLineAndPos(token, m_line, m_position, m_endPosition );
}

runtime_error::runtime_error( const Runtime& runtime, const std::string& error, const Token& token )
    : std::runtime_error(error),
      m_error( error )
{
    runtime.getLineAndPos(token, m_line, m_position, m_endPosition );
}
