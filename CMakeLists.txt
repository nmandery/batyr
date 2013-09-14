cmake_minimum_required (VERSION 2.6)

project (geopoll)
FILE(GLOB ServerSources src/*.cpp src/*/*.cpp)


#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# http://stackoverflow.com/questions/7724569/debug-vs-release-in-cmake
# Initialize CXXFLAGS.
set(CMAKE_CXX_FLAGS                "-Wall -Wno-format-extra-args -Wextra -Wformat-nonliteral -Wformat-security -Wformat=2")
set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g -D_DEBUG=1")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

# Compiler-specific C++11 activation.
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
    if ((GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7))
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    elseif (GCC_VERSION VERSION_EQUAL 4.6)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    else ()
        message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.6 or greater.")
    endif ()
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
else ()
    message(FATAL_ERROR "Your C++ compiler does not support C++11.")
endif ()


find_library(POCO_FOUNDATION PocoFoundation)
find_library(POCO_NET PocoNet)
find_library(POCO_UTIL PocoUtil)
find_package( Threads )

ADD_EXECUTABLE(geopoll ${ServerSources})
target_link_libraries(geopoll ${POCO_FOUNDATION} ${POCO_UTIL} ${POCO_NET} ${CMAKE_THREAD_LIBS_INIT})  

