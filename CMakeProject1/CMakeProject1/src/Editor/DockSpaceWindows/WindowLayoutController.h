#pragma once
#include "imgui.h"
#include "Editor/DockSpaceWindows/EditorViews.h"
#include <GLFW/glfw3.h>

class WindowLayoutController
{
public:
	WindowLayoutController(GLFWwindow* m_target);
	

	void Draw();
	GLFWwindow* m_target = nullptr;
	ImGuiIO m_imGuiIO;
	HierarchyView m_HierarchyView;
	InspectorView m_InspectorView;
	SceneView m_SceneView;
private:

}; 
