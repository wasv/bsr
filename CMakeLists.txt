cmake_minimum_required(VERSION 3.12)  # CMake version check
set(PROJECT brr)
project(${PROJECT})                      # Create project "BitBorn"
set(CMAKE_CXX_STANDARD 20)            # Enable c++20 standard
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


find_package( PkgConfig REQUIRED )
pkg_check_modules( SDL2 sdl2 REQUIRED )

find_package( OpenGL REQUIRED )

# Add project include/ directory
include_directories( src/include/ )

# Add executable target with source files listed in SOURCES variable
file( GLOB SOURCES src/*.cpp )
add_executable( ${PROJECT} ${SOURCES} )
target_include_directories( ${PROJECT} PRIVATE ${SDL2_INCLUDE_DIR} )
target_link_libraries( ${PROJECT} PRIVATE ${SDL2_LIBRARY} OpenGL::OpenGL )

install( TARGETS ${PROJECT} )
