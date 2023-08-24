cmake_minimum_required(VERSION 3.10)

project(Log2FileLib)

# Specifying C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Adding the source files to the library
add_library(${PROJECT_NAME} STATIC
    log2file.hpp
    log2file.cpp
)

add_executable(${PROJECT_NAME}Test
    test/test.cpp
)

target_link_libraries(${PROJECT_NAME}Test ${PROJECT_NAME})
