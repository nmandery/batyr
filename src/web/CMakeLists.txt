cmake_minimum_required(VERSION 2.8)

project(web)

add_custom_target(web-resources
    COMMAND bash ./generate-header.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})


# vim: ft=cmake
