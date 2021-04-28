cmake_minimum_required(VERSION 3.12)  # CMake version check
set(PROJECT bsr)
project(${PROJECT} CXX)               # Create project
set(CMAKE_CXX_STANDARD 20)            # Enable c++20 standard
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 sdl2 REQUIRED)

pkg_check_modules(LIBAV 
    libavcodec
    libswscale
    libavutil
    )
if(LIBAV_FOUND AND OpenGL_EGL_FOUND)
    set(build_rec 1)
    message("Offscreen rendering supported.")
else()
    set(build_rec 0)
    message("Offscreen rendering NOT supported.")
    message("LibAV = ${LIBAV_FOUND}")
    message("EGL = ${OpenGL_EGL_FOUND}")
endif()

# Build common library
file(GLOB RENDERER_SRC src/renderer/*.cpp)
add_library(renderer ${RENDERER_SRC})

target_include_directories(renderer PRIVATE 
    OpenGL::OpenGL GLEW::glew
    PUBLIC src/renderer/include/
    )
target_link_libraries(renderer PRIVATE 
    OpenGL::OpenGL GLEW::glew
    )

# Add executable target with source files listed in SOURCES variable
add_executable(bsr src/bsr.cpp)
target_include_directories(bsr PRIVATE 
    ${SDL2_INCLUDE_DIR}
    renderer
    )
target_link_libraries(bsr PRIVATE
    ${SDL2_LIBRARIES}
    renderer
    )
install(TARGETS bsr)

# Add executable target for bsr_rec program.
if(build_rec)
    file(GLOB ENCODER_SRC src/encoder/*.cpp)
    add_library(encoder ${ENCODER_SRC})
    target_link_libraries(encoder PRIVATE
        ${LIBAV_LIBRARIES}
        )
    target_include_directories(encoder PRIVATE 
        ${LIBAV_INCLUDE_DIR}
        PUBLIC
        src/encoder/include
        )

    add_executable(bsr_rec src/rec.cpp)
    target_include_directories(bsr_rec PRIVATE 
        OpenGL::EGL
        encoder
        )
    target_link_libraries(bsr_rec PRIVATE
        OpenGL::EGL
        renderer
        encoder
        )
    install(TARGETS bsr_rec)
endif()

