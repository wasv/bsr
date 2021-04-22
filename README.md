# bsr
Basic Shader Renderer for GLSL.

## Building
```sh
mkdir build
cd build
cmake ..
cmake --build .
```

## Usage
`./brr <path to shader file>`

The shader file will automatically be reloaded whenever it is written to.

## Limitations
- Resolution is currently hardcoded to 1920x1080 in windowed mode.
- Only runs on Linux.
