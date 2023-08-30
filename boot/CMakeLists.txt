function(add_efi_executable ARCH TARGET_NAME TARGET_SRCS)
    message("Adding EFI executable for ${ARCH} called ${TARGET_NAME} built by ${TARGET_SRCS}")
    find_path(GNUEFI_INCLUDE_DIR NAMES "efi.h" "efilib.h" PATH_SUFFIXES "efi" REQUIRED)
    find_file(GNUEFI_CRT0 NAMES "crt0-efi-${ARCH}.o" HINTS /usr/lib /usr/local/lib REQUIRED)
    find_file(GNUEFI_LINKER_SCRIPT NAMES "elf_${ARCH}_efi.lds" HINTS /usr/lib /usr/local/lib REQUIRED)
    find_library(GNUEFI_LIBEFI "libefi.a" REQUIRED)
    find_library(GNUEFI_LIBGNUEFI "libgnuefi.a" REQUIRED)
    find_program(CMAKE_OBJCOPY objcopy REQUIRED)
    include_directories(${GNUEFI_INCLUDE_DIR} ${GNUEFI_INCLUDE_DIR}/${ARCH} ${GNUEFI_INCLUDE_DIR}/protocol)
    add_library(${TARGET_NAME}_shared SHARED ${TARGET_SRCS} ${GNUEFI_CRT0})
    target_compile_options(${TARGET_NAME}_shared PRIVATE -fpic -fno-stack-protector -fshort-wchar -mno-red-zone)
    target_compile_definitions(${TARGET_NAME}_shared PRIVATE EFI_FUNCTION_WRAPPER)
    target_link_libraries(${TARGET_NAME}_shared PRIVATE ${GNUEFI_LIBEFI} ${GNUEFI_LIBGNUEFI})
    target_link_options(${TARGET_NAME}_shared PRIVATE -Wl,-shared -Wl,-Bsymbolic -Wl,-T${GNUEFI_LINKER_SCRIPT} -nostdlib -Wl,-znocombreloc)
    add_custom_command(
        OUTPUT ${TARGET_NAME}.efi
        COMMAND ${CMAKE_OBJCOPY} -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc
            --target=efi-app-x86_64 --subsystem=10 lib${TARGET_NAME}_shared.so ${TARGET_NAME}.efi
        DEPENDS ${TARGET_NAME}_shared
        COMMENT "Converting ELF to EFI executable"
    )
    add_custom_target(boot ALL
        DEPENDS ${TARGET_NAME}.efi
    )
endfunction()
add_efi_executable(x86_64 boot "src/Main.c")