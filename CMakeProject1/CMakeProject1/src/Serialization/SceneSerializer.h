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


class SceneSerializer
{
private:

     // =========================================================
     // Serialized Component
     // =========================================================

     struct SerializedComponent
     {
          uint64_t objId = 0;
          TypeId typeId = 0;
          std::string data;
     };


     // =========================================================
     // Serialized GameObject
     // =========================================================

     struct SerializedGameObject
     {
          uint64_t id = 0;
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
               Serialize(target);


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


     // =========================================================
     // Load Scene
     //
     // 两阶段：
     //
     // 第一阶段：
     //     创建 GameObject
     //     创建 Component
     //     建立 ID Dictionary
     //
     // 第二阶段：
     //     Deserialize Data
     //
     // 目前不处理 Reference
     // =========================================================

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
               Parse(content);


          // ============================================================
          // Component Dictionary
          //
          // 仅供本次 LoadScene 后续阶段使用
          //
          // Serialized ObjId -> 新创建的 Component*
          // ============================================================

          std::unordered_map<uint64_t, Component*> components;


          // ============================================================
          // Scene Scope
          // ============================================================

          const int scopeLevel = 0;
          const int scopeID = target->sceneIndex;


          // ============================================================
          // Phase 1
          //
          // 创建全部 GameObject / Component
          // 注册 ReferenceInfo
          //
          // 此阶段不 Deserialize
          // ============================================================

          for (auto& serializedObject : objects)
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

               for (auto& serializedComponent :
                    serializedObject.components)
               {
                    Component* component = nullptr;


                    // ====================================================
                    // Find Reflection Type
                    // ====================================================

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


                    // ====================================================
                    // Transform Special Case
                    //
                    // GameObject constructor 已经创建 Transform
                    // ====================================================

                    const TypeInfo* transformInfo =
                         ReflectionRegistry::Instance()
                         .Find(
                              typeid(Transform)
                         );


                    if (transformInfo &&
                         serializedComponent.typeId ==
                         transformInfo->id)
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


                    // ====================================================
                    // Register Component
                    //
                    // 这里非常重要：
                    //
                    // serialized objId
                    //      ↓
                    // ReferenceInfo.ObjectID
                    //      ↓
                    // Scene m_Objects
                    //
                    // 三者统一
                    // ====================================================

                    target->AddComponent(
                         component,
                         scopeLevel,
                         scopeID,
                         serializedComponent.objId
                    );


                    // ====================================================
                    // Local Lookup
                    //
                    // 后面的 Deserialize / LoadReference 使用
                    // ====================================================

                    components[
                         serializedComponent.objId
                    ] = component;
               }


               // ========================================================
               // Register + Add GameObject To Scene
               // ========================================================

               target->AddGameObject(
                    std::move(gameObject),
                    scopeLevel,
                    scopeID,
                    serializedObject.id
               );
          }


       
          for (const auto& serializedObject : objects)
          {
               for (const auto& serializedComponent :
                    serializedObject.components)
               {
                    auto it =
                         components.find(
                              serializedComponent.objId
                         );

                    if (it == components.end())
                         continue;


                    Component* component =
                         it->second;


                    JsonSerializer::Deserialize(
                         *component,
                         serializedComponent.data
                    );
               }
          }
          

          for (const auto& serializedObject : objects)
          {
               for (const auto& serializedComponent :
                    serializedObject.components)
               {
                    auto it =
                         components.find(
                              serializedComponent.objId
                         );

                    if (it == components.end())
                         continue;


                    Component* component =
                         it->second;


                    JsonSerializer::LoadReference(
                         *component,
                         serializedComponent.data
                    );
               }
          }


          // ============================================================
          // Phase 4
          //
          // Scene Loader Created
          //
          // 此时：
          //
          // 普通数据       ✓
          // References     ✓
          // Scene Registry ✓
          //
          // Component 可以正式初始化
          // ============================================================

