set(STATIC_LIB "${CMAKE_PROJECT_NAME}-static")
set(TEST_OBJ "${CMAKE_PROJECT_NAME}-test")
set(TEST_RUNNABLE "${CMAKE_PROJECT_NAME}-test-runnable")
set(TEST_STATIC_LIB "${CMAKE_PROJECT_NAME}-test-static")

include(Testing)
add_library(${TEST_OBJ} OBJECT
        file_utils.cc
        file_utils.h
        fake_configuration.h
        test_utils.h
        test_utils.cc
        tape_sorter_test_base.h
        test_base.h
        test_base.cc
)
target_include_directories(${TEST_OBJ} PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_link_libraries(${TEST_OBJ} PUBLIC ${STATIC_LIB})
target_link_libraries(${TEST_OBJ} PUBLIC gtest gmock)

add_executable(${TEST_RUNNABLE}
        file_tape_test.cc
        tape_sorter_test.cc
)
target_link_libraries(${TEST_RUNNABLE} PUBLIC ${TEST_OBJ})

# clang-format
include(Format)
Format(${TEST_RUNNABLE} .)

AddTests(${TEST_RUNNABLE})

add_library(${TEST_STATIC_LIB} STATIC)
target_link_libraries(${TEST_STATIC_LIB} ${TEST_OBJ})
