#pragma once

#include <stdexcept>
#include <string>

class Parser;
class Runtime;
class Token;

struct syntax_error: public std::runtime_error
{
    std::string m_error;
    int         m_line;
    int         m_position;
    int         m_endPosition;

    syntax_error( const Parser* parser, const std::string& error, const Token& token );

    ~syntax_error() {}
};

struct runtime_error: public std::runtime_error
{
    std::string m_error;
    int         m_line;
    int         m_position;
    int         m_endPosition;

    runtime_error( const Runtime& runtime, const std::string& error, const Token& token );

    ~runtime_error() override {}
};

