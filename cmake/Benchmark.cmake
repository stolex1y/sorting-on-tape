include_guard(GLOBAL)

include(FetchContent)

FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.8.3
)

FetchContent_MakeAvailable(benchmark)

macro(AddBenchmark target)
    add_custom_target("${target}-runnable"
            COMMAND $<TARGET_FILE:${target}>
            --benchmark_time_unit=s
            --benchmark_out=benchmarks.txt
            --benchmark_out_format=console
            WORKING_DIRECTORY $<TARGET_FILE_DIR:${target}>
    )
    target_link_libraries(${target} PRIVATE benchmark::benchmark benchmark::benchmark_main)
endmacro()
