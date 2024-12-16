#!/bin/bash

if [ ! -d "build" ]; then
    mkdir build
fi

cd build

cmake ..
make

echo "\n-------------------------\n\n"
echo "../example.txt" | ./pl0_compiler

echo "Done"
echo "\n-------------------------\n\n"

# echo "../example2.txt" | ./pl0_compiler

# echo "Done"
# echo "\n-------------------------\n\n"

# echo "../example3.txt" | ./pl0_compiler

# echo "Done"
# echo "\n-------------------------\n\n"