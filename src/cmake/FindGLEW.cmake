# Finds:
# GLEW_INCLUDE_DIR
# GLEW_LIBRARIES

include(FindPackageHandleStandardArgs)

find_path(GLEW_INCLUDE_DIR GL/glew.h
  PATH_SUFFIXES include
  PATHS "${PROJECT_EXTERNALS_PATH}/glew-cmake/include" "${PROJECT_EXTERNALS_PATH}/glew-cmake" "${PROJECT_EXTERNALS_PATH}/glew"
  DOC "The path to the GLEW header files."
)

if( GLEW_INCLUDE_DIR STREQUAL "" )
	message("Failed to find GLEW_INCLUDE_DIR! Using default path!")
	set(GLEW_INCLUDE_DIR "${PROJECT_EXTERNALS_PATH}/glew-cmake/include" )
endif()

find_library(GLEW_LIBRARIES
  NAMES glew
  PATH_SUFFIXES lib
  HINTS "${CMAKE_BINARY_DIR}/lib" "${CMAKE_BINARY_DIR}/externals/glew-cmake" "${CMAKE_BINARY_DIR}/externals/glew" "${CMAKE_BINARY_DIR}/build-windows/lib/Debug" "${CMAKE_BINARY_DIR}/build-windows/lib/Release"
)

message("GLEW_INCLUDE_DIR: " ${GLEW_INCLUDE_DIR})
message("GLEW_LIBRARIES: " ${GLEW_LIBRARIES})

FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLEW DEFAULT_MSG
                                  GLEW_LIBRARIES GLEW_INCLUDE_DIR)

set(ALL_LIBRARIES ${GLEW_LIBRARIES} ${ALL_LIBRARIES})