#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "GameObjectSystem.h"


class Scene
{
public:

     // ============================================================
     // Scene
     // ============================================================

     Scene() = default;


     // ============================================================
     // Scene Index
     // ============================================================

     int sceneIndex = 0;


     // ============================================================
     // Add GameObject
     // ============================================================

     GameObject& AddGameObject()
     {
          auto gameObject = std::make_unique<GameObject>();

          GameObject* ptr = gameObject.get();

          RegisterGameObject(ptr);

          m_GameObjects.push_back(std::move(gameObject));

          return *ptr;
     }


     // ============================================================
     // Add Existing GameObject
     // ============================================================

     void AddGameObject(std::unique_ptr<GameObject> gameObject)
     {
          if (!gameObject)
               return;

          RegisterGameObject(gameObject.get());

          m_GameObjects.push_back(std::move(gameObject));
     }


     // ============================================================
     // Get GameObject By Vector Index
     // ============================================================

     GameObject& GetGameObject(size_t index)
     {
          return *m_GameObjects.at(index);
     }


     const GameObject& GetGameObject(size_t index) const
     {
          return *m_GameObjects.at(index);
     }


     // ============================================================
     // Get All GameObjects
     // ============================================================

     const std::vector<std::unique_ptr<GameObject>>& GetAllGameObjects() const
     {
          return m_GameObjects;
     }


     // ============================================================
     // Get Object Of Type
     //
     // ObjectID -> EngineObject -> T
     // ============================================================

     template<typename T>
     T* GetObjectOfType(int objectID)
     {
          auto it = m_Objects.find(objectID);

          if (it == m_Objects.end())
               return nullptr;

          return dynamic_cast<T*>(it->second);
     }


     // ============================================================
     // Get Object
     // ============================================================

     EngineObject* getObject(int objectID)
     {
          auto it = m_Objects.find(objectID);

          if (it == m_Objects.end())
               return nullptr;

          return it->second;
     }


     // ============================================================
     // Register GameObject
     //
     // GameObject 和 Component 使用同一个 ObjectID Pool
     // ============================================================

     uint64_t RegisterGameObject(GameObject* gameObject)
     {
          if (!gameObject)
               return 0;


          // =====================================================
          // GameObject Reference Info
          // =====================================================

          gameObject->ReferenceInfo.ScopeLevel = 0;
          gameObject->ReferenceInfo.ScopeID = sceneIndex;
          gameObject->ReferenceInfo.ObjectID = m_NextObjectID++;

          uint64_t objectID = gameObject->ReferenceInfo.ObjectID;

          m_Objects[objectID] = gameObject;


          // =====================================================
          // Register Existing Components
          //
          // 包括 GameObject 构造时已经存在的 Transform
          // =====================================================

          for (auto& component : gameObject->GetComponents())
          {
               OnComponentAdded(component.get());
          }


          // =====================================================
          // Listen For Future Components
          // =====================================================

          gameObject->OnComponentAdded.Subscribe(
               [this](Component* component)
               {
                    OnComponentAdded(component);
               }
          );


          return objectID;
     }


     // ============================================================
     // Component Added
     // ============================================================

     uint64_t OnComponentAdded(Component* component)
     {
          if (!component)
               return 0;


          // =====================================================
          // Component Reference Info
          // =====================================================

          component->ReferenceInfo.ScopeLevel = 0;
          component->ReferenceInfo.ScopeID = sceneIndex;
          component->ReferenceInfo.ObjectID = m_NextObjectID++;

          uint64_t objectID = component->ReferenceInfo.ObjectID;

          m_Objects[objectID] = component;

          return objectID;
     }


private:

     // ============================================================
     // GameObject Ownership
     // ============================================================

     std::vector<std::unique_ptr<GameObject>> m_GameObjects;


     // ============================================================
     // Object Lookup
     //
     // GameObject 和 Component 全部在这里
     // ============================================================

     std::unordered_map<int, EngineObject*> m_Objects;


     // ============================================================
     // Object ID Pool
     // ============================================================

     int m_NextObjectID = 0;
};