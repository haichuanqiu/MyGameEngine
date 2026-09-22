#pragma once

#include "Engine/Scene.h"
#include "Reflection.h"
#include "ComponentRegistry.h"
#include "JsonSerializer.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cctype>
#include <fstream>
#include <iostream>


class SceneSerializer
{
private:

     // =========================================================
     // Serialized Component
     //
     // objectId:
     //
     // 不再来自 Component 外层 ObjId。
     //
     // 现在来自：
     //
     // Data
     // {
     //      "ReferenceInfo":
     //      {
     //           "ScopeLevel": 0,
     //           "ScopeID": 0,
     //           "ObjectID": 2
     //      }
     // }
     // =========================================================

     struct SerializedComponent
     {
          uint64_t objectId = 0;

          TypeId typeId = 0;

          std::string data;
     };


     // =========================================================
     // Serialized GameObject
     //
     // objectId:
     //
     // 来自 GameObject 外层：
     //
     // ObjectId: 1
     // =========================================================

     struct SerializedGameObject
     {
          uint64_t objectId = 0;

          std::string name;


          std::vector<SerializedComponent>
               components;
     };


public:

     // =========================================================
     // Save Scene To File
     // =========================================================

     static bool SaveSceneTo(
          const Scene& target,
          const std::string& path)
     {
          std::ofstream file(
               path,
               std::ios::out |
               std::ios::trunc
          );


          if (!file.is_open())
               return false;


          const std::string content =
               Serialize(
                    target
               );


          file.write(
               content.data(),
               static_cast<std::streamsize>(
                    content.size()
                    )
          );


          if (!file.good())
               return false;


          file.close();


          return true;
     }


     // ============================================================
     // Phase 1
     //
     // Create GameObjects / Components
     //
     // Register:
     //
     // GameObject:
     //      ObjectId
     //
     // Component:
     //      Data.ReferenceInfo.ObjectID
     //
     // 此阶段不 Deserialize。
     // ============================================================

     static void CreateObjects(
          Scene* target,
          std::vector<SerializedGameObject>& objects,
          std::unordered_map<uint64_t, Component*>& components,
          int scopeLevel,
          int scopeID)
     {
          for (
               auto& serializedObject :
               objects
               )
          {
               // ========================================================
               // Create GameObject
               // ========================================================

               auto gameObject =
                    std::make_unique<GameObject>();


               gameObject->name =
                    serializedObject.name;


               GameObject* gameObjectPtr =
                    gameObject.get();


               // ========================================================
               // Components
               // ========================================================

               for (
                    auto& serializedComponent :
                    serializedObject.components
                    )
               {
                    Component* component =
                         nullptr;


                    // ===================================================
                    // Find Reflection Type
                    // ===================================================

                    const TypeInfo* typeInfo =
                         ReflectionRegistry::Instance()
                         .FindById(
                              serializedComponent.typeId
                         );


                    if (!typeInfo)
                    {
                         std::cerr
                              << "[SceneSerializer Error] "
                              << "Unknown Component TypeId: "
                              << serializedComponent.typeId
                              << std::endl;


                         continue;
                    }


                    // ===================================================
                    // Transform Special Case
                    //
                    // GameObject constructor 已经创建 Transform。
                    // ===================================================

                    const TypeInfo* transformInfo =
                         ReflectionRegistry::Instance()
                         .Find(
                              typeid(Transform)
                         );


                    if (
                         transformInfo &&
                         serializedComponent.typeId ==
                         transformInfo->id
                         )
                    {
                         component =
                              gameObjectPtr->transform;
                    }
                    else
                    {
                         component =
                              ComponentRegistry::Instance()
                              .Create(
                                   serializedComponent.typeId,
                                   *gameObjectPtr
                              );
                    }


                    if (!component)
                    {
                         std::cerr
                              << "[SceneSerializer Error] "
                              << "Failed to create Component. "
                              << "TypeId="
                              << serializedComponent.typeId
                              << std::endl;


                         continue;
                    }


                    // ===================================================
                    // Register Component
                    //
                    // ID 来自：
                    //
                    // Data.ReferenceInfo.ObjectID
                    //
                    // ParseComponents 已经提前读取。
                    // ===================================================

                    target->AddComponent(
                         component,
                         scopeLevel,
                         scopeID,
                         serializedComponent.objectId
                    );


                    // ===================================================
                    // Local Lookup
                    //
                    // ObjectID -> Component*
                    //
                    // Phase 2 / 3 / 4 使用。
                    // ===================================================

                    components[
                         serializedComponent.objectId
                    ] =
                         component;
               }


               // ========================================================
               // Register GameObject
               //
               // ID 来自：
               //
               // ObjectId:
               // ========================================================

               target->AddGameObject(
                    std::move(
                         gameObject
                    ),
                    scopeLevel,
                    scopeID,
                    serializedObject.objectId
               );
          }
     }


