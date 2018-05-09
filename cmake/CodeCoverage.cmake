# Create code coverage targets

macro(create_code_coverage_targets MAIN_TARGET)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        find_program(LCOV_EXE lcov)
        find_program(GENHTML_EXE genhtml)

        if(LCOV_EXE AND GENHTML_EXE)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

            set(COVERAGE_BASE "coverage_base.info")
            set(COVERAGE_TEST "coverage_test.info")
            set(COVERAGE_ALL "coverage_all.info")
            set(COVERAGE_EXTRACTED "coverage.info")
            set(COVERAGE_DIR "coverage")

            # Initialization
            add_custom_command(
                OUTPUT ${COVERAGE_BASE}
                COMMAND ${LCOV_EXE} --capture --initial -d ${CMAKE_BINARY_DIR} -o ${COVERAGE_BASE}
                DEPENDS ${MAIN_TARGET}
            )
            add_custom_target(coverage_init ALL DEPENDS ${COVERAGE_BASE})

            # Gather traces after running tests (with `make coverage`)
            add_custom_target(coverage_test ${LCOV_EXE} --capture -d ${CMAKE_BINARY_DIR} -o ${COVERAGE_TEST} DEPENDS check)
            add_custom_target(coverage_merge ${LCOV_EXE} -a ${COVERAGE_BASE} -a ${COVERAGE_TEST} -o ${COVERAGE_ALL} DEPENDS coverage_test)
            add_custom_target(coverage_extract ${LCOV_EXE}
                -e ${COVERAGE_ALL} \"${CMAKE_SOURCE_DIR}/src/*\"
                -e ${COVERAGE_ALL} \"${CMAKE_BINARY_DIR}/src/*\"
                -o ${COVERAGE_EXTRACTED}
                DEPENDS coverage_merge)
            add_custom_target(coverage_html ${GENHTML_EXE} ${COVERAGE_EXTRACTED} -o ${COVERAGE_DIR} DEPENDS coverage_extract)
            add_custom_target(coverage
                ${CMAKE_COMMAND} -E echo "To view code coverage results, run `firefox ${COVERAGE_DIR}/index.html` or refresh the page if already open"
                DEPENDS coverage_html VERBATIM)
        else()
            message(WARNING "Code coverage requires `lcov` and `genhtml` executables. "
                            "They should both be available after installing the package `lcov`.")
        endif()
    else()
        message(WARNING "Code coverage is only enabled with a GNU compiler (g++)")
    endif()
endmacro()
