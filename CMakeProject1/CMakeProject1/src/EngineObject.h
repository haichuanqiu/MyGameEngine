#pragma once
#include "Assets/ReferenceDescription.h"
#include "Serialization/Reflection.h"
class GameObject;
class Component;
class EngineObject
{
public:
	static void DestroyGameObject(GameObject* target);
	static void DestroyComponent(Component* target);
public:
	bool waitingToDestroy = false;
	virtual ~EngineObject() = default;
	ReferenceDescription ReferenceInfo;
};

REFLECT(
	EngineObject,
	FIELD(EngineObject, ReferenceInfo),

)