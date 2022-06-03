// SPDX-License-Identifier: MIT
// Copyright © 2022 Sebastian Larsson

#include <load-gltf/load-gltf.hpp>

#include <spdlog/spdlog.h>

void load_gltf()
{
#ifdef NDEBUG
	SPDLOG_INFO("load-gltf/0.1: Hello World Release!");
#else
	SPDLOG_INFO("load-gltf/0.1: Hello World Debug!");
#endif

	// ARCHITECTURES
#ifdef _M_X64
	SPDLOG_INFO("  load-gltf/0.1: _M_X64 defined");
#endif

#ifdef _M_IX86
	SPDLOG_INFO("  load-gltf/0.1: _M_IX86 defined");
#endif

#if __i386__
	SPDLOG_INFO("  load-gltf/0.1: __i386__ defined");
#endif

#if __x86_64__
	SPDLOG_INFO("  load-gltf/0.1: __x86_64__ defined");
#endif

	// Libstdc++
#if defined _GLIBCXX_USE_CXX11_ABI
	SPDLOG_INFO("  load-gltf/0.1: _GLIBCXX_USE_CXX11_ABI ={}", _GLIBCXX_USE_CXX11_ABI);
#endif

	// COMPILER VERSIONS
#if _MSC_VER
	SPDLOG_INFO("  load-gltf/0.1: _MSC_VER={}", _MSC_VER);
#endif

#if _MSVC_LANG
	SPDLOG_INFO("  load-gltf/0.1: _MSVC_LANG={}", _MSVC_LANG);
#endif

#if __cplusplus
	SPDLOG_INFO("  load-gltf/0.1: __cplusplus={}", __cplusplus);
#endif

#if __INTEL_COMPILER
	SPDLOG_INFO("  load-gltf/0.1: __INTEL_COMPILER={}", __INTEL_COMPILER);
#endif

#if __GNUC__
	SPDLOG_INFO("  load-gltf/0.1: __GNUC__={}", __GNUC__);
#endif

#if __GNUC_MINOR__
	SPDLOG_INFO("  load-gltf/0.1: __GNUC_MINOR__={}", __GNUC_MINOR__);
#endif

#if __clang_major__
	SPDLOG_INFO("  load-gltf/0.1: __clang_major__={}", __clang_major__);
#endif

#if __clang_minor__
	SPDLOG_INFO("  load-gltf/0.1: __clang_minor__={}", __clang_minor__);
#endif

#if __apple_build_version__
	SPDLOG_INFO("  load-gltf/0.1: __apple_build_version__={}", __apple_build_version__);
#endif

	// SUBSYSTEMS

#if __MSYS__
	SPDLOG_INFO("  load-gltf/0.1: __MSYS__={}", __MSYS__);
#endif

#if __MINGW32__
	SPDLOG_INFO("  load-gltf/0.1: __MINGW32__={}", __MINGW32__);
#endif

#if __MINGW64__
	SPDLOG_INFO("  load-gltf/0.1: __MINGW64__={}", __MINGW64__);
#endif

#if __CYGWIN__
	SPDLOG_INFO("  load-gltf/0.1: __CYGWIN__={}", __CYGWIN__);
#endif
}
