#pragma once

#include <glm/glm.hpp>


// ============================================================
// GPU Point Light
// ============================================================

struct alignas(16) GPUPointLight
{
	glm::vec4 position;
	glm::vec4 colorIntensity;
	glm::vec4 params;
};


// ============================================================
// GPU Spot Light
// ============================================================

struct alignas(16) GPUSpotLight
{
	glm::vec4 position;
	glm::vec4 direction;
	glm::vec4 colorIntensity;
	glm::vec4 params;
};


// ============================================================
// GPU Directional Light
// ============================================================

struct alignas(16) GPUDirectionalLight
{
	glm::vec4 direction;
	glm::vec4 colorIntensity;
};


// ============================================================
// GPU Light Block
// ============================================================

struct GPULightBlock
{
	GPUPointLight pointLights[10];
	GPUSpotLight spotLights[10];
	GPUDirectionalLight directionalLights[10];

	glm::vec4 ambientColor;

	int pointLightCount;
	int spotLightCount;
	int directionalLightCount;
	int padding;
};


// ============================================================
// CPU Camera Data
// ============================================================

struct CameraData
{
	glm::mat4 View;
	glm::mat4 Projection;
	glm::mat4 ViewProjection;

	glm::vec3 Position;
	unsigned int targetFramebuffer;
};


// ============================================================
// GPU Camera Data
// ============================================================

struct GPUCameraData
{
	glm::mat4 View;
	glm::mat4 Projection;
	glm::mat4 ViewProjection;

	glm::vec4 Position;
};