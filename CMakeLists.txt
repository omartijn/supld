cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

project(supld
    LANGUAGES CXX
)

find_package(Boost REQUIRED)

if (Boost_VERSION_MACRO LESS 108100)
    message(STATUS "Your boost version is too old, and does not contain Boost.URL. Falling back to external submodule.")

    if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/url/CMakeLists.txt")
        message(FATAL_ERROR "The 'url' submodule is not checked out. Please run 'git -C ${CMAKE_CURRENT_SOURCE_DIR} submodule update --init' and re-run CMake")
    endif()

    set(BOOST_URL_FIND_PACKAGE_BOOST ON)
    add_subdirectory(url)
endif()
