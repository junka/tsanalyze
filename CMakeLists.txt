project(Tsanalyze)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src SRC_LIST)

SET(CMAKE_C_COMPILER "/usr/bin/gcc")
message(${CMAKE_SYSTEM_NAME})


IF(${CMAKE_BUILD_TYPE} MATCHES "debug")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -g")
ELSE()
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3 -std=c11")
ENDIF()

ADD_EXECUTABLE(tsanalyze ${SRC_LIST})

