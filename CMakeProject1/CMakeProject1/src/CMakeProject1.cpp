
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
      EditorApplication editor;

     //testScene::OnSceneCreated( *editor.m_Engine);
     testLoadScene::OnSceneCreated(*editor.m_Engine);
     double targetFrameTime = 1.0 / 60.0;
    
     editor.EngineRunning=true;
     while (!editor.MiniEngineWindow.ShouldClose())
     {
          
          double frameStart = glfwGetTime();
          editor.EditorUpdate();
          double frameEnd = glfwGetTime();
          double elapsed = frameEnd - frameStart;
          if (elapsed < targetFrameTime)
          {
               double sleepTime = targetFrameTime - elapsed;
               std::this_thread::sleep_for(std::chrono::duration<double>(sleepTime));
          }
     }

     // ------------------------
     // 清理
     // ------------------------
     editor.mylayout.ShutDown();
     glfwTerminate();
     return 0;
}