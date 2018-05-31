function(MAKE_CONFIG_FILE)

    set(CMAKE_BUILD_OPTIONS)
    set(CMAKE_BUILD_CONFIG)

    # Find all options

    file(READ "${CMAKE_SOURCE_DIR}/CMakeLists.txt" CMAKELISTS_STR)
    string(REGEX MATCHALL "[Oo][Pp][Tt][Ii][Oo][Nn] *\\([^\n]*" OPTION_LIST ${CMAKELISTS_STR})
    foreach(opt ${OPTION_LIST})
        string(REGEX REPLACE "[Oo][Pp][Tt][Ii][Oo][Nn] *\\( *([^ ]*).*" "\\1" opt ${opt})
        list(APPEND CMAKE_BUILD_OPTIONS
			"#ifndef ${opt}"
			"#cmakedefine ${opt}"
			"#endif")
    endforeach()
    string(REPLACE ";" "\n" CMAKE_BUILD_OPTIONS "${CMAKE_BUILD_OPTIONS}")

    # Find config

    foreach(cfgvalue ${DEPINSTALL_CONFIG_${CONFIG}_LIST})
        list(APPEND CMAKE_BUILD_CONFIG "#define ${cfgvalue} \"${${cfgvalue}}\"")
    endforeach()
    string(REPLACE ";" "\n" CMAKE_BUILD_CONFIG "${CMAKE_BUILD_CONFIG}")

    # Configure file (2 passes because we generate "#cmakedefine" for options)

    configure_file("${CMAKE_SOURCE_DIR}/src/config.h.in" "${CMAKE_BINARY_DIR}/src/config.h.out" @ONLY)
    configure_file("${CMAKE_BINARY_DIR}/src/config.h.out" "${CMAKE_BINARY_DIR}/src/config.h" @ONLY)
endfunction()

make_config_file()
