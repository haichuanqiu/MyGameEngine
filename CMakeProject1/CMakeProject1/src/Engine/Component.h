#pragma once

class GameObject;

class Component
{
public:
	virtual ~Component() = default;

	GameObject* gameObject = nullptr;
	virtual void OnAdded(GameObject* owner) {}
	virtual void OnRemoved(GameObject* owner) {}

};