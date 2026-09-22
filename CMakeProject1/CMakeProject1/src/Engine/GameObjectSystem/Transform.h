#pragma once

#include "Engine/GameObjectSystem/GameObject.h"
#include "Engine/DataStructure.h"
#include "Serialization/ComponentRegistry.h"
#include "Event.h"
class Transform : public Component
{
public:
     Event<const Transform&> OnChange;

public:
     // =========================
     // Position
     // =========================

     const Vector3& GetPosition() const
     {
          return position;
     }

     void SetPosition(const Vector3& value)
     {
          position = value;
          NotifyChanged();
     }

     // =========================
     // Scale
     // =========================

     const Vector3& GetScale() const
     {
          return scale;
     }

     void SetScale(const Vector3& value)
     {
          scale = value;
          NotifyChanged();
     }

     // =========================
     // Rotation
     // =========================

     const Quaternion& GetRotation() const
     {
          return rotation;
     }

     void SetRotation(const Quaternion& value)
     {
          rotation = value;
          NotifyChanged();
     }
private:
     void NotifyChanged()
     {
   
          OnChange.Invoke(*this);
     }
private:
     Vector3 position;
     Vector3 scale{ 1.0f, 1.0f, 1.0f };
     Quaternion rotation;

     REFLECT_FRIEND(Transform);
};

REFLECT_BASE(
     Transform,
     Component,
     FIELD(Transform, position),
     FIELD(Transform, scale),
     FIELD(Transform, rotation),
)
REGISTER_COMPONENT(Transform)