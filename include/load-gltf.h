// SPDX-License-Identifier: MIT
// Copyright © 2022 Sebastian Larsson

#pragma once

#ifdef _WIN32
#define load_gltf_EXPORT __declspec(dllexport)
#else
#define load-gltf_EXPORT
#endif

load_gltf_EXPORT void load_gltf();
