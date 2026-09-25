#pragma once
#include "RenderingRelatedComponents.h"
#include "Serialization/EngineObjectHandle.h"
class ReferenceTest : public Component
{
public:
	

	EngineObjectHandle<Material> material;
	EngineObjectHandle<GameObject> myRefGameObject;
	EngineObjectHandle<Renderer> rd;
	EngineObjectHandle<Transform> ts;
	int testInt = -1;
	float testFloat = -1;
	bool testBool=false;
	std::string testString="";
	void Update() {
		Transform* t=ts.get();
		if (t) {
			auto s = t->GetPosition();
			Vector3 vec(s.x + 1, s.y, s.z);
			t->SetPosition(vec);
		}
	}

private:
};
REFLECT_BASE(
	ReferenceTest,
	Component,
	REF_HANDLE_FIELD(ReferenceTest, material),
	REF_HANDLE_FIELD(ReferenceTest, myRefGameObject),
	REF_HANDLE_FIELD(ReferenceTest, rd),
	REF_HANDLE_FIELD(ReferenceTest, ts),
	FIELD(ReferenceTest,testInt),
	FIELD(ReferenceTest, testFloat),
	FIELD(ReferenceTest, testString),
	FIELD(ReferenceTest, testBool)
	)
REGISTER_COMPONENT(ReferenceTest)
class DeleteTest : public Component
{
public:
	EngineObjectHandle<GameObject>toDelete;
	EngineObjectHandle<Renderer> toDeleteRd;
	int targetFrame=10;
	int currentFrame;
	void Update() {

		currentFrame+=1;
		if (targetFrame == currentFrame+1) {
			if(toDelete.get())
			DestroyGameObject(toDelete.get());
			if (toDelete.get())
			DestroyGameObject(toDelete.get());
			if (toDelete.get())
			DestroyGameObject(toDelete.get());
		}
		if (targetFrame== currentFrame) {
			if (toDeleteRd.get())
			DestroyComponent(toDeleteRd.get());
			if (toDeleteRd.get())
				DestroyComponent(toDeleteRd.get());
			if (toDeleteRd.get())
				DestroyComponent(toDeleteRd.get());
		}
	}

private:
};
REFLECT_BASE(
	DeleteTest,
	Component,
	REF_HANDLE_FIELD(DeleteTest, toDelete),
	REF_HANDLE_FIELD(DeleteTest, toDeleteRd),
	FIELD(DeleteTest, targetFrame),
	FIELD(DeleteTest, currentFrame)

)
REGISTER_COMPONENT(DeleteTest)

