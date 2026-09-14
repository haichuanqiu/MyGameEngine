#pragma once
#include "Serialization/reflection.h"
class ReferenceDescription
{
public:
	static bool Suitable(ReferenceDescription TargetScope, ReferenceDescription ValueScope) {
		if (TargetScope.ScopeLevel < ValueScope.ScopeLevel) {
			return true;
		}
		else if (TargetScope.ScopeLevel == ValueScope.ScopeLevel) {
			return TargetScope.ScopeID == ValueScope.ScopeID;
		}
		return false;
	}
	int ScopeLevel;//scene=0,asset=1
	int ScopeID;
	int ObjectID;
	ReferenceDescription() = default;
};

REFLECT(
	ReferenceDescription,
	FIELD(ReferenceDescription, ScopeLevel),
	FIELD(ReferenceDescription, ScopeID),
	FIELD(ReferenceDescription, ObjectID)
	)