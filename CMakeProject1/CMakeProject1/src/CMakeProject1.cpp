
#include "Window.h"
#include "Editor/DockSpaceWindows/WindowLayoutController.h"
#include <iostream>
#include <thread>
#include <chrono>
#include "Testing/tryLoadScene.h"
#include "Testing/OnSceneCreated.h"
#include "Editor/EditorApplication.h"

#include "Assets/Material.h"
#include "Assets/AssetManager.h"
#include "rendering/ShaderManager.h"
int main()
{
     // ------------------------
     // 初始化 GLFW
     // ------------------------

     Window MiniEngineWindow(1280, 720, "MiniEngine");
     if (!MiniEngineWindow.TryInit())
     {
          std::cout << "Failed to initialize GLAD" << std::endl;
          return -1;
     }
     glEnable(GL_DEPTH_TEST);
     glDepthFunc(GL_LESS);
     WindowLayoutController mylayout(MiniEngineWindow.GetNativeWindow());
     int frameBuffer= mylayout.m_SceneView.GetFramebuffer();
     EditorApplication editor(frameBuffer);
     Scene* scene =
          &editor.m_Engine->currentScene;

     mylayout.m_HierarchyView.SetTarget(scene);

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
          [&]()
          {
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

    //testScene::OnSceneCreated(editor, *editor.m_Engine, mylayout);
     testLoadScene::OnSceneCreated(*editor.m_Engine);
     double targetFrameTime = 1.0 / 60.0;
    

     while (!MiniEngineWindow.ShouldClose())
     {
          
          double frameStart = glfwGetTime();
          MiniEngineWindow.PollEvents();
          mylayout.Draw();
          editor.UpdateSceneView();
          MiniEngineWindow.SwapBuffers();
          double frameEnd = glfwGetTime();
          double elapsed = frameEnd - frameStart;

          // 4. 如果耗时小于目标帧间隔，则 sleep 剩余时间
          if (elapsed < targetFrameTime)
          {
               double sleepTime = targetFrameTime - elapsed;
               // 使用高精度 sleep（C++11 标准库）
               std::this_thread::sleep_for(std::chrono::duration<double>(sleepTime));
          }
     }

     // ------------------------
     // 清理
     // ------------------------
     mylayout.ShutDown();
     glfwTerminate();
     //EditorApplication editor();
   //  RenderSystem::RenderForCamera();
     return 0;
}