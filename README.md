# LoadGltf #

A library to load and parse GLTF files

## Examples ##

TODO

## Getting started ##

Using the library currently requires it to be built locally.

### Building

Prerequisites:

- [CMake](https://cmake.org/)
- [Conan](https://conan.io/)

To install dependencies for development:

```bash
conan install . -s build_type=Debug --build missing
conan install . -s build_type=Release --build missing
```

To build and install the package locally:

```bash
conan create . -s build_type=Debug --build missing
conan create . -s build_type=Release --build missing
```

## Licence ##

This project is provided under the MIT license. See LICENCE.txt for the full text.
