CMAKE_MINIMUM_REQUIRED(VERSION 3.18.0)
project(Tsanalyze)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src SRC_LIST)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

# SET(CMAKE_C_COMPILER "/usr/local/opt/llvm/bin/clang")
# SET(CMAKE_CXX_COMPILER "/usr/local/opt/llvm/bin/clang++")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -Wfatal-errors -Wstrict-prototypes -Wshadow -Wno-unused-parameter")


IF(${CMAKE_BUILD_TYPE} MATCHES "debug")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g")
    add_compile_options(-fno-omit-frame-pointer -fmacro-backtrace-limit=0)
    add_compile_options(-fsanitize=address -fsanitize=leak -fsanitize-recover=all)
    add_link_options(-fsanitize=address -fsanitize=leak -fsanitize-recover=all)
ELSE()
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
ENDIF()

ADD_EXECUTABLE(tsanalyze ${SRC_LIST})

find_package(Python 3.8 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 REQUIRED)

pybind11_add_module(tsana ./pybind/binding.cpp ${SRC_LIST})