     // ============================================================
     // Phase 2
     //
     // Deserialize Normal Data
     // ============================================================

     static void DeserializeComponents(
          const std::vector<SerializedGameObject>& objects,
          const std::unordered_map<uint64_t, Component*>& components)
     {
          for (
               const auto& serializedObject :
               objects
               )
          {
               for (
                    const auto& serializedComponent :
                    serializedObject.components
                    )
               {
                    auto it =
                         components.find(
                              serializedComponent.objectId
                         );


                    if (
                         it ==
                         components.end()
                         )
                    {
                         continue;
                    }


                    Component* component =
                         it->second;


                    JsonSerializer::Deserialize(
                         *component,
                         serializedComponent.data
                    );
               }
          }
     }


     // ============================================================
     // Phase 3
     //
     // Load References
     // ============================================================

     static void LoadComponentReferences(
          const std::vector<SerializedGameObject>& objects,
          const std::unordered_map<uint64_t, Component*>& components)
     {
          for (
               const auto& serializedObject :
               objects
               )
          {
               for (
                    const auto& serializedComponent :
                    serializedObject.components
                    )
               {
                    auto it =
                         components.find(
                              serializedComponent.objectId
                         );


                    if (
                         it ==
                         components.end()
                         )
                    {
                         continue;
                    }


                    Component* component =
                         it->second;


                    JsonSerializer::LoadReference(
                         *component,
                         serializedComponent.data
                    );
               }
          }
     }


     // ============================================================
     // Phase 4
     //
     // Notify Components
     // ============================================================

     static void NotifyComponentsCreated(
          const std::vector<SerializedGameObject>& objects,
          const std::unordered_map<uint64_t, Component*>& components)
     {
          for (
               const auto& serializedObject :
               objects
               )
          {
               for (
                    const auto& serializedComponent :
                    serializedObject.components
                    )
               {
                    auto it =
                         components.find(
                              serializedComponent.objectId
                         );


                    if (
                         it ==
                         components.end()
                         )
                    {
                         continue;
                    }


                    Component* component =
                         it->second;


                    component
                         ->OnCreatedBySceneLoader();
               }
          }
     }


     // ============================================================
     // Load Scene
     // ============================================================

     static void LoadScene(
          Scene* target,
          const std::string& content)
     {
          if (!target)
               return;


          // ============================================================
          // Parse
          // ============================================================

          std::vector<SerializedGameObject> objects =
               Parse(
                    content
               );


          // ============================================================
          // Component Lookup
          // ============================================================

          std::unordered_map<uint64_t, Component*>
               components;


          // ============================================================
          // Scene Scope
          // ============================================================

          const int scopeLevel =
               0;


          const int scopeID =
               target->sceneIndex;


          // ============================================================
          // Phase 1
          // ============================================================

          CreateObjects(
               target,
               objects,
               components,
               scopeLevel,
               scopeID
          );


          // ============================================================
          // Phase 2
          // ============================================================

          DeserializeComponents(
               objects,
               components
          );


          // ============================================================
          // Phase 3
          // ============================================================

          LoadComponentReferences(
               objects,
               components
          );


          // ============================================================
          // Phase 4
          // ============================================================

          NotifyComponentsCreated(
               objects,
               components
          );
     }


     // ============================================================
     // Test Load Context
     //
     // TestLoadScene1:
     //
     //      Phase 1
     //      Phase 2
     //
     // TestLoadScene2:
     //
     //      Phase 3
     //      Phase 4
     // ============================================================

     inline static std::vector<SerializedGameObject>
          s_TestLoadObjects;


     inline static std::unordered_map<uint64_t, Component*>
          s_TestLoadComponents;


