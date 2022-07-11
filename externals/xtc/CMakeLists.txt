cmake_minimum_required(VERSION 3.0)
project(xtc C)

#SET(CMAKE_BUILD_TYPE "Release")
# where the include files are located
include_directories(
	"${PROJECT_SOURCE_DIR}/include"
)

link_directories(
	"${PROJECT_SOURCE_DIR}/src"
)

file(GLOB XTC_SRC
    "src/*.c"
)



file(GLOB XTC_HEADERS
	"include/*.h"
)

message (STATUS "Project binary dir: ${XTC_HEADERS}")
message (STATUS "Project source dir: ${XTC_SRC}")

add_library(
	#static lib
	xtc STATIC
		${XTC_SRC}
)
