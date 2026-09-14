#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "Serialization/ComponentRegistry.h"
#include "EngineObject.h"
#include "Event.h"
class GameObject;
class Transform;
class Renderer;


// ============================================================
// Component
// ============================================================

class Component : public EngineObject
{
public:

     virtual ~Component() = default;

     GameObject* gameObject = nullptr;

     virtual void OnAdded(GameObject* owner) {}

     virtual void OnRemoved(GameObject* owner) {}
};
REFLECT_BASE(
     Component,
     EngineObject
)

// ============================================================
// GameObject
// ============================================================

class GameObject : public EngineObject
{
public:

     GameObject();

     ~GameObject() = default;

     GameObject(const GameObject&) = delete;

     GameObject& operator=(const GameObject&) = delete;

     GameObject(GameObject&&) noexcept = default;

     GameObject& operator=(GameObject&&) noexcept = default;


     // ============================================================
     // Component Added Event
     // ============================================================

     Event<Component*> OnComponentAdded;


     // ============================================================
     // Add Component
     // ============================================================

     template<typename T, typename... Args>
     T* AddComponent(Args&&... args)
     {
          auto component =
               std::make_unique<T>(
                    std::forward<Args>(args)...
               );

          component->gameObject =
               this;

          T* ptr =
               component.get();

          m_Components.push_back(
               std::move(component)
          );

          if constexpr (
               std::is_same_v<T, Transform>
               )
          {
               transform =
                    ptr;
          }

          if constexpr (
               std::is_same_v<T, Renderer>
               )
          {
               renderer =
                    ptr;
          }

          ptr->OnAdded(
               this
          );


          // =====================================================
          // Notify Component Added
          // =====================================================

          OnComponentAdded.Invoke(
               ptr
          );


          return ptr;
     }


     std::vector<
          std::unique_ptr<Component>
     >& GetComponents()
     {
          return m_Components;
     }


     const std::vector<
          std::unique_ptr<Component>
     >& GetComponents() const
     {
          return m_Components;
     }


public:

     std::string name;

     Transform* transform =
          nullptr;

     Renderer* renderer =
          nullptr;


private:

     std::vector<
          std::unique_ptr<Component>
     > m_Components;

     REFLECT_FRIEND(GameObject);
};


REFLECT_BASE(
     GameObject,
     EngineObject
)


     template<typename T>
Component* CreateComponent(GameObject& gameObject)
{
     return gameObject.AddComponent<T>();
}