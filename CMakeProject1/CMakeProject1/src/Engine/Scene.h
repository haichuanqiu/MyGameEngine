#pragma once

#include <memory>
#include <vector>

#include "GameObject.h"

class Scene
{
public:
     GameObject& AddGameObject()
     {
          auto gameObject = std::make_unique<GameObject>();

          GameObject* ptr = gameObject.get();

          m_GameObjects.push_back(std::move(gameObject));

          return *ptr;
     }

     GameObject& GetGameObject(size_t index)
     {
          return *m_GameObjects.at(index);
     }

     const GameObject& GetGameObject(size_t index) const
     {
          return *m_GameObjects.at(index);
     }

     void AddGameObject(std::unique_ptr<GameObject> gameObject)
     {
          m_GameObjects.push_back(std::move(gameObject));
     }

     const std::vector<std::unique_ptr<GameObject>>&
          GetAllGameObjects() const
     {
          return m_GameObjects;
     }

private:
     std::vector<std::unique_ptr<GameObject>> m_GameObjects;
};