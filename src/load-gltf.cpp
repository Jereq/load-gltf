// SPDX-License-Identifier: MIT
// Copyright © 2022 Sebastian Larsson

#include <load-gltf/load-gltf.hpp>

#include <load-gltf/structs.hpp>

#include <simdjson.h>
#include <spdlog/spdlog.h>

#include <array>
#include <charconv>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {
	// ************* Parser helpers *************************

	uint32_t parseUint32(simdjson::ondemand::value& json)
	{
		double doubleValue = json.get_double();
		auto castValue = static_cast<uint32_t>(doubleValue);
		if (doubleValue != castValue)
		{
			throw std::runtime_error("Failed to parse uint32");
		}
		return castValue;
	}

	// Base case
	template<typename ResultType>
	void parseValue(simdjson::ondemand::value& json, ResultType&)
	{
		static_assert(std::is_void_v<ResultType>, "Unhandled type");
	}

	template<typename ResultType>
	void parseValue(simdjson::ondemand::value& json, std::optional<ResultType>& val)
	{
		val = ResultType{};
		parseValue(json, val.value());
	}

	template<>
	void parseValue(simdjson::ondemand::value& json, uint32_t& val)
	{
		val = parseUint32(json);
	}

	template<>
	void parseValue(simdjson::ondemand::value& json, double& val)
	{
		val = json.get_double();
	}

	template<>
	void parseValue(simdjson::ondemand::value& json, bool& val)
	{
		val = json.get_bool();
	}

	template<>
	void parseValue(simdjson::ondemand::value& json, std::string& val)
	{
		val = static_cast<std::string_view>(json.get_string());
	}

	template<typename ArrayElementType, size_t ArraySize>
	void parseValue(simdjson::ondemand::value& json, std::array<ArrayElementType, ArraySize>& val)
	{
		std::vector<ArrayElementType> parseBuffer;
		for (simdjson::ondemand::value value: json.get_array())
		{
			parseValue(value, parseBuffer.emplace_back());
		}

		if (parseBuffer.size() != ArraySize)
		{
			throw std::invalid_argument("Wrong number of elements in array");
		}

		std::move(parseBuffer.cbegin(), parseBuffer.cend(), val.begin());
	}

	template<typename ElementType>
	void parseValue(simdjson::ondemand::value& json, std::vector<ElementType>& val)
	{
		val.clear();
		for (simdjson::ondemand::value value: json.get_array())
		{
			parseValue(value, val.emplace_back());
		}
	}

	template<typename ElementType>
	void parseValue(simdjson::ondemand::value& json, std::unordered_map<std::string, ElementType>& val)
	{
		val.clear();
		for (simdjson::ondemand::field&& field: json.get_object())
		{
			std::string_view key = field.unescaped_key();
			ElementType value = {};
			parseValue(field.value(), value);
			val.insert_or_assign(std::string(key), std::move(value));
		}
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
	void parseValue(simdjson::ondemand::value& json, lg::Extension& val)
	{
		// No implementation yet
	}

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::Extras& val)
	{
		// No implementation yet
	}

	auto const accessorSparseIndicesParser = ObjectParser<lg::AccessorSparseIndices>("accessorSparseIndices")
		("bufferView", &lg::AccessorSparseIndices::bufferView)
		("byteOffset", &lg::AccessorSparseIndices::byteOffset)
		("componentType", &lg::AccessorSparseIndices::componentType)
		("extensions", &lg::AccessorSparseIndices::extensions)
		("extras", &lg::AccessorSparseIndices::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::AccessorSparseIndices& val)
	{
		val = accessorSparseIndicesParser.parse(json);
	}

	auto const accessorSparseValuesParser = ObjectParser<lg::AccessorSparseValues>("accessorSparseValues")
		("bufferView", &lg::AccessorSparseValues::bufferView)
		("byteOffset", &lg::AccessorSparseValues::byteOffset)
		("extensions", &lg::AccessorSparseValues::extensions)
		("extras", &lg::AccessorSparseValues::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::AccessorSparseValues& val)
	{
		val = accessorSparseValuesParser.parse(json);
	}

	auto const accessorSparseParser = ObjectParser<lg::AccessorSparse>("accessorSparse")
		("count", &lg::AccessorSparse::count)
		("indices", &lg::AccessorSparse::indices)
		("values", &lg::AccessorSparse::values)
		("extensions", &lg::AccessorSparse::extensions)
		("extras", &lg::AccessorSparse::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::AccessorSparse& val)
	{
		val = accessorSparseParser.parse(json);
	}

	auto const accessorParser = ObjectParser<lg::Accessor>("accessor")
		("bufferView", &lg::Accessor::bufferView)
		("byteOffset", &lg::Accessor::byteOffset)
		("componentType", &lg::Accessor::componentType)
		("normalized", &lg::Accessor::normalized)
		("count", &lg::Accessor::count)
		("type", &lg::Accessor::type)
		("max", &lg::Accessor::max)
		("min", &lg::Accessor::min)
		("sparse", &lg::Accessor::sparse)
		("name", &lg::Accessor::name)
		("extensions", &lg::Accessor::extensions)
		("extras", &lg::Accessor::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::Accessor& val)
	{
		val = accessorParser.parse(json);
	}

	auto const channelTargetParser = ObjectParser<lg::AnimationChannelTarget>("animation channel target")
		("node", &lg::AnimationChannelTarget::node)
		("path", &lg::AnimationChannelTarget::path)
		("extensions", &lg::AnimationChannelTarget::extensions)
		("extras", &lg::AnimationChannelTarget::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::AnimationChannelTarget& val)
	{
		val = channelTargetParser.parse(json);
	}

	auto const animationChannelParser = ObjectParser<lg::AnimationChannel>("animation channel")
		("sampler", &lg::AnimationChannel::sampler)
		("target", &lg::AnimationChannel::target)
		("extensions", &lg::AnimationChannel::extensions)
		("extras", &lg::AnimationChannel::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::AnimationChannel& val)
	{
		val = animationChannelParser.parse(json);
	}

	auto const animationSamplerParser = ObjectParser<lg::AnimationSampler>("animation sampler")
		("input", &lg::AnimationSampler::input)
		("interpolation", &lg::AnimationSampler::interpolation)
		("output", &lg::AnimationSampler::output)
		("extensions", &lg::AnimationSampler::extensions)
		("extras", &lg::AnimationSampler::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::AnimationSampler& val)
	{
		val = animationSamplerParser.parse(json);
	}

	auto const animationParser = ObjectParser<lg::Animation>("animation")
		("channels", &lg::Animation::channels)
		("samplers", &lg::Animation::samplers)
		("name", &lg::Animation::name)
		("extensions", &lg::Animation::extensions)
		("extras", &lg::Animation::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::Animation& val)
	{
		val = animationParser.parse(json);
	}

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

	auto const assetParser = ObjectParser<lg::Asset>("asset")
		("copyright", &lg::Asset::copyright)
		("generator", &lg::Asset::generator)
		("version", &lg::Asset::version)
		("minVersion", &lg::Asset::minVersion)
		("extensions", &lg::Asset::extensions)
		("extras", &lg::Asset::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::Asset& val)
	{
		val = assetParser.parse(json);
	}

	auto const bufferParser = ObjectParser<lg::Buffer>("buffer")
		("uri", &lg::Buffer::uri)
		("byteLength", &lg::Buffer::byteLength)
		("name", &lg::Buffer::name)
		("extensions", &lg::Buffer::extensions)
		("extras", &lg::Buffer::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::Buffer& val)
	{
		val = bufferParser.parse(json);
	}

	auto const bufferViewParser = ObjectParser<lg::BufferView>("buffer view")
		("buffer", &lg::BufferView::buffer)
		("byteOffset", &lg::BufferView::byteOffset)
		("byteLength", &lg::BufferView::byteLength)
		("byteStride", &lg::BufferView::byteStride)
		("target", &lg::BufferView::target)
		("name", &lg::BufferView::name)
		("extensions", &lg::BufferView::extensions)
		("extras", &lg::BufferView::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::BufferView& val)
	{
		val = bufferViewParser.parse(json);
	}

	auto const cameraOrthographicParser = ObjectParser<lg::CameraOrthographic>("camera orthographic")
		("xmag", &lg::CameraOrthographic::xmag)
		("ymag", &lg::CameraOrthographic::ymag)
		("zfar", &lg::CameraOrthographic::zfar)
		("znear", &lg::CameraOrthographic::znear)
		("extensions", &lg::CameraOrthographic::extensions)
		("extras", &lg::CameraOrthographic::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::CameraOrthographic& val)
	{
		val = cameraOrthographicParser.parse(json);
	}

	auto const cameraPerspectiveParser = ObjectParser<lg::CameraPerspective>("camera perspective")
		("aspectRatio", &lg::CameraPerspective::aspectRatio)
		("yfov", &lg::CameraPerspective::yfov)
		("zfar", &lg::CameraPerspective::zfar)
		("znear", &lg::CameraPerspective::znear)
		("extensions", &lg::CameraPerspective::extensions)
		("extras", &lg::CameraPerspective::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::CameraPerspective& val)
	{
		val = cameraPerspectiveParser.parse(json);
	}

	auto const cameraParser = ObjectParser<lg::Camera>("camera")
		("orthographic", &lg::Camera::orthographic)
		("perspective", &lg::Camera::perspective)
		("type", &lg::Camera::type)
		("name", &lg::Camera::name)
		("extensions", &lg::Camera::extensions)
		("extras", &lg::Camera::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::Camera& val)
	{
		val = cameraParser.parse(json);
	}

	auto const imageParser = ObjectParser<lg::Image>("image")
		("uri", &lg::Image::uri)
		("mimeType", &lg::Image::mimeType)
		("bufferView", &lg::Image::bufferView)
		("name", &lg::Image::name)
		("extensions", &lg::Image::extensions)
		("extras", &lg::Image::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::Image& val)
	{
		val = imageParser.parse(json);
	}

	auto const textureInfoParser = ObjectParser<lg::TextureInfo>("texture info")
		("index", &lg::TextureInfo::index)
		("texCoord", &lg::TextureInfo::texCoord)
		("extensions", &lg::TextureInfo::extensions)
		("extras", &lg::TextureInfo::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::TextureInfo& val)
	{
		val = textureInfoParser.parse(json);
	}

	auto const materialNormalTextureParser = ObjectParser<lg::MaterialNormalTexture>("material normal texture")
		("index", &lg::MaterialNormalTexture::index)
		("texCoord", &lg::MaterialNormalTexture::texCoord)
		("scale", &lg::MaterialNormalTexture::scale)
		("extensions", &lg::MaterialNormalTexture::extensions)
		("extras", &lg::MaterialNormalTexture::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::MaterialNormalTexture& val)
	{
		val = materialNormalTextureParser.parse(json);
	}

	auto const materialOcclusionTextureParser = ObjectParser<lg::MaterialOcclusionTexture>("material occlusion texture")
		("index", &lg::MaterialOcclusionTexture::index)
		("texCoord", &lg::MaterialOcclusionTexture::texCoord)
		("strength", &lg::MaterialOcclusionTexture::strength)
		("extensions", &lg::MaterialOcclusionTexture::extensions)
		("extras", &lg::MaterialOcclusionTexture::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::MaterialOcclusionTexture& val)
	{
		val = materialOcclusionTextureParser.parse(json);
	}

	auto const materialPbrMetallicRoughnessParser = ObjectParser<lg::MaterialPbrMetallicRoughness>(
		"material PBR metallic roughness")
		("baseColorFactor", &lg::MaterialPbrMetallicRoughness::baseColorFactor)
		("baseColorTexture", &lg::MaterialPbrMetallicRoughness::baseColorTexture)
		("metallicFactor", &lg::MaterialPbrMetallicRoughness::metallicFactor)
		("roughnessFactor", &lg::MaterialPbrMetallicRoughness::roughnessFactor)
		("metallicRoughnessTexture", &lg::MaterialPbrMetallicRoughness::metallicRoughnessTexture)
		("extensions", &lg::MaterialPbrMetallicRoughness::extensions)
		("extras", &lg::MaterialPbrMetallicRoughness::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::MaterialPbrMetallicRoughness& val)
	{
		val = materialPbrMetallicRoughnessParser.parse(json);
	}

	auto const materialParser = ObjectParser<lg::Material>("material")
		("name", &lg::Material::name)
		("extensions", &lg::Material::extensions)
		("extras", &lg::Material::extras)
		("pbrMetallicRoughness", &lg::Material::pbrMetallicRoughness)
		("normalTexture", &lg::Material::normalTexture)
		("occlusionTexture", &lg::Material::occlusionTexture)
		("emissiveTexture", &lg::Material::emissiveTexture)
		("emissiveFactor", &lg::Material::emissiveFactor)
		("alphaMode", &lg::Material::alphaMode)
		("alphaCutoff", &lg::Material::alphaCutoff)
		("doubleSided", &lg::Material::doubleSided);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::Material& val)
	{
		val = materialParser.parse(json);
	}

	std::unordered_map<std::string, uint32_t> parseAttributes(simdjson::ondemand::value& json)
	{
		std::unordered_map<std::string, uint32_t> result = {};
		for (simdjson::ondemand::field field: json.get_object())
		{
			std::string_view key = field.unescaped_key();
			uint32_t value = parseUint32(field.value());
			result.insert_or_assign(std::string(key), value);
		}
		return result;
	}

	template<>
	void parseValue(simdjson::ondemand::value& json, std::unordered_map<std::string, uint32_t>& val)
	{
		val = parseAttributes(json);
	}

	auto const meshPrimitiveParser = ObjectParser<lg::MeshPrimitive>("mesh primitive")
		("attributes", &lg::MeshPrimitive::attributes)
		("indices", &lg::MeshPrimitive::indices)
		("material", &lg::MeshPrimitive::material)
		("mode", &lg::MeshPrimitive::mode)
		("targets", &lg::MeshPrimitive::targets)
		("extensions", &lg::MeshPrimitive::extensions)
		("extras", &lg::MeshPrimitive::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::MeshPrimitive& val)
	{
		val = meshPrimitiveParser.parse(json);
	}

	auto const meshParser = ObjectParser<lg::Mesh>("mesh")
		("primitives", &lg::Mesh::primitives)
		("weights", &lg::Mesh::weights)
		("name", &lg::Mesh::name)
		("extensions", &lg::Mesh::extensions)
		("extras", &lg::Mesh::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::Mesh& val)
	{
		val = meshParser.parse(json);
	}

	auto const nodeParser = ObjectParser<lg::Node>("node")
		("camera", &lg::Node::camera)
		("children", &lg::Node::children)
		("skin", &lg::Node::skin)
		("matrix", &lg::Node::matrix)
		("mesh", &lg::Node::mesh)
		("rotation", &lg::Node::rotation)
		("scale", &lg::Node::scale)
		("translation", &lg::Node::translation)
		("weights", &lg::Node::weights)
		("name", &lg::Node::name)
		("extensions", &lg::Node::extensions)
		("extras", &lg::Node::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::Node& val)
	{
		val = nodeParser.parse(json);
	}

	auto const samplerParser = ObjectParser<lg::Sampler>("sampler")
		("magFilter", &lg::Sampler::magFilter)
		("minFilter", &lg::Sampler::minFilter)
		("wrapS", &lg::Sampler::wrapS)
		("wrapT", &lg::Sampler::wrapT)
		("name", &lg::Sampler::name)
		("extensions", &lg::Sampler::extensions)
		("extras", &lg::Sampler::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::Sampler& val)
	{
		val = samplerParser.parse(json);
	}

	auto const sceneParser = ObjectParser<lg::Scene>("scene")
		("nodes", &lg::Scene::nodes)
		("name", &lg::Scene::name)
		("extensions", &lg::Scene::extensions)
		("extras", &lg::Scene::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::Scene& val)
	{
		val = sceneParser.parse(json);
	}

	auto const skinParser = ObjectParser<lg::Skin>("skin")
		("inverseBindMatrices", &lg::Skin::inverseBindMatrices)
		("skeleton", &lg::Skin::skeleton)
		("joints", &lg::Skin::joints)
		("name", &lg::Skin::name)
		("extensions", &lg::Skin::extensions)
		("extras", &lg::Skin::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::Skin& val)
	{
		val = skinParser.parse(json);
	}

	auto const textureParser = ObjectParser<lg::Texture>("texture")
		("sampler", &lg::Texture::sampler)
		("source", &lg::Texture::source)
		("name", &lg::Texture::name)
		("extensions", &lg::Texture::extensions)
		("extras", &lg::Texture::extras);

	template<>
	void parseValue(simdjson::ondemand::value& json, lg::Texture& val)
	{
		val = textureParser.parse(json);
	}

	auto const gltfParser = ObjectParser<lg::Gltf>("GLTF")
		("extensionsUsed", &lg::Gltf::extensionsUsed)
		("extensionsRequired", &lg::Gltf::extensionsRequired)
		("accessors", &lg::Gltf::accessors)
		("animations", &lg::Gltf::animations)
		("asset", &lg::Gltf::asset)
		("buffers", &lg::Gltf::buffers)
		("bufferViews", &lg::Gltf::bufferViews)
		("cameras", &lg::Gltf::cameras)
		("images", &lg::Gltf::images)
		("materials", &lg::Gltf::materials)
		("meshes", &lg::Gltf::meshes)
		("nodes", &lg::Gltf::nodes)
		("samplers", &lg::Gltf::samplers)
		("scene", &lg::Gltf::scene)
		("scenes", &lg::Gltf::scenes)
		("skins", &lg::Gltf::skins)
		("textures", &lg::Gltf::textures)
		("extensions", &lg::Gltf::extensions)
		("extras", &lg::Gltf::extras);
}

lg::Gltf lg::loadGltf(std::string_view inputJson)
{
	return lg::loadGltfPrePadded(simdjson::padded_string(inputJson));
}

static_assert(lg::paddingSize == simdjson::SIMDJSON_PADDING, "Padding must be the same");

lg::Gltf lg::loadGltfPrePadded(std::string_view paddedInputJson)
{
	simdjson::ondemand::parser parser;
	simdjson::ondemand::document doc = parser.iterate(paddedInputJson, paddedInputJson.size() + lg::paddingSize);
	SPDLOG_INFO("Loading Gltf...");
	lg::Gltf result = gltfParser.parse(doc);
	// TODO: Implement validation
	return result;
}
