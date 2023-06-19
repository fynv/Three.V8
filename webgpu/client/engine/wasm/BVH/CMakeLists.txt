cmake_minimum_required (VERSION 3.0)
project(BVH)

set(INCLUDE_DIR    
    ../../../../../thirdparty    
    ../../../../../thirdparty/glm/glm
    ../../../../../thirdparty/bvh/include
    .
)

include_directories(${INCLUDE_DIR})

add_executable(BVH api.cpp)
 
