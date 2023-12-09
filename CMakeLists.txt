cmake_minimum_required(VERSION 3.27)
project("sorting-on-tape" VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(Testing)
include(Doxygen)
include(Ccache)

add_subdirectory(src bin)
add_subdirectory(test)
