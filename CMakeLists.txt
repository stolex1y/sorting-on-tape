cmake_minimum_required(VERSION 3.27)
file(STRINGS "version.txt" PROJECT_VERSION)
project("sorting-on-tape" VERSION "${PROJECT_VERSION}" LANGUAGES CXX)
message("Project version ${CMAKE_PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(Testing)
include(Doxygen)
include(Ccache)

add_subdirectory(src bin)
add_subdirectory(test)
