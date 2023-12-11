set(OBJ_LIB_TARGET ${CMAKE_PROJECT_NAME})
set(STATIC_LIB_TARGET "${OBJ_LIB_TARGET}-static")
set(TEST_TARGET "${OBJ_LIB_TARGET}-test")

add_executable(${TEST_TARGET}
        file_tape_test.cc
        file_utils.cc
        file_utils.h
        fake_configuration.h
        test_utils.h
        test_utils.cc
)
target_include_directories(${TEST_TARGET} PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")

# clang-format
include(Format)
Format(${TEST_TARGET} .)

target_link_libraries(${TEST_TARGET} PUBLIC ${STATIC_LIB_TARGET})

AddTests(${TEST_TARGET})
