cmake_minimum_required(VERSION 2.8)

project(batyrcommon)

FILE(GLOB_RECURSE COMMON_SOURCES *.cpp)

add_library(${PROJECT_NAME} STATIC
    ${COMMON_SOURCES}
    )

# vim: ft=cmake
