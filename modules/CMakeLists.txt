set(modules_TARGETS
    "graphics"
)
foreach(TARGET IN LISTS ${modules_TARGETS})
    add_subdirectory(${TARGET})
endforeach()
add_custom_command(
    OUTPUT initramfs.tar
    COMMAND tar -cvf initramfs.tar ${modules_TARGETS}
    DEPENDS ${modules_TARGETS}
    COMMENT "Archiving ${modules_TARGETS} into initramfs.tar"
)
add_custom_target(
    initramfs ALL DEPENDS initramfs.tar
)