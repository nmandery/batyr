cmake_minimum_required(VERSION 2.8)

project(web)

#find_program(DISCOUNT markdown)
#
#message(STATUS "Discount markdown: ${DISCOUNT}")

add_custom_target(manual-html
    COMMAND python ./generate-manual-html.py ${CMAKE_CURRENT_LIST_DIR}/../../MANUAL.md
                ${CMAKE_CURRENT_SOURCE_DIR}/static/partials/manual.html.in
                ${CMAKE_CURRENT_SOURCE_DIR}/static/partials/manual.html
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}

)

add_custom_target(web-resources
    COMMAND bash ./generate-header.sh
    DEPENDS manual-html
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})


# vim: ft=cmake
