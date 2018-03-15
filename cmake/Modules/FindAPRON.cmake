# Find APRON (PAGAI-specific)
#
# Influential variables:
#
#     EXTERNAL_PREFIX       Prefix where local installations are done
#     APRON_VERSION         Version of apron to search for
#     USE_SYSTEM            Look for a system install
#     ENABLE_PPL            PPL is enabled
#
# This script sets the following variables:
#
#     APRON_INCLUDE_DIRS    Directories to include
#     APRON_LIBRARIES       Libraries to link with
#

if(NOT USE_SYSTEM)

    set(APRON_PATH "${EXTERNAL_PREFIX}/apron-${APRON_VERSION}")
    if(ENABLE_PPL)
        set(APRON_PATH "${APRON_PATH}-withppl")
    endif()

    find_path(APRON_INCLUDE_DIR NAMES ap_global1.h PATHS "${APRON_PATH}/include" NO_DEFAULT_PATH)

    find_library(APRON_polkaMPQ_LIBRARY NAMES polkaMPQ PATHS "${APRON_PATH}/lib" NO_DEFAULT_PATH)
    find_library(APRON_octD_LIBRARY NAMES octD PATHS "${APRON_PATH}/lib" NO_DEFAULT_PATH)
    find_library(APRON_boxMPFR_LIBRARY NAMES boxMPFR PATHS "${APRON_PATH}/lib" NO_DEFAULT_PATH)
    if(ENABLE_PPL)
        find_library(APRON_ap_pkgrid_LIBRARY NAMES ap_pkgrid PATHS "${APRON_PATH}/lib" NO_DEFAULT_PATH)
        find_library(APRON_ap_ppl_LIBRARY NAMES ap_ppl PATHS "${APRON_PATH}/lib" NO_DEFAULT_PATH)
    else()
        set(APRON_ap_pkgrid_LIBRARY "unused")
        set(APRON_ap_ppl_LIBRARY "unused")
    endif()
    find_library(APRON_apron_LIBRARY NAMES apron PATHS "${APRON_PATH}/lib" NO_DEFAULT_PATH)
    unset(APRON_PATH)

else()

    find_path(APRON_INCLUDE_DIR NAMES ap_global1.h)
    find_library(APRON_polkaMPQ_LIBRARY NAMES polkaMPQ)
    find_library(APRON_octD_LIBRARY NAMES octD)
    find_library(APRON_boxMPFR_LIBRARY NAMES boxMPFR)
    if(ENABLE_PPL)
        find_library(APRON_ap_pkgrid_LIBRARY NAMES ap_pkgrid)
        find_library(APRON_ap_ppl_LIBRARY NAMES ap_ppl)
    else()
        set(APRON_ap_pkgrid_LIBRARY "unused")
        set(APRON_ap_ppl_LIBRARY "unused")
    endif()
    find_library(APRON_apron_LIBRARY NAMES apron)

endif()

# Finally, send the results

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(APRON DEFAULT_MSG APRON_VERSION
    APRON_INCLUDE_DIR
    APRON_polkaMPQ_LIBRARY
    APRON_octD_LIBRARY
    APRON_boxMPFR_LIBRARY
    APRON_ap_pkgrid_LIBRARY
    APRON_ap_ppl_LIBRARY
    APRON_apron_LIBRARY)
mark_as_advanced(
    APRON_INCLUDE_DIR
    APRON_polkaMPQ_LIBRARY
    APRON_octD_LIBRARY
    APRON_boxMPFR_LIBRARY
    APRON_ap_pkgrid_LIBRARY
    APRON_ap_ppl_LIBRARY
    APRON_apron_LIBRARY)

# Check special cases (unused libraries)

if("${APRON_ap_pkgrid_LIBRARY}" STREQUAL "unused")
    unset(APRON_ap_pkgrid_LIBRARY)
endif()
if("${APRON_ap_ppl_LIBRARY}" STREQUAL "unused")
    unset(APRON_ap_ppl_LIBRARY)
endif()

set(APRON_INCLUDE_DIRS ${APRON_INCLUDE_DIR})
set(APRON_LIBRARIES
    ${APRON_polkaMPQ_LIBRARY}
    ${APRON_octD_LIBRARY}
    ${APRON_boxMPFR_LIBRARY}
    ${APRON_ap_pkgrid_LIBRARY}
    ${APRON_ap_ppl_LIBRARY}
    ${APRON_apron_LIBRARY})
