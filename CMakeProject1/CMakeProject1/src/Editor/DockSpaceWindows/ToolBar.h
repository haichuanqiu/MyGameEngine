#pragma once
class ToolBar
{
public:
	ToolBar()=default;

	void Draw() {
		ImGui::Begin("ToolBar");
		if (InPlayMode) {
			if (ImGui::Button("Stop"))
			{
				OnStopButtonPressed.Invoke();
			}
		}
		else {
			if (ImGui::Button("Start"))
			{
				OnPlayButtonPressed.Invoke();
			}
		}
		
		ImGui::End();
	}
	bool InPlayMode;
	Event<> OnPlayButtonPressed;
	Event<> OnStopButtonPressed;
private:
	
};