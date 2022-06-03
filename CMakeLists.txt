cmake_minimum_required(VERSION 3.22)
project(load-gltf LANGUAGES CXX)



find_package(spdlog REQUIRED)

add_library(load-gltf
        src/load-gltf.cpp
        include/load-gltf/load-gltf.hpp
        include/load-gltf/structs.hpp
        include/load-gltf/defs.hpp
        )
target_include_directories(load-gltf PUBLIC include)
target_link_libraries(load-gltf
        PRIVATE spdlog::spdlog)
target_compile_features(load-gltf PUBLIC cxx_std_17)

set_target_properties(load-gltf PROPERTIES
        PUBLIC_HEADER
        "include/load-gltf/defs.hpp"
        "include/load-gltf/load-gltf.hpp"
        "include/load-gltf/structs.hpp"
        )
install(TARGETS load-gltf)
