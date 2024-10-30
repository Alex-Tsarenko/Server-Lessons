cmake_minimum_required(VERSION 3.14)

project(DbgServerClient LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DDEBUG)

add_executable(Interpreter
  main.cpp

  Lexer.h
  Expr.h
  Parser.h
  Object.h
  Token.h
  TokenType.h
  TokenTypeStrings.h
)

install(TARGETS Interpreter
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
