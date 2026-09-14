#pragma once

#include "Engine/GameObjectSystem.h"
#include "Engine/DataStructure.h"
#include "rendering/Shader.h"
#include "Serialization/ComponentRegistry.h"
#include "Assets/Material.h"
class GameObject;
class RenderSystem;
class Renderer : public Component
{  
public:
	Material* material;
	int renderSystemIndex = -1;
	
private:
};
REFLECT(
	Renderer,
	FIELD(Renderer, renderSystemIndex),
	REF_FIELD(Renderer, material),

)
REGISTER_COMPONENT(Renderer)
class PointLight :public Component {

public:
	Vector3 Color = Vector3(1.0f, 1.0f, 1.0f);

	float Intensity = 1.0f;
	float Range = 10.0f;

	int id = -1;

	void SetDefaultTarget(RenderSystem* target) {
		m_target= target;
	}
	void UpdateLightData();
	void UpdateLightData(RenderSystem& target);
	void OnAdded(GameObject* owner) override;

	void OnTransformChanged(const Transform& transform)
	{
		if(m_target){
		UpdateLightData(*m_target);
		}
	}

private:
	RenderSystem* m_target=nullptr;
	GameObject* m_owner = nullptr; 
};

REFLECT(
	PointLight,

	FIELD(PointLight, Intensity),
	FIELD(PointLight, Range),
	FIELD(PointLight, Color)
)
REGISTER_COMPONENT(PointLight)
class Camera : public Component
{
public:
	// Render target
	unsigned int renderInfo_targetFramebuffer = 0;

	// Projection
	float FieldOfView = 60.0f;
	float NearClip = 0.1f;
	float FarClip = 1000.0f;

	// Viewport
	int Width = 1280;
	int Height = 720;

	bool Perspective = true;

};

REFLECT(
	Camera,

	FIELD(Camera, FieldOfView),
	FIELD(Camera, NearClip),
	FIELD(Camera, FarClip),
	FIELD(Camera, Width),
	FIELD(Camera, Height),
	FIELD(Camera, Perspective)

)
REGISTER_COMPONENT(Camera)