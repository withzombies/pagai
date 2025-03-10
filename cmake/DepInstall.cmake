# Define functions to manage the installation of external
# dependencies via a custom target: "make dep_install"

include(CMakeParseArguments)

# Call this once before adding dependency targets

macro(DEPINSTALL_CREATE_TARGET)
    add_custom_target(dep_install)
endmacro()

# Show the available configurations of external versions

macro(DEPINSTALL_SHOW_CONFIGS)
    message("")
    message("List of available configurations:")
    foreach(CONFIG ${DEPINSTALL_CONFIG_LIST})
        message("")
        message("    ${CONFIG}")
        foreach(KEY ${DEPINSTALL_CONFIG_${CONFIG}_LIST})
            message("    │   ${KEY} ${DEPINSTALL_${CONFIG}_${KEY}}")
        endforeach()
    endforeach()
    message("")
endmacro()

# Check that a valid configuration has been specified
#
# After this function has been called, either an error is triggered, or all version
# variables that have been specified using "DepInstall_add(CONFIG ...)" are now set
# with values corresponding to the ones belonging to the specified configuration.
#
# Example in some CMakeLists.txt:
#
#     DepInstall_add_config(one DEP_VERSION 1.0)
#     DepInstall_add_config(two DEP_VERSION 2.0)
#     DepInstall_check_config()
#
#     # if ${CONFIG} is "one", ${DEP_VERSION} is 1.0 here
#     # if ${CONFIG} is "two", ${DEP_VERSION} is 2.0 here
#     # otherwise, an error is triggered (and a help message printed)

macro(DEPINSTALL_CHECK_CONFIG)
    if(NOT CONFIG)
        depinstall_show_configs()
        message(FATAL_ERROR "Please specify a configuration using -DCONFIG=config_name.")
    else()
        # We can't use "if ... IN_LIST ..." before cmake 3.0
        # but this is what it does

        set(CONFIG_FOUND FALSE)
        foreach(arg ${DEPINSTALL_CONFIG_LIST})
            if("${arg}" STREQUAL "${CONFIG}")
                set(CONFIG_FOUND TRUE)
            endif()
        endforeach()

        if(NOT CONFIG_FOUND)
            depinstall_show_configs()
            message(FATAL_ERROR "Unknown configuration: ${CONFIG}")
        endif()

        foreach(KEY ${DEPINSTALL_CONFIG_${CONFIG}_LIST})
            set(${KEY} "${DEPINSTALL_${CONFIG}_${KEY}}")
        endforeach()
    endif()
endmacro()

# Find a package that could've been installed with DepInstall_add
#
# This version of find_package explicits which variables are defined
# (with the "DEFINES" list) and prints a custom message (pointing
# to "make dep_install") if a package has not been found.

