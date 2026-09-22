#pragma once

#include <any>
#include <cstdio>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <vector>

#include "imgui.h"

#include "Serialization/Reflection.h"
#include "EngineObject.h"

#include "Assets/AssetManager.h"
#include "Assets/ReferenceResolver.h"

#include "Engine/GameObjectSystem.h"


// ============================================================
// Custom Inspector Factory
// ============================================================

class CustomInspectorFactory
{
private:

     struct InspectorEntry
     {
          std::type_index type;

          std::function<bool(EngineObject*)>
               canDraw;

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
               std::type_index(
                    typeid(T)
               ),

               [](EngineObject* object) -> bool
               {
                    if (!object)
                         return false;

                    return
                         dynamic_cast<T*>(
                              object
                         ) != nullptr;
               },

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
          // Override Existing
          // ======================================================

          for (auto& entry : m_entries)
          {
               if (
                    entry.type ==
                    std::type_index(
                         typeid(T)
                    )
                    )
               {
                    entry =
                         std::move(
                              newEntry
                         );

                    return;
               }
          }


          // ======================================================
          // New
          // ======================================================

          m_entries.push_back(
               std::move(
                    newEntry
               )
          );
     }


     // ============================================================
     // Create And Draw
     // ============================================================

     bool CreateAndDraw(
          EngineObject* object)
     {
          if (!object)
               return false;


          const std::type_index runtimeType(
               typeid(*object)
          );


          // ======================================================
          // Exact Match
          // ======================================================

          for (auto& entry : m_entries)
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
          // Inheritance Match
          // ======================================================

          for (auto& entry : m_entries)
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


          return false;
     }


private:

     std::vector<
          InspectorEntry
     > m_entries;
};


// ============================================================
// Object Display Name
//
// 只返回 Object 本身的显示名称。
// 不包含 ReferenceDescription。
// ============================================================

inline std::string GetObjectDisplayName(
     EngineObject* object)
{
     // ============================================================
     // Null
     // ============================================================

     if (!object)
          return "null";


     // ============================================================
     // Asset
     // ============================================================

     if (
          auto* asset =
          dynamic_cast<Asset*>(
               object
               )
          )
     {
          if (
               asset->filePath.empty()
               )
          {
               return "Unnamed Asset";
          }


          return
               std::filesystem::path(
                    asset->filePath
               )
               .filename()
               .string();
     }


     // ============================================================
     // GameObject
     // ============================================================

     if (
          auto* gameObject =
          dynamic_cast<GameObject*>(
               object
               )
          )
     {
          if (
               gameObject->name.empty()
               )
          {
               return "Unnamed GameObject";
          }


          return
               gameObject->name;
     }


     // ============================================================
     // Component
     // ============================================================

     if (
          auto* component =
          dynamic_cast<Component*>(
               object
               )
          )
     {
          if (
               !component->gameObject
               )
          {
               return "Orphan Component";
          }


          if (
               component
               ->gameObject
               ->name
               .empty()
               )
          {
               return "Unnamed GameObject";
          }


          return
               component
               ->gameObject
               ->name;
     }


     // ============================================================
     // Unknown EngineObject
     // ============================================================

     return "Unknown Object";
}


// ============================================================
// Object Reference Display Name
//
// Name (ScopeLevel, ScopeID, ObjectID)
//
// Example:
//
// Player (0, 0, 1)
// Material.AssetObject (1, 0, 5)
// ============================================================

inline std::string GetObjectReferenceDisplayName(
     EngineObject* object)
{
     if (!object)
          return "null";


     std::string name =
          GetObjectDisplayName(
               object
          );


     const ReferenceDescription& ref =
          object->ReferenceInfo;


     return
          name +
          " (" +
          std::to_string(
               ref.ScopeLevel
          ) +
          ", " +
          std::to_string(
               ref.ScopeID
          ) +
          ", " +
          std::to_string(
               ref.ObjectID
          ) +
          ")";
}


// ============================================================
// Resolve Current Reference
// ============================================================

