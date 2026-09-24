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
     std::string filePath;

     // ============================================================
     // Scene Index
     // ============================================================

     int sceneIndex = 0;


     // ============================================================
     // Add GameObject
     // ============================================================
    void ClearScene()
{

    m_Objects.clear();

    m_GameObjects.clear();

    m_NextObjectID = 1;
}
     GameObject& AddGameObject()
     {
          auto gameObject = std::make_unique<GameObject>();

          GameObject* ptr = gameObject.get();

          RegisterGameObject(ptr);

          m_GameObjects.push_back(std::move(gameObject));

          return *ptr;
     }

     void AddGameObject(
          std::unique_ptr<GameObject> gameObject,
          int scopeLevel,
          int scopeID,
          int objectID)
     {
          if (!gameObject)
          {
               std::cerr
                    << "[Scene Error] AddGameObject: gameObject is nullptr"
                    << std::endl;

               return;
          }
          if (m_NextObjectID < objectID) {
               m_NextObjectID = objectID;
          }

          // ============================================================
          // Check Duplicate ObjectID
          // ============================================================

          auto it = m_Objects.find(objectID);

          if (it != m_Objects.end())
          {
               std::cerr
                    << "[Scene Error] Duplicate ObjectID: "
                    << objectID
                    << ", old object = "
                    << static_cast<void*>(it->second)
                    << ", new GameObject = "
                    << static_cast<void*>(gameObject.get())
                    << ". Old mapping will be overwritten."
                    << std::endl;

               m_Objects.erase(it);
          }


          // ============================================================
          // Set ReferenceInfo
          // ============================================================

          gameObject->ReferenceInfo.ScopeLevel = scopeLevel;
          gameObject->ReferenceInfo.ScopeID = scopeID;
          gameObject->ReferenceInfo.ObjectID = objectID;


          // ============================================================
          // Register
          // ============================================================

          GameObject* ptr = gameObject.get();

          m_Objects[objectID] = ptr;

          std::cout
               << "Register"
               << gameObject.get()->name
               << std::endl;
          RegisterFutureComponent(gameObject.get());

          m_GameObjects.push_back(
               std::move(gameObject)
          );
   
     }

     void AddComponent(
          Component* component,
          int scopeLevel,
          int scopeID,
          int objectID)
     {
          if (!component)
          {
               std::cerr
                    << "[Scene Error] AddComponent: component is nullptr"
                    << std::endl;

               return;
          }

          if (m_NextObjectID < objectID) {
               m_NextObjectID = objectID;
          }
          // ============================================================
          // Check Duplicate ObjectID
          // ============================================================

          auto it = m_Objects.find(objectID);

          if (it != m_Objects.end())
          {
               std::cerr
                    << "[Scene Error] Duplicate ObjectID: "
                    << objectID
                    << ", old object = "
                    << static_cast<void*>(it->second)
                    << ", new Component = "
                    << static_cast<void*>(component)
                    << ". Old mapping will be overwritten."
                    << std::endl;

               m_Objects.erase(it);
          }


          // ============================================================
          // Set ReferenceInfo
          // ============================================================

          component->ReferenceInfo.ScopeLevel = scopeLevel;
          component->ReferenceInfo.ScopeID = scopeID;
          component->ReferenceInfo.ObjectID = objectID;


          // ============================================================
          // Register
          // ============================================================

          m_Objects[objectID] = component;
          TryAddRenderer(component);
     }

     void TryAddRenderer(Component* component);
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
          RegisterFutureComponent(gameObject);
         

          return objectID;
     }
     void RegisterFutureComponent(GameObject* gameObject) {
          
          gameObject->OnComponentAdded.Subscribe(
               [this](Component* component)
               {
                    OnComponentAdded(component);
     
               }
          );

     }

     // ============================================================
     // Component Added
     // ============================================================

     uint64_t OnComponentAdded(Component* component) {
          if (!component)
               return 0;




          component->ReferenceInfo.ScopeLevel = 0;
          component->ReferenceInfo.ScopeID = sceneIndex;
          component->ReferenceInfo.ObjectID = m_NextObjectID++;

          uint64_t objectID = component->ReferenceInfo.ObjectID;

          m_Objects[objectID] = component;

          std::cout
               << "Register Component"
               << component->ReferenceInfo.ScopeLevel
               << component->ReferenceInfo.ScopeID 

               << component->ReferenceInfo.ObjectID

               << std::endl;
         TryAddRenderer(component);

          return objectID;
     }
    

     template<typename T>
     std::vector<T*> FindAllOfType()
     {
          std::vector<T*> result;

          for (auto& [objectID, object] : m_Objects)
          {
               if (!object)
                    continue;

               T* typedObject =
                    dynamic_cast<T*>(object);

               if (typedObject)
               {
                    result.push_back(
                         typedObject
                    );
               }
          }

          return result;
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