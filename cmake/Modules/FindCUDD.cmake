# Find CUDD (PAGAI-specific)
#
# Influential variables:
#
#     EXTERNAL_PREFIX       Prefix where local installations are done
#     USE_SYSTEM            Use system installation
#     CUDD_VERSION          Version of cudd to look for
#
# This script sets the following variables:
#
#     CUDD_INCLUDE_DIRS     Directories to include for cudd
#     CUDD_LIBRARIES        Libraries to link with for cudd
#

if(NOT USE_SYSTEM)
    
    set(CUDD_PATH "${EXTERNAL_PREFIX}/cudd-${CUDD_VERSION}/src/cudd")
    find_path(CUDD_INCLUDE_DIR NAMES cudd.h PATHS "${CUDD_PATH}/include" NO_DEFAULT_PATH)
    find_library(CUDD_obj_LIBRARY NAMES obj PATHS "${CUDD_PATH}/obj" "${CUDD_PATH}/lib" NO_DEFAULT_PATH)
    find_library(CUDD_cudd_LIBRARY NAMES cudd PATHS "${CUDD_PATH}/cudd" "${CUDD_PATH}/lib" NO_DEFAULT_PATH)
    find_library(CUDD_util_LIBRARY NAMES util PATHS "${CUDD_PATH}/util" "${CUDD_PATH}/lib" NO_DEFAULT_PATH)
    find_library(CUDD_mtr_LIBRARY NAMES mtr PATHS "${CUDD_PATH}/mtr" "${CUDD_PATH}/lib" NO_DEFAULT_PATH)
    find_library(CUDD_st_LIBRARY NAMES st PATHS "${CUDD_PATH}/st" "${CUDD_PATH}/lib" NO_DEFAULT_PATH)
    find_library(CUDD_epd_LIBRARY NAMES epd PATHS "${CUDD_PATH}/epd" "${CUDD_PATH}/lib" NO_DEFAULT_PATH)
    unset(CUDD_PATH)

else()

    find_path(CUDD_INCLUDE_DIR NAMES cudd.h)
    find_library(CUDD_obj_LIBRARY NAMES obj)
    find_library(CUDD_cudd_LIBRARY NAMES cudd)
    find_library(CUDD_util_LIBRARY NAMES util)
    find_library(CUDD_mtr_LIBRARY NAMES mtr)
    find_library(CUDD_st_LIBRARY NAMES st)
    find_library(CUDD_epd_LIBRARY NAMES epd)

endif()

# Finally, send the results

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CUDD DEFAULT_MSG CUDD_VERSION
    CUDD_INCLUDE_DIR
    CUDD_obj_LIBRARY
    CUDD_cudd_LIBRARY
    CUDD_util_LIBRARY
    CUDD_mtr_LIBRARY
    CUDD_st_LIBRARY
    CUDD_epd_LIBRARY)
mark_as_advanced(
    CUDD_INCLUDE_DIR
    CUDD_obj_LIBRARY
    CUDD_cudd_LIBRARY
    CUDD_util_LIBRARY
    CUDD_mtr_LIBRARY
    CUDD_st_LIBRARY
    CUDD_epd_LIBRARY)

set(CUDD_INCLUDE_DIRS ${CUDD_INCLUDE_DIR})
set(CUDD_LIBRARIES
    ${CUDD_obj_LIBRARY}
    ${CUDD_cudd_LIBRARY}
    ${CUDD_util_LIBRARY}
    ${CUDD_mtr_LIBRARY}
    ${CUDD_st_LIBRARY}
    ${CUDD_epd_LIBRARY})
