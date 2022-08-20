cmake_minimum_required(VERSION 3.22)
project(load-gltf LANGUAGES CXX)

find_package(spdlog REQUIRED)
find_package(simdjson REQUIRED)

set(load-gltf-HDRS
        include/load-gltf/load-gltf.hpp
        include/load-gltf/structs.hpp
        include/load-gltf/defs.hpp
        )

add_library(load-gltf
        src/load-gltf.cpp
        ${load-gltf-HDRS}
        )
target_include_directories(load-gltf PUBLIC include)
target_link_libraries(load-gltf
        PRIVATE
        simdjson::simdjson
        spdlog::spdlog
        )
target_compile_features(load-gltf PUBLIC cxx_std_20)

set_target_properties(load-gltf PROPERTIES
        PUBLIC_HEADER "${load-gltf-HDRS}"
        )
install(TARGETS load-gltf
        PUBLIC_HEADER DESTINATION include/load-gltf)
