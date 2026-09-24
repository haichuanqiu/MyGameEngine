#pragma once

#include "Engine/GameObjectSystem.h"
#include "Editor/DataInEditor/ClassInspector.h"

#include <memory>
#include <vector>

class InspectorView
{
public:

     void Draw();

     void SetTarget(EngineObject* object)
     {
          if (!object)
          {
               ClearTarget();
               return;
          }

          m_Inspector =
               std::make_unique<ClassInspector>(
                    "Object",
                    object
               );

     }

     void ClearTarget()
     {
          m_Inspector.reset();
     }

private:

     std::unique_ptr<ClassInspector> m_Inspector;
};