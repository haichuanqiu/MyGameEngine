#include "AssetManager.h"
#include "ReferenceResolver.h"
#include "EngineObject.h"
#include "Engine/Engine.h"

EngineObject* ReferenceResolver::GetItem(ReferenceDescription refDes) {
	if (refDes.ScopeLevel == 1) {
		return AssetManager::Instance().Find<Asset>(refDes.ObjectID);

	}
	else if (refDes.ScopeLevel == 0) {
		return Engine::Instance().GetScene(refDes.ScopeID)->getObject(refDes.ObjectID);
		//return nullptr;
	}
	return nullptr;
}