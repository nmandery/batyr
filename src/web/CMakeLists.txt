cmake_minimum_required(VERSION 2.8)

project(web)

find_package( PythonInterp REQUIRED)

# markdown converte used in src/web/generate-manual-html.py
find_program(MARKDOWN_EXECUTABLE NAMES markdown DOC "discount markdown converter")
if(NOT MARKDOWN_EXECUTABLE) 
    message(FATAL_ERROR "${PROJECT_NAME} requires the discount markdown converter \"markdown\" on the default PATH")
endif()
message(STATUS "Using discount markdown converter ${MARKDOWN_EXECUTABLE}")

set(ENV{MARKDOWN_EXECUTABLE} ${MARKDOWN_EXECUTABLE})
add_custom_target(manual-html
    COMMAND MARKDOWN_EXECUTABLE=${MARKDOWN_EXECUTABLE} ${PYTHON_EXECUTABLE} ./generate-manual-html.py ${CMAKE_CURRENT_LIST_DIR}/../../MANUAL.md
                ${CMAKE_CURRENT_SOURCE_DIR}/static/partials/manual.html.in
                ${CMAKE_CURRENT_SOURCE_DIR}/static/partials/manual.html
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}

)

add_custom_target(web-resources
    COMMAND bash ./generate-header.sh
    DEPENDS manual-html
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})


# vim: ft=cmake
