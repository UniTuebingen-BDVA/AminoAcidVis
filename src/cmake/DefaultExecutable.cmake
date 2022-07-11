cmake_minimum_required(VERSION 2.8)

get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" ProjectId ${ProjectId})
project(${ProjectId})

message( "PROJECT_LIBRARIES_PATH: " ${PROJECT_LIBRARIES_PATH} )

include_directories(
    ${PROJECT_LIBRARIES_PATH}
)

file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADER *.h)

add_executable(${ProjectId} ${SOURCES} ${HEADER} ${EXTERNAL_CODE})

message( "EXTERNAL_CODE: " ${EXTERNAL_CODE} )
message( "ALL_LIBRARIES: " ${ALL_LIBRARIES} )

target_link_libraries(
    ${ProjectId}
    ${ALL_LIBRARIES}
    ${GTK3_LIBRARIES}
)