          for (const auto& serializedObject : objects)
          {
               for (const auto& serializedComponent :
                    serializedObject.components)
               {
                    auto it =
                         components.find(
                              serializedComponent.objId
                         );

                    if (it == components.end())
                         continue;


                    Component* component =
                         it->second;


                    component->OnCreatedBySceneLoader();
               }
          }
     }

     // =========================================================
     // Serialize Scene
     // =========================================================

     static std::string Serialize(
          const Scene& target)
     {
          // =====================================================
          // 先给所有 GameObject / Component 分配临时 ID
          // =====================================================

          auto map =
               Scan(target);


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
               // GameObject ID
               // =================================================

               auto gameObjectIt =
                    map.find(
                         &gameObject
                    );


               if (
                    gameObjectIt !=
                    map.end()
                    )
               {
                    content +=
                         "id: ";

                    content +=
                         std::to_string(
                              gameObjectIt->second
                         );

                    content +=
                         "\n";
               }


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
                    // 没有 Reflection
                    //
                    // 不写 Component Block
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
                    // ObjId
                    // =============================================

                    auto componentIt =
                         map.find(
                              component.get()
                         );


                    if (
                         componentIt !=
                         map.end()
                         )
                    {
                         uint64_t objId =
                              componentIt->second;


                         content +=
                              "ObjId:";

                         content +=
                              std::to_string(
                                   objId
                              );

                         content +=
                              "\n";
                    }


                    // =============================================
                    // Data
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
     // Scan
     //
     // 给当前 Scene 中所有对象分配临时 ID
     //
     // GameObject:
     //     1
     //     4
     //     7
     //
     // Component:
     //     2
     //     3
     //     5
     //     6
     //
     // 目前这些 ID 只是一次 Serialize 的临时 ID。
     // =========================================================

     static std::unordered_map<
          const void*,
          uint64_t
     >
          Scan(const Scene& target)
     {
          std::unordered_map<
               const void*,
               uint64_t
          > map;


          uint64_t nextID =
               1;


          for (
               const auto& gameObjectPtr :
               target.GetAllGameObjects()
               )
          {
               GameObject& gameObject =
                    *gameObjectPtr;


               // =================================================
               // GameObject ID
               // =================================================

               map[
                    &gameObject
               ] =
                    nextID++;


                    // =================================================
                    // Components
                    // =================================================

                    const auto& components =
                         gameObject.GetComponents();


                    for (
                         const auto& componentPtr :
                         components
                         )
                    {
                         map[
                              componentPtr.get()
                         ] =
                              nextID++;
                    }
          }


          return map;
     }


     // =========================================================
     // Parse
     // =========================================================

     static std::vector<SerializedGameObject>
          Parse(
               const std::string& content)
     {
          std::vector<
               SerializedGameObject
          > result;


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
               // ID
               // =================================================

               object.id =
                    ReadUInt64(
                         objectText,
                         "id:"
                    );


               // =================================================
               // Components
               // =================================================

               ParseComponents(
                    objectText,
                    object.components
               );


               result.push_back(
                    std::move(object)
               );


               pos =
                    objectEnd + 1;
          }


          return result;
     }


     // =========================================================
     // Parse Components
     // =========================================================

     static void ParseComponents(
          const std::string& objectText,
          std::vector<
          SerializedComponent
          >& result)
     {
          size_t pos =
               0;


          while (true)
          {
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
               // ObjId
               // =================================================

               component.objId =
                    ReadUInt64(
                         componentText,
                         "ObjId:"
                    );


               // =================================================
               // Data
               // =================================================

               component.data =
                    ReadData(
                         componentText
                    );


               result.push_back(
                    std::move(component)
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


          return text.substr(
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


          if (end == pos)
               return 0;


          return std::stoull(
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
     // 例如：
     //
     // {
     //     Data: {
     //         x: 1
     //     }
     // }
     //
     // 找到最外层对应的 }
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


               if (escape)
               {
                    escape =
                         false;

                    continue;
               }


               if (
                    c == '\\' &&
                    inString
                    )
               {
                    escape =
                         true;

                    continue;
               }


               if (c == '"')
               {
                    inString =
                         !inString;

                    continue;
               }


               if (inString)
                    continue;


               if (c == '{')
               {
                    ++depth;
               }
               else if (c == '}')
               {
                    --depth;


                    if (depth == 0)
                    {
                         return i;
                    }
               }
          }


          return std::string::npos;
     }
};