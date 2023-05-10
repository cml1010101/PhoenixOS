#!/bin/bash
CFLAGS="-I/usr/include/efi -I/usr/include/efi/x86_64 -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone \
    -maccumulate-outgoing-args"
LDFLAGS="-shared -Bsymbolic -T/usr/include/elf_x86_64_efi.lds -lefi -lgnuefi"
OBJCOPYFLAGS="-j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc \
    --target efi-app-x86_64 --subsystem=10"
compile() {
    local includeDirectory="./include"
    local sourceDirectory="./src"
    for source_file in $(find "$sourceDirectory" -name "*.c")
    do
        echo "Compiling" $source_file
        gcc -c $source_file -o "$source_file".o -I"$includeDirectory" $CFLAGS
        if [ $? -eq 0 ]
        then
            echo "Success"
        else
            echo "Failure"
            exit 1
        fi
    done
    return
}
link() {
    local sourceDirectory="./src"
    local object_files=""
    for object_file in $(find "$sourceDirectory" -name "*.c.o")
    do
        object_files="$object_files $object_file"
    done
    echo "Linking" $object_files
    ld $object_files /usr/lib/crt0-efi-x86_64.o $LDFLAGS -o BOOTX64.so
    if [ $? -eq 0 ]
    then
        echo "Success"
    else
        echo "Failure"
        exit 1
    fi
    objcopy $OBJCOPYFLAGS BOOTX64.so -o BOOTX64.efi
    if [ $? -eq 0 ]
    then
        echo "Success"
    else
        echo "Failure"
        exit 1
    fi
    return
}
clean() {
    local sourceDirectory="./src"
    for object_file in $(find "$sourceDirectory" -name "*.c.o")
    do
        rm -rf $object_file
    done
    rm -rf BOOTX64.efi BOOTX64.so
    return
}
$@