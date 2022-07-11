include(FindPackageHandleStandardArgs)

find_path(XTC_INCLUDE_DIR xdrfile.h
  PATH_SUFFIXES include 
  PATHS "${PROJECT_EXTERNALS_PATH}/xtc" 
)

find_library(XTC_LIBRARIES
  NAMES xtc
  PATH_SUFFIXES lib
  HINTS "${CMAKE_BINARY_DIR}/externals/xtc" "${CMAKE_BINARY_DIR}/externals/xtc/Release" "${CMAKE_BINARY_DIR}/externals/xtc/Debug"
)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(XTC DEFAULT_MSG
                                  XTC_LIBRARIES XTC_INCLUDE_DIR)
