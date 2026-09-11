#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <utility>
#include "Serialization/Reflection.h"
class Component;
class GameObject;

using TypeId = uint64_t;


// 这里只声明
template<typename T>
Component* CreateComponent(GameObject& gameObject);


class ComponentRegistry
{
public:
     using CreateFn = std::function<Component* (GameObject&)>;

     static ComponentRegistry& Instance()
     {
          static ComponentRegistry instance;
          return instance;
     }

     ComponentRegistry(const ComponentRegistry&) = delete;
     ComponentRegistry& operator=(const ComponentRegistry&) = delete;

     void Register(TypeId typeId, CreateFn createFn)
     {
          m_Factories[typeId] = std::move(createFn);
     }

     Component* Create(TypeId typeId, GameObject& gameObject) const
     {
          auto it = m_Factories.find(typeId);

          if (it == m_Factories.end())
               return nullptr;

          return it->second(gameObject);
     }

     bool IsRegistered(TypeId typeId) const
     {
          return m_Factories.contains(typeId);
     }

     void Unregister(TypeId typeId)
     {
          m_Factories.erase(typeId);
     }

     void Clear()
     {
          m_Factories.clear();
     }

private:
     ComponentRegistry() = default;
     ~ComponentRegistry() = default;

     std::unordered_map<TypeId, CreateFn> m_Factories;
};


template<typename T>
class ComponentAutoRegister
{
public:
     explicit ComponentAutoRegister(TypeId typeId)
     {
          ComponentRegistry::Instance().Register(
               typeId,
               [](GameObject& gameObject) -> Component*
               {
                    return CreateComponent<T>(gameObject);
               }
          );
     }
};


#define REGISTER_COMPONENT(ClassName) \
    inline ComponentAutoRegister<ClassName> \
        g_ComponentAutoRegister_##ClassName(GetTypeId(#ClassName));