cmake_minimum_required(VERSION 3.22)
project(PackageTest CXX)

find_package(load-gltf CONFIG REQUIRED)

add_executable(example src/example.cpp)
target_link_libraries(example load-gltf::load-gltf)
target_compile_features(example PUBLIC cxx_std_17)