inline EngineObject* GetReferenceObject(
     const PropertyNode& node)
{
     if (
          node.reference.isNull
          )
     {
          return nullptr;
     }


     ReferenceDescription ref;


     ref.ScopeLevel =
          node.reference.ScopeLevel;


     ref.ScopeID =
          node.reference.ScopeID;


     ref.ObjectID =
          node.reference.ObjectID;


     return
          ReferenceResolver::Instance()
          .GetItem(
               ref
          );
}


// ============================================================
// Reference Display Name
//
// Current Reference:
//
// Name (ScopeLevel, ScopeID, ObjectID)
//
// Missing:
//
// Missing Reference (ScopeLevel, ScopeID, ObjectID)
// ============================================================

inline std::string GetReferenceDisplayName(
     const PropertyNode& node)
{
     if (
          node.reference.isNull
          )
     {
          return "null";
     }


     EngineObject* object =
          GetReferenceObject(
               node
          );


     // ============================================================
     // Reference ID exists, but Resolve failed
     // ============================================================

     if (!object)
     {
          return
               "Missing Reference (" +
               std::to_string(
                    node.reference.ScopeLevel
               ) +
               ", " +
               std::to_string(
                    node.reference.ScopeID
               ) +
               ", " +
               std::to_string(
                    node.reference.ObjectID
               ) +
               ")";
     }


     return
          GetObjectReferenceDisplayName(
               object
          );
}


// ============================================================
// Draw Reference Selector
// ============================================================

inline void DrawReferenceSelector(
     const PropertyNode& node,
     EngineObject* owner)
{
     if (!owner)
          return;


     // ============================================================
     // Current Display Name
     // ============================================================

     std::string displayName =
          GetReferenceDisplayName(
               node
          );


     // ============================================================
     // Field Name
     // ============================================================

     ImGui::Text(
          "%s",
          node.name.c_str()
     );


     ImGui::SameLine();


     // ============================================================
     // Current Reference Button
     // ============================================================

     if (
          ImGui::Button(
               displayName.c_str()
          )
          )
     {
          ImGui::OpenPopup(
               "ReferenceSelectorPopup"
          );
     }


     // ============================================================
     // Popup
     // ============================================================

     if (
          ImGui::BeginPopup(
               "ReferenceSelectorPopup"
          )
          )
     {
          // ======================================================
          // Null
          // ======================================================

          if (
               ImGui::Selectable(
                    "null"
               )
               )
          {
               if (
                    node.setReference
                    )
               {
                    node.setReference(
                         nullptr
                    );
               }


               ImGui::CloseCurrentPopup();
          }


          ImGui::Separator();


          // ======================================================
          // Find All Scope-Compatible EngineObjects
          //
          // owner:
          //
          // 当前拥有这个 Field 的对象。
          //
          // ReferenceResolver 根据 owner ReferenceInfo
          // 找到所有合法 Scope 中的对象。
          // ======================================================

          std::vector<EngineObject*> candidates =
               ReferenceResolver::Instance()
               .FindAllOfType<EngineObject>(
                    owner->ReferenceInfo
               );


          bool foundCandidate =
               false;


          // ======================================================
          // Filter By REF_FIELD Runtime Type
          // ======================================================

          for (
               EngineObject* candidate :
               candidates
               )
          {
               if (!candidate)
                    continue;


               // =================================================
               // Reflection Generated Runtime Type Check
               // =================================================

               if (
                    !node.canSetReference
                    )
               {
                    continue;
               }


               if (
                    !node.canSetReference(
                         candidate
                    )
                    )
               {
                    continue;
               }


               foundCandidate =
                    true;


               // =================================================
               // Display Name
               //
               // Name (ScopeLevel, ScopeID, ObjectID)
               // =================================================

               std::string candidateName =
                    GetObjectReferenceDisplayName(
                         candidate
                    );


               // =================================================
               // Unique ImGui ID
               //
               // Visible:
               //
               // Player (0, 0, 5)
               //
               // Internal:
               //
               // ##ReferenceCandidate_0_0_5
               // =================================================

               std::string selectableName =
                    candidateName +
                    "##ReferenceCandidate_" +
                    std::to_string(
                         candidate
                         ->ReferenceInfo
                         .ScopeLevel
                    ) +
                    "_" +
                    std::to_string(
                         candidate
                         ->ReferenceInfo
                         .ScopeID
                    ) +
                    "_" +
                    std::to_string(
                         candidate
                         ->ReferenceInfo
                         .ObjectID
                    );


               // =================================================
               // Is Current
               // =================================================

               bool selected =
                    !node.reference.isNull &&
                    node.reference.ScopeLevel ==
                    candidate
                    ->ReferenceInfo
                    .ScopeLevel &&
                    node.reference.ScopeID ==
                    candidate
                    ->ReferenceInfo
                    .ScopeID &&
                    node.reference.ObjectID ==
                    candidate
                    ->ReferenceInfo
                    .ObjectID;


               // =================================================
               // Select
               // =================================================

               if (
                    ImGui::Selectable(
                         selectableName.c_str(),
                         selected
                    )
                    )
               {
                    // =============================================
                    // Safety Check
                    // =============================================

                    if (
                         node.canSetReference(
                              candidate
                         ) &&
                         node.setReference
                         )
                    {
                         node.setReference(
                              candidate
                         );
                    }


                    ImGui::CloseCurrentPopup();
               }
          }


          // ======================================================
          // No Candidate
          // ======================================================

          if (!foundCandidate)
          {
               ImGui::TextDisabled(
                    "No compatible objects"
               );
          }


          ImGui::EndPopup();
     }
}


