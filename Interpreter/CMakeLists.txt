cmake_minimum_required(VERSION 3.14)

project(DbgServerClient LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DDEBUG)

add_executable(Interpreter
  main.cpp
  test.h

  Runtime.h
  Runtime.cpp
  Expr.h
  Error.h
  Error.cpp
  Lexer.h
  Logs.h
  Namespace.h
  ObjectValue.h
  ClassObject.h
  Parser.h
  Token.h
  TokenType.h
  TokenTypeStrings.h
)

install(TARGETS Interpreter
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
