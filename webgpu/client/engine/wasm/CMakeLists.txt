cmake_minimum_required (VERSION 3.0)
project(wasm)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s EXPORTED_RUNTIME_METHODS=\"['ccall', 'cwrap']\" -s EXPORT_ES6=1 -s ALLOW_MEMORY_GROWTH=1")

add_subdirectory(GeoGen)
add_subdirectory(GltfLoad)

