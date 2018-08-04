cmake_minimum_required(VERSION 3.9)
project(typedLetLang)

set(CMAKE_CXX_STANDARD 17)

add_executable(typedLetLang main.cpp src/Parser.cpp src/Parser.h src/Utils.cpp src/Utils.h)