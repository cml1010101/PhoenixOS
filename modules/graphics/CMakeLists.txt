add_library(graphics.elf SHARED
    "src/Graphics.cpp"
)
set_target_properties(graphics.elf PROPERTIES OUTPUT_NAME "graphics.elf" PREFIX "" SUFFIX "")
target_include_directories(graphics.elf PRIVATE ${CMAKE_SOURCE_DIR}/kernel/include)