#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "GameObjectSystem.h"

class Scene
{
public:

     Scene() = default;
     std::string filePath;
     int sceneIndex = 0;
     GameObject& AddGameObject()
     {
          auto gameObject = std::make_unique<GameObject>();

          GameObject* ptr = gameObject.get();

          RegisterGameObject(ptr);

          m_GameObjects.push_back(std::move(gameObject));

          return *ptr;
     }


public:

     const std::vector<std::unique_ptr<GameObject>>& GetAllGameObjects() const
     {
          return m_GameObjects;
     }



     EngineObject* getObject(int objectID)
     {
          auto it = m_Objects.find(objectID);

          if (it == m_Objects.end())
               return nullptr;
          if(it->second->waitingToDestroy){
               return nullptr;
          }
          return it->second;
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
public:
     // ============================================================
     // internal delete
     // ============================================================
     void InternalClearSceneImmediate()
     {

          m_Objects.clear();

          m_GameObjects.clear();

          m_NextObjectID = 1;
     }
     bool InternalUnregisterAndDeleteComponentImmediate(uint64_t objectID)
     {

          auto it = m_Objects.find(objectID);

          if (it == m_Objects.end())
          {
               return false;
          }
          Component* component = dynamic_cast<Component*>(it->second);


          if (!component)
          {
               std::cout
                    << "not component"
                    << std::endl;
               return false;
          }

          GameObject* gameObject =
               component->gameObject;
          TryRemoveRenderer(
               component
          );
          bool success = gameObject->InternalRemoveComponentImmediate(objectID);
          if (!success) {

               return false;
          }
          m_Objects.erase(it);
          return true;
     }
     bool InternalUnregisterDeleteGameObjectImmediate(int objectID)
     {
          auto it = std::find_if(
               m_GameObjects.begin(),
               m_GameObjects.end(),

               [objectID](
                    const std::unique_ptr<GameObject>& gameObject)
               {
                    return gameObject &&
                         gameObject->ReferenceInfo.ObjectID
                         == objectID;
               }
          );


          if (it == m_GameObjects.end())
               return false;


          GameObject* gameObject =
               it->get();
          for (const auto& component :
               gameObject->GetComponents())
          {
               if (!component)
                    continue;


               TryRemoveRenderer(
                    component.get()
               );


               m_Objects.erase(
                    component->ReferenceInfo.ObjectID
               );
          }

          m_Objects.erase(
               gameObject->ReferenceInfo.ObjectID
          );
          m_GameObjects.erase(it);


          return true;
     }

public:
       // ============================================================
       // for scene serialization
       // ============================================================

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

          if (m_NextObjectID < objectID + 1) {
               m_NextObjectID = objectID + 1;
          }

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

          if (m_NextObjectID < objectID+1) {
               m_NextObjectID = objectID + 1;
          }

          // ============================================================
          // Register
          // ============================================================

          m_Objects[objectID] = component;
          TryAddRenderer(component);
     }



private:
     void TryAddRenderer(Component* component);
     void TryRemoveRenderer(Component* component);
     uint64_t OnComponentAdded(Component* component) {
          if (!component)
               return 0;




          component->ReferenceInfo.ScopeLevel = 0;
          component->ReferenceInfo.ScopeID = sceneIndex;
          component->ReferenceInfo.ObjectID = m_NextObjectID++;

          uint64_t objectID = component->ReferenceInfo.ObjectID;

          m_Objects[objectID] = component;

          TryAddRenderer(component);

          return objectID;
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

     template<typename T>
     T* GetObjectOfType(int objectID)
     {
          auto it = m_Objects.find(objectID);

          if (it == m_Objects.end())
               return nullptr;

          return dynamic_cast<T*>(it->second);
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
     // GameObject Ownership
     // ============================================================

     std::vector<std::unique_ptr<GameObject>> m_GameObjects;


     // ============================================================
     // Object Lookup
     //
     // GameObject 和 Component 全部在这里
     // ============================================================

     std::unordered_map<uint64_t, EngineObject*> m_Objects;


     // ============================================================
     // Object ID Pool
     // ============================================================

     int m_NextObjectID = 0;
};