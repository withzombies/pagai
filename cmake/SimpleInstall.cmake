# Download and install an archive (binary form) as an external project
#
# Variables to define:
#
#     URL               Target to download
#     DOWNLOAD_DEST     Local name of downloaded file
#     HASH              Hash to check downloaded target
#     UNTAR_TO          If set, downloaded file will be extraced to this following dir

if(NOT URL OR NOT DOWNLOAD_DEST)
    message(FATAL_ERROR "URL and DOWNLOAD_DEST must be defined")
endif()

if(HASH)
    file(DOWNLOAD "${URL}" "${DOWNLOAD_DEST}" SHOW_PROGRESS EXPECTED_HASH "${HASH}")
else()
    file(DOWNLOAD "${URL}" "${DOWNLOAD_DEST}" SHOW_PROGRESS)
endif()

if(UNTAR_TO)
    execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory "${UNTAR_TO}")
    execute_process(COMMAND "${CMAKE_COMMAND}" -E tar xf "${DOWNLOAD_DEST}" WORKING_DIRECTORY "${UNTAR_TO}")
endif()
