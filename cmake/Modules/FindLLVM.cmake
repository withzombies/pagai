# Find LLVM (PAGAI-specific)
#
# Influential variables:
#
#     EXTERNAL_PREFIX       Prefix where local installations are done
#     ENABLE_PROFILING      Tells if profiling should be enabled or not (ON/OFF)
#     LLVM_VERSION          LLVM version to look for
#     USE_SYSTEM            Use system installation
#
# This script sets the following variables:
#
#     LLVM_PATH             Path to the LLVM install
#     LLVM_CXX_FLAGS        Compiler flags required
#     LLVM_LD_FLAGS         Linker flags required
#     LLVM_LIBRARIES        Libraries to link with
#

if(NOT USE_SYSTEM)
    find_program(LLVM_CONFIG_EXE NAMES llvm-config PATHS "${EXTERNAL_PREFIX}/llvm-${LLVM_VERSION}/bin" NO_DEFAULT_PATH)
else()
    find_program(LLVM_CONFIG_EXE NAMES llvm-config)
endif()

if(LLVM_CONFIG_EXE)
    execute_process(
        COMMAND "${LLVM_CONFIG_EXE}" --version
        OUTPUT_VARIABLE LLVM_CONFIG_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE) # also removes trailing newlines

    if("${LLVM_FIND_VERSION}" STREQUAL "${LLVM_CONFIG_VERSION}")
        # C++ flags
        execute_process(
            COMMAND "${LLVM_CONFIG_EXE}" --cxxflags
            OUTPUT_VARIABLE LLVM_CONFIG_CXXFLAGS
            OUTPUT_STRIP_TRAILING_WHITESPACE)

        # llvm-config has a -fno-rtti -fno-exception by default, but we use dynamic_cast<>
        string(REPLACE "-fno-rtti" "" LLVM_CONFIG_CXXFLAGS "${LLVM_CONFIG_CXXFLAGS}")
        string(REPLACE "-fno-exceptions" "" LLVM_CONFIG_CXXFLAGS "${LLVM_CONFIG_CXXFLAGS}")
        # Remove useless flag set by LLVM (unknown by clang, cause false warning messages)
        string(REPLACE "-Wno-maybe-uninitialized" "" LLVM_CXX_FLAGS "${LLVM_CXX_FLAGS}")

        # when compiling with -pg, -fomit-frame-pointer is not allowed
        if(ENABLE_PROFILING)
            string(REPLACE "-fomit-frame-pointer" "" LLVM_CONFIG_CXXFLAGS "${LLVM_CONFIG_CXXFLAGS}")
        endif()

        # LD flags
        execute_process(
            COMMAND "${LLVM_CONFIG_EXE}" --ldflags
            OUTPUT_VARIABLE LLVM_CONFIG_LDFLAGS
            OUTPUT_STRIP_TRAILING_WHITESPACE)

        # Libraries
        execute_process(
            COMMAND "${LLVM_CONFIG_EXE}" --libs bitreader selectiondag all
            OUTPUT_VARIABLE LLVM_CONFIG_LIBS
            OUTPUT_STRIP_TRAILING_WHITESPACE)

        # Add clang libs
        set(LLVM_CONFIG_LIBS "-lclangFrontendTool -lclangFrontend -lclangDriver -lclangSerialization -lclangCodeGen -lclangParse -lclangSema -lclangStaticAnalyzerFrontend -lclangStaticAnalyzerCheckers -lclangStaticAnalyzerCore -lclangAnalysis -lclangARCMigrate -lclangEdit -lclangAST -lclangLex -lclangBasic ${LLVM_CONFIG_LIBS}")

        # Add system libs for LLVM
        # For LLVM < 3.5, the argument "--system-libs" does not exist and will
        # not return anything, after it returns the required system libraries
        execute_process(
            COMMAND "${LLVM_CONFIG_EXE}" --system-libs ERROR_QUIET
            OUTPUT_VARIABLE LLVM_SYSTEM_LIBS
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(LLVM_SYSTEM_LIBS)
            set(LLVM_CONFIG_LIBS "${LLVM_CONFIG_LIBS} ${LLVM_SYSTEM_LIBS}")
        endif()
    else()
        message(STATUS "Local install of LLVM (${LLVM_CONFIG_VERSION}) does not match version requirements (${LLVM_FIND_VERSION}), "
            "fallback to finding it in the system path")
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LLVM DEFAULT_MSG LLVM_VERSION LLVM_CONFIG_LIBS LLVM_CONFIG_CXXFLAGS LLVM_CONFIG_LDFLAGS)
mark_as_advanced(LLVM_CONFIG_CXXFLAGS LLVM_CONFIG_LDFLAGS LLVM_CONFIG_LIBS)

set(LLVM_PATH "${EXTERNAL_PREFIX}/llvm-${LLVM_VERSION}")
set(LLVM_CXX_FLAGS "${LLVM_CONFIG_CXXFLAGS}")
set(LLVM_LD_FLAGS "${LLVM_CONFIG_LDFLAGS}")
set(LLVM_LIBRARIES "${LLVM_CONFIG_LIBS}")
