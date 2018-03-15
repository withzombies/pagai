# Find PPL (PAGAI-specific)
#
# Influential variables:
#
#     EXTERNAL_PREFIX       Prefix where local installations are done
#     PPL_VERSION           Version to find
#     USE_SYSTEM            Use system installation
#
# This script sets the following variables:
#
#     PPL_LIBRARIES         Libraries to link with
#

if(NOT USE_SYSTEM)

    set(PPL_PATH "${EXTERNAL_PREFIX}/ppl-${PPL_VERSION}")
    find_library(PPL_LIBRARY NAMES ppl PATHS "${PPL_PATH}/lib" NO_DEFAULT_PATH)
    unset(PPL_PATH)

else()

    find_library(PPL_LIBRARY NAMES ppl)

endif()

# Finally, send the results

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PPL DEFAULT_MSG PPL_VERSION PPL_LIBRARY)
mark_as_advanced(PPL_LIBRARY)

set(PPL_LIBRARIES ${PPL_LIBRARY})
