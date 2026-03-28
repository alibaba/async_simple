include(FetchContent)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(ASIO_PC QUIET asio)
endif()

find_path(
    ASIO_INCLUDE_DIR
    NAMES asio.hpp
    HINTS ${ASIO_PC_INCLUDE_DIRS} /usr/include /usr/local/include
)

if(ASIO_INCLUDE_DIR)
    message(STATUS "[Asio] Found system Asio at: ${ASIO_INCLUDE_DIR}")

    if(NOT TARGET asio::asio)
        add_library(asio::asio INTERFACE IMPORTED GLOBAL)
        target_include_directories(asio::asio INTERFACE "${ASIO_INCLUDE_DIR}")
        target_compile_definitions(asio::asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)
    endif()
else()
    message(STATUS "[Asio] System Asio not found — fetching via FetchContent")

    FetchContent_Declare(
        asio
        GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
        GIT_TAG asio-1-34-2
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(asio)

    if(NOT TARGET asio::asio)
        add_library(asio::asio INTERFACE IMPORTED GLOBAL)
        target_include_directories(asio::asio INTERFACE "${asio_SOURCE_DIR}/asio/include")
        target_compile_definitions(asio::asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)
    endif()

    set(ASIO_INCLUDE_DIR
        "${asio_SOURCE_DIR}/asio/include"
        CACHE PATH
        "Asio include directory (fetched)"
        FORCE
    )
endif()

set(ASIO_FOUND TRUE CACHE BOOL "Asio available")
set(ASIO_INCLUDE_DIRS "${ASIO_INCLUDE_DIR}" CACHE PATH "Asio include dirs")
mark_as_advanced(ASIO_INCLUDE_DIR ASIO_INCLUDE_DIRS ASIO_FOUND)
