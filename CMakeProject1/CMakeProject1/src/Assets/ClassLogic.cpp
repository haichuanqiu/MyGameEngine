#include "AssetManager.h"
#include "ReferenceResolver.h"
#include "EngineObject.h"
#include "Engine/Engine.h"

EngineObject* ReferenceResolver::GetItem(ReferenceDescription refDes) {
	if (refDes.ScopeLevel == 1) {
		return AssetManager::Instance().Find<Asset>(refDes.ObjectID);

	}
	else if (refDes.ScopeLevel == 0) {
          auto obj= Engine::Instance().GetScene(refDes.ScopeID)->getObject(refDes.ObjectID);

          if (!obj|| obj->waitingToDestroy) {
               return nullptr;
          }
		return Engine::Instance().GetScene(refDes.ScopeID)->getObject(refDes.ObjectID);
          
	}
	return nullptr;
}

template<typename T>
std::vector<T*> ReferenceResolver::FindAllOfType(
     ReferenceDescription targetScope)
{
     std::vector<T*> result;


     // ============================================================
     // Scene
     // ============================================================

     if (targetScope.ScopeLevel == 0)
     {
          Scene* scene =
               Engine::Instance()
               .GetScene(
                    targetScope.ScopeID
               );

          if (scene)
          {
               auto objects =
                    scene->FindAllOfType<T>();

               for (T* object : objects)
               {
                    if (!object)
                         continue;

                    if (
                         !ReferenceDescription::Suitable(
                              targetScope,
                              object->ReferenceInfo
                         )
                         )
                    {
                         continue;
                    }

                    result.push_back(
                         object
                    );
               }
          }
     }


     // ============================================================
     // Assets
     // ============================================================

     auto assets =
          AssetManager::Instance()
          .FindAllOfType<T>();

     for (T* asset : assets)
     {
          if (!asset)
               continue;

          if (
               !ReferenceDescription::Suitable(
                    targetScope,
                    asset->ReferenceInfo
               )
               )
          {
               continue;
          }

          result.push_back(
               asset
          );
     }


     return result;
}


// ============================================================
// Explicit Template Instantiation
// ============================================================

template EngineObject*
ReferenceResolver::GetItemOfType<EngineObject>(
     ReferenceDescription
);

template std::vector<EngineObject*>
ReferenceResolver::FindAllOfType<EngineObject>(
     ReferenceDescription
);
void Vertices::SetPreset(int preset)
{
     currentPreset = preset;

     Mesh myMesh = CreateMesh();

     RenderSystemIndex =
          Engine::Instance()
          .vertexDataController
          .registerMesh(myMesh);
}