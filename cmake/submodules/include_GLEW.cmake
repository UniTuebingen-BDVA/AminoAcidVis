#GLEW
set(BUILD_UTILS false)
set(glew-cmake_BUILD_MULTI_CONTEXT OFF)
set(glew-cmake_BUILD_SHARED OFF)
add_subdirectory("${PROJECT_EXTERNALS_PATH}/glew-cmake")
set (GLEW_LIBRARY libglew_static)
add_definitions(-DGLEW_STATIC)
include_directories(${PROJECT_EXTERNALS_PATH}/glew-cmake/include)
