cmake_minimum_required(VERSION 2.8)


set(BATYR_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# NOTE: the ordering of the subdirectories matters for cmake to find
# all its dependencies

# external libraries and headers
add_subdirectory(3rdparty)

# include files shared by all projects
add_subdirectory(common)
add_subdirectory(core)

# the web gui for the server. static html/css/js/... files
if (ENABLE_HTTP_WEB_GUI)
    add_subdirectory(web)
endif (ENABLE_HTTP_WEB_GUI)

# the server application
add_subdirectory(server)


# vim: ft=cmake
