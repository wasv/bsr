cmake_minimum_required(VERSION 3.12)  # CMake version check
set(PROJECT brr)
project(${PROJECT} CXX)               # Create project
set(CMAKE_CXX_STANDARD 20)            # Enable c++20 standard
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


find_package( PkgConfig REQUIRED )
pkg_check_modules( SDL2 sdl2 REQUIRED )

find_package( OpenGL REQUIRED )
find_package( GLEW REQUIRED )


# Iterate through input files
make_directory(shaders/)
file(GLOB shader_files res/shaders/*.glsl)
foreach(shader_file ${shader_files})
  get_filename_component(shader_name ${shader_file} NAME_WE)
  file(READ ${shader_file} shader_contents)

  file(GENERATE OUTPUT shaders/${shader_name}.h CONTENT "#pragma once\n#include <string>\nconst std::string ${shader_name}_source = R\"(\n${shader_contents})\";")
  add_custom_target( shaders DEPENDS shaders/${shader_name}.h )
endforeach()

# Add project include/ directory
include_directories( src/include/ )

# Add executable target with source files listed in SOURCES variable
file( GLOB SOURCES src/*.cpp )
add_executable( ${PROJECT} ${SOURCES} )
add_dependencies( ${PROJECT} shaders )
target_include_directories( ${PROJECT} PRIVATE ${SDL2_INCLUDE_DIR} OpenGL::OpenGL GLEW::glew ${CMAKE_BINARY_DIR}/shaders/ )
target_link_libraries( ${PROJECT} PRIVATE ${SDL2_LIBRARIES} OpenGL::OpenGL GLEW::glew)

install( TARGETS ${PROJECT} )
