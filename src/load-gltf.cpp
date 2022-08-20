// SPDX-License-Identifier: MIT
// Copyright © 2022 Sebastian Larsson

#include <load-gltf/load-gltf.hpp>

#include <load-gltf/structs.hpp>

#include <simdjson.h>
#include <spdlog/spdlog.h>

#include <charconv>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <tuple>
#include <utility>

namespace {
	// ************* Parser helpers *************************

	// Base case
	template<typename ResultType>
	void parseValue(simdjson::ondemand::value& json, ResultType&)
	{
		static_assert(std::is_void_v<ResultType>, "Unhandled type");
	}

	/**
	 * Represents a single field to be parsed in an object
	 *
	 * @tparam Type the type of the object
	 * @tparam MemberType the type of the member "field"
	 */
	template<typename Type, typename MemberType>
	struct ObjectParserField
	{
		std::string_view name;
		MemberType Type::* fieldPtr;
	};

	/**
	 * Parser builder for a JSON object
	 *
	 * @tparam ResultType
	 * @tparam MemberTypes
	 * @todo: Explore constexpr, requires compile-time strings
	 */
	template<typename ResultType, typename...MemberTypes>
	struct ObjectParser
	{
		std::string_view name;
		std::tuple<ObjectParserField<ResultType, MemberTypes>...> fields;

		explicit ObjectParser(std::string_view name, ObjectParserField<ResultType, MemberTypes>... fields) noexcept
			: name(name), fields(fields...) {}

		template<typename MemberType>
		ObjectParser<ResultType, MemberTypes..., MemberType>
		operator()(std::string_view fieldName, MemberType ResultType::* memberPtr) const noexcept
		{
			return [&]<size_t...I>(std::index_sequence<I...>)
			{
				return ObjectParser<ResultType, MemberTypes..., MemberType>{name, std::get<I>(fields)...,
					{fieldName, memberPtr}};
			}(std::index_sequence_for<MemberTypes...>());
		}

		ResultType parse(simdjson::ondemand::object& json) const
		{
			ResultType result = {};
			for (simdjson::ondemand::field field: json)
			{
				std::string_view propertyName = field.unescaped_key();
				simdjson::ondemand::value propertyValue = field.value();

				auto matchAndAssign = [&result, &propertyName, &propertyValue]
					<typename FieldType>(ObjectParserField<ResultType, FieldType> objectParserField)
				{
					if (objectParserField.name == propertyName)
					{
						parseValue(propertyValue, result.*objectParserField.fieldPtr);
						return true;
					}
					else
					{
						return false;
					}
				};
				[&]<size_t...I>(std::index_sequence<I...>)
				{
					if (!(false || ... || matchAndAssign(std::get<I>(fields))))
					{
						SPDLOG_INFO("Unknown {} property: {}", name, propertyName);
					}
				}(std::index_sequence_for<MemberTypes...>());
			}
			return result;
		}

		ResultType parse(simdjson::ondemand::value& json) const
		{
			simdjson::ondemand::object object = json.get_object();
			return parse(object);
		}

		ResultType parse(simdjson::ondemand::document& json) const
		{
			simdjson::ondemand::object object = json.get_object();
			return parse(object);
		}
	};

	// ********************* Parser definitions *********************

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::Version& val)
	{
		std::string_view versionString = json.get_string();
		uint32_t major = 0;
		uint32_t minor = 0;
		char const* endPtr = versionString.data() + versionString.length();
		auto [majorEnd, majorEc] = std::from_chars(versionString.data(), endPtr, major);
		if (majorEc != std::errc{} || majorEnd == endPtr || majorEnd[0] != '.')
		{
			throw std::invalid_argument("Failed to parse major version");
		}
		auto [minorEnd, minorEc] = std::from_chars(majorEnd + 1, endPtr, minor);
		if (minorEc != std::errc{} || minorEnd != endPtr)
		{
			throw std::invalid_argument("Failed to parse minor version");
		}

		val = {major, minor};
	}

	// TODO: Add remaining fields
	auto const assetParser = ObjectParser<lg::Asset>("asset")
		("version", &lg::Asset::version);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::Asset& val)
	{
		val = assetParser.parse(json);
	}

	auto const gltfParser = ObjectParser<lg::Gltf>("GLTF")
		("asset", &lg::Gltf::asset);
}

lg::Gltf lg::loadGltf(std::string_view inputJson)
{
	simdjson::padded_string paddedString(inputJson);
	simdjson::ondemand::parser parser;
	simdjson::ondemand::document doc = parser.iterate(paddedString);
	simdjson::ondemand::object const& top = doc.get_object();
	SPDLOG_INFO("Loading Gltf...");
	lg::Gltf result = gltfParser.parse(doc);
	// TODO: Implement validation
	return result;
}
