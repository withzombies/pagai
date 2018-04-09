include(CMakeParseArguments)

add_custom_target(build_tests)

# Common macros to simplify further test functions

# Variables that can be set before calling this macro:
#
#     OPTIONS               Add type-specific options
#     ONE_VALUE_ARGS        Add type-specific one-value arguments
#     MULTI_VALUE_ARGS      Add type-specific multi-value arguments
#
macro(COMMON_TEST_PARSE_ARGUMENTS)
    set(OPTIONS
        MUST_FAIL               # If true, the test's return status is inverted (test fail = success)
        ${OPTIONS}              # Type-specific options
    )
    set(ONE_VALUE_ARGS
        ONLY_IF_CONFIG          # Do the test only for the given config (see main CMakeLists.txt)
        ${ONE_VALUE_ARGS}       # Type-specific one-value arguments
    )
    set(MULTI_VALUE_ARGS
        PAGAI_EXTRA_ARGS        # Specify extra arguments to give to pagai for the test
        ${MULTI_VALUE_ARGS}     # Type-specific multi-value arguments
    )
    cmake_parse_arguments(ARG "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})
endmacro()

macro(COMMON_TEST_CREATE_TARGET TEST_NAME TEST_DRIVER)
    if(NOT ARG_ONLY_IF_CONFIG OR "${ARG_ONLY_IF_CONFIG}" STREQUAL "${CONFIG}")
        add_test(
			NAME "${TEST_NAME}"
            COMMAND ${TEST_DRIVER}
                $<TARGET_FILE:pagai>
                ${ARGN}                     # Type-specific pagai arguments
                -I ${CMAKE_SOURCE_DIR}      # Include path to "pagai_assert.h"
                ${ARG_PAGAI_EXTRA_ARGS}     # Test-specific pagai arguments
        )

        if(ARG_MUST_FAIL)
            set_tests_properties(${TEST_TARGET_NAME} PROPERTIES WILL_FAIL TRUE)
        endif()
    endif()
endmacro()

# Non-regression tests settings

set(NONREG_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/non_regression")
set(NONREG_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/non_regression")
file(MAKE_DIRECTORY "${NONREG_OUTPUT_DIR}")

function(ADD_NONREG_TEST TARGET)
    common_test_parse_arguments(${ARGN})
    common_test_create_target("nonreg_${TARGET}" "${NONREG_SOURCE_DIR}/test_driver.sh"
        "${NONREG_SOURCE_DIR}/${TARGET}.c"
        "${NONREG_OUTPUT_DIR}/${TARGET}.out.c"
    )
endfunction()

# Asserts tests settings

set(ASSERTS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/asserts")

function(ADD_ASSERTS_TEST TARGET)
    common_test_parse_arguments(${ARGN})
    common_test_create_target("asserts_${TARGET}" "${ASSERTS_SOURCE_DIR}/test_driver.sh"
        "${ASSERTS_SOURCE_DIR}/${TARGET}.c"
    )
endfunction()

# Command line

set(COMMAND_LINE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/command_line")

function(ADD_COMMANDLINE_TEST TARGET)
    common_test_parse_arguments(${ARGN})
    common_test_create_target("commandline_${TARGET}" "")
endfunction()

# Reproduce known bugs

set(KNOWN_BUGS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/known_bugs")

function(ADD_KNOWN_BUG_REPRODUCTION TARGET)
    set(OPTIONS
        DIFFERENT_OUTPUTS   # Reproduce the bug = get different outputs (file.out.1.c, file.out.2.c, etc. from the source folder)
    )
    common_test_parse_arguments(${ARGN})

    if(ARG_DIFFERENT_OUTPUTS)
        set(TEST_DRIVER "${KNOWN_BUGS_SOURCE_DIR}/test_driver_different_outputs.sh")
    else()
        message(FATAL_ERROR "Unspecified type for the reproduction test of a known bug")
    endif()
    common_test_create_target("reproduce_known_bugs_${TARGET}" "${TEST_DRIVER}"
        "${KNOWN_BUGS_SOURCE_DIR}/${TARGET}.c"
    )
endfunction()

# Unit tests

set(UNIT_TESTS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/unit")

function(ADD_UNIT_TEST TARGET)
    set(MULTI_VALUE_ARGS
        PAGAI_SOURCE_FILES  # Source files to compile with the unit test
    )
    common_test_parse_arguments(${ARGN})

    if(NOT ARG_ONLY_IF_CONFIG OR "${ARG_ONLY_IF_CONFIG}" STREQUAL "${CONFIG}")
        add_executable(${TARGET} EXCLUDE_FROM_ALL "${UNIT_TESTS_SOURCE_DIR}/${TARGET}.cc" ${ARG_PAGAI_SOURCE_FILES})
        add_test(NAME unit_${TARGET} COMMAND ${TARGET})
        add_dependencies(build_tests ${TARGET})
    endif()
endfunction()

# Tests

add_nonreg_test(empty_main)
add_nonreg_test(one_variable_for)
add_nonreg_test(one_variable_while)

add_asserts_test(simple)
add_asserts_test(two_variables_for)

add_commandline_test(version PAGAI_EXTRA_ARGS --version)
add_commandline_test(help PAGAI_EXTRA_ARGS --help)

add_unit_test(canonize_line PAGAI_SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/utilities.cc")

# Known bug reproduction

add_known_bug_reproduction(three_variables_for DIFFERENT_OUTPUTS ONLY_IF_CONFIG llvm3.6)        # Issue #6 on GitLab
