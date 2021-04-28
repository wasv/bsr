cmake_minimum_required(VERSION 3.12)  # CMake version check
set(PROJECT bsr)
project(${PROJECT} CXX)               # Create project
set(CMAKE_CXX_STANDARD 20)            # Enable c++20 standard
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 sdl2 REQUIRED)

pkg_check_modules(LIBAV 
    libavcodec
    libswscale
    libavutil
    )
set(build_rec ${LIBAV_FOUND})

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

# Add project include/ directory
include_directories(src/include/)

# Build common library
file(GLOB COMMON_SRC src/common/*.cpp)
add_library(common ${COMMON_SRC})

target_include_directories(common PRIVATE 
    OpenGL::OpenGL GLEW::glew
    )
target_link_libraries(common PRIVATE 
    OpenGL::OpenGL GLEW::glew
    )

# Add executable target with source files listed in SOURCES variable
add_executable(bsr src/bsr.cpp)
target_include_directories(bsr PRIVATE 
    ${SDL2_INCLUDE_DIR}
    )
target_link_libraries(bsr PRIVATE
    ${SDL2_LIBRARIES}
    common
    )

# Add executable target for GLRec program.
if(build_rec)
    add_executable(bsr_rec src/bsr_rec.cpp)
    target_include_directories(bsr_rec PRIVATE 
        ${SDL2_INCLUDE_DIR}
        )
    target_link_libraries(bsr_rec PRIVATE
        ${SDL2_LIBRARIES}
        common
        )
endif()

install(TARGETS bsr bsr_rec)