// ============================================================
// Default Reflection Draw
//
// owner 用来决定 Reference Field 当前处于什么 Scope。
// ============================================================

inline void DrawPropertyNode(
     const PropertyNode& node,
     int indent,
     EngineObject* owner)
{
     ImGui::PushID(
          node.name.c_str()
     );


     ImGui::Indent(
          indent * 16.0f
     );


     switch (
          node.type
          )
     {
          // ============================================================
          // Int
          // ============================================================

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


     // ============================================================
     // Float
     // ============================================================

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


     // ============================================================
     // Bool
     // ============================================================

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


     // ============================================================
     // String
     // ============================================================

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


     // ============================================================
     // Reference
     // ============================================================

     case FieldType::Reference:
     {
          DrawReferenceSelector(
               node,
               owner
          );


          break;
     }


     // ============================================================
     // Struct / Vector
     // ============================================================

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
                    indent + 1,
                    owner
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
// Default Inspector
// ============================================================

inline void DrawDefaultInspector(
     const char* name,
     EngineObject* object)
{
     if (!object)
          return;


     // ============================================================
     // Runtime Type
     // ============================================================

     const std::type_info& typeInfo =
          typeid(*object);


     const TypeInfo* type =
          ReflectionRegistry::Instance()
          .Find(
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


     // ============================================================
     // Title
     // ============================================================

     ImGui::Text(
          "%s",
          type->name
     );


     // ============================================================
     // Property Node
     // ============================================================

     PropertyNode node;


     node.name =
          name;


     node.type =
          FieldType::Struct;


     node.typeName =
          type->name;


     node.children.clear();


     // ============================================================
     // Build Fields
     // ============================================================

     BuildTypeFields(
          node,
          type,
          object
     );


     // ============================================================
     // Draw
     // ============================================================

     for (
          const auto& child :
          node.children
          )
     {
          DrawPropertyNode(
               child,
               0,
               object
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
     // Custom Inspector
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
     // Reflection Type
     // ============================================================

     const TypeInfo* type =
          ReflectionRegistry::Instance()
          .Find(
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
     // Title
     // ============================================================

     ImGui::Text(
          "%s",
          type->name
     );


     // ============================================================
     // Property Node
     // ============================================================

     m_Node.name =
          m_name;


     m_Node.type =
          FieldType::Struct;


     m_Node.typeName =
          type->name;


     m_Node.children.clear();


     // ============================================================
     // Build Fields
     // ============================================================

     BuildTypeFields(
          m_Node,
          type,
          m_object
     );


     // ============================================================
     // Draw
     // ============================================================

     for (
          const auto& child :
          m_Node.children
          )
     {
          DrawPropertyNode(
               child,
               0,
               m_object
          );
     }
}