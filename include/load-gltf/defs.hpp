// SPDX-License-Identifier: MIT
// Copyright © 2022 Sebastian Larsson

#pragma once

#ifdef _WIN32
#define LG_EXPORT __declspec(dllexport)
#else
#define load-gltf_EXPORT
#endif
