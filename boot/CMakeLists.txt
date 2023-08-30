function(add_efi_executable ARCH TARGET_NAME TARGET_SRCS)
    message("Adding EFI executable for ${ARCH} called ${TARGET_NAME} built by ${TARGET_SRCS}")
    find_path(GNUEFI_INCLUDE_DIR NAMES "efi.h" "efilib.h" PATH_SUFFIXES "efi" REQUIRED)
    find_file(GNUEFI_CRT0 NAMES "crt0-efi-${ARCH}.o" HINTS /usr/lib /usr/local/lib REQUIRED)
    find_file(GNUEFI_LINKER_SCRIPT NAMES "elf_${ARCH}_efi.lds" HINTS /usr/lib /usr/local/lib REQUIRED)
    find_path(GNUEFI_LIBS NAMES "libgnuefi.a" "libefi.a" HINTS /usr/lib /usr/local/lib REQUIRED)
    find_program(CMAKE_C_COMPILER gcc REQUIRED)
    find_program(CMAKE_CXX_COMPILER g++ REQUIRED)
    find_program(CMAKE_LD ld REQUIRED)
    find_program(CMAKE_OBJCOPY objcopy REQUIRED)
    include_directories(${GNUEFI_INCLUDE_DIR} ${GNUEFI_INCLUDE_DIR}/${ARCH} ${GNUEFI_INCLUDE_DIR}/protocol)
    add_library(${TARGET_NAME}_static STATIC ${TARGET_SRCS})
    target_compile_options(${TARGET_NAME}_static PRIVATE -fpic -fno-stack-protector -fshort-wchar -mno-red-zone)
    target_compile_definitions(${TARGET_NAME}_static PRIVATE EFI_FUNCTION_WRAPPER)
    add_custom_command(
        OUTPUT lib${TARGET_NAME}_shared.so
        COMMAND ${CMAKE_LD} $<TARGET_FILE:${TARGET_NAME}_static> ${GNUEFI_CRT0} -o lib${TARGET_NAME}_shared.so -shared -Bsymbolic
            -T${GNUEFI_LINKER_SCRIPT} -L${GNUEFI_LIBS} -lefi -lgnuefi -nostdlib -znocombreloc
        COMMENT "Linking shared library lib${TARGET_NAME}_shared.so"
    )
    add_custom_target(boot_shared ALL
        DEPENDS lib${TARGET_NAME}_shared.so
    )
    add_custom_command(
        OUTPUT ${TARGET_NAME}.efi
        COMMAND objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc
            --target=efi-app-x86_64 --subsystem=10 lib${TARGET_NAME}_shared.so ${TARGET_NAME}.efi
        DEPENDS ${TARGET_NAME}_shared
        COMMENT "Converting ELF to EFI executable"
    )
    add_custom_target(boot ALL
        DEPENDS ${TARGET_NAME}.efi
    )
endfunction()
add_efi_executable(x86_64 boot "src/Main.c")