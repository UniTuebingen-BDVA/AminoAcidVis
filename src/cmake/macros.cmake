MACRO(GENERATE_SUBDIRS result curdir bindir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(names "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
		IF (NOT ${child} MATCHES "\\..*")
			if(EXISTS ${curdir}/${child}/CMakeLists.txt)
				string(REPLACE " " "_" child ${child})
				SET(names ${names} ${child})
				message("BUILD FOR '${child}' GENERATED")
			endif()
		ENDIF()
    ENDIF()
  ENDFOREACH()
  SET(${result} ${${result}} ${names})
  	FOREACH(n ${names})
  		add_subdirectory(${curdir}/${n} ${bindir}/${n})
  	ENDFOREACH()
ENDMACRO()

MACRO(link_dependency name)
	if("${ARGN}" STREQUAL "")
		set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} $ENV{${name}_ROOT})
	else()
		set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} $ENV{${ARGN}})
	endif()
        find_package(${name} REQUIRED)
	include(${CMAKE_MODULE_PATH}/Link${name}.cmake)
ENDMACRO()

MACRO(link_dependency_QUIET name)
	if("${ARGN}" STREQUAL "")
		set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} $ENV{${name}_ROOT})
	else()
		set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} $ENV{${ARGN}})
	endif()
        find_package(${name} QUIET)
    if(${name}_FOUND)
		include(${CMAKE_MODULE_PATH}/Link${name}.cmake)	
	endif()	
ENDMACRO()