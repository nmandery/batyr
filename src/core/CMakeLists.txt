cmake_minimum_required(VERSION 2.8)

project(batyrcore)

include_directories(
    ${BATYR_INCLUDE_DIR}
    ${GDAL_INCLUDE_DIR}
    ${Poco_INCLUDE_DIRS}
    ${POSTGRES_INCLUDE_DIR}
    ${RAPIDJSON_INCLUDE_DIR}
)

FILE(GLOB_RECURSE COMMON_SOURCES *.cpp)

add_library(${PROJECT_NAME} STATIC
    ${COMMON_SOURCES}
    )

target_link_libraries( ${PROJECT_NAME}
    batyrcommon
    )

# vim: ft=cmake
