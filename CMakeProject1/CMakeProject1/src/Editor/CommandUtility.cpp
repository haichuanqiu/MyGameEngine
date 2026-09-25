#include "Engine/Engine.h"
#include "CommandUtility.h"
 Scene* CommandUtility::GetEditorCurrentScene()
{
     return Engine::Instance().GetCurrentScene();
}

 void CommandUtility::CreateGameObjectInCurrentScene()
{
     Scene* scene = GetEditorCurrentScene();

     if (!scene)
          return;


     GameObject& gameObject =
          scene->AddGameObject();


     gameObject.name =
          "New GameObject";


     gameObject.transform->SetPosition(
          Vector3(
               0.0f,
               0.0f,
               0.0f
          )
     );
     
}

 void CommandUtility::EditorTimeDestroyGameObject(GameObject* target) {
     EngineObject::DestroyGameObject(target);
 }
 void CommandUtility::EditorTimeDestroyComponent(Component* component) {
      EngineObject::DestroyComponent(component);
 }