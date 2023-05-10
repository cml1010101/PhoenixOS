#!/bin/bash
echo "Compiling source files"
cd boot
cd x86_64
./x86_64.sh compile
cd ..
cd ..
cd arch
cd x86_64
./x86_64.sh compile
cd ..
cd ..
cd kernel
./kernel.sh compile
cd ..
echo "Done compiling source files"
echo "Linking objects"
cd boot
cd x86_64
./x86_64.sh link
cd ..
cd ..
cd arch
cd x86_64
./x86_64.sh link
cd ..
cd ..
cd kernel
./kernel.sh link
cd ..
echo "Done linking objects"
echo "All done"