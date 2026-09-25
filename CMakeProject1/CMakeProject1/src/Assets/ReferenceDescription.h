#pragma once
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
	int ScopeLevel=-1;//scene=0,asset=1
	int ScopeID=-1;
	int ObjectID=-1;
	ReferenceDescription() = default;
};

