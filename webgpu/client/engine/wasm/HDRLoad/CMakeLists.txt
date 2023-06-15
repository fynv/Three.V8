cmake_minimum_required (VERSION 3.0)
project(HDRLoad)

set(INCLUDE_DIR    
    ../../../../../thirdparty    
    ../../../../../thirdparty/glm/glm
    .
)

include_directories(${INCLUDE_DIR})

add_executable(HDRLoad api.cpp)
 
