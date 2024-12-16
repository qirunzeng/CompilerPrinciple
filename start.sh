#!/bin/bash

# Check if build directory exists, if not create it
if [ ! -d "build" ]; then
    mkdir build
fi

# Navigate to build directory
cd build

# Run cmake and make
cmake ..
make

echo "\n-------------------------\n\n"
# Execute the example.txt file with pl0_compiler
echo "../example.txt" | ./pl0_compiler

echo "Done"
echo "\n-------------------------\n\n"

echo "../example2.txt" | ./pl0_compiler

echo "Done"
echo "\n-------------------------\n\n"

echo "../example3.txt" | ./pl0_compiler

echo "Done"
echo "\n-------------------------\n\n"