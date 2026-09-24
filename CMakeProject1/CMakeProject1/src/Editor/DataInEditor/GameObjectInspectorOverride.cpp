// PointLightCustomInspector.cpp
#include "ClassInspector.h"
#include "Engine/Components/RenderingRelatedComponents.h"   // 你的 PointLight 定义


// 自动注册，不需要集中管理

static bool s_PointLightInspectorRegistered = []()
     {
          CustomInspectorFactory::Instance().Register<PointLight>(
               [](PointLight& light) -> bool
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
                    return true;
               }
          );

          return true;
     }();

static bool s_GameObjectInspectorRegistered = []()
     {
          CustomInspectorFactory::Instance().Register<GameObject>(
               [](GameObject& gameObject) -> bool
               {
                  // PointLightCustomInspector.cpp
#include "ClassInspector.h"
#include "Engine/Components/RenderingRelatedComponents.h"   // 你的 PointLight 定义


// 自动注册，不需要集中管理

static bool s_PointLightInspectorRegistered = []()
     {
          CustomInspectorFactory::Instance().Register<PointLight>(
               [](PointLight& light) -> bool
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
                    return true;
               }
          );

          return true;
     }();

static bool s_GameObjectInspectorRegistered = []()
     {
          CustomInspectorFactory::Instance().Register<GameObject>(
               [](GameObject& gameObject) -> bool
               {
                    if (gameObject.waitingToDestroy)
                    {
                         return false;
                    }

                    ImGui::Text("Name");
                    ImGui::SameLine();

                    ImGui::SetNextItemWidth(-1.0f);

                    char nameBuffer[256];

                    strncpy_s(
                         nameBuffer,
                         sizeof(nameBuffer),
                         gameObject.name.c_str(),
                         _TRUNCATE
                    );

                    if (ImGui::InputText(
                         "##GameObjectName",
                         nameBuffer,
                         sizeof(nameBuffer)))
                    {
                         gameObject.name = nameBuffer;
                    }

                    ImGui::Separator();


                    // ==========================================
                    // Components
                    // ==========================================

                    auto& components =
                         gameObject.GetComponents();

                    for (size_t i = 0;
                         i < components.size();
                         ++i)
                    {
                         auto& component =
                              components[i];

                         if (!component)
                              continue;


                         ImGui::PushID(
                              static_cast<int>(i)
                         );


                         ClassInspector inspector(
                              "Component",
                              component.get()
                         );
                         
                         inspector.Draw();


                         ImGui::PopID();

                         ImGui::Separator();
                    }
                    if (ImGui::Button("Add Component"))
                    {
                         ImGui::OpenPopup(
                              "AddComponentPopup"
                         );
                    }


                    if (ImGui::BeginPopup(
                         "AddComponentPopup"))
                    {
                         auto& registry =
                              ComponentRegistry::Instance();


                         for (const auto& [typeId, info] :
                              registry.GetAll())
                         {
                              if (ImGui::MenuItem(
                                   info.name.c_str()))
                              {
                                   registry.Create(
                                        typeId,
                                        gameObject
                                   );
                              }
                         }


                         ImGui::EndPopup();
                    }
                    return true;
               }
          );

          return true;
     }();

                    ImGui::Text("Name");
                    ImGui::SameLine();

                    ImGui::SetNextItemWidth(-1.0f);

                    char nameBuffer[256];

                    strncpy_s(
                         nameBuffer,
                         sizeof(nameBuffer),
                         gameObject.name.c_str(),
                         _TRUNCATE
                    );

                    if (ImGui::InputText(
                         "##GameObjectName",
                         nameBuffer,
                         sizeof(nameBuffer)))
                    {
                         gameObject.name = nameBuffer;
                    }

                    ImGui::Separator();


                    // ==========================================
                    // Components
                    // ==========================================

                    auto& components =
                         gameObject.GetComponents();

                    for (size_t i = 0;
                         i < components.size();
                         ++i)
                    {
                         auto& component =
                              components[i];

                         if (!component)
                              continue;


                         ImGui::PushID(
                              static_cast<int>(i)
                         );


                         ClassInspector inspector(
                              "Component",
                              component.get()
                         );

                         inspector.Draw();


                         ImGui::PopID();

                         ImGui::Separator();
                    }
                    if (ImGui::Button("Add Component"))
                    {
                         ImGui::OpenPopup(
                              "AddComponentPopup"
                         );
                    }


                    if (ImGui::BeginPopup(
                         "AddComponentPopup"))
                    {
                         auto& registry =
                              ComponentRegistry::Instance();


                         for (const auto& [typeId, info] :
                              registry.GetAll())
                         {
                              if (ImGui::MenuItem(
                                   info.name.c_str()))
                              {
                                   registry.Create(
                                        typeId,
                                        gameObject
                                   );
                              }
                         }


                         ImGui::EndPopup();
                    }
                    return true;
               }
          );

          return true;
     }();