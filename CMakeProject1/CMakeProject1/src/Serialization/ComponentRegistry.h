#pragma once
#include <string>
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
     struct ComponentInfo
     {
          TypeId typeId = 0;

          std::string name;

          CreateFn createFn;
     };

     const std::unordered_map<
          TypeId,
          ComponentInfo
     >& GetAll() const
     {
          return m_Factories;
     }
   

     static ComponentRegistry& Instance()
     {
          static ComponentRegistry instance;
          return instance;
     }

     ComponentRegistry(const ComponentRegistry&) = delete;
     ComponentRegistry& operator=(const ComponentRegistry&) = delete;

     void Register(
        TypeId typeId,
        std::string name,
        CreateFn createFn)
    {
          m_Factories[typeId] =
            ComponentInfo{
                typeId,
                std::move(name),
                std::move(createFn)
            };
    }

     Component* Create(TypeId typeId, GameObject& gameObject) const
     {
          auto it = m_Factories.find(typeId);

          if (it == m_Factories.end())
               return nullptr;

          return it->second.createFn(gameObject);
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

     std::unordered_map<TypeId, ComponentInfo> m_Factories;
};


template<typename T>
class ComponentAutoRegister
{
public:
     ComponentAutoRegister(
          TypeId typeId,
          std::string name)
     {
          ComponentRegistry::Instance()
               .Register(
                    typeId,
                    std::move(name),

                    [](GameObject& gameObject)
                    -> Component*
                    {
                         return CreateComponent<T>(
                              gameObject
                         );
                    }
               );
     }
};


#define REGISTER_COMPONENT(ClassName) \
    inline ComponentAutoRegister<ClassName> \
        g_ComponentAutoRegister_##ClassName( \
            GetTypeId(#ClassName), \
            #ClassName \
        );