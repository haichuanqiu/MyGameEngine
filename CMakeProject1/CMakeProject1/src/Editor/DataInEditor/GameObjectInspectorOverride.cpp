// PointLightCustomInspector.cpp
#include "ClassInspector.h"
#include "Engine/Components/RenderingRelatedComponents.h"   // 你的 PointLight 定义


// 自动注册，不需要集中管理

static bool s_PointLightInspectorRegistered = []()
     {
          CustomInspectorFactory::Instance().Register<PointLight>(
               [](PointLight& light)
               {
                    bool changed = false;

                    // ==========================================
                    // Color —— 直接写死
                    // ==========================================
                    if (ImGui::ColorEdit3("Color", &light.Color.x))
                         changed = true;

                    // ==========================================
                    // Intensity
                    // ==========================================
                    if (ImGui::DragFloat(
                         "Intensity",
                         &light.Intensity,
                         0.01f,
                         0.0f,
                         100.0f))
                         changed = true;

                    // ==========================================
                    // Range
                    // ==========================================
                    if (ImGui::DragFloat(
                         "Range",
                         &light.Range,
                         0.01f,
                         0.0f,
                         1000.0f))
                         changed = true;

                    // ==========================================
                    // id —— 只读展示
                    // ==========================================
                    ImGui::Text("Light ID: %d", light.id);

                    ImGui::Separator();

                    // ==========================================
                    // 数值发生变化 → 调用无参 UpdateLightData()
                    // ==========================================
                    light.UpdateLightData();
               }
          );

          return true;
     }();

static bool s_GameObjectInspectorRegistered = []()
     {
          CustomInspectorFactory::Instance().Register<GameObject>(
               [](GameObject& gameObject)
               {
                    // ==========================================
                    // GameObject Name
                    // ==========================================
                    ImGui::Text("Name: %s", gameObject.name.c_str());

                    ImGui::Separator();

                    // ==========================================
                    // Components
                    // ==========================================
                    auto& components =
                         gameObject.GetComponents();

                    for (size_t i = 0; i < components.size(); ++i)
                    {
                         auto& component = components[i];

                         if (!component)
                              continue;

                         ImGui::PushID(static_cast<int>(i));

                         ClassInspector inspector(
                              "Component",
                              component.get()
                         );

                         inspector.Draw();

                         ImGui::PopID();

                         ImGui::Separator();
                    }
               }
          );

          return true;
     }();