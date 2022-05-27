message(STATUS "FindAio.cmake: ${AIO_ROOT}")
if("${AIO_ROOT}" STREQUAL "")
    set(AIO_ROOT "/usr")
endif()

find_path(LIBAIO_INCLUDE_DIR NAMES libaio.h
  PATHS ${AIO_ROOT}/include/)

find_library(LIBAIO_LIBRARIES NAME aio
  PATHS ${AIO_ROOT}/lib/)

if(NOT TARGET aio)
  add_library(aio SHARED IMPORTED)
  set_target_properties(aio
    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
    "${LIBAIO_INCLUDE_DIR}"
    IMPORTED_LOCATION
    "${LIBAIO_LIBRARIES}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(aio DEFAULT_MSG LIBAIO_LIBRARIES)
mark_as_advanced(LIBAIO_INCLUDE_DIR LIBAIO_LIBRARIES)
