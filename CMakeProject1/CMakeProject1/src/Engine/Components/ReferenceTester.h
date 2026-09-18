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
		std::cout
			<< "ts ReferenceInfo = "
			<< ts->ReferenceInfo.ScopeLevel
			<< ", "
			<< ts->ReferenceInfo.ScopeID
			<< ", "
			<< ts->ReferenceInfo.ObjectID
			<< std::endl;
		auto s=ts->GetPosition();
		Vector3 vec(s.x + 1, s.y, s.z);
		ts->SetPosition(vec);
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
