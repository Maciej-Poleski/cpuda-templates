cmake_minimum_required(VERSION 2.8)
project(cpuda-templates)

add_definitions(-std=c++1y)

add_executable(cpuda-templates module.cpp kernel1.cpp main.cpp)

install(TARGETS cpuda-templates RUNTIME DESTINATION bin)
