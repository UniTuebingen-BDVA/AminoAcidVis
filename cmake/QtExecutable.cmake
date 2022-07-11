cmake_minimum_required(VERSION 3.0)
CMAKE_POLICY(SET CMP0020 NEW)

get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" ProjectId ${ProjectId})
project(${ProjectId})

if(NOT DEFINED RESOURCE_FILES OR UNIX)
	SET(RESOURCE_FILES "")
endif()

message( "PROJECT_LIBRARIES_PATH: " ${PROJECT_LIBRARIES_PATH} )

include_directories(
    ${PROJECT_LIBRARIES_PATH}
)

file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADER *.h)


# find and build the given UI files
set(UI_SRCS "") #all srcs files
file(GLOB children_files RELATIVE ${PROJECT_UI_PATH} ${PROJECT_UI_PATH}/*)
FOREACH(ui_file ${UI_FILES})
	if(NOT IS_DIRECTORY ${PROJECT_UI_PATH}/${ui_file})
		if(${ui_file} MATCHES "..*[.]ui")
			string(REPLACE ".ui" "" ui_file_name ${ui_file})
			QT5_WRAP_UI("UI_${ui_file_name}_Src" ${PROJECT_UI_PATH}/${ui_file})
			set(UI_SRCS ${UI_SRCS} "${UI_${ui_file_name}_Src}") 
		endif()
	endif()
ENDFOREACH()

if(WIN32)
	option(PROJECT_START_FROM_WINMAIN "If on the console will be hidden")
	if(PROJECT_START_FROM_WINMAIN)
		add_executable(${ProjectId} WIN32 ${SOURCES} ${HEADER} ${EXTERNAL_CODE} ${UI_SRCS} )
	else()
		add_executable(${ProjectId} ${SOURCES} ${HEADER} ${EXTERNAL_CODE} ${UI_SRCS} )
	endif()
else()
	add_executable(${ProjectId} ${SOURCES} ${HEADER} ${EXTERNAL_CODE} ${UI_SRCS} )
endif()

message( "EXTERNAL_CODE: " ${EXTERNAL_CODE} )
message( "ALL_LIBRARIES: " ${ALL_LIBRARIES} )

target_link_libraries(
    ${ProjectId}
    ${ALL_LIBRARIES}
    ${GTK3_LIBRARIES}
    ${RESOURCE_FILES}
)
