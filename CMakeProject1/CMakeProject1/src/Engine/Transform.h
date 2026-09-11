#pragma once

#include "Component.h"
#include "DataStructure.h"
#include "Reflection.h"
#include "ComponentRegistry.h"
class Transform : public Component
{
public:
     using Callback = std::function<void(const Transform&)>;

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

     // =========================
     // Callback
     // =========================

     void AddCallback(Callback callback)
     {
          callbacks.push_back(std::move(callback));
     }
private:
     void NotifyChanged()
     {
          if (callbacks.empty())
               return;

          for (auto& callback : callbacks)
          {
               callback(*this);
          }
     }
private:
     Vector3 position;
     Vector3 scale{ 1.0f, 1.0f, 1.0f };
     Quaternion rotation;
     std::vector<Callback> callbacks;

     REFLECT_FRIEND(Transform);
};

REFLECT(
     Transform,

     FIELD(Transform, position),
     FIELD(Transform, scale),
     FIELD(Transform, rotation),
)
REGISTER_COMPONENT(Transform)