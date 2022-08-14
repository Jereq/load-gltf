from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake, cmake_layout

class LoadGltfConan(ConanFile):
    name = "load-gltf"
    version = "0.0.1"
    description = "A library to load and parse GLTF files."
    url = "https://github.com/Jereq/load-gltf"
    license = "MIT"
    author = "Sebastian Larsson (sjereq@gmail.com)"
    topics = "GLTF"

    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    exports_sources = "CMakeLists.txt", "src/*", "include/*"

    requires = "spdlog/1.10.0", "simdjson/2.2.2"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["load-gltf"]