function(DEPINSTALL_FIND_PACKAGE PACKAGE)
    set(OPTIONS)
    set(ONE_VALUE_ARGS
        VERSION             # Specify required version
        IF_OPT              # Find package only if an option is ON
        USE_SYSTEM          # Use system install (no local installation)
    )
    set(MULTI_VALUE_ARGS
        DEFINES             # Variables expected from output
    )
    cmake_parse_arguments(DEPINSTALL_FIND_PACKAGE "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    if(DEPINSTALL_FIND_PACKAGE_IF_OPT)
        if(${DEPINSTALL_FIND_PACKAGE_IF_OPT})
            set(DO_FIND TRUE)
        else()
            set(DO_FIND FALSE)
        endif()
    else()
        set(DO_FIND TRUE)
    endif()

    if(DO_FIND)
        set(USE_SYSTEM ${DEPINSTALL_FIND_PACKAGE_USE_SYSTEM})
        find_package(${PACKAGE} ${DEPINSTALL_FIND_PACKAGE_VERSION})
        unset(USE_SYSTEM)
        if(NOT ${PACKAGE}_FOUND)
            message(WARNING "Package ${PACKAGE} ${DEPINSTALL_FIND_PACKAGE_VERSION} could not be found on your system. "
                "We created a Makefile target to download and install dependencies for you. "
                "If you want to go on with that, run \"make dep_install\" and then run cmake "
                "again (it will normally detect the local installations.")
            set(DEPINSTALL_PACKAGE_NOTFOUND TRUE PARENT_SCOPE)
        endif()

        foreach(FWD_VAR ${DEPINSTALL_FIND_PACKAGE_DEFINES})
            set(${FWD_VAR} "${${FWD_VAR}}" PARENT_SCOPE)
        endforeach()
    else()
        foreach(FWD_VAR ${DEPINSTALL_FIND_PACKAGE_DEFINES})
            set(${FWD_VAR} PARENT_SCOPE) # equivalent of "unset(... PARENT_SCOPE)" which is not supported in early CMakes
        endforeach()
    endif()
endfunction()

# Add a target for DepInstall (generally created with ExternalProject_add)
# which will be a dependency of "make dep_install".

function(DEPINSTALL_ADD_TARGET TARGET_NAME)
    set(OPTIONS)
    set(ONE_VALUE_ARGS
        INFO            # Add a STATUS message to display
    )
    set(MULTI_VALUE_ARGS
        DEPENDS         # Only for "TARGET" type, specify extra dependencies
    )
    cmake_parse_arguments(DEPINSTALL_ADD_TARGET "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})
    if(DEPINSTALL_ADD_TARGET_UNPARSED_ARGUMENTS)
        message(WARNING "Unknown arguments for DepInstall_add: ${DEPINSTALL_ADD_TARGET_UNPARSED_ARGUMENTS}")
    endif()
    
    set_target_properties(${TARGET_NAME} PROPERTIES EXCLUDE_FROM_ALL TRUE)
    add_dependencies(dep_install ${TARGET_NAME})
    if(DEPINSTALL_ADD_TARGET_DEPENDS)
        add_dependencies(${TARGET_NAME} ${DEPINSTALL_ADD_TARGET_DEPENDS})
    endif()

    if(DEPINSTALL_ADD_TARGET_INFO)
        message(STATUS "${DEPINSTALL_ADD_TARGET_INFO}")
    endif()
endfunction()

# Add a configuration, i.e. a set of versions for dependencies
# that correspond to a configuration that is known to be valid.
#
# Defines the variables:
#
#     DEPINSTALL_CONFIG_LIST                        List of configuration names
#     DEPINSTALL_CONFIG_${CONFIG_NAME}_LIST         List of entry names in config ${CONFIG_NAME}
#     DEPINSTALL_${CONFIG_NAME}_${ENTRY_NAME}       Value of the entry named ${ENTRY_NAME} in config ${CONFIG_NAME}

function(DEPINSTALL_ADD_CONFIG CONFIG_NAME)
    set(OPTIONS)
    set(ONE_VALUE_ARGS)
    set(MULTI_VALUE_ARGS)
    cmake_parse_arguments(DEPINSTALL_ADD_CONFIG "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    # append this config name to the config list
    list(APPEND DEPINSTALL_CONFIG_LIST "${CONFIG_NAME}")

    set(CFGNAME CONFIG_${CONFIG_NAME})
    set(IS_ENTRY_NAME TRUE)
    foreach(arg ${DEPINSTALL_ADD_CONFIG_UNPARSED_ARGUMENTS})
        if(IS_ENTRY_NAME)
            # append this version key to the list for this configuration
            list(APPEND DEPINSTALL_CONFIG_${CONFIG_NAME}_LIST "${arg}")

            set(ENTRY_NAME "${arg}")
            set(IS_ENTRY_NAME FALSE)
        else()
            set(DEPINSTALL_${CONFIG_NAME}_${ENTRY_NAME} "${arg}" PARENT_SCOPE)
            set(IS_ENTRY_NAME TRUE)
        endif()
    endforeach()

    set(DEPINSTALL_CONFIG_LIST ${DEPINSTALL_CONFIG_LIST} PARENT_SCOPE)
    set(DEPINSTALL_CONFIG_${CONFIG_NAME}_LIST ${DEPINSTALL_CONFIG_${CONFIG_NAME}_LIST} PARENT_SCOPE)
endfunction()

