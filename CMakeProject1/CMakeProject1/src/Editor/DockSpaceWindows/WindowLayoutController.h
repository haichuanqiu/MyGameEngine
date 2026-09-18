#pragma once
#include "imgui.h"
#include "Editor/DockSpaceWindows/EditorViews.h"
#include <GLFW/glfw3.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
class WindowLayoutController
{
public:
	WindowLayoutController(GLFWwindow* m_target);
	
	void ShutDown() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

	}
	void Draw();
	GLFWwindow* m_target = nullptr;
	ImGuiIO m_imGuiIO;
	HierarchyView m_HierarchyView;
	InspectorView m_InspectorView;
	SceneView m_SceneView;
	AssetView m_AssetView;
	ProfilerView m_ProfilerView;
	ToolBar m_ToolBar;
private:

}; 
