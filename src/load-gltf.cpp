// SPDX-License-Identifier: MIT
// Copyright © 2022 Sebastian Larsson

#include <load-gltf/load-gltf.hpp>

#include <load-gltf/structs.hpp>

#include <simdjson.h>
#include <spdlog/spdlog.h>

#include <string_view>

lg::Gltf lg::loadGltf(std::string_view inputJson)
{
	simdjson::padded_string paddedString(inputJson);
	simdjson::ondemand::parser parser;
	simdjson::ondemand::document doc = parser.iterate(paddedString);
	simdjson::ondemand::object const& top = doc.get_object();
	// TODO: Implement parsing
	SPDLOG_INFO("Loading Gltf...");
	// TODO: Implement validation
	return {};
}
