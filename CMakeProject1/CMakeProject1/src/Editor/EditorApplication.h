#pragma once
#include "Editor/DockSpaceWindows/WindowLayoutController.h"
class EditorApplication
{
public:

     EditorApplication( )
          : EditorCamreaGameObject(), MiniEngineWindow(1280, 720, "MiniEngine"), mylayout(MiniEngineWindow.GetNativeWindow())
     {
          int framebufferId = mylayout.m_SceneView.GetFramebuffer();
          m_Engine =&Engine::Instance();
          RegisterEditorCamera(framebufferId);


         
          RegisterUIEvent();
     }

     void EditorUpdate() {
          MiniEngineWindow.PollEvents();
          mylayout.Draw();
           UpdateSceneView();
          MiniEngineWindow.SwapBuffers();
          if (EngineRunning) {
               m_Engine->Update();
          }
     }
     void UpdateSceneView()
     {
          m_Engine
               ->renderSystem
               .RenderForCamera(
                    *EditorCamera
               );
     }

public:
     Window MiniEngineWindow;
     WindowLayoutController mylayout;
     Engine* m_Engine =
          nullptr;

     bool EngineRunning;
     GameObject EditorCamreaGameObject;

     GameObject* ChoosenGameObject =
          nullptr;

     Camera* EditorCamera =
          nullptr;

     WindowLayoutController* WindowLayout =
          nullptr;

     int SceenChoosenItemHierarchyIndex =
          -1;
private:
     void RegisterEditorCamera(int SceneFrameBufferID) {
 
          EditorCamreaGameObject.name =
               "Camrea";


          EditorCamreaGameObject.transform
               ->SetPosition(
                    Vector3(0, 0, 5)
               );


          EditorCamera =
               EditorCamreaGameObject
               .AddComponent<Camera>();


          EditorCamera
               ->renderInfo_targetFramebuffer =
               SceneFrameBufferID;

     }
void RegisterUIEvent(){
     mylayout.m_HierarchyView.SetTarget(&m_Engine->currentScene);

     mylayout.m_HierarchyView.OnGameObjectClicked.Subscribe(
          [&](GameObject* gameObject)
          {
               if (gameObject)
               {
                    mylayout.m_InspectorView.SetTarget(gameObject);
               }
               else
               {
                    mylayout.m_InspectorView.ClearTarget();
               }
          }
     );
     mylayout.m_AssetView.OnObjectIDClicked.Subscribe(
          [&](int runtimeID)
          {
               auto asset =
                    AssetManager::Instance()
                    .Find<Asset>(runtimeID);

               if (asset)
               {
                    mylayout.m_InspectorView.SetTarget(
                         asset
                    );
               }
               else
               {
                    mylayout.m_InspectorView.ClearTarget();
               }
          }
     );
     mylayout.m_HierarchyView.OnSaveClicked.Subscribe(
          [&](Scene* target)
          {
               Scene* scene =
                    target;
               if (!scene)
               {
                    std::cout
                         << "[Scene Save] No scene to save."
                         << std::endl;

                    return;
               }


               const std::string savePath =
                    "../../../../CMakeProject1/assets/SceneData";


               // -----------------------------------------------
               // Serialize
               // -----------------------------------------------

               auto serialized =
                    SceneSerializer::Serialize(
                         *scene
                    );


               // -----------------------------------------------
               // Save
               // -----------------------------------------------

               SceneSerializer::SaveSceneTo(
                    *scene,
                    savePath
               );


               std::cout
                    << "[Scene Save] Scene saved to: "
                    << savePath
                    << std::endl;


               std::cout
                    << serialized
                    << std::endl;
          }
     );
}
};