#include "WindowLayoutController.h"
#include "imgui.h"
#include <iostream>
// ImGui
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

WindowLayoutController::WindowLayoutController(GLFWwindow* target)
    : m_target(target),
      m_SceneView(),
      m_HierarchyView(),
      m_InspectorView(),
	m_AssetView(),
	m_ProfilerView(),
	m_ToolBar(),
	m_ContextMenu()
{
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 1.5f;
    ImGui::StyleColorsDark();

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(target, true);
    ImGui_ImplOpenGL3_Init("#version 330");

	m_HierarchyView.OnRightClickedEmptySpace.Subscribe([&](Vector2 target){m_ContextMenu.Open(ContextMenu::OpenType::Hierarchy, target);});

}

void WindowLayoutController::Draw() {

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::DockSpaceOverViewport(ImGuiDockNodeFlags_PassthruCentralNode, ImGui::GetMainViewport());
	m_HierarchyView.Draw();
	m_InspectorView.Draw();
	m_SceneView.Draw();	
	m_AssetView.Draw();
	m_ProfilerView.Draw();
	m_ToolBar.Draw();
	m_ContextMenu.Draw();
	ImGui::Render();
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


}