# Find YICES (PAGAI-specific)
#
# Influential variables:
#
#     EXTERNAL_PREFIX       Prefix where local installations are done
#     YICES_VERSION         Version to find
#     USE_SYSTEM            Use system installation
#
# This script sets the following variables:
#
#     YICES_INCLUDE_DIRS    Directories to include
#     YICES_LIBRARIES       Libraries to link with
#

if(NOT USE_SYSTEM)

    set(YICES_PATH "${EXTERNAL_PREFIX}/yices-${YICES_VERSION}")
    find_path(YICES_INCLUDE_DIR NAMES yices_c.h PATHS "${YICES_PATH}/include" NO_DEFAULT_PATH)
    find_library(YICES_LIBRARY NAMES yices PATHS "${YICES_PATH}/lib" NO_DEFAULT_PATH)
    unset(YICES_PATH)

else()

    find_path(YICES_INCLUDE_DIR NAMES yices_c.h)
    find_library(YICES_LIBRARY NAMES yices)

endif()

# Finally send the results

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(YICES DEFAULT_MSG YICES_VERSION YICES_INCLUDE_DIR YICES_LIBRARY)
mark_as_advanced(YICES_INCLUDE_DIR YICES_LIBRARY)

set(YICES_INCLUDE_DIRS ${YICES_INCLUDE_DIR})
set(YICES_LIBRARIES ${YICES_LIBRARY})
