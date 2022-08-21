// SPDX-License-Identifier: MIT
// Copyright © 2022 Sebastian Larsson

#pragma once

#include <load-gltf/structs.hpp>

#include <string_view>

namespace lg {
	// TODO: Docs
	// TODO: Error-reporting (replace exceptions & log)
	// TODO: Alternative inputs (byte, etc)
	// TODO: Allocator support
	LG_EXPORT Gltf loadGltf(std::string_view inputJson);

	constexpr std::size_t paddingSize = 64;

	LG_EXPORT Gltf loadGltfPrePadded(std::string_view paddedInputJson);
}
