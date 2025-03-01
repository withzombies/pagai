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
    BUILD_COMMAND $(MAKE) ${CUDD_XCFLAGS} && $(MAKE) -C obj ${CUDD_XCFLAGS} testobj

    # Just check that the build went OK, it's fast
    TEST_BEFORE_INSTALL 1
    TEST_COMMAND obj/testobj

    INSTALL_COMMAND ""
)
DepInstall_add_target(cudd)

# Z3

if(ENABLE_Z3)
    find_package(PythonInterp 2 REQUIRED)
    ExternalProject_add(z3
        PREFIX "${EXTERNAL_PREFIX}/z3-${Z3_VERSION}"

        GIT_REPOSITORY "https://github.com/Z3Prover/z3"
        GIT_TAG "z3-${Z3_VERSION}"

        # No update command (this git won't be updated)
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND $(MAKE)

        BINARY_DIR "${EXTERNAL_PREFIX}/z3-${Z3_VERSION}/src/z3/build"
    )
    ExternalProject_add_step(z3 python_mkmake
        WORKING_DIRECTORY <SOURCE_DIR>
        DEPENDEES download
        COMMAND "${PYTHON_EXECUTABLE}" scripts/mk_make.py --prefix=<INSTALL_DIR>
        DEPENDERS configure)
    DepInstall_add_target(z3)
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
    DepInstall_add_target(yices)
endif()

# LLVM + clang

if(LLVM_VERSION VERSION_GREATER 3.4.2)
    set(CLANG_ARCHIVE_WE "cfe-${LLVM_VERSION}.src")
    set(CLANG_ARCHIVE_EXT "tar.xz")
    set(LLVM_ARCHIVE "llvm-${LLVM_VERSION}.src.tar.xz")
elseif(LLVM_VERSION VERSION_GREATER 3.4)
    set(CLANG_ARCHIVE_WE "cfe-${LLVM_VERSION}.src")
    set(CLANG_ARCHIVE_EXT "tar.gz")
    set(LLVM_ARCHIVE "llvm-${LLVM_VERSION}.src.tar.gz")
else()
    set(CLANG_ARCHIVE_WE "clang-${LLVM_VERSION}")
    set(CLANG_ARCHIVE_EXT "src.tar.gz")
    set(LLVM_ARCHIVE "llvm-${LLVM_VERSION}.src.tar.gz")
endif()

set(CLANG_DOWNLOAD_PATH_TMP "${CMAKE_BINARY_DIR}/external/${CLANG_ARCHIVE_WE}.${CLANG_ARCHIVE_EXT}")
file(DOWNLOAD
    "http://releases.llvm.org/${LLVM_VERSION}/${CLANG_ARCHIVE_WE}.${CLANG_ARCHIVE_EXT}"
    "${CLANG_DOWNLOAD_PATH_TMP}"
)
ExternalProject_add(llvm
    PREFIX "${EXTERNAL_PREFIX}/llvm-${LLVM_VERSION}"

    URL "http://releases.llvm.org/${LLVM_VERSION}/${LLVM_ARCHIVE}"

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --enable-bindings=none --prefix=<INSTALL_DIR>
    BUILD_COMMAND REQUIRES_RTTI=1 $(MAKE)
)
ExternalProject_add_step(llvm get_clang
    WORKING_DIRECTORY <SOURCE_DIR>/tools
    DEPENDEES download
    COMMAND ${CMAKE_COMMAND} -E remove_directory clang
    COMMAND ${CMAKE_COMMAND} -E tar xf "${CLANG_DOWNLOAD_PATH_TMP}"
    COMMAND ${CMAKE_COMMAND} -E rename "${CLANG_ARCHIVE_WE}" clang
    DEPENDERS configure
)
DepInstall_add_target(llvm)

# PPL

if(ENABLE_PPL)
    ExternalProject_add(ppl
        PREFIX "${EXTERNAL_PREFIX}/ppl-${PPL_VERSION}"
        URL "ftp://ftp.cs.unipr.it/pub/ppl/releases/${PPL_VERSION}/ppl-${PPL_VERSION}.tar.gz"
        CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --enable-interfaces=c,c++
        BUILD_COMMAND $(MAKE)
    )
    DepInstall_add_target(ppl)
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

    URL "https://gricad-gitlab.univ-grenoble-alpes.fr/pagai/apron-clone/raw/master/archives/apron-${APRON_VERSION}.tar.xz"

    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND "${CMAKE_COMMAND}"
        -DAPRON_SOURCEDIR=<SOURCE_DIR>
        -DAPRON_INSTALL_PREFIX=<INSTALL_DIR>
        -DAPRON_HAS_SHARED=${APRON_HAS_SHARED}
        -DPPL_INSTALL_PREFIX=${PPL_PREFIX}
        -DPPL_ENABLED=${APRON_PPL_ENABLED}
        -P "${CMAKE_SOURCE_DIR}/cmake/ConfigureApron.cmake"
    BUILD_COMMAND $(MAKE)
)
if(ENABLE_PPL)
    DepInstall_add_target(apron DEPENDS ppl)
else()
    DepInstall_add_target(apron)
endif()
