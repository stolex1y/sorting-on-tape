set(STATIC_LIB "${CMAKE_PROJECT_NAME}-static")
set(TEST_STATIC_LIB "${CMAKE_PROJECT_NAME}-test-static")
set(BENCHMARK "${CMAKE_PROJECT_NAME}-benchmark")

add_executable(${BENCHMARK}
        tape_sorter_benchmark.cc
)
target_include_directories(${BENCHMARK} PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")

# clang-format
include(Format)
Format(${BENCHMARK} .)

include(Benchmark)
AddBenchmark(${BENCHMARK})
target_link_libraries(${BENCHMARK} PUBLIC
        ${STATIC_LIB}
        ${TEST_STATIC_LIB}
)
