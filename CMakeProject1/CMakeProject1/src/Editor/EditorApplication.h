#pragma once
#include "Editor/DockSpaceWindows/WindowLayoutController.h"
#include "Profiler/profiler.h"
class EditorApplication
{
public:

     EditorApplication( )
          : EditorCamreaGameObject(), MiniEngineWindow(1280, 720, "MiniEngine"), mylayout(MiniEngineWindow.GetNativeWindow())
     {
          int framebufferId = mylayout.m_SceneView.GetFramebuffer();
          m_Engine =&Engine::Instance();
          RegisterEditorCamera(framebufferId);
          EngineRunning=false;

         
          RegisterUIEvent();
     }

     void EditorUpdate()
     {
          ProfilerManager::Instance().NewFrame();
          ENGINE_PROFILE_SCOPE("Editor Frame");
          {
               ENGINE_PROFILE_SCOPE("Poll Events");

               MiniEngineWindow.PollEvents();
          }
          if (EngineRunning)
          {
               ENGINE_PROFILE_SCOPE("Engine Update");

               m_Engine->Update();
          }
          {
               ENGINE_PROFILE_SCOPE("Scene View Render");

               UpdateSceneView();
          }

          {
               ENGINE_PROFILE_SCOPE("Editor UI");

               mylayout.Draw();
          }
          {
               ENGINE_PROFILE_SCOPE("Swap Buffers");

               MiniEngineWindow.SwapBuffers();
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
     bool EngineRunning;
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
     void RebuidScene() {
          mylayout.m_InspectorView.ClearTarget();
          mylayout.m_HierarchyView.ClearTarget();
          std::cout << "RebuildScene" << std::endl;
          const std::string scenePath =
               m_Engine->currentScene.filePath;

          std::ifstream file(
               scenePath,
               std::ios::in
          );

          if (!file.is_open())
          {
               std::cerr
                    << "Failed to open Scene file: "
                    << scenePath
                    << std::endl;

               return;
          }

          std::stringstream buffer;

          buffer << file.rdbuf();

          file.close();

          const std::string sceneData =
               buffer.str();

          m_Engine->currentScene.ClearScene();

          SceneSerializer::LoadScene(
               &m_Engine->currentScene,
               sceneData
          );
          mylayout.m_HierarchyView.SetTarget(&m_Engine->currentScene);
          
     }
  
void RegisterUIEvent(){
     mylayout.m_ToolBar.OnPlayButtonPressed.Subscribe(
          [&]() {
               std::cout << "StartPlaying" << std::endl;
               EngineRunning = true;
               mylayout.m_ToolBar.InPlayMode = true;
          }
     );
     mylayout.m_ToolBar.OnStopButtonPressed.Subscribe(
          [&]() {
               EngineRunning = false;
               mylayout.m_ToolBar.InPlayMode = false;
               RebuidScene();
          }
     );

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