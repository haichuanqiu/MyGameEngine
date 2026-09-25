#include "Engine/Components/RenderingRelatedComponents.h"
#include "rendering/RenderSystem.h"
#include "Engine/GameObjectSystem.h"

#include "Engine/Engine.h"
void PointLight::UpdateLightData(RenderSystem& target)
{
     
     id = target.GetLighting().UpdatePointLightData(
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
void PointLight::OnCreatedBySceneLoader() {

     SetDefaultTarget(
          &Engine::Instance().renderSystem
     );
     id=-1;

     UpdateLightData(
          Engine::Instance().renderSystem
     );
     if (m_TransformSubscription == -1)
          return;
     if (!gameObject)
          return;

     if (!gameObject->transform)
          return;

     m_TransformSubscription =
          gameObject->transform->OnChange.Subscribe(
               [this](const Transform& transform)
               {
                    OnTransformChanged(transform);
               }
          );
}
PointLight::~PointLight() {
     m_target->GetLighting().RemovePointLight(id);
}
void PointLight::OnAdded(GameObject* owner)
{
     if(m_TransformSubscription==-1)
          return;
     if (!owner)
          return;

     if (!owner->transform)
          return;

     m_owner = owner;

     m_TransformSubscription =
          owner->transform->OnChange.Subscribe(
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
void Scene::TryRemoveRenderer(Component* component) {
     if (Renderer* renderer = dynamic_cast<Renderer*>(component))
     {
          Engine::Instance().renderSystem.RemoveRenderer(renderer);
     }
}

void Scene::TryAddRenderer(Component* component) {
     if (Renderer* renderer = dynamic_cast<Renderer*>(component))
     {
          Engine::Instance().renderSystem.RegisterRenderer(renderer);
     }
}

void EngineObject::DestroyGameObject(GameObject* target) {
     target->waitingToDestroy = true;
     Engine::Instance().waitForFrameUpdateQueues.AddToDestoryQueue(target);
}

void EngineObject::DestroyComponent(Component* target)
     {
     
     target->waitingToDestroy = true;
     Engine::Instance().waitForFrameUpdateQueues.AddToDestoryComponentQueue(target);
}


void WaitForFrameUpdateQueues::DestoryAllInDestoryStack() {
     for (auto& gm : destoryStack)
     {
          if (!gm)
               continue;
          int id = gm->ReferenceInfo.ObjectID;
          Engine::Instance().currentScene->InternalUnregisterDeleteGameObjectImmediate(id);
          destoryStack.clear();
     }
}

void WaitForFrameUpdateQueues::DestoryAllInDestoryComponentQueue() {
     for (auto& compo : destoryComponentStack)
     {
          if (!compo)
               continue;
          int id = compo->ReferenceInfo.ObjectID;
          bool success=Engine::Instance().currentScene->InternalUnregisterAndDeleteComponentImmediate(id);
          destoryComponentStack.clear();
     }
}