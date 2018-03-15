# Find default include/library paths (using default C++ compiler)
#
# Sets the following list-variables:
#
#     CMAKE_INCLUDE_PATH        Default include directories
#     CMAKE_LIBRARY_PATH        Default library directories

if(CMAKE_DEFAULT_CXX_INCLUDE_DIRECTORIES AND CMAKE_DEFAULT_CXX_LIBRARY_DIRECTORIES)
    return()
endif()

# Find cxx default include paths

file(WRITE "${CMAKE_BINARY_DIR}/empty.cpp" "")
execute_process(COMMAND ${CMAKE_CXX_COMPILER} -E -Wp,-v ${CMAKE_BINARY_DIR}/empty.cpp OUTPUT_VARIABLE PROG_OUT ERROR_VARIABLE PROG_ERR)
file(REMOVE "${CMAKE_BINARY_DIR}/empty.cpp")

string(REPLACE "\n" ";" OUTPUT_LIST "${PROG_OUT}\n${PROG_ERR}")
foreach(TENTATIVE_PATH ${OUTPUT_LIST})
    string(STRIP "${TENTATIVE_PATH}" TENTATIVE_PATH)
    if(EXISTS "${TENTATIVE_PATH}")
        # Make canonical path
        get_filename_component(TENTATIVE_PATH "${TENTATIVE_PATH}" ABSOLUTE)
        list(APPEND DEFAULT_CXX_INCLUDE_DIRECTORIES "${TENTATIVE_PATH}")
    endif()
endforeach()
if(DEFAULT_CXX_INCLUDE_DIRECTORIES)
    list(REMOVE_DUPLICATES DEFAULT_CXX_INCLUDE_DIRECTORIES)
    set(CMAKE_INCLUDE_PATH "${DEFAULT_CXX_INCLUDE_DIRECTORIES}" CACHE STRING "Default include directories searched by the C++ compiler" FORCE)
endif()

# Find cxx default library paths

execute_process(COMMAND ${CMAKE_CXX_COMPILER} --print-search-dirs OUTPUT_VARIABLE PROG_OUT ERROR_VARIABLE PROG_ERR)
string(REPLACE "\n" ";" OUTPUT_LIST "${PROG_OUT}\n${PROG_ERR}")
string(REGEX MATCH "libraries:.*" OUTPUT_LIST ${OUTPUT_LIST})
string(REPLACE ":" ";" OUTPUT_LIST "${OUTPUT_LIST}")

foreach(TENTATIVE_PATH ${OUTPUT_LIST})
    string(STRIP "${TENTATIVE_PATH}" TENTATIVE_PATH)
    if(EXISTS "${TENTATIVE_PATH}")
        # Make canonical path
        get_filename_component(TENTATIVE_PATH "${TENTATIVE_PATH}" ABSOLUTE)
        list(APPEND DEFAULT_CXX_LIBRARY_DIRECTORIES "${TENTATIVE_PATH}")
    endif()
endforeach()
if(DEFAULT_CXX_LIBRARY_DIRECTORIES)
    list(REMOVE_DUPLICATES DEFAULT_CXX_LIBRARY_DIRECTORIES)
    set(CMAKE_LIBRARY_PATH "${DEFAULT_CXX_LIBRARY_DIRECTORIES}" CACHE STRING "Default library directories searched by the C++ compiler" FORCE)
endif()

# Clean up

unset(PROG_OUT)
unset(PROG_ERR)
unset(OUTPUT_LIST)
unset(DEFAULT_CXX_INCLUDE_DIRECTORIES)
unset(DEFAULT_CXX_LIBRARY_DIRECTORIES)
