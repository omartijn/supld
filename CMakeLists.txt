cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

project(supld
    LANGUAGES CXX
)

find_package(Boost REQUIRED)
find_package(OpenSSL COMPONENTS Crypto SSL REQUIRED)
find_package(Threads REQUIRED)

if (Boost_VERSION_MACRO LESS 108100)
    message(STATUS "Your boost version is too old, and does not contain Boost.URL. Falling back to external submodule.")

    if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/url/CMakeLists.txt")
        message(FATAL_ERROR "The 'url' submodule is not checked out. Please run 'git -C ${CMAKE_CURRENT_SOURCE_DIR} submodule update --init' and re-run CMake")
    endif()

    set(BOOST_URL_FIND_PACKAGE_BOOST ON)
    add_subdirectory(url)
endif()

add_library(http)
target_link_libraries(http PUBLIC Boost::boost)
target_link_libraries(http PUBLIC Boost::url)
target_link_libraries(http PUBLIC OpenSSL::Crypto)
target_link_libraries(http PUBLIC OpenSSL::SSL)
target_link_libraries(http PUBLIC Threads::Threads)

target_sources(http
    PUBLIC
        http/client.h
        http/streaming_download_handler.h
    PRIVATE
        http/client.cpp
        http/impl/streaming_download_operation.h
)
