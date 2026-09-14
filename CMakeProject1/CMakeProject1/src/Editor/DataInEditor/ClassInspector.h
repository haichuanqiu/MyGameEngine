#pragma once

#include <any>
#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <vector>

#include "imgui.h"

#include "Serialization/Reflection.h"
#include "EngineObject.h"


// ============================================================
// 自定义 Inspector 工厂
//
// 支持：
//
// 1. 精确类型 Override
//
//    Register<Material>()
//    Material 对象优先使用 Material Inspector
//
// 2. 自动继承匹配
//
//    Register<Asset>()
//
//    Material : Asset
//    Texture  : Asset
//
//    即使没有 Register<Material>()，
//    也会通过 dynamic_cast 自动匹配到 Asset Inspector
//
// 不需要手动 RegisterInheritance。
// ============================================================

class CustomInspectorFactory
{
private:

     struct InspectorEntry
     {
          // 注册的 Inspector 类型
          std::type_index type;


          // 当前 EngineObject 是否可以转换成该类型
          std::function<bool(EngineObject*)>
               canDraw;


          // 实际绘制
          std::function<void(EngineObject*)>
               draw;
     };


public:

     static CustomInspectorFactory& Instance()
     {
          static CustomInspectorFactory inst;

          return inst;
     }


     // ============================================================
     // Register
     // ============================================================

     template<typename T>
     void Register(
          std::function<void(T&)> drawFn)
     {
          static_assert(
               std::is_base_of_v<EngineObject, T>,
               "Custom Inspector type must derive from EngineObject."
               );


          InspectorEntry newEntry
          {
               // ================================================
               // 注册类型
               // ================================================

               std::type_index(typeid(T)),


               // ================================================
               // 自动判断继承关系
               //
               // 例如：
               //
               // EngineObject* object -> Material
               //
               // dynamic_cast<Asset*>(object)
               //
               // Material : Asset
               //
               // 所以成功。
               // ================================================

               [](EngineObject* object) -> bool
               {
                    if (!object)
                         return false;

                    return
                         dynamic_cast<T*>(
                              object
                         ) != nullptr;
               },


               // ================================================
               // 绘制
               // ================================================

               [drawFn](EngineObject* object)
               {
                    if (!object)
                         return;


                    T* castedObject =
                         dynamic_cast<T*>(
                              object
                         );


                    if (!castedObject)
                         return;


                    drawFn(
                         *castedObject
                    );
               }
          };


          // ======================================================
          // 如果这个类型已经注册过
          // 直接覆盖旧 Inspector
          // ======================================================

          for (auto& entry :
               m_entries)
          {
               if (
                    entry.type ==
                    std::type_index(typeid(T))
                    )
               {
                    entry =
                         std::move(newEntry);

                    return;
               }
          }


          // ======================================================
          // 新注册
          // ======================================================

          m_entries.push_back(
               std::move(newEntry)
          );
     }


     // ============================================================
     // CreateAndDraw
     // ============================================================

     bool CreateAndDraw(
          EngineObject* object)
     {
          if (!object)
               return false;


          // ======================================================
          // 获取真正的运行时类型
          // ======================================================

          const std::type_index runtimeType(
               typeid(*object)
          );


          // ======================================================
          // 第一阶段：
          //
          // 精确类型匹配
          //
          // 比如：
          //
          // Material
          //
          // Register<Material>()
          //
          // 那么一定优先使用 Material Inspector。
          // ======================================================

          for (auto& entry :
               m_entries)
          {
               if (
                    entry.type ==
                    runtimeType
                    )
               {
                    entry.draw(
                         object
                    );

                    return true;
               }
          }


          // ======================================================
          // 第二阶段：
          //
          // 自动继承匹配
          //
          // 比如：
          //
          // Material : Asset
          //
          // 没有 Register<Material>()
          //
          // 但是有：
          //
          // Register<Asset>()
          //
          // dynamic_cast<Asset*>(Material)
          //
          // 会成功。
          // ======================================================

          for (auto& entry :
               m_entries)
          {
               if (
                    entry.canDraw(
                         object
                    )
                    )
               {
                    entry.draw(
                         object
                    );

                    return true;
               }
          }


          // ======================================================
          // 没有任何 Custom Inspector
          // ======================================================

          return false;
     }


private:

     std::vector<
          InspectorEntry
     > m_entries;
};


// ============================================================
// 默认反射绘制
// ============================================================

