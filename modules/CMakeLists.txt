file(COPY ${CMAKE_SOURCE_DIR}/modules/modules.cfg DESTINATION ${CMAKE_BINARY_DIR}/modules)
add_subdirectory(graphics)
set(MODULES
    "graphics"
)