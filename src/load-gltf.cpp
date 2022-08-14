// SPDX-License-Identifier: MIT
// Copyright © 2022 Sebastian Larsson

#include <load-gltf/load-gltf.hpp>

#include <load-gltf/structs.hpp>

#include <spdlog/spdlog.h>

#include <string_view>

lg::Gltf lg::loadGltf(std::string_view inputJson)
{
	// TODO: Pad input
	// TODO: Implement parsing
	SPDLOG_INFO("Loading Gltf...");
	// TODO: Implement validation
	return {};
}
