cmake_minimum_required(VERSION 2.8)

project(batyrd)


FILE(GLOB_RECURSE SERVER_SOURCES *.cpp)

include_directories(
    ${BATYR_INCLUDE_DIR}
    ${RAPIDJSON_SOURCE_DIR}
    ${INIPARSER_SOURCE_DIR}
    #${GDAL_INCLUDE_DIR}
    ${Poco_INCLUDE_DIRS}
    ${POSTGRES_INCLUDE_DIR}
    ${INIPARSER_INCLUDE_DIR}
    ${RAPIDJSON_INCLUDE_DIR}
)

add_executable(batyrd ${SERVER_SOURCES})

# the server requires the assets header to be build
add_dependencies(batyrd web-assets)

target_link_libraries(batyrd 
    ${Poco_LIBRARIES} 
    #${ZeroMQ_LIBRARIES} 
    ${POSTGRES_LIBRARY} 
    ${GDAL_LIBRARY} 
    ${CMAKE_THREAD_LIBS_INIT} )


# vim: ft=cmake
