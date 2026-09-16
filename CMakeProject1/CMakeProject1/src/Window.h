#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
class Window
{
public:
     Window(int width, int height, const char* title)
          : m_Width(width),
          m_Height(height)
     {
          // ----------------------------------------
          // GLFW
          // ----------------------------------------

          if (!glfwInit())
          {
               throw std::runtime_error(
                    "Failed to initialize GLFW"
               );
          }

          glfwWindowHint(
               GLFW_CONTEXT_VERSION_MAJOR,
               3
          );

          glfwWindowHint(
               GLFW_CONTEXT_VERSION_MINOR,
               3
          );

          glfwWindowHint(
               GLFW_OPENGL_PROFILE,
               GLFW_OPENGL_CORE_PROFILE
          );


          // ----------------------------------------
          // Window
          // ----------------------------------------

          m_Window = glfwCreateWindow(
               width,
               height,
               title,
               nullptr,
               nullptr
          );

          if (!m_Window)
          {
               glfwTerminate();

               throw std::runtime_error(
                    "Failed to create GLFW window"
               );
          }


          // ----------------------------------------
          // OpenGL Context
          // ----------------------------------------

          glfwMakeContextCurrent(m_Window);


          // ----------------------------------------
          // GLAD
          // 必须在任何 glXXX 调用之前
          // ----------------------------------------

          if (!gladLoadGLLoader(
               (GLADloadproc)glfwGetProcAddress
          ))
          {
               glfwDestroyWindow(m_Window);
               m_Window = nullptr;

               glfwTerminate();

               throw std::runtime_error(
                    "Failed to initialize GLAD"
               );
          }


          // ----------------------------------------
          // Callback
          // ----------------------------------------

          glfwSetFramebufferSizeCallback(
               m_Window,
               FramebufferSizeCallback
          );


          // ----------------------------------------
          // OpenGL state
          // ----------------------------------------

          glViewport(
               0,
               0,
               width,
               height
          );

          glEnable(GL_DEPTH_TEST);
          glDepthFunc(GL_LESS);
     }

	~Window() {
		if (m_Window)
		{
			glfwDestroyWindow(m_Window);
			m_Window = nullptr;
		}
		if (!TryInit())
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
			//return -1;
		}
		
	}
	bool TryInit() {
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			return false;
		}
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		return true;
	}
	void PollEvents() {
		glfwPollEvents();
	}
	void SwapBuffers() {
		glfwSwapBuffers(m_Window);
	}

	void MakeContextCurrent() {
		glfwMakeContextCurrent(m_Window);
	}

	bool ShouldClose() const {
		return glfwWindowShouldClose(m_Window);
	}

	GLFWwindow* GetNativeWindow() const {
		return m_Window;
	}
private:
	static void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
	}
private:
	GLFWwindow* m_Window = nullptr;

	int m_Width;
	int m_Height;
};