     // ============================================================
     // Test Load Scene 1
     //
     // Phase 1 + Phase 2
     // ============================================================

     static void TestLoadScene1(
          Scene* target,
          const std::string& content)
     {
          if (!target)
               return;


          // ============================================================
          // Reset Previous Context
          // ============================================================

          s_TestLoadObjects.clear();

          s_TestLoadComponents.clear();


          // ============================================================
          // Parse
          // ============================================================

          s_TestLoadObjects =
               Parse(
                    content
               );


          // ============================================================
          // Scene Scope
          // ============================================================

          const int scopeLevel =
               0;


          const int scopeID =
               target->sceneIndex;


          // ============================================================
          // Phase 1
          //
          // Create Objects
          // ============================================================

          CreateObjects(
               target,
               s_TestLoadObjects,
               s_TestLoadComponents,
               scopeLevel,
               scopeID
          );


          // ============================================================
          // Phase 2
          //
          // Deserialize
          // ============================================================

          DeserializeComponents(
               s_TestLoadObjects,
               s_TestLoadComponents
          );
     }


     // ============================================================
     // Test Load Scene 2
     //
     // Phase 3 + Phase 4
     // ============================================================

     static void TestLoadScene2()
     {
          // ============================================================
          // Phase 3
          //
          // Load References
          // ============================================================

          LoadComponentReferences(
               s_TestLoadObjects,
               s_TestLoadComponents
          );


          // ============================================================
          // Phase 4
          //
          // Notify Components Created
          // ============================================================

          NotifyComponentsCreated(
               s_TestLoadObjects,
               s_TestLoadComponents
          );


          // ============================================================
          // Clear Test Context
          // ============================================================

          s_TestLoadObjects.clear();

          s_TestLoadComponents.clear();
     }


     // =========================================================
     // Serialize Scene
     // =========================================================

     static std::string Serialize(
          const Scene& target)
     {
          std::string content;


          // =====================================================
          // GameObjects
          // =====================================================

          for (
               const auto& gameObjectPtr :
               target.GetAllGameObjects()
               )
          {
               GameObject& gameObject =
                    *gameObjectPtr;


               // =================================================
               // GameObject Begin
               // =================================================

               content +=
                    "{\n";


               content +=
                    "GameObject \n";


               // =================================================
               // Name
               // =================================================

               content +=
                    "name: ";


               content +=
                    gameObject.name;


               content +=
                    "\n";


               // =================================================
               // Object ID
               //
               // GameObject 自己的 ObjectID。
               //
               // 不再使用 Serialize 临时 ID。
               //
               // ScopeLevel / ScopeID 不需要保存在这里。
               // =================================================

               content +=
                    "ObjectId: ";


               content +=
                    std::to_string(
                         gameObject
                         .ReferenceInfo
                         .ObjectID
                    );


               content +=
                    "\n";


               // =================================================
               // Components
               // =================================================

               const auto& components =
                    gameObject.GetComponents();


               for (
                    size_t i = 0;
                    i < components.size();
                    ++i
                    )
               {
                    const auto& component =
                         components[i];


                    // =============================================
                    // Reflection
                    // =============================================

                    const TypeInfo* info =
                         ReflectionRegistry::Instance()
                         .Find(
                              typeid(*component)
                         );


                    // =============================================
                    // No Reflection
                    //
                    // 不写 Component Block。
                    // =============================================

                    if (!info)
                    {
                         continue;
                    }


                    // =============================================
                    // Component Begin
                    // =============================================

                    content +=
                         "Component{\n";


                    // =============================================
                    // TypeId
                    // =============================================

                    TypeId typeId =
                         info->id;


                    content +=
                         "TypeId:";


                    content +=
                         std::to_string(
                              typeId
                         );


                    content +=
                         "\n";


                    // =============================================
                    // Data
                    //
                    // Component ObjectID 已经包含在：
                    //
                    // ReferenceInfo.ObjectID
                    //
                    // 所以外层不再保存 ObjId。
                    // =============================================

                    content +=
                         "Data: ";


                    content +=
                         JsonSerializer::Serialize(
                              *component
                         );


                    content +=
                         "\n";


                    // =============================================
                    // Component End
                    // =============================================

                    content +=
                         "}\n";
               }


               // =================================================
               // GameObject End
               // =================================================

               content +=
                    "}\n";
          }


          return content;
     }


private:

