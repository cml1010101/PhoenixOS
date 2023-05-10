#!/bin/bash
CFLAGS=""
CCFLAGS=""
ASMFLAGS="-f elf64"
LDFLAGS="-nostdlib -Tlinker.ld"
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
    for source_file in $(find "$sourceDirectory" -name "*.cpp")
    do
        echo "Compiling" $source_file
        g++ -c $source_file -o "$source_file".o -I"$includeDirectory" $CCFLAGS
        if [ $? -eq 0 ]
        then
            echo "Success"
        else
            echo "Failure"
            exit 1
        fi
    done
    for source_file in $(find "$sourceDirectory" -name "*.asm")
    do
        echo "Compiling" $source_file
        nasm $source_file -o "$source_file".o $ASMFLAGS
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
    for object_file in $(find "$sourceDirectory" -name "*.cpp.o")
    do
        object_files="$object_files $object_file"
    done
    for object_file in $(find "$sourceDirectory" -name "*.asm.o")
    do
        object_files="$object_files $object_file"
    done
    echo "Linking" $object_files
    ld $object_files $LDFLAGS -o kernel.elf
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
    for object_file in $(find "$sourceDirectory" -name "*.cpp.o")
    do
        rm -rf $object_file
    done
    for object_file in $(find "$sourceDirectory" -name "*.asm.o")
    do
        rm -rf $object_file
    done
    rm -rf kernel.elf
    return
}
$@