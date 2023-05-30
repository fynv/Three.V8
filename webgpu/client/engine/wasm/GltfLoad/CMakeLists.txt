cmake_minimum_required (VERSION 3.0)
project(GltfLoad)

set(INCLUDE_DIR    
    ../../../../../thirdparty    
    ../../../../../thirdparty/glm/glm
    .
)

include_directories(${INCLUDE_DIR})

add_executable(GltfLoad api.cpp)
 
