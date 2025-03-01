cmake_minimum_required(VERSION 2.8.4 FATAL_ERROR)

# Check that user does an "out-of-source" build
if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    message(FATAL_ERROR "In-source build is not enabled. This avoids polluting the source tree and eases clean-up. "
                        "Please build from a different directory.\n"
                        "    Example: mkdir build && cd build && cmake ..\n"
                        "Before that, please clean up manually files that CMake generated in the source tree:\n"
                        "    rm -rf CMakeCache.txt CMakeFiles/")
endif()

project(PAGAI CXX)

set(CMAKE_CXX_STANDARD 11)

include(cmake/SetupDefaultCompilerDirs.cmake)

# PAGAI versioning
# See: https://semver.org/

set(PAGAI_VERSION_MAJOR 1)
set(PAGAI_VERSION_MINOR 0)
set(PAGAI_VERSION_PATCH 0)
set(PAGAI_VERSION ${PAGAI_VERSION_MAJOR}.${PAGAI_VERSION_MINOR}.${PAGAI_VERSION_PATCH})

# Options

option(ENABLE_PROFILING "Whether to build using -pg" OFF)
option(ENABLE_DEBUGGING "Whether to build using -g" OFF)
option(ENABLE_COVERAGE "Whether to enable coverage tests (gcc only)" OFF)
option(ENABLE_YICES "Whether to enable Yices" ON)
option(ENABLE_Z3 "Whether to enable Z3 through api" ON)
option(ENABLE_PPL "Whether APRON has been compiled with PPL (otherwise uses NewPolka)" OFF)
option(PRINT_DEBUG "Activate debug traces" OFF)
option(PRINT_DEBUG_SMT "Print the SMT formulae during the analysis" OFF)
option(USE_SYSTEM_APRON "Whether to use Apron from system (no local installation)" OFF)
option(USE_SYSTEM_CUDD "Whether to use CUDD from system (no local installation)" OFF)
option(USE_SYSTEM_Z3 "Whether to use Z3 from system (no local installation)" OFF)
option(USE_SYSTEM_YICES "Whether to use Yices from system (no local installation)" OFF)
option(USE_SYSTEM_LLVM "Whether to use LLVM from system (no local installation)" OFF)
option(USE_SYSTEM_PPL "Whether to use PPL from system (no local installation)" OFF)

# Configuration

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
include(cmake/ExternalProject.patch.cmake)

if(PRINT_DEBUG)
    add_definitions(-DPRINT_DEBUG)
endif()
if(ENABLE_Z3)
    add_definitions(-DHAS_Z3)
endif()
if(ENABLE_YICES)
    add_definitions(-DHAS_YICES)
endif()
if(ENABLE_PPL)
    add_definitions(-DPPL_ENABLED)
endif()
if(PRINT_DEBUG_SMT)
    add_definitions(-DPRINT_DEBUG_SMT)
endif()

# External projects

# Setting up external path in the "source" dir allows for an easy and light clean (rm -rf build/)
# without having to reinstall all the dependencies (which is very long)

set(EXTERNAL_PREFIX "${CMAKE_SOURCE_DIR}/external/install" CACHE PATH "Path to auto-installed external dependencies")

# Dependencies that are not auto-installed

find_package(GMP REQUIRED)
find_package(MPFR REQUIRED)
find_package(BISON 3.0)
find_package(FLEX 2.5.35)
find_package(Boost REQUIRED COMPONENTS program_options)
find_package(Curses REQUIRED)

# Possibly auto-installed dependencies (the "find_package" files are in the cmake/ directory)

include(cmake/DepInstall.cmake)

DepInstall_add_config(llvm3.4
    LLVM_VERSION 3.4
    CUDD_VERSION 2.5.1
    APRON_VERSION 0.9.11
    Z3_VERSION 4.5.0
    PPL_VERSION 1.1
    YICES_VERSION 1.0.40)

DepInstall_add_config(llvm3.6
    LLVM_VERSION 3.6.2
    CUDD_VERSION 2.5.1
    APRON_VERSION 0.9.11
    Z3_VERSION 4.5.0
    PPL_VERSION 1.1
    YICES_VERSION 1.0.40)

DepInstall_check_config() # sets variables corresponding to version keys of the chosen config

add_subdirectory(external)

DepInstall_find_package(CUDD
    VERSION ${CUDD_VERSION}
    USE_SYSTEM ${USE_SYSTEM_CUDD}
    DEFINES CUDD_INCLUDE_DIRS CUDD_LIBRARIES)
