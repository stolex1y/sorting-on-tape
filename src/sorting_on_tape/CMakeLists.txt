set(OBJ_LIB ${CMAKE_PROJECT_NAME})
set(RUNNABLE "${CMAKE_PROJECT_NAME}-runnable")
set(STATIC_LIB "${CMAKE_PROJECT_NAME}-static")

add_library(${OBJ_LIB} OBJECT
        tape.h
        file_tape.h
        configuration.cc
        configuration.h
        temp_tape_provider.h
        temp_file_tape_provider.h
        memory_literals.h
        tape_sorter.h
        core/thread_pool.cc
        core/thread_pool.h
        core/tasks.h
)
target_include_directories(${OBJ_LIB} PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

#warnings
target_compile_options(${OBJ_LIB} PRIVATE "-Werror" "-Wall" "-Wextra" "-Wpedantic")

add_executable(${RUNNABLE}
        main.cc
)
target_link_libraries(${RUNNABLE} PUBLIC ${OBJ_LIB})

# clang-format
include(Format)
Format(${RUNNABLE} .)

add_library(${STATIC_LIB} STATIC)
target_link_libraries(${STATIC_LIB} ${OBJ_LIB})

# copy config.properties
add_custom_command(
        TARGET ${RUNNABLE} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/config.properties
        ${CMAKE_CURRENT_BINARY_DIR}/config.properties
)
