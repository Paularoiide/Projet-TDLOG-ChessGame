cmake_minimum_required(VERSION 3.5)

project(TDLOG_ChessGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(TDLOG_ChessGame main.cpp
    ai.h
    ai.cpp
    board.cpp
    board.h
    game.cpp
    game.h
    piece.h
    move.h
)

include(GNUInstallDirs)
install(TARGETS TDLOG_ChessGame
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
