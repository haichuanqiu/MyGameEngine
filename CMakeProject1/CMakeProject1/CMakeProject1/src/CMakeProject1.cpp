#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

// ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
     glViewport(0, 0, width, height);
}

int main()
{
     // ------------------------
     // 初始化 GLFW
     // ------------------------
     glfwInit();

     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
     glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

     GLFWwindow* window = glfwCreateWindow(1280, 720, "MiniEngine", nullptr, nullptr);
     if (!window)
     {
          std::cout << "Failed to create window\n";
          glfwTerminate();
          return -1;
     }

     glfwMakeContextCurrent(window);
     glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

     // ------------------------
     // 初始化 GLAD（必须在 GLFW 后）
     // ------------------------
     if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
     {
          std::cout << "Failed to initialize GLAD\n";
          return -1;
     }

     std::cout << "OpenGL Loaded\n";

     // ------------------------
     // 初始化 ImGui
     // ------------------------
     IMGUI_CHECKVERSION();
     ImGui::CreateContext();

     ImGuiIO& io = ImGui::GetIO();
     (void)io;

     ImGui::StyleColorsDark();

     ImGui_ImplGlfw_InitForOpenGL(window, true);
     ImGui_ImplOpenGL3_Init("#version 330");

     // ------------------------
     // 主循环
     // ------------------------
     while (!glfwWindowShouldClose(window))
     {
          glfwPollEvents();

          // 清屏
          glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
          glClear(GL_COLOR_BUFFER_BIT);

          // ------------------------
          // ImGui Begin
          // ------------------------
          ImGui_ImplOpenGL3_NewFrame();
          ImGui_ImplGlfw_NewFrame();
          ImGui::NewFrame();

          // -------- GUI --------
          ImGui::Begin("Mini Engine");

          ImGui::Text("Hello Engine!");

          static float value = 0.0f;
          ImGui::SliderFloat("Value", &value, 0.0f, 1.0f);

          ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);

          ImGui::End();

          // ------------------------
          // ImGui Render
          // ------------------------
          ImGui::Render();
          ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

          glfwSwapBuffers(window);
     }

     // ------------------------
     // 清理
     // ------------------------
     ImGui_ImplOpenGL3_Shutdown();
     ImGui_ImplGlfw_Shutdown();
     ImGui::DestroyContext();

     glfwTerminate();

     return 0;
}