     // =========================================================
     // Parse
     // =========================================================

     static std::vector<SerializedGameObject>
          Parse(
               const std::string& content)
     {
          std::vector<SerializedGameObject>
               result;


          size_t pos =
               0;


          while (true)
          {
               size_t objectBegin =
                    content.find(
                         "{\nGameObject",
                         pos
                    );


               if (
                    objectBegin ==
                    std::string::npos
                    )
               {
                    break;
               }


               size_t objectEnd =
                    FindMatchingBrace(
                         content,
                         objectBegin
                    );


               if (
                    objectEnd ==
                    std::string::npos
                    )
               {
                    break;
               }


               std::string objectText =
                    content.substr(
                         objectBegin,
                         objectEnd -
                         objectBegin +
                         1
                    );


               SerializedGameObject object;


               // =================================================
               // Name
               // =================================================

               object.name =
                    ReadString(
                         objectText,
                         "name:"
                    );


               // =================================================
               // Object ID
               //
               // 新格式：
               //
               // ObjectId: 1
               // =================================================

               object.objectId =
                    ReadUInt64(
                         objectText,
                         "ObjectId:"
                    );


               // =================================================
               // Components
               // =================================================

               ParseComponents(
                    objectText,
                    object.components
               );


               result.push_back(
                    std::move(
                         object
                    )
               );


               pos =
                    objectEnd + 1;
          }


          return result;
     }


     // =========================================================
     // Parse Components
     //
     // Component 外层格式：
     //
     // Component{
     //
     //      TypeId:123
     //
     //      Data:
     //      {
     //           "ReferenceInfo":
     //           {
     //                "ScopeLevel": 0,
     //                "ScopeID": 0,
     //                "ObjectID": 2
     //           }
     //      }
     // }
     //
     // Component ObjectID 直接从 Data 中读取。
     // =========================================================

     static void ParseComponents(
          const std::string& objectText,
          std::vector<SerializedComponent>& result)
     {
          size_t pos =
               0;


          while (true)
          {
               // =================================================
               // Find Component
               // =================================================

               size_t componentBegin =
                    objectText.find(
                         "Component{",
                         pos
                    );


               if (
                    componentBegin ==
                    std::string::npos
                    )
               {
                    break;
               }


               // =================================================
               // Component Brace Begin
               // =================================================

               size_t braceBegin =
                    objectText.find(
                         '{',
                         componentBegin
                    );


               if (
                    braceBegin ==
                    std::string::npos
                    )
               {
                    break;
               }


               // =================================================
               // Component Brace End
               // =================================================

               size_t componentEnd =
                    FindMatchingBrace(
                         objectText,
                         braceBegin
                    );


               if (
                    componentEnd ==
                    std::string::npos
                    )
               {
                    break;
               }


               // =================================================
               // Component Text
               // =================================================

               std::string componentText =
                    objectText.substr(
                         braceBegin,
                         componentEnd -
                         braceBegin +
                         1
                    );


               SerializedComponent component;


               // =================================================
               // TypeId
               // =================================================

               component.typeId =
                    ReadUInt64(
                         componentText,
                         "TypeId:"
                    );


               // =================================================
               // Data
               // =================================================

               component.data =
                    ReadData(
                         componentText
                    );


               // =================================================
               // Object ID
               //
               // 不再读取：
               //
               // ObjId:
               //
               // 现在读取：
               //
               // Data.ReferenceInfo.ObjectID
               //
               // JSON Serializer 当前输出：
               //
               // "ObjectID": 2
               // =================================================

               component.objectId =
                    ReadUInt64(
                         component.data,
                         "\"ObjectID\":"
                    );


               // =================================================
               // Validate Object ID
               // =================================================

               if (
                    component.objectId ==
                    0
                    )
               {
                    std::cerr
                         << "[SceneSerializer Error] "
                         << "Component ObjectID not found. "
                         << "TypeId="
                         << component.typeId
                         << std::endl;
               }


               // =================================================
               // Add
               // =================================================

               result.push_back(
                    std::move(
                         component
                    )
               );


               pos =
                    componentEnd + 1;
          }
     }


     // =========================================================
     // Read Data
     // =========================================================

