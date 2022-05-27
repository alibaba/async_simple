if("${GOOGLETEST_ROOT}" STREQUAL "")
    set(GOOGLETEST_ROOT "/usr/local")
endif()

find_path(GTEST_INCLUDE_DIR
    NAMES gtest/gtest.h
    PATHS ${GOOGLETEST_ROOT}/include)

find_library(GTEST_LIBRARIES
    NAME gtest
    PATHS ${GOOGLETEST_ROOT}/lib)

if (NOT TARGET gtest)
    add_library(gtest STATIC IMPORTED)
    set_target_properties(gtest
        PROPERTIES INTERFACE_COMPILE_DEFINITIONS
        HAS_GTEST
        INTERFACE_INCLUDE_DIRECTORIES
        "${GTEST_INCLUDE_DIR}"
        IMPORTED_LOCATION
        "${GTEST_LIBRARIES}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(gtest DEFAULT_MSG GTEST_LIBRARIES GTEST_INCLUDE_DIR)

MARK_AS_ADVANCED(
    GTEST_INCLUDE_DIR
    GTEST_LIBRARIES)
