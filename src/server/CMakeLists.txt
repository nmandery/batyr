cmake_minimum_required(VERSION 2.8)

project(batyrd)


FILE(GLOB_RECURSE SERVER_SOURCES *.cpp)

include_directories(
    ${BATYR_INCLUDE_DIR}
    ${GDAL_INCLUDE_DIR}
    ${Poco_INCLUDE_DIRS}
    ${POSTGRES_INCLUDE_DIR}
    ${RAPIDJSON_INCLUDE_DIR}
)

if (ENABLE_HTTP_WEB_GUI)
    # ensure the file exists
    # otherwise cmake is unable to resolve its dependencies
    # and find all files to build the executable
    execute_process(
        COMMAND touch ${BATYR_INCLUDE_DIR}/web/http_resources.h
    )

    # also add header of resources file to rebuild after
    # it has been updated. this will result in always compiling
    # at least one file and linking this project, but at least
    # the resources will always up to date
    LIST(APPEND SERVER_SOURCES ${BATYR_INCLUDE_DIR}/web/http_resources.h)
endif (ENABLE_HTTP_WEB_GUI)


add_executable( ${PROJECT_NAME}
    ${SERVER_SOURCES}
    )


if (ENABLE_HTTP_WEB_GUI)
    # the server requires the resources header to be build
    add_dependencies(batyrd web-resources)
endif (ENABLE_HTTP_WEB_GUI)

target_link_libraries( ${PROJECT_NAME}
    ${Poco_LIBRARIES} 
    ${POSTGRES_LIBRARY} 
    ${GDAL_LIBRARY} 
    ${CMAKE_THREAD_LIBS_INIT}
    batyrcommon
    batyrcore
    )

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# vim: ft=cmake
