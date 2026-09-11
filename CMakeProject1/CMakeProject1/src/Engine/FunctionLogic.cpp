#include "Renderer.h"
#include "rendering/RenderSystem.h"
#include "GameObject.h"
void PointLight::UpdateLightData(RenderSystem& target)
{
     id = target.UpdatePointLightData(
          id,
          gameObject->transform->GetPosition(),
          Color,
          Intensity,
          Range
     );
}
void PointLight::UpdateLightData()
{
	if (!m_target)
		return;

	UpdateLightData(*m_target);
}
void PointLight::OnAdded(GameObject* owner) 
	{
		owner->transform->AddCallback(
			[this](const Transform& transform)
			{
				OnTransformChanged(transform);
			}
		);
	}
GameObject::GameObject()
{
	transform = AddComponent<Transform>();
}