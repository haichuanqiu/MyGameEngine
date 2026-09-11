#pragma once

#include <cstdint>
#include <vector>
#include <stdexcept>

constexpr uint32_t MAX_VERTEX_STREAMS = 8;

enum class VertexSemantic : uint16_t
{
	Position,
	Normal,
	Tangent,
	Color,
	TexCoord0,
	TexCoord1,
	BoneIndices,
	BoneWeights,
};

enum class VertexFormat : uint16_t
{
	Float,
	Float2,
	Float3,
	Float4,

	Half2,
	Half4,

	UInt8x4,
	UInt16x4,
	UInt32x4,

	UNorm8x4,
	UNorm16x4,
};

struct VertexAttribute
{
	VertexSemantic semantic;
	VertexFormat format;

	uint32_t offset;
	uint32_t stream;
};

struct VertexLayoutDesc
{
	std::vector<VertexAttribute> attributes;

	uint32_t stride[MAX_VERTEX_STREAMS] = {};
};



struct Mesh
{
	// CPU 数据
	std::vector<uint8_t> vertexData;
	std::vector<uint8_t> indexData;

	// CPU 侧描述：这些数据怎么解释
	VertexLayoutDesc vertexLayout;

	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;
};