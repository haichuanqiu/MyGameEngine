#pragma once
class WaitForFrameUpdateQueues {
public:
	struct EnableStatusConfig
	{
		GameObject* gameObject;
		bool toEnable;
	};
	void AddToDestoryQueue(GameObject* target) {
		destoryStack.push_back(target);
	}
	void AddToenableStatusChangeQueue(GameObject* target,bool enable) {
		EnableStatusConfig config;
		config.gameObject=target;
		config.toEnable=enable;
		enableStatusChangeStack.push_back(config);
	}
	std::vector <GameObject*> destoryStack;
	std::vector <EnableStatusConfig> enableStatusChangeStack;
	void ChangeAllEnableStatus() {
		for (auto& operation : enableStatusChangeStack)
		{
			if (operation.gameObject)
				continue;

			operation.gameObject->enabled = operation.toEnable;
		}
		enableStatusChangeStack.clear();
	}
	void DestoryAllInDestoryStack();
};