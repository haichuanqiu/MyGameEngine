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

#include <typeinfo>
#include <utility>


enum class InspectorDrawResult
{
     NotHandled,
     KeepTarget,
     ClearTarget
};


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

          std::function<InspectorDrawResult(EngineObject*)>
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
     //
     // Custom Inspector:
     //
     // true:
     //     object 仍然有效
     //
     // false:
     //     要求 Inspector 清除当前 target
     // ============================================================

     template<typename T>
     void Register(
          std::function<bool(T&)> drawFn)
     {
          static_assert(
               std::is_base_of_v<EngineObject, T>,
               "Custom Inspector type must derive from EngineObject."
               );


          InspectorEntry newEntry
          {
               // =================================================
               // Type
               // =================================================

               std::type_index(
                    typeid(T)
               ),


                    // =================================================
                    // Can Draw
                    // =================================================

                    [](EngineObject* object) -> bool
                    {
                         if (!object)
                              return false;


                         return
                              dynamic_cast<T*>(
                                   object
                              ) != nullptr;
                    },


                    // =================================================
                    // Draw
                    // =================================================

                    [drawFn](EngineObject* object)
                         -> InspectorDrawResult
                    {
                         if (!object)
                         {
                              return
                                   InspectorDrawResult::
                                   ClearTarget;
                         }


                         T* castedObject =
                              dynamic_cast<T*>(
                                   object
                              );


                         if (!castedObject)
                         {
                              return
                                   InspectorDrawResult::
                                   NotHandled;
                         }


                         const bool keepTarget =
                              drawFn(
                                   *castedObject
                              );


                         if (!keepTarget)
                         {
                              return
                                   InspectorDrawResult::
                                   ClearTarget;
                         }


                         return
                              InspectorDrawResult::
                              KeepTarget;
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

     InspectorDrawResult CreateAndDraw(
          EngineObject* object)
     {
          if (!object)
          {
               return
                    InspectorDrawResult::
                    ClearTarget;
          }


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
                    return
                         entry.draw(
                              object
                         );
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
                    return
                         entry.draw(
                              object
                         );
               }
          }


          // ======================================================
          // No Custom Inspector
          // ======================================================

          return
               InspectorDrawResult::
               NotHandled;
     }


private:

     CustomInspectorFactory() = default;


     std::vector<
          InspectorEntry
     > m_entries;
};


// ============================================================
// Object Display Name
// ============================================================

inline std::string GetObjectDisplayName(
     EngineObject* object)
{
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


     return "Unknown Object";
}


// ============================================================
// Object Reference Display Name
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


     if (!object)
     {
          node.setReference(nullptr);
     
          return
               "Missing Reference";
          
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


     std::string displayName =
          GetReferenceDisplayName(
               node
          );


     ImGui::Text(
          "%s",
          node.name.c_str()
     );


     ImGui::SameLine();


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
          // Candidates
          // ======================================================

          std::vector<EngineObject*> candidates =
               ReferenceResolver::Instance()
               .FindAllOfType<EngineObject>(
                    owner->ReferenceInfo
               );


          bool foundCandidate =
               false;


          for (
               EngineObject* candidate :
               candidates
               )
          {
               if (!candidate)
                    continue;


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


               std::string candidateName =
                    GetObjectReferenceDisplayName(
                         candidate
                    );


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


               if (
                    ImGui::Selectable(
                         selectableName.c_str(),
                         selected
                    )
                    )
               {
                    if (
                         node.canSetReference(
                              candidate
                         )&&
                         candidate 
                         &&
                         node.setReference
                         )
                    {
                         if (candidate->waitingToDestroy)
                         {
                              node.setReference(nullptr);
                         }
                         else
                         {
                              node.setReference(candidate);
                         }
                    }


                    ImGui::CloseCurrentPopup();
               }
          }


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


     ImGui::Text(
          "%s",
          type->name
     );


     PropertyNode node;


     node.name =
          name;


     node.type =
          FieldType::Struct;


     node.typeName =
          type->name;


     node.children.clear();


     BuildTypeFields(
          node,
          type,
          object
     );


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


     // ============================================================
     // Draw
     //
     // true:
     //     target 可以继续保留
     //
     // false:
     //     调用者应该把自己的 target 设置成 nullptr
     // ============================================================

     bool Draw();


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

inline bool ClassInspector::Draw()
{
     if (
          !m_object ||
          !m_typeInfo
          )
     {
          return false;
     }


     // ============================================================
     // Custom Inspector
     // ============================================================

     InspectorDrawResult customResult =
          CustomInspectorFactory::Instance()
          .CreateAndDraw(
               m_object
          );


     // ============================================================
     // Custom Inspector requests target clear
     // ============================================================

     if (
          customResult ==
          InspectorDrawResult::
          ClearTarget
          )
     {
          m_object =
               nullptr;


          m_typeInfo =
               nullptr;


          return false;
     }


     // ============================================================
     // Custom Inspector handled this object
     // ============================================================

     if (
          customResult ==
          InspectorDrawResult::
          KeepTarget
          )
     {
          return true;
     }


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


          return true;
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


     return true;
}