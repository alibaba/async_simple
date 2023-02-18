if("${GOOGMOCK_ROOT}" STREQUAL "")
    set(GOOGMOCK_ROOT "/usr/local")
endif()

find_path(GMOCK_INCLUDE_DIR
    NAMES gmock/gmock.h
    PATHS ${GOOGMOCK_ROOT}/include)

find_library(GMOCK_LIBRARIES
    NAME gmock
    PATHS ${GOOGMOCK_ROOT}/lib)

if (NOT TARGET gmock)
    add_library(gmock STATIC IMPORTED)
    set_target_properties(gmock
        PROPERTIES INTERFACE_COMPILE_DEFINITIONS
        HAS_GMOCK
        INTERFACE_INCLUDE_DIRECTORIES
        "${GMOCK_INCLUDE_DIR}"
        IMPORTED_LOCATION
        "${GMOCK_LIBRARIES}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMock DEFAULT_MSG GMOCK_LIBRARIES GMOCK_INCLUDE_DIR)

MARK_AS_ADVANCED(
    GMOCK_INCLUDE_DIR
    GMOCK_LIBRARIES)
