#pragma once

#include <GLFW/glfw3.h>
class Window
{
public:
	Window(int width, int height, const char* title) {
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		m_Window = glfwCreateWindow(width, height, title, nullptr, nullptr);
		glfwMakeContextCurrent(m_Window);
		glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallback);
	}
	~Window() {
		if (m_Window)
		{
			glfwDestroyWindow(m_Window);
			m_Window = nullptr;
		}
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