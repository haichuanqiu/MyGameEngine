#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <iostream>
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

     virtual void Update(){}
     virtual  void OnCreatedBySceneLoader(){ };
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

     ~GameObject()
     {
          InternalClearComponentsImmediate();
          OnComponentAdded.UnsubscribeAll();
     }

     GameObject(const GameObject&) = delete;

     GameObject& operator=(const GameObject&) = delete;

     GameObject(GameObject&&) noexcept = default;

     GameObject& operator=(GameObject&&) noexcept = default;


     // ============================================================
     // Component Added Event
     // ============================================================

     Event<Component*> OnComponentAdded;
     Event<ReferenceDescription> OnComponentRemoved;

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
     bool enabled=true;
     bool InternalRemoveComponentImmediate(int objectID)
     {
          auto it =
               std::find_if(
                    m_Components.begin(),
                    m_Components.end(),
                    [objectID](
                         const std::unique_ptr<Component>& component
                         )
                    {
                         return
                              component.get()->ReferenceInfo.ObjectID ==
                              objectID;
                    }
               );


          // ============================================================
          // Not Found
          // ============================================================

          if (
               it ==
               m_Components.end()
               )
          {
               return false;
          }


          // ============================================================
          // 现在才能安全解引用
          // ============================================================

          Component* component =
               it->get();

          OnComponentRemoved.Invoke(
               component->ReferenceInfo
          );

          component->OnRemoved(
               this
          );

          component->gameObject =
               nullptr;


          m_Components.erase(
               it
          );

          return true;
     }

     void InternalClearComponentsImmediate()
     {
          for (auto& component : m_Components)
          {
               if (!component)
                    continue;

               component->OnRemoved(this);

               component->gameObject = nullptr;
          }
          transform = nullptr;
          renderer = nullptr;

          m_Components.clear();
     }

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