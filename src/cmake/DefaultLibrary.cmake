cmake_minimum_required(VERSION 2.8)
if(CMAKE_VERSION VERSION_GREATER "2.8.11")
    CMAKE_POLICY(SET CMP0022 OLD)
endif()
CMAKE_POLICY(SET CMP0020 NEW)

get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" ProjectId ${ProjectId})
project(${ProjectId})

include_directories(
    ${PROJECT_LIBRARIES_PATH}
)

file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADER *.h)

add_library(${ProjectId} ${SOURCES} ${HEADER} ${EXTERNAL_CODE})

list(REMOVE_ITEM ALL_LIBRARIES ${ProjectId})
target_link_libraries(
    ${ProjectId}
    ${ALL_LIBRARIES}
)
