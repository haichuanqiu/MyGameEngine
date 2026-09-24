#include "ClassInspector.h"
#include "Assets/AssetManager.h"
static bool s_AssetInspectorRegistered = []()
     {
          CustomInspectorFactory::Instance().Register<Asset>(
               [](Asset& asset)-> bool
               {
                    // ==========================================
                    // 默认 Reflection Inspector
                    // ==========================================

                    DrawDefaultInspector(
                         "Asset",
                         &asset
                    );


                    // ==========================================
                    // Save
                    // ==========================================

                    ImGui::Separator();

                    if (ImGui::Button("Save"))
                    {
                         AssetManager::Instance().Save<Asset>(asset.ReferenceInfo.ObjectID);
                    }
                    return true;
               }
          );
          
          return true;
 
     }();


