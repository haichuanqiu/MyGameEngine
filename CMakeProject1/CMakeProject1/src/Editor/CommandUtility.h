#pragma once

class GameObject;
class Scene;
namespace CommandUtility
{
     Scene* GetEditorCurrentScene();


     void CreateGameObjectInCurrentScene();
     void EditorTimeDestroyGameObject(GameObject* target) ;
}