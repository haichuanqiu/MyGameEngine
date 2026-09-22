#pragma once

#include <cstdint>
#include <functional>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "Serialization/Reflection.h"


// ============================================================
// Class Registry
//
// 用于通过 TypeId 动态创建任意注册过的 Class。
//
// 例如：
//
// REGISTER_CLASS(Material)
//
// TypeId typeId = GetTypeId("Material");
//
// void* object =
//      ClassRegistry::Instance().Create(typeId);
//
// Material* material =
//      static_cast<Material*>(object);
//
// delete material;
//
// 或者：
//
// Material* material =
//      ClassRegistry::Instance()
//      .Create<Material>(
//           GetTypeId("Material")
//      );
//
// delete material;
//
// ============================================================

class ClassRegistry
{
public:

     // ========================================================
     // Create Function
     //
     // 实际保存：
     //
     // []() -> void*
     // {
     //      return new Material();
     // }
     //
     // ========================================================

     using CreateFn =
          std::function<void* ()>;


     // ========================================================
     // Instance
     // ========================================================

     static ClassRegistry& Instance()
     {
          static ClassRegistry instance;

          return instance;
     }


     // ========================================================
     // Delete
     // ========================================================

     ClassRegistry(
          const ClassRegistry&) = delete;


     ClassRegistry& operator=(
          const ClassRegistry&) = delete;


     // ========================================================
     // Register
     // ========================================================

     template<typename T>
     void Register(
          TypeId typeId)
     {
          static_assert(
               std::is_default_constructible_v<T>,
               "Registered class must be default constructible"
               );


          m_Factories[typeId] =
               []() -> void*
               {
                    return new T();
               };
     }

     void* Create(
          TypeId typeId) const
     {
          auto it =
               m_Factories.find(
                    typeId
               );


          if (it ==
               m_Factories.end())
          {
               return nullptr;
          }


          return it->second();
     }


     // ========================================================
     // Create<T>
     //
     // 已经知道目标类型的时候使用。
     //
     // Material* material =
     //      Create<Material>(typeId);
     //
     // ========================================================

     template<typename T>
     T* Create(
          TypeId typeId) const
     {
          void* object =
               Create(typeId);


          if (!object)
               return nullptr;


          return static_cast<T*>(
               object
               );
     }


     // ========================================================
     // Is Registered
     // ========================================================

     bool IsRegistered(
          TypeId typeId) const
     {
          return m_Factories.contains(
               typeId
          );
     }


     // ========================================================
     // Unregister
     // ========================================================

     void Unregister(
          TypeId typeId)
     {
          m_Factories.erase(
               typeId
          );
     }


     // ========================================================
     // Clear
     // ========================================================

     void Clear()
     {
          m_Factories.clear();
     }


private:

     // ========================================================
     // Constructor
     // ========================================================

     ClassRegistry() = default;


     // ========================================================
     // Destructor
     // ========================================================

     ~ClassRegistry() = default;


     // ========================================================
     // Factories
     // ========================================================

     std::unordered_map<
          TypeId,
          CreateFn
     > m_Factories;
};


// ============================================================
// Class Auto Register
//
// 程序启动时自动把 T 注册进 ClassRegistry。
// ============================================================

template<typename T>
class ClassAutoRegister
{
public:

     explicit ClassAutoRegister(
          TypeId typeId)
     {
          ClassRegistry::Instance()
               .Register<T>(
                    typeId
               );
     }
};


// ============================================================
// REGISTER_CLASS
//
// 例如：
//
// REGISTER_CLASS(Material)
//
// 等价于注册：
//
// GetTypeId("Material")
//      ->
// []()
// {
//      return new Material();
// }
//
// ============================================================

#define REGISTER_CLASS(ClassName)                           \
                                                            \
     inline ClassAutoRegister<ClassName>                    \
          g_ClassAutoRegister_##ClassName(                  \
               GetTypeId(#ClassName)                        \
          );