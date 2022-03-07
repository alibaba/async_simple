if("${BENCHMARK_ROOT}" STREQUAL "")
    set(BENCHMARK_ROOT "/usr/local")
endif()

find_path(BENCHMARK_INCLUDE_DIR
    NAMES benchmark/benchmark.h
    PATHS ${BENCHMARK_ROOT}/include)

find_library(BENCHMARK_LIBRARIES
    NAME benchmark
    PATHS ${GOOGLETEST_ROOT})

if (NOT TARGET benchmark)
    add_library(benchmark STATIC IMPORTED)
    set_target_properties(benchmark
        PROPERTIES INTERFACE_COMPILE_DEFINITIONS
        HAS_BENCHMARK
        INTERFACE_INCLUDE_DIRECTORIES
        "${BENCHMARK_INCLUDE_DIR}"
        IMPORTED_LOCATION
        "${BENCHMARK_LIBRARIES}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(benchmark DEFAULT_MSG BENCHMARK_LIBRARIES BENCHMARK_INCLUDE_DIR)

MARK_AS_ADVANCED(
    BENCHMARK_INCLUDE_DIR
    BENCHMARK_LIBRARIES)

