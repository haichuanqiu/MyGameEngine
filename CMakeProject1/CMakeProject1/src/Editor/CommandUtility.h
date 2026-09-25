#pragma once

class GameObject;
class Scene;
namespace CommandUtility
{
     Scene* GetEditorCurrentScene();
     void EditorTimeDestroyComponent(Component* component);

     void CreateGameObjectInCurrentScene();
     void EditorTimeDestroyGameObject(GameObject* target) ;
}