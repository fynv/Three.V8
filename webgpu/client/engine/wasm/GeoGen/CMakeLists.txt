cmake_minimum_required (VERSION 3.0)
project(GeoGen)

set(INCLUDE_DIR    
    ../../../../../thirdparty    
    ../../../../../thirdparty/glm/glm
    .
)

include_directories(${INCLUDE_DIR})

add_executable(GeoGen api.cpp)
 
