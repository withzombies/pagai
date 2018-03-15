# Phony target (to run all the installations)

DepInstall_create_target()

# Configuration

if(NOT EXTERNAL_PREFIX)
    message(FATAL_ERROR "Please make sure EXTERNAL_PREFIX is set")
endif()

include(ExternalProject)

set(CUDD_XCFLAGS "")
set(YICES_ARCH i686-pc-linux-gnu-static-gmp)

execute_process(COMMAND uname -m OUTPUT_VARIABLE ARCH)
if(${ARCH} MATCHES amd64 OR ${ARCH} MATCHES x86_64)
    set(CUDD_XCFLAGS "XCFLAGS=-mtune=native -DHAVE_IEEE_754 -DBSD -DSIZEOF_VOID_P=8 -DSIZEOF_LONG=8")
    set(Z3_ARCH -x64)
    set(YICES_ARCH x86_64-unknown-linux-gnu-static-gmp)
elseif(${ARCH} MATCHES sparc64)
    set(CUDD_XCFLAGS "XCFLAGS=-DHAVE_IEEE_754 -DBSD -DSIZEOF_VOID_P=4 -DSIZEOF_LONG=4")
endif()

execute_process(COMMAND uname OUTPUT_VARIABLE OS)
if("${OS}" STREQUAL "Darwin" OR "${OS}" STREQUAL "Cygwin")
    set(APRON_HAS_SHARED "")
else()
    set(APRON_HAS_SHARED 1)
endif()

# CUDD

ExternalProject_add(cudd
    PREFIX "${EXTERNAL_PREFIX}/cudd-${CUDD_VERSION}"

    GIT_REPOSITORY "https://github.com/ivmai/cudd"
    GIT_TAG "cudd-${CUDD_VERSION}"

    # No update command (this git won't be updated)
    UPDATE_COMMAND ""
    PATCH_COMMAND patch -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/cudd-${CUDD_VERSION}.patch

    CONFIGURE_COMMAND ""

    BUILD_IN_SOURCE 1
    BUILD_COMMAND ${CMAKE_MAKE_PROGRAM} ${CUDD_XCFLAGS} && ${CMAKE_MAKE_PROGRAM} -C obj ${CUDD_XCFLAGS} testobj

    # Just check that the build went OK, it's fast
    TEST_BEFORE_INSTALL 1
    TEST_COMMAND obj/testobj

    INSTALL_COMMAND ""
)
DepInstall_add(TARGET cudd)

# Z3

if(ENABLE_Z3)
    find_package(PythonInterp REQUIRED)
    ExternalProject_add(z3
        PREFIX "${EXTERNAL_PREFIX}/z3-${Z3_VERSION}"

        GIT_REPOSITORY "https://github.com/Z3Prover/z3"
        GIT_TAG "${Z3_VERSION}"

        # No update command (this git won't be updated)
        UPDATE_COMMAND ""

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    )
    ExternalProject_add_step(z3 bootstrap_create
        WORKING_DIRECTORY <SOURCE_DIR>
        DEPENDEES update
        COMMAND ${PYTHON_EXECUTABLE} contrib/cmake/bootstrap.py create
        DEPENDERS configure
    )
    DepInstall_add(TARGET z3)
endif()

# YICES

if(ENABLE_YICES)
    add_custom_command(OUTPUT "${EXTERNAL_PREFIX}/yices-${YICES_VERSION}/bin/yices"
        COMMAND "${CMAKE_COMMAND}"
        -DURL:STRING="http://yices.csl.sri.com/cgi-bin/yices-newdownload.cgi?file=yices-${YICES_VERSION}-${YICES_ARCH}.tar.gz&accept=I+accept"
        -DDOWNLOAD_DEST="${CMAKE_CURRENT_BINARY_DIR}/yices.tar.gz"
        -DUNTAR_TO="${EXTERNAL_PREFIX}"
        -P "${CMAKE_SOURCE_DIR}/cmake/SimpleInstall.cmake")
    add_custom_target(yices DEPENDS "${EXTERNAL_PREFIX}/yices-${YICES_VERSION}/bin/yices")
    DepInstall_add(TARGET yices)
endif()

# LLVM + clang

set(CLANG_ARCHIVE           "clang-${LLVM_VERSION}.src.tar.gz")
set(CLANG_DOWNLOAD_PATH_TMP "${CMAKE_BINARY_DIR}/${CLANG_ARCHIVE}")
file(DOWNLOAD
    "http://releases.llvm.org/${LLVM_VERSION}/${CLANG_ARCHIVE}"
    "${CLANG_DOWNLOAD_PATH_TMP}"
)
ExternalProject_add(llvm
    PREFIX "${EXTERNAL_PREFIX}/llvm-${LLVM_VERSION}"

    URL "http://releases.llvm.org/${LLVM_VERSION}/llvm-${LLVM_VERSION}.src.tar.gz"

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --enable-bindings=none --prefix=<INSTALL_DIR>
    BUILD_COMMAND REQUIRES_RTTI=1 ${CMAKE_MAKE_PROGRAM}
)
ExternalProject_add_step(llvm get_clang
    WORKING_DIRECTORY <SOURCE_DIR>/tools
    DEPENDEES download
    COMMAND ${CMAKE_COMMAND} -E remove_directory clang
    COMMAND ${CMAKE_COMMAND} -E tar xf "${CLANG_DOWNLOAD_PATH_TMP}"
    COMMAND ${CMAKE_COMMAND} -E remove "${CLANG_DOWNLOAD_PATH_TMP}"
    COMMAND ${CMAKE_COMMAND} -E rename "clang-${LLVM_VERSION}" clang
    DEPENDERS configure
)
DepInstall_add(TARGET llvm)

# PPL

if(ENABLE_PPL)
    ExternalProject_add(ppl
        PREFIX "${EXTERNAL_PREFIX}/ppl-${PPL_VERSION}"
        URL "ftp://ftp.cs.unipr.it/pub/ppl/releases/${PPL_VERSION}/ppl-${PPL_VERSION}.tar.gz"
        CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --enable-interfaces=c,c++
    )
    DepInstall_add(TARGET ppl)
endif()

# APRON

set(APRON_PREFIX "${EXTERNAL_PREFIX}/apron-${APRON_VERSION}")
if(ENABLE_PPL)
    set(APRON_PREFIX "${APRON_PREFIX}-withppl")
    set(APRON_PPL_ENABLED 1)
    get_target_property(PPL_PREFIX ppl INSTALL_DIR)
else()
    unset(APRON_PPL_ENABLED)
    unset(PPL_PREFIX)
endif()

ExternalProject_add(apron
    PREFIX "${APRON_PREFIX}"

    URL "http://forge.imag.fr/frs/download.php/702/apron_${APRON_VERSION}.tar.gz"

    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND "${CMAKE_COMMAND}"
        -DAPRON_SOURCEDIR=<SOURCE_DIR>
        -DAPRON_INSTALL_PREFIX=<INSTALL_DIR>
        -DAPRON_HAS_SHARED=${APRON_HAS_SHARED}
        -DPPL_INSTALL_PREFIX=${PPL_PREFIX}
        -DPPL_ENABLED=${APRON_PPL_ENABLED}
        -P "${CMAKE_SOURCE_DIR}/cmake/ConfigureApron.cmake"
)

DepInstall_add(TARGET apron)
if(ENABLE_PPL)
    add_dependencies(apron ppl)
endif()