inline void DrawPropertyNode(
     const PropertyNode& node,
     int indent)
{
     ImGui::PushID(
          node.name.c_str()
     );


     ImGui::Indent(
          indent * 16.0f
     );


     switch (node.type)
     {
     case FieldType::Int:
     {
          int value =
               std::any_cast<int>(
                    node.value
               );


          if (
               ImGui::DragInt(
                    node.name.c_str(),
                    &value
               )
               )
          {
               node.set(
                    value
               );
          }

          break;
     }
     case FieldType::Reference:
     {
          // ========================================================
          // Null Reference
          // ========================================================

          if (node.reference.isNull)
          {
               ImGui::Text(
                    "%s : Reference [null]",
                    node.name.c_str()
               );

               break;
          }


          // ========================================================
          // Reference Header
          // ========================================================

          ImGui::Text(
               "%s : Reference",
               node.name.c_str()
          );


          // ========================================================
          // Reference Information
          // ========================================================

          ImGui::Indent(
               16.0f
          );


          ImGui::Text(
               "Scope Index: %d",
               node.reference.ScopeLevel
          );


          ImGui::Text(
               "Scope ID: %d",
               node.reference.ScopeID
          );


          ImGui::Text(
               "Object ID: %d",
               node.reference.ObjectID
          );


          ImGui::Unindent(
               16.0f
          );


          break;
     }

     case FieldType::Float:
     {
          float value =
               std::any_cast<float>(
                    node.value
               );


          if (
               ImGui::DragFloat(
                    node.name.c_str(),
                    &value,
                    0.01f
               )
               )
          {
               node.set(
                    value
               );
          }

          break;
     }


     case FieldType::Bool:
     {
          bool value =
               std::any_cast<bool>(
                    node.value
               );


          if (
               ImGui::Checkbox(
                    node.name.c_str(),
                    &value
               )
               )
          {
               node.set(
                    value
               );
          }

          break;
     }


     case FieldType::String:
     {
          std::string value =
               std::any_cast<std::string>(
                    node.value
               );


          char buffer[1024];


          std::snprintf(
               buffer,
               sizeof(buffer),
               "%s",
               value.c_str()
          );


          if (
               ImGui::InputText(
                    node.name.c_str(),
                    buffer,
                    sizeof(buffer)
               )
               )
          {
               node.set(
                    std::string(
                         buffer
                    )
               );
          }

          break;
     }


     case FieldType::Struct:
     case FieldType::Vector:
     {
          ImGui::Text(
               "%s : %s",
               node.name.c_str(),
               node.typeName.c_str()
          );


          for (
               const auto& child :
               node.children
               )
          {
               DrawPropertyNode(
                    child,
                    indent + 1
               );
          }

          break;
     }


     default:
          break;
     }


     ImGui::Unindent(
          indent * 16.0f
     );


     ImGui::PopID();
}


// ============================================================
// 默认 Inspector
//
// 这个函数会跳过 CustomInspectorFactory，
// 直接走 Reflection。
//
// 很适合这种情况：
//
// Register<Asset>(
//      [](Asset& asset)
//      {
//           DrawDefaultInspector(
//                "Asset",
//                &asset
//           );
//
//           ImGui::Button("Save");
//      }
// );
//
// 这样不会递归调用 Asset Custom Inspector。
// ============================================================

inline void DrawDefaultInspector(
     const char* name,
     EngineObject* object)
{
     if (!object)
          return;


     // ======================================================
     // 获取对象真实运行时类型
     //
     // 即使传进来的是 Asset*
     //
     // 真实对象如果是 Material，
     // typeid(*object) 仍然是 Material。
     // ======================================================

     const std::type_info& typeInfo =
          typeid(*object);


     const TypeInfo* type =
          ReflectionRegistry::Instance().Find(
               typeInfo
          );


     if (!type)
     {
          ImGui::Text(
               "No Reflection Data: %s",
               typeInfo.name()
          );

          return;
     }


     // ======================================================
     // Inspector 标题
     // ======================================================

     ImGui::Text(
          "%s",
          type->name
     );


     // ======================================================
     // 创建 PropertyNode
     // ======================================================

     PropertyNode node;


     node.name =
          name;


     node.type =
          FieldType::Struct;


     node.typeName =
          type->name;


     node.children.clear();


     // ======================================================
     // Reflection 构建字段
     //
     // BuildTypeFields 如果你的 Reflection 已经处理继承，
     // 那么这里会自动得到：
     //
     // Asset Fields
     // +
     // Material Fields
     // ======================================================

     BuildTypeFields(
          node,
          type,
          object
     );


     // ======================================================
     // 绘制
     // ======================================================

     for (
          const auto& child :
          node.children
          )
     {
          DrawPropertyNode(
               child,
               0
          );
     }
}


// ============================================================
// ClassInspector
// ============================================================

class ClassInspector
{
public:

     ClassInspector(
          const char* name,
          EngineObject* object)
          :
          m_name(name),
          m_object(object)
     {
          if (m_object)
          {
               m_typeInfo =
                    &typeid(*m_object);
          }
     }


     void Draw();


private:

     std::string m_name;


     EngineObject* m_object =
          nullptr;


     const std::type_info* m_typeInfo =
          nullptr;


     PropertyNode m_Node;
};


// ============================================================
// ClassInspector::Draw
// ============================================================

inline void ClassInspector::Draw()
{
     if (
          !m_object ||
          !m_typeInfo
          )
     {
          return;
     }


     // ============================================================
     // 自定义 Inspector
     //
     // Factory 内部会：
     //
     // 1. 先找完全匹配
     //
     //    Material -> Material Inspector
     //
     // 2. 再自动通过 dynamic_cast 找父类 Inspector
     //
     //    Material -> Asset Inspector
     //
     // ============================================================

     if (
          CustomInspectorFactory::Instance()
          .CreateAndDraw(
               m_object
          )
          )
     {
          return;
     }


     // ============================================================
     // 没有 Custom Inspector
     //
     // 使用默认 Reflection Inspector
     // ============================================================

     const TypeInfo* type =
          ReflectionRegistry::Instance().Find(
               *m_typeInfo
          );


     if (!type)
     {
          ImGui::Text(
               "No Reflection Data: %s",
               m_typeInfo->name()
          );

          return;
     }


     // ============================================================
     // Inspector 标题
     // ============================================================

     ImGui::Text(
          "%s",
          type->name
     );


     // ============================================================
     // PropertyNode
     // ============================================================

     m_Node.name =
          m_name;


     m_Node.type =
          FieldType::Struct;


     m_Node.typeName =
          type->name;


     m_Node.children.clear();


     // ============================================================
     // 生成所有 Reflection Fields
     //
     // 如果 Reflection 支持继承：
     //
     // Base
     //   ↓
     // Derived
     //
     // 会自动全部生成。
     // ============================================================

     BuildTypeFields(
          m_Node,
          type,
          m_object
     );


     // ============================================================
     // 绘制
     // ============================================================

     for (
          const auto& child :
          m_Node.children
          )
     {
          DrawPropertyNode(
               child,
               0
          );
     }
}