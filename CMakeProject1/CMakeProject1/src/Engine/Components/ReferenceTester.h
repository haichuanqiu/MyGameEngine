#pragma once
#include "RenderingRelatedComponents.h"
class ReferenceTest : public Component
{
public:
	Material* material;
	GameObject* myRefGameObject;
	Renderer* rd;
	Transform* ts;
	int testInt = -1;
	float testFloat = -1;
	bool testBool=false;
	std::string testString="";
	void Update() {
		if (ts) {
			auto s = ts->GetPosition();
			Vector3 vec(s.x + 1, s.y, s.z);
			ts->SetPosition(vec);
		}
	}

private:
};
REFLECT_BASE(
	ReferenceTest,
	Component,
	REF_FIELD(ReferenceTest, material),
	REF_FIELD(ReferenceTest, myRefGameObject),
	REF_FIELD(ReferenceTest, rd),
	REF_FIELD(ReferenceTest, ts),
	FIELD(ReferenceTest,testInt),
	FIELD(ReferenceTest, testFloat),
	FIELD(ReferenceTest, testString),
	FIELD(ReferenceTest, testBool)
	)
REGISTER_COMPONENT(ReferenceTest)
class DeleteTest : public Component
{
public:
	GameObject* toDelete;
	int targetFrame=10;
	int currentFrame;
	void Update() {

		currentFrame+=1;
		if (targetFrame== currentFrame) {
			
			DestroyGameObject(toDelete);
			std::cout
				<< "Call DestroyGameObject"
				<< std::endl;
		}
	}

private:
};
REFLECT_BASE(
	DeleteTest,
	Component,
	REF_FIELD(DeleteTest, toDelete),
	FIELD(DeleteTest, targetFrame),
	FIELD(DeleteTest, currentFrame)

)
REGISTER_COMPONENT(DeleteTest)

