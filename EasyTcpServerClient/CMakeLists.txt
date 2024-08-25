cmake_minimum_required(VERSION 3.14)

project(DbgServerClient LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BOOST_INCLUDE_DIR "/usr/local/include")
set(BOOST_LIB_DIR "/usr/local/lib")
#set(CMAKE_PREFIX_PATH "~/Qt/6.4.0/macos")

#find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
#find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(DbgServerClient
  main.cpp

  TcpServer.h
  TcpClient.h
  
  TicTacProtocol.h
  TicTacTcpServer.h
  TicTacClient.h

  DbgTicTacClient.h
)
#target_link_libraries(DbgServerClient Qt${QT_VERSION_MAJOR}::Core)

include_directories("/usr/local/include")

install(TARGETS DbgServerClient
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
