# A cmake tempalte for a c++ project using QT and glew(OpenGl).
string(REPLACE "/" ";" p2list "${CMAKE_SOURCE_DIR}")
string(REPLACE "\\" ";" p2list "${p2list}")
list(REVERSE p2list)
list(GET p2list 0 first)
list(GET p2list 1 ProjectId)
string(REPLACE " " "_" ProjectId ${ProjectId})
project(${ProjectId})

include(${CMAKE_MODULE_PATH}/doxygen.cmake)
include(${CMAKE_MODULE_PATH}/macros.cmake)

set(CMAKE_CONFIGURATION_TYPES Debug;Release)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++0x") # Use c++11


# == OpenGL==
find_package(OpenGL3 REQUIRED)

# == The OpenGL Extension Wrangler Library (https://github.com/Perlmint/glew-cmake.git) ==
add_subdirectory("${PROJECT_EXTERNALS_PATH}/glew-cmake")
link_dependency_QUIET(GLEW) 



# == xtc decompression C libary ==
add_subdirectory("${PROJECT_EXTERNALS_PATH}/xtc")
link_dependency_QUIET(XTC) 

# pre build
if(NOT GLEW_FOUND OR NOT XTC_FOUND)
  message(STATUS " ")
  message(STATUS "=======================================================================================================")
  message(STATUS "Project created only for GLEW and XTC. Please build GLEW and XTC and then rerun CMake to build the rest!")
  message(STATUS "=======================================================================================================")
  message(STATUS " ") 
  return()
endif()
if (UNIX)
   set(ALL_LIBRARIES ${ALL_LIBRARIES} -lGL)
endif (UNIX)

# == QT gui ==
#don't forget to install it windows or linux
set(QTCP_PROJECT_DIR ${PROJECT_SOURCE_DIR})

set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# find QT5 cmake files on windows
IF (WIN32)
find_path(QT_CMAKE_DIR Qt5/Qt5Config.cmake
  PATH_SUFFIXES cmake
  PATHS "D:/Qt/5.5/mingw492_32/lib" "C:/Qt/5.5/mingw492_32/lib" "E:/Qt/5.5/mingw492_32/lib" 
  DOC "The path to the QT cmake files."
)
set (CMAKE_PREFIX_PATH ${QT_CMAKE_DIR}) 
endif(WIN32)

#find_package(Qt5Core REQUIRED )
find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)

find_package(Qt5Quick REQUIRED)
find_package(Qt5QuickWidgets REQUIRED)

include_directories(
	${Qt5Widgets_INCLUDES}
	${QT_QTOPENGL_INCLUDE_DIR} 
)

add_definitions(${Qt5Widgets_DEFINITIONS})

set(ALL_LIBRARIES ${ALL_LIBRARIES} Qt5::Widgets Qt5::OpenGL Qt5::Qml Qt5::Quick Qt5::QuickWidgets) 

# Compiler flags
if(CMAKE_COMPILER_IS_GNUCXX)
	option(PROJECT_FULLY_OPTIMIZE "Should the binary be fully optimized? Warning compiling will take considerably longer!")
	if(PROJECT_FULLY_OPTIMIZE)
		set(CMAKE_CXX_FLAGS  ${CMAKE_CXX_FLAGS} "-O3")        ## Optimize
		string (REPLACE ";" " " CMAKE_CXX_FLAGS_STR "${CMAKE_CXX_FLAGS}")
		set(CMAKE_CXX_FLAGS  ${CMAKE_CXX_FLAGS_STR})
		set(CMAKE_EXE_LINKER_FLAGS "-s")  ## Strip binary
	else()
		#-DCMAKE_C_FLAGS="-g2" -DCMAKE_CXX_FLAGS="-g2"
		# set(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS} "-g2")
		set(CMAKE_CXX_FLAGS  ${CMAKE_CXX_FLAGS} "-g2")
		string (REPLACE ";" " " CMAKE_CXX_FLAGS_STR "${CMAKE_CXX_FLAGS}")
		set(CMAKE_CXX_FLAGS  ${CMAKE_CXX_FLAGS_STR})
	endif()
endif()


#GLM math libaray (https://github.com/g-truc/glm)
file(GLOB_RECURSE GLM
    "${PROJECT_EXTERNALS_PATH}/glm/glm/*.hpp")
include_directories("${PROJECT_EXTERNALS_PATH}/glm")

#
include_directories("${PROJECT_EXTERNALS_PATH}/xtc/include")

add_definitions(
        -DGLM_FORCE_RADIANS
        -DGLM_FORCE_PURE
        -DNOMINMAX
		-DGLM_ENABLE_EXPERIMENTAL
)


# Collect all code
set(EXTERNAL_CODE
        ${GLM}
        ${GLEW}
        ${MOCSrcs}
        ${UI_SRCS}
)

# Include externals folder
include_directories(${PROJECT_EXTERNALS_PATH})


include_directories(
    ${CORE_PATH}
)

add_definitions(-DDEFAULT_RESOURCES_PATH="${PROJECT_DEFAULT_RESOURCES_PATH}" -DPROJECT_VERSION="${PROJECT_VERSION}")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # using Clang
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  add_definitions(-Wall -Wextra)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  add_definitions(/W2)
endif()

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
GENERATE_SUBDIRS(ALL_LIBRARIES ${PROJECT_LIBRARIES_PATH} ${PROJECT_BINARY_DIR}/libraries)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
GENERATE_SUBDIRS(ALL_EXECUTABLES ${PROJECT_EXECUTABLES_PATH} ${PROJECT_BINARY_DIR}/executables)


file (COPY "${CMAKE_MODULE_PATH}/gdb_prettyprinter.py" DESTINATION ${PROJECT_BINARY_DIR})
