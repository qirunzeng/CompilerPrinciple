cmake_minimum_required(VERSION 3.10)

# Set the project name
project(pl0_compiler)

# Specify the C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# Add the executable
add_executable(pl0_compiler main.c pl0.h  pl0.c set.h set.c)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Link libraries (if any)
# target_link_libraries(pl0_compiler <library_name>)