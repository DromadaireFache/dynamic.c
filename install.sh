#!/bin/bash

# Install script for dynamic.c library

# 1. GCC or Clang check
if command -v gcc >/dev/null 2>&1; then
    COMPILER="gcc"
elif command -v clang >/dev/null 2>&1; then
    COMPILER="clang"
else
    echo "Error: No C compiler found. Please install GCC or Clang."
    exit 1
fi

CFLAGS="$@"

# 2. Create lib directory
echo "Creating lib directory..."
mkdir -p lib


# 3. Compile source file
echo $COMPILER $CFLAGS -c dynamic.c/src/dynamic.c -o lib/dynamic.o
$COMPILER $CFLAGS -c dynamic.c/src/dynamic.c -o lib/dynamic.o
if [ $? -ne 0 ]; then
    echo "Error: Compilation failed."
    exit 1
fi

# 4. Create static library
echo "Creating static library libdynamic.a..."
ar rcs lib/libdynamic.a lib/dynamic.o
if [ $? -ne 0 ]; then
    echo "Error: Failed to create static library."
    exit 1
fi

# 5. Remove git repository
mv dynamic.c/dynamic.h lib/dynamic.h
rm -rf dynamic.c
echo "Installation completed successfully."
echo "Library files are located in the 'lib' directory."