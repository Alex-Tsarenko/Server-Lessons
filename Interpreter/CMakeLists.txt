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
  Expr.cpp
  Error.h
  Error.cpp
  Lexer.h
  Logs.h
  ClassOrNamespace.h
  ObjectValue.h
  ObjectValue.cpp
  ClassObject.h
  Parser.h
  Token.h
  TokenType.h
  TokenTypeStrings.h
)

#-Werror=return-type
add_compile_options(-Werror=return-type)

install(TARGETS Interpreter
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
