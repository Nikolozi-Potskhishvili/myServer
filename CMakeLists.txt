cmake_minimum_required(VERSION 3.10)
project(myServer)

# Set the C standard
set(CMAKE_C_STANDARD 11)

# Add the executable
add_executable(myServer src/main.c)
target_link_libraries(myServer ws2_32)

# Copy the index.html to the build directory
configure_file(${CMAKE_SOURCE_DIR}/resources/index.html ${CMAKE_BINARY_DIR}/index.html COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/resources/pingPaint.png ${CMAKE_BINARY_DIR}/pingPaint.png COPYONLY)