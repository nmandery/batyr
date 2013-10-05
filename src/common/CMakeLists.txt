cmake_minimum_required(VERSION 2.8)

project(batyrcommon)

FILE(GLOB_RECURSE COMMON_SOURCES *.cpp)


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_library(${PROJECT_NAME} STATIC
    ${COMMON_SOURCES}
    )

# vim: ft=cmake
