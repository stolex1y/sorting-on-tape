set(OBJ_LIB_TARGET ${CMAKE_PROJECT_NAME})
set(RUNNABLE_TARGET "${OBJ_LIB_TARGET}-runnable")
set(STATIC_LIB_TARGET "${OBJ_LIB_TARGET}-static")

add_library(${OBJ_LIB_TARGET} OBJECT
        tape.h
        file_tape.h
        configuration.cc
        configuration.h
)
target_include_directories(${OBJ_LIB_TARGET} PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

#warnings
target_compile_options(${OBJ_LIB_TARGET} PRIVATE "-Werror" "-Wall" "-Wextra" "-Wpedantic")

add_executable(${RUNNABLE_TARGET}
        main.cc
)
target_link_libraries(${RUNNABLE_TARGET} PUBLIC ${OBJ_LIB_TARGET})

# clang-format
include(Format)
Format(${RUNNABLE_TARGET} .)

add_library(${STATIC_LIB_TARGET} STATIC)
target_link_libraries(${STATIC_LIB_TARGET} ${OBJ_LIB_TARGET})

# copy config.properties
add_custom_command(
        TARGET ${RUNNABLE_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/config.properties
        ${CMAKE_CURRENT_BINARY_DIR}/config.properties
)
