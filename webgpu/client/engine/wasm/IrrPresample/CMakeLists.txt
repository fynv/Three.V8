cmake_minimum_required (VERSION 3.0)
project(IrrPresample)

set(INCLUDE_DIR    
    ../../../../../thirdparty    
    ../../../../../thirdparty/glm/glm
    .
)

include_directories(${INCLUDE_DIR})

add_executable(IrrPresample api.cpp)
 
