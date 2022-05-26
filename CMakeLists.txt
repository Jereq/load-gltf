cmake_minimum_required(VERSION 3.22)
project(load-gltf LANGUAGES CXX)

find_package(spdlog REQUIRED)

add_library(load-gltf src/load-gltf.cpp)
target_include_directories(load-gltf PUBLIC include)
target_link_libraries(load-gltf
        PRIVATE spdlog::spdlog)

set_target_properties(load-gltf PROPERTIES PUBLIC_HEADER "include/load-gltf.h")
install(TARGETS load-gltf)
