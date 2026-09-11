#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include "Editor/DockSpaceWindows/WindowLayoutController.h"
// ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "rendering/RenderSystem.h"
#include <thread>
#include <chrono>
#include "Window.h"
#include "tryLoadScene.h"
#include "OnSceneCreated.h"
#include "Editor/EditorApplication.h"
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
     glViewport(0, 0, width, height);
}

int main()
{
     // ------------------------
     // 初始化 GLFW
     // ------------------------

     Window MiniEngineWindow(1280, 720, "MiniEngine");
     if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
     {
          std::cout << "Failed to initialize GLAD" << std::endl;
          return -1;
     }
     glEnable(GL_DEPTH_TEST);
     glDepthFunc(GL_LESS);
     WindowLayoutController mylayout(MiniEngineWindow.GetNativeWindow());
     int frameBuffer= mylayout.m_SceneView.GetFramebuffer();
     EditorApplication editor(frameBuffer);
     mylayout.m_HierarchyView.SetTarget(
          &editor.m_Engine->currentScene);

     mylayout.m_HierarchyView.SetChosenIndex(
          &editor.SceenChoosenItemHierarchyIndex);

     // Inspector 也指向同一个 Scene 和 index
     mylayout.m_InspectorView.SetTarget(
          &editor.m_Engine->currentScene);

     mylayout.m_InspectorView.SetChosenIndex(
          &editor.SceenChoosenItemHierarchyIndex);
     testScene::OnSceneCreated(*editor.m_Engine);
     //testLoadScene::OnSceneCreated(*editor.m_Engine);
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
     ImGui_ImplOpenGL3_Shutdown();
     ImGui_ImplGlfw_Shutdown();
     ImGui::DestroyContext();

     glfwTerminate();
     //EditorApplication editor();
   //  RenderSystem::RenderForCamera();
     return 0;
}