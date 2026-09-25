// PointLightCustomInspector.cpp
#include "ClassInspector.h"
#include "Engine/Components/RenderingRelatedComponents.h"   // 你的 PointLight 定义
#include "Editor/CommandUtility.h"

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
                    // ============================================================
                    // Destroyed / Waiting To Destroy
                    // ============================================================

                    if (gameObject.waitingToDestroy)
                    {
                         return false;
                    }


                    // ============================================================
                    // GameObject Name
                    // ============================================================

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

                    if (
                         ImGui::InputText(
                              "##GameObjectName",
                              nameBuffer,
                              sizeof(nameBuffer)
                         )
                         )
                    {
                         gameObject.name =
                              nameBuffer;
                    }


                    // ============================================================
                    // Delete GameObject
                    // ============================================================

                    if (
                         ImGui::Button(
                              "Delete GameObject##DeleteGameObject"
                         )
                         )
                    {
                         CommandUtility::EditorTimeDestroyGameObject(
                              &gameObject
                         );

                         return false;
                    }


                    ImGui::Separator();


                    // ============================================================
                    // Components
                    // ============================================================

                    auto& components =
                         gameObject.GetComponents();


                    for (
                         size_t i = 0;
                         i < components.size();
                         ++i
                         )
                    {
                         auto& component =
                              components[i];


                         if (!component)
                              continue;


                         // ========================================================
                         // Component ID Scope
                         //
                         // 从这里开始，这个 Component 里面所有 ImGui 控件
                         // 都会拥有独立 ID。
                         // ========================================================

                         ImGui::PushID(
                              component.get()
                         );


                         // ========================================================
                         // Delete Component
                         // ========================================================

                         bool deleteRequested =
                              false;


                         if (
                              ImGui::Button(
                                   "Delete Component##Delete"
                              )
                              )
                         {
                              deleteRequested =
                                   true;
                         }


                         // ========================================================
                         // Component Inspector
                         // ========================================================

                         if (!deleteRequested)
                         {
                              ClassInspector inspector(
                                   "Component",
                                   component.get()
                              );


                              inspector.Draw();
                         }


                         ImGui::Separator();


                         ImGui::PopID();


                         // ========================================================
                         // Request Destroy
                         //
                         // 放在 PopID 后面执行逻辑更清楚。
                         // 如果这里会立即删除 Component，
                         // 后面绝对不能再访问 component。
                         // ========================================================

                         if (deleteRequested)
                         {
                              CommandUtility::EditorTimeDestroyComponent(
                                   component.get()
                              );

                              // 如果 DestroyComponent 可能立刻修改
                              // components vector，不要继续当前循环。
                              break;
                         }
                    }


                    // ============================================================
                    // Add Component
                    // ============================================================

                    if (
                         ImGui::Button(
                              "Add Component##AddComponent"
                         )
                         )
                    {
                         ImGui::OpenPopup(
                              "AddComponentPopup"
                         );
                    }


                    // ============================================================
                    // Add Component Popup
                    // ============================================================

                    if (
                         ImGui::BeginPopup(
                              "AddComponentPopup"
                         )
                         )
                    {
                         auto& registry =
                              ComponentRegistry::Instance();


                         for (
                              const auto& [typeId, info] :
                              registry.GetAll()
                              )
                         {
                              if (
                                   ImGui::MenuItem(
                                        info.name.c_str()
                                   )
                                   )
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