// SPDX-License-Identifier: MIT
// Copyright © 2022 Sebastian Larsson

#pragma once

#include <load-gltf/defs.hpp>

#include <array>
#include <cstdint>
#include <unordered_map>
#include <optional>
#include <string>
#include <vector>

namespace lg {
	struct LG_EXPORT Extension
	{
		// TODO: JSON structure or extension specific structs?
	};

	struct LG_EXPORT Extras
	{
		// TODO: JSON value
	};

	struct LG_EXPORT AccessorSparseIndices
	{
		std::uint32_t bufferView = {};
		std::uint32_t byteOffset = 0;
		std::uint32_t componentType = {};
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT AccessorSparseValues
	{
		std::uint32_t bufferView = {};
		std::uint32_t byteOffset = 0;
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT AccessorSparse
	{
		std::uint32_t count = {};
		AccessorSparseIndices indices;
		AccessorSparseValues values;
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT Accessor
	{
		std::optional<std::uint32_t> bufferView;
		std::uint32_t byteOffset = 0;
		std::uint32_t componentType = {};
		bool normalized = false;
		std::uint32_t count = {};
		std::string type;
		std::vector<double> max;
		std::vector<double> min;
		std::optional<AccessorSparse> sparse;
		std::optional<std::string> name;
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT AnimationChannelTarget
	{
		std::optional<std::uint32_t> node;
		std::string path;
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT AnimationChannel
	{
		std::uint32_t sampler = {};
		AnimationChannelTarget target;
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT AnimationSampler
	{
		std::uint32_t input = {};
		std::string interpolation = "LINEAR";
		std::uint32_t output = {};
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT Animation
	{
		std::vector<AnimationChannel> channels;
		std::vector<AnimationSampler> samplers;
		std::optional<std::string> name;
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT Version
	{
		std::uint32_t major = {};
		std::uint32_t minor = {};
	};

	struct LG_EXPORT Asset
	{
		std::optional<std::string> copyright;
		std::optional<std::string> generator;
		Version version;
		std::optional<Version> minVersion;
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT Buffer
	{
		std::optional<std::string> uri;
		std::uint32_t byteLength = {};
		std::optional<std::string> name;
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT BufferView
	{
		std::uint32_t buffer = {};
		std::uint32_t byteOffset = 0;
		std::uint32_t byteLength = {};
		std::optional<std::uint32_t> byteStride;
		std::optional<std::uint32_t> target;
		std::optional<std::string> name;
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT CameraOrthographic
	{
		double xmag = {};
		double ymag = {};
		double zfar = {};
		double znear = {};
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT CameraPerspective
	{
		std::optional<double> aspectRatio;
		double yfov = {};
		std::optional<double> zfar;
		double znear = {};
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT Camera
	{
		std::optional<CameraOrthographic> orthographic;
		std::optional<CameraPerspective> perspective;
		std::string type;
		std::optional<std::string> name;
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT Image
	{
		std::optional<std::string> uri;
		std::optional<std::string> mimeType;
		std::optional<std::uint32_t> bufferView;
		std::optional<std::string> name;
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT TextureInfo
	{
		std::uint32_t index = {};
		std::uint32_t texCoord = 0;
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT MaterialNormalTexture
	{
		std::uint32_t index = {};
		std::uint32_t texCoord = 0;
		double scale = 1.0;
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT MaterialOcclusionTexture
	{
		std::uint32_t index = {};
		std::uint32_t texCoord = 0;
		double strength = 1.0;
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT MaterialPbrMetallicRoughness
	{
		std::array<double, 4> baseColorFactor = {1.0, 1.0, 1.0, 1.0};
		std::optional<TextureInfo> baseColorTexture;
		double metallicFactor = 1.0;
		double roughnessFactor = 1.0;
		std::optional<TextureInfo> metallicRoughnessTexture;
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT Material
	{
		std::optional<std::string> name;
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
		std::optional<MaterialPbrMetallicRoughness> pbrMetallicRoughness;
		std::optional<MaterialNormalTexture> normalTexture;
		std::optional<MaterialOcclusionTexture> occlusionTexture;
		std::optional<TextureInfo> emissiveTexture;
		std::array<double, 3> emissiveFactor = {0.0, 0.0, 0.0};
		std::string alphaMode = "OPAQUE";
		double alphaCutoff = 0.5;
		bool doubleSided = false;
	};

	struct LG_EXPORT MeshPrimitive
	{
		std::unordered_map<std::string, std::uint32_t> attributes;
		std::optional<std::uint32_t> indices;
		std::optional<std::uint32_t> material;
		std::uint32_t mode = 4;
		std::vector<std::uint32_t> targets;
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT Mesh
	{
		std::vector<MeshPrimitive> primitives;
		std::vector<double> weights;
		std::optional<std::string> name;
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT Node
	{
		std::optional<std::uint32_t> camera;
		std::vector<std::uint32_t> children;
		std::optional<std::uint32_t> skin;
		std::array<double, 16> matrix = {
			1.0, 0.0, 0.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0,
			0.0, 0.0, 0.0, 1.0};
		std::optional<std::uint32_t> mesh;
		std::array<double, 4> rotation = {0.0, 0.0, 0.0, 1.0};
		std::array<double, 3> scale = {1.0, 1.0, 1.0};
		std::array<double, 3> translation = {0.0, 0.0, 0.0};
		std::vector<double> weights;
		std::optional<std::string> name;
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT Sampler
	{
		std::optional<std::uint32_t> magFilter;
		std::optional<std::uint32_t> minFilter;
		std::uint32_t wrapS = 10497;
		std::uint32_t wrapT = 10497;
		std::optional<std::string> name;
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT Scene
	{
		std::vector<std::uint32_t> nodes;
		std::optional<std::string> name;
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT Skin
	{
		std::optional<std::uint32_t> inverseBindMatrices;
		std::optional<std::uint32_t> skeleton;
		std::vector<std::uint32_t> joints;
		std::optional<std::string> name;
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT Texture
	{
		std::optional<std::uint32_t> sampler;
		std::optional<std::uint32_t> source;
		std::optional<std::string> name;
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};

	struct LG_EXPORT Gltf
	{
		std::vector<std::string> extensionsUsed;
		std::vector<std::string> extensionsRequired;
		std::vector<Accessor> accessors;
		std::vector<Animation> animations;
		Asset asset;
		std::vector<Buffer> buffers;
		std::vector<BufferView> bufferViews;
		std::vector<Camera> cameras;
		std::vector<Image> images;
		std::vector<Material> materials;
		std::vector<Mesh> meshes;
		std::vector<Node> nodes;
		std::vector<Sampler> samplers;
		std::optional<std::uint32_t> scene;
		std::vector<Scene> scenes;
		std::vector<Skin> skins;
		std::vector<Texture> textures;
		std::unordered_map<std::string, Extension> extensions;
		std::optional<Extras> extras;
	};
}
