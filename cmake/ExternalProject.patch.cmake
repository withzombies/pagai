# Versions of ExternalProject.cmake before CMake 3.1.0 did not recognize ".tar.xz" names
# (even though "tar" supported this format). This patch fixes it.

if(CMAKE_VERSION VERSION_LESS 3.1.0)
    if(NOT CMAKE_MODULE_PATH)
        message(FATAL_ERROR "CMAKE_MODULE_PATH must be set")
    endif()

    file(READ ${CMAKE_ROOT}/Modules/ExternalProject.cmake EXTERNAL_PROJECT_CONTENT)
    string(REGEX REPLACE "tar([^|]?[^|]?[^|]?[^|]?[^|]?)gz" "tar\\1gz|tar\\1xz" EXTERNAL_PROJECT_CONTENT "${EXTERNAL_PROJECT_CONTENT}")
    file(WRITE ${CMAKE_MODULE_PATH}/ExternalProject.cmake "${EXTERNAL_PROJECT_CONTENT}")
    unset(EXTERNAL_PROJECT_CONTENT)
endif()
