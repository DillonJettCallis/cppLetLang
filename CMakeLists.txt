cmake_minimum_required(VERSION 3.9)
project(typedLetLang)

set(CMAKE_CXX_STANDARD 17)
set(CC clang)
set(CXX clang++)

add_executable(typedLetLang main.cpp src/Parser.cpp src/Parser.h src/Utils.cpp src/Utils.h src/Typechecker.cpp src/Typechecker.h src/Compiler.cpp src/Compiler.h src/Ast.cpp src/Ast.h src/Tokens.cpp src/Tokens.h src/Types.cpp src/Types.h)