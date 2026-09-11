#pragma once

// ============================================================
// 依赖
// ============================================================

#include <any>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "imgui.h"
#include "Serialization/Reflection.h"   // 你的反射系统，只引入，不复制


// ============================================================
// 自定义 Inspector 工厂（方案 A）
// ============================================================

class CustomInspectorFactory
{
public:
     static CustomInspectorFactory& Instance()
     {
          static CustomInspectorFactory inst;
          return inst;
     }

     // 注册某个类型的自定义绘制函数
     template<typename T>
     void Register(std::function<void(T&)> drawFn)
     {
          m_drawFns[std::type_index(typeid(T))] =
               [drawFn](void* obj)
               {
                    drawFn(*static_cast<T*>(obj));
               };
     }

     // 尝试分发，返回 true 表示已经由自定义 Inspector 绘制
     bool CreateAndDraw(const std::type_info& type, void* object)
     {
          auto it = m_drawFns.find(std::type_index(type));
          if (it != m_drawFns.end())
          {
               it->second(object);
               return true;
          }
          return false;
     }

private:
     std::unordered_map<
          std::type_index,
          std::function<void(void*)>
     > m_drawFns;
};


// ============================================================
// 默认反射绘制函数
// ============================================================

inline void DrawPropertyNode(const PropertyNode& node, int indent)
{
     ImGui::PushID(node.name.c_str());
     ImGui::Indent(indent * 16.0f);

     switch (node.type)
     {
     case FieldType::Int:
     {
          int value = std::any_cast<int>(node.value);
          if (ImGui::DragInt(node.name.c_str(), &value))
               node.set(value);
          break;
     }

     case FieldType::Float:
     {
          float value = std::any_cast<float>(node.value);
          if (ImGui::DragFloat(node.name.c_str(), &value, 0.01f))
               node.set(value);
          break;
     }

     case FieldType::Bool:
     {
          bool value = std::any_cast<bool>(node.value);
          if (ImGui::Checkbox(node.name.c_str(), &value))
               node.set(value);
          break;
     }

     case FieldType::Struct:
     case FieldType::Vector:
     {
          ImGui::Text("%s : %s", node.name.c_str(), node.typeName.c_str());
          for (const auto& child : node.children)
               DrawPropertyNode(child, indent + 1);
          break;
     }

     default:
          break;
     }

     ImGui::Unindent(indent * 16.0f);
     ImGui::PopID();
}


// ============================================================
// 便捷模板：从对象构建节点并绘制（保持默认行为）
// ============================================================

template<typename T>
void DrawDefaultInspector(const char* name, T& object)
{
     DrawPropertyNode(BuildPropertyNode(name, object), 0);
}


// ============================================================
// ClassInspector
// ============================================================

class ClassInspector
{
public:
     template<typename T>
     ClassInspector(const char* name, T& object)
          : m_Node(BuildPropertyNode(name, object)),
          m_object(&object),
          m_typeInfo(&typeid(object))   // 保存动态类型
     {}

     void Draw();

private:
     PropertyNode m_Node;
     void* m_object = nullptr;
     const std::type_info* m_typeInfo = nullptr;
};


inline void ClassInspector::Draw()
{
     // 优先使用自定义 Inspector
     if (CustomInspectorFactory::Instance().CreateAndDraw(*m_typeInfo, m_object))
          return;

     // 否则回退到默认反射绘制
     DrawPropertyNode(m_Node, 0);
}