DepInstall_find_package(LLVM
    VERSION ${LLVM_VERSION}
    USE_SYSTEM ${USE_SYSTEM_LLVM}
    DEFINES LLVM_CXX_FLAGS LLVM_LD_FLAGS LLVM_LIBRARIES)
DepInstall_find_package(APRON
    VERSION ${APRON_VERSION}
    USE_SYSTEM ${USE_SYSTEM_APRON}
    DEFINES APRON_INCLUDE_DIRS APRON_LIBRARIES)
DepInstall_find_package(Z3 IF_OPT ENABLE_Z3
    VERSION ${Z3_VERSION}
    USE_SYSTEM ${USE_SYSTEM_Z3}
    DEFINES Z3_INCLUDE_DIRS Z3_LIBRARIES)
DepInstall_find_package(YICES IF_OPT ENABLE_YICES
    VERSION ${YICES_VERSION}
    USE_SYSTEM ${USE_SYSTEM_YICES}
    DEFINES YICES_INCLUDE_DIRS YICES_LIBRARIES)
DepInstall_find_package(PPL IF_OPT ENABLE_PPL
    VERSION ${PPL_VERSION}
    USE_SYSTEM ${USE_SYSTEM_PPL}
    DEFINES PPL_LIBRARIES)

if(DEPINSTALL_PACKAGE_NOTFOUND)
    return()
endif()

# Defines the following variables:
#
#     PAGAI_SRC     Source files for PAGAI
#
add_subdirectory(src)

# Defines the build targets

flex_target(SMTlib2lexer ${CMAKE_SOURCE_DIR}/src/SMTlib2lexer.lex
    ${CMAKE_BINARY_DIR}/src/SMTlib2lexer.cc
    COMPILE_FLAGS --always-interactive
)
bison_target(SMTlib2parser ${CMAKE_SOURCE_DIR}/src/SMTlib2parser.yy
    ${CMAKE_BINARY_DIR}/src/SMTlib2parser.cc
    DEFINES_FILE ${CMAKE_BINARY_DIR}/src/SMTlib2parser.hh
    COMPILE_FLAGS --report=state
)
add_flex_bison_dependency(SMTlib2lexer SMTlib2parser)

add_executable(pagai
    ${PAGAI_SRC}
    ${BISON_SMTlib2parser_OUTPUT_SOURCE}
    ${FLEX_SMTlib2lexer_OUTPUTS}
)

# Final configuration steps

# Must be configured here (after all CMake variables are defined)
include(cmake/MakeConfigFile.cmake)
include(cmake/CodeCoverage.cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXX_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LLVM_LD_FLAGS}")
if(ENABLE_COVERAGE)
    create_code_coverage_targets(pagai)
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}/src # to compile flex/yacc files
    ${CMAKE_BINARY_DIR}/src # files generated by flex/yacc
    ${Boost_INCLUDE_DIR}
    ${GMP_INCLUDE_DIRS}
    ${MPFR_INCLUDE_DIRS}
    ${CUDD_INCLUDE_DIRS}
    ${Z3_INCLUDE_DIRS}
    ${YICES_INCLUDE_DIRS}
    ${APRON_INCLUDE_DIRS}
)
target_link_libraries(pagai
    ${Boost_LIBRARIES}
    ${LLVM_LIBRARIES}
    ${APRON_LIBRARIES}
    ${CURSES_LIBRARIES} # TODO: find why?
    ${CUDD_LIBRARIES}
    ${Z3_LIBRARIES}
    ${YICES_LIBRARIES} # only set if ENABLE_YICES is ON
    ${MPFR_LIBRARIES}
    ${PPL_LIBRARIES} # only set if ENABLE_PPL is ON
    ${GMP_LIBRARIES}
)

# Tests

enable_testing()
add_subdirectory(tests)

# Create documentation target (requires doxygen)

find_program(DOXYGEN_EXE doxygen)
if(DOXYGEN_EXE)
    configure_file("${CMAKE_SOURCE_DIR}/src/Doxyfile.in" "${CMAKE_BINARY_DIR}/src/Doxyfile" @ONLY)
    add_custom_target(doc COMMAND "${DOXYGEN_EXE}" "${CMAKE_BINARY_DIR}/src/Doxyfile")
else()
    message(WARNING "Doxygen could not be found, thus the documentation cannot be built")
endif()
