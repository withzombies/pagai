# Find Z3 (PAGAI-specific)
#
# Influential variables:
#
#     EXTERNAL_PREFIX       Prefix where local installations are done
#     Z3_VERSION            Commit of Z3 to look for (local install)
#
# This script sets the following variables:
#
#     Z3_INCLUDE_DIRS       Directories to include
#     Z3_LIBRARIES          Libraries to link with
#

if(NOT USE_SYSTEM)

    set(Z3_PATH "${EXTERNAL_PREFIX}/z3-${Z3_VERSION}")
    find_path(Z3_INCLUDE_DIR NAMES z3.h PATHS "${Z3_PATH}/include" NO_DEFAULT_PATH)
    find_library(Z3_LIBRARY NAMES z3-gmp z3 PATHS "${Z3_PATH}/lib" NO_DEFAULT_PATH)
    unset(Z3_PATH)

else()

    find_path(Z3_INCLUDE_DIR NAMES z3.h)
    find_library(Z3_LIBRARY NAMES z3-gmp z3)

endif()

# Finally send the results

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Z3 DEFAULT_MSG Z3_INCLUDE_DIR Z3_LIBRARY)
mark_as_advanced(Z3_INCLUDE_DIR Z3_LIBRARY)

set(Z3_INCLUDE_DIRS ${Z3_INCLUDE_DIR})
set(Z3_LIBRARIES ${Z3_LIBRARY})
