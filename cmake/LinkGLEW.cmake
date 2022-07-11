include_directories(
        ${GLEW_INCLUDE_DIR}
)

link_libraries(
        ${GLEW_LIBRARIES}
)

add_definitions(
	-DGLEW_STATIC
)

target_compile_options(libglew_shared PUBLIC -nostdlib)