     static std::string ReadData(
          const std::string& text)
     {
          size_t dataPos =
               text.find(
                    "Data:"
               );


          if (
               dataPos ==
               std::string::npos
               )
          {
               return {};
          }


          size_t braceBegin =
               text.find(
                    '{',
                    dataPos
               );


          if (
               braceBegin ==
               std::string::npos
               )
          {
               return {};
          }


          size_t braceEnd =
               FindMatchingBrace(
                    text,
                    braceBegin
               );


          if (
               braceEnd ==
               std::string::npos
               )
          {
               return {};
          }


          return
               text.substr(
                    braceBegin,
                    braceEnd -
                    braceBegin +
                    1
               );
     }


     // =========================================================
     // Read UInt64
     // =========================================================

     static uint64_t ReadUInt64(
          const std::string& text,
          const std::string& key)
     {
          size_t pos =
               text.find(
                    key
               );


          if (
               pos ==
               std::string::npos
               )
          {
               return 0;
          }


          pos +=
               key.size();


          // =====================================================
          // Skip Whitespace
          // =====================================================

          while (
               pos < text.size() &&
               std::isspace(
                    static_cast<unsigned char>(
                         text[pos]
                         )
               )
               )
          {
               ++pos;
          }


          // =====================================================
          // Read Number
          // =====================================================

          size_t end =
               pos;


          while (
               end < text.size() &&
               std::isdigit(
                    static_cast<unsigned char>(
                         text[end]
                         )
               )
               )
          {
               ++end;
          }


          if (
               end ==
               pos
               )
          {
               return 0;
          }


          return
               std::stoull(
                    text.substr(
                         pos,
                         end - pos
                    )
               );
     }


     // =========================================================
     // Read String
     // =========================================================

     static std::string ReadString(
          const std::string& text,
          const std::string& key)
     {
          size_t pos =
               text.find(
                    key
               );


          if (
               pos ==
               std::string::npos
               )
          {
               return {};
          }


          pos +=
               key.size();


          // =====================================================
          // Skip Leading Whitespace
          // =====================================================

          while (
               pos < text.size() &&
               std::isspace(
                    static_cast<unsigned char>(
                         text[pos]
                         )
               )
               )
          {
               ++pos;
          }


          // =====================================================
          // Find End Of Line
          // =====================================================

          size_t end =
               text.find(
                    '\n',
                    pos
               );


          if (
               end ==
               std::string::npos
               )
          {
               end =
                    text.size();
          }


          std::string value =
               text.substr(
                    pos,
                    end - pos
               );


          // =====================================================
          // Remove Trailing Whitespace
          // =====================================================

          while (
               !value.empty() &&
               std::isspace(
                    static_cast<unsigned char>(
                         value.back()
                         )
               )
               )
          {
               value.pop_back();
          }


          return value;
     }


     // =========================================================
     // Find Matching Brace
     //
     // Example:
     //
     // {
     //      Data:
     //      {
     //           "ReferenceInfo":
     //           {
     //                "ObjectID": 2
     //           }
     //      }
     // }
     //
     // 找到最外层对应的 }。
     // =========================================================

     static size_t FindMatchingBrace(
          const std::string& text,
          size_t begin)
     {
          if (
               begin >= text.size() ||
               text[begin] != '{'
               )
          {
               return std::string::npos;
          }


          int depth =
               0;


          bool inString =
               false;


          bool escape =
               false;


          for (
               size_t i = begin;
               i < text.size();
               ++i
               )
          {
               char c =
                    text[i];


               // =====================================================
               // Escaped Character
               // =====================================================

               if (escape)
               {
                    escape =
                         false;


                    continue;
               }


               // =====================================================
               // Escape
               // =====================================================

               if (
                    c == '\\' &&
                    inString
                    )
               {
                    escape =
                         true;


                    continue;
               }


               // =====================================================
               // String
               // =====================================================

               if (
                    c == '"'
                    )
               {
                    inString =
                         !inString;


                    continue;
               }


               if (inString)
                    continue;


               // =====================================================
               // Brace
               // =====================================================

               if (
                    c == '{'
                    )
               {
                    ++depth;
               }
               else if (
                    c == '}'
                    )
               {
                    --depth;


                    if (
                         depth ==
                         0
                         )
                    {
                         return i;
                    }
               }
          }


          return std::string::npos;
